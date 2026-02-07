#include <ostream>
#include <fstream>
#include <array>
#include <string>
#include <filesystem>
#include "Aes-128.cpp"
#include <memory>

#pragma once

using namespace std;

using Buffer = array<unsigned char, 4096>;
using Chunk = array<unsigned char, 16>;

class FileRead {
private:
	streampos fileCursorPosition = 0;
    streampos fileTotalSize = 0;
    size_t chunk_index = 0;
    Buffer cachedBuffer;
    bool bufferValid = false;
    bool fileOpened = false;
    size_t actualBufferSize = 0;

public:
    FileRead() : actualBufferSize(0) {}

    Buffer& getBuffer(const string& file_path) {
        if (!bufferValid) {
            ifstream file(file_path, ios::binary);

            if (!file.is_open()) {
                throw runtime_error("Failed to open file: " + file_path);
            }

            // Получаем размер файла при первом открытии
            if (!fileOpened) {
                file.seekg(0, ios::end);          // перейти в конец
                fileTotalSize = file.tellg();     // получить размер
                file.seekg(0, ios::beg);          // вернуться в начало
                fileOpened = true;
                fileCursorPosition = 0;
            }

            file.seekg(fileCursorPosition, ios::beg);
            file.read(reinterpret_cast<char*>(cachedBuffer.data()), cachedBuffer.size());

            actualBufferSize = file.gcount();  // Сохраняем сколько реально прочитано
            fileCursorPosition += actualBufferSize;
            bufferValid = true;
            chunk_index = 0;  // Сбрасываем индекс чанков для нового буфера

            file.close();
        }

        return cachedBuffer;
    }

    Chunk getChunk(const Buffer& inputBuffer) {
        Chunk result;
        size_t start_pos = chunk_index * 16;
        size_t bytes_available = actualBufferSize - start_pos;

        if (bytes_available >= 16) {
            // Полный чанк - просто копируем
            memcpy(result.data(), inputBuffer.data() + start_pos, 16);
        }
        else {
            // Неполный чанк - применяем PKCS#7 padding
            if (bytes_available > 0) {
                memcpy(result.data(), inputBuffer.data() + start_pos, bytes_available);
            }

            // Заполняем оставшееся значение = количеству добавленных байтов
            unsigned char padding_value = 16 - bytes_available;
            for (size_t i = bytes_available; i < 16; i++) {
                result[i] = padding_value;
            }

        }

        chunk_index++;
        if (chunk_index * 16 >= actualBufferSize) {
            bufferValid = false;
            chunk_index = 0;
        }
        
        return result;
    }

    size_t getActualBufferSize() const {
        return actualBufferSize;  // Просто возвращаем сохраненное значение
    }

    bool hasMoreChunksInBuffer() const {
        if (!bufferValid) return false;  // Буфер не валиден - нет чанков

        // Чанки есть, если текущий индекс * 16 < реального размера буфера
        return (chunk_index * 16) < actualBufferSize;
    }

    bool isEOF() const {
        // Конец файла если:
        // 1. Позиция чтения >= размера файла
        // 2. И текущий буфер полностью обработан (или невалиден)
        bool atEndOfFile = (fileCursorPosition >= fileTotalSize);
        bool bufferProcessed = (!bufferValid || (chunk_index * 16 >= actualBufferSize));

        return atEndOfFile && bufferProcessed;
    }

    bool isBufferValid() {
        return bufferValid;
    }

    void resetCursor() {
        fileCursorPosition = 0;
        chunk_index = 0;
        bufferValid = false;  
    }

    streampos getCursorPosition() const { return fileCursorPosition; }
};

class FileWrite {
private:
    streampos fileCursorPosition = 0;
    size_t chunk_index = 0;
    Buffer cachedBuffer;

public:
    void postBuffer(const string& file_path) {
        ofstream file(file_path, ios::binary);

        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + file_path);
        }

        file.seekp(fileCursorPosition, ios::beg);
        file.write(reinterpret_cast<char*>(cachedBuffer.data()), cachedBuffer.size());

        fileCursorPosition = file.tellp();
        file.close();
    }

    void postChunk(const Chunk& inputChunk) {
        copy(inputChunk.begin(), inputChunk.end(), cachedBuffer.begin() + chunk_index * 16);
        chunk_index++;

    }

    void resetCursor() {
        fileCursorPosition = 0;
        chunk_index = 0;
    }

    Buffer getBufferForWrite() { return cachedBuffer; }

    bool isBufferFull() const { return chunk_index == 256; }

    bool isBufferEmpty() const { return chunk_index == 0; }

    size_t getBufferSize() const { return chunk_index * 16; }

    void resetWriteBuffer() { chunk_index = 0; }

    streampos getCursorPosition() const { return fileCursorPosition; }
};

class FileProcess {
private:
    string input_file_path;
    string output_file_path;
    string mode_cryption;
    Block sourse_key;
    FileRead fr;
    FileWrite fw;
    unique_ptr<AESEncryptor> encryptor;  
    unique_ptr<AESDecryptor> decryptor;

public:
    FileProcess(const string& input_file_path, const string& output_file_path,
        const string mode_cryption, const Block& sourse_key)
        : input_file_path(input_file_path), output_file_path(output_file_path),
        mode_cryption(mode_cryption), sourse_key(sourse_key) {};

    pair<Chunk, size_t> removePadding(Chunk& chunk) {
        unsigned char paddingValue = chunk[15];

        // Проверяем paddingValue
        if (paddingValue == 0 || paddingValue > 16) {
            throw runtime_error("Invalid padding value");
        }

        size_t dataSize = 16 - paddingValue;

        // Проверяем структуру паддинга
        for (size_t i = dataSize; i < 16; i++) {
            if (chunk[i] != paddingValue) {
                throw runtime_error("Invalid padding structure");
            }
        }

        return { chunk, dataSize };  // Возвращаем чанк И реальный размер
    }

    void processFile() {
        // Проверяем режим и создаем нужный объект ОДИН РАЗ
        if (mode_cryption == "Encryption") {
            encryptor = make_unique<AESEncryptor>();
            encryptor->setKey(sourse_key);
        }
        else if (mode_cryption == "Decryption") {
            decryptor = make_unique<AESDecryptor>();
            decryptor->setKey(sourse_key);
        }

        // Открываем выходной файл ОДИН РАЗ
        ofstream outFile(output_file_path, ios::binary);
        if (!outFile) throw runtime_error("Cannot open output file");

        while (true) {
            // 1. Получить буфер (прочитает файл если нужно)
            Buffer& inputBuffer = fr.getBuffer(input_file_path);

            // 2. Обработать все чанки из ЭТОГО буфера
            while (fr.hasMoreChunksInBuffer()) {
                Chunk plainChunk = fr.getChunk(inputBuffer);
                Chunk processedChunk;
                if (mode_cryption == "Encryption") {
                    processedChunk = encryptor->encryptBlock(plainChunk);
                }
                else if (mode_cryption == "Decryption") {
                    processedChunk = decryptor->decryptBlock(plainChunk);

                    // ТОЛЬКО ДЛЯ ДЕШИФРОВАНИЯ: проверяем последний чанк
                    if (fr.isEOF() && !fr.hasMoreChunksInBuffer()) {
                        pair<Chunk, size_t> processedPair = removePadding(processedChunk);
                        outFile.write(reinterpret_cast<char*>(processedPair.first.data()), processedPair.second);
                        break;
                    }
                }
                fw.postChunk(processedChunk);

                // 3. Если буфер записи заполнился - сбросить в файл
                if (fw.isBufferFull()) {
                    outFile.write(reinterpret_cast<char*>(fw.getBufferForWrite().data()), fw.getBufferSize());
                    fw.resetWriteBuffer();
                }
            }

            // 4. Проверка завершения
            if (fr.isEOF()) {
                // Записать остатки
                if (!fw.isBufferEmpty()) {
                    outFile.write(reinterpret_cast<char*>(fw.getBufferForWrite().data()),
                        fw.getBufferSize());
                }
                break;
            }
        }
    }
};

class Test {
public:
    void enc(string input, string output, Block& key) {
        FileProcess fp(input, output, "Encryption", key);
        try {
            fp.processFile();
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
    }

    void dec(string input, string output, Block& key) {
        FileProcess fp(input, output, "Decryption", key);
        try {
            fp.processFile();
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
    }
};

