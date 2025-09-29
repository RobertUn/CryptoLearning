#include <algorithm>
#include <ostream>
#include <array>
#include <cstring>
#include <iostream>
#include <Windows.h>
#include <iomanip>
#include <cassert>

using namespace std;

// Константный массив для SubBytes преобразования
const unsigned char s_box[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};
const unsigned char math_matrix[4][4] = {
    {0x02, 0x03, 0x01, 0x01},
    {0x01, 0x02, 0x03, 0x01},
    {0x01, 0x01, 0x02, 0x03},
    {0x03, 0x01, 0x01, 0x02}
};
const unsigned char Rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};
const int ROW = 4;
const int COL = 4;

using State = array<array<unsigned char, COL>, ROW>;
using Column = array<unsigned char, COL>;
using Key = array<array<unsigned char, COL>, ROW>;
using Word = array<unsigned char, ROW>;
using RoundsKeys = array<array<unsigned char, 16>, 11>;

State SubBytes(State state) {
    for (auto& row : state) {
        for (auto& col : row) {
            col = s_box[col];
        }
    }
    return state;
}

State ShiftRows(State state) {
    for (int i = 0; i < ROW; i++) {
        rotate(state[i].begin(), state[i].begin() + i, state[i].end());
    }
    return state;
}

unsigned char xtime(unsigned char x) {
    unsigned char result = x << 1;
    if (x & 0x80) {
        result ^= 0x1B;
    }
    return result;
}

Column getColumn(State state, int colNumber) {
    Column result;
    for (int i = 0; i < ROW; i++) {
        result[i] = state[i][colNumber];
    }
    return result;
}

Column MatrixMultiply(const Column& col) {
    Column result = { 0, 0, 0, 0 }; // инициализируем нулями

    for (int i = 0; i < 4; i++) {         // для каждой строки матрицы и элемента в итоговом массиве
        for (int j = 0; j < 4; j++) {     // для каждого элемента в строке и для того, чтобы пройтись по исходной колонке 4 раза
            switch (math_matrix[i][j]) {   // берем коэффициент из матрицы
            case 0x01:
                result[i] ^= col[j];
                break;
            case 0x02:
                result[i] ^= xtime(col[j]);
                break;
            case 0x03:
                result[i] ^= xtime(col[j]) ^ col[j];
                break;
            }
        }
    }
    return result;
}

State MixColumns(State state) {
    State new_state;

    for (int j = 0; j < 4; j++) {
        Column col = getColumn(state, j);
        Column new_col = MatrixMultiply(col);
        for (int i = 0; i < 4; i++) {
            new_state[i][j] = new_col[i];
        }
    }
    return new_state;
}

State AddRoundKey(State state, const Key& key) {
    // State и Key оба в column-major представлении
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] ^= key[i][j];
        }
    }
    return state;
}

Word g_function(Word word, int round_index) {
    // 1. RotWord
    rotate(word.begin(), word.begin() + 1, word.end());
    // 2. SubWord
    for (int i = 0; i < 4; i++) {
        word[i] = s_box[word[i]];
    }
    // 3. XOR с Rcon
    word[0] ^= Rcon[round_index];

    return word;
}

RoundsKeys KeyExpansion(Key source_key) {
    array<Word, 44> words;

    // ИСПРАВЛЕНИЕ: row-major заполнение (стандарт AES)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            words[i][j] = source_key[j][i];  // row-major
        }
    }

    // Генерация остальных слов (эта часть правильная)
    for (int i = 4; i < 44; i++) {
        if (i % 4 == 0) {
            Word temp = words[i - 1];
            Word transformed = g_function(temp, i / 4 - 1);
            for (int j = 0; j < 4; j++) {
                words[i][j] = words[i - 4][j] ^ transformed[j];
            }
        }
        else {
            for (int j = 0; j < 4; j++) {
                words[i][j] = words[i - 4][j] ^ words[i - 1][j];
            }
        }
    }

    // ИСПРАВЛЕНИЕ: правильная конвертация
    RoundsKeys roundKeys;
    for (int round = 0; round < 11; round++) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                roundKeys[round][i * 4 + j] = words[round * 4 + i][j];  // row-major
            }
        }
    }

    return roundKeys;
}

int main() {
    // Настройка кодировки
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian");

    // 1. Исходные данные
    unsigned char plaintext[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    // Заполняем Key в column-major порядке!
    Key key = { {
        {0x00, 0x04, 0x08, 0x0c},  // столбец 0
        {0x01, 0x05, 0x09, 0x0d},  // столбец 1  
        {0x02, 0x06, 0x0a, 0x0e},  // столбец 2
        {0x03, 0x07, 0x0b, 0x0f}   // столбец 3
    } };

    cout << "Исходный текст: ";
    for (int i = 0; i < 16; i++) {
        cout << hex << setw(2) << setfill('0') << (int)plaintext[i] << " ";
    }
    cout << endl;

    cout << "Ключ: ";
    for (int i = 0; i < 16; i++) {
        cout << hex << setw(2) << setfill('0') << (int)((unsigned char*)&key)[i] << " ";
    }
    cout << endl;

    // 2. Расширяем ключ
    RoundsKeys roundKeys = KeyExpansion(key);

    cout << "Расширенные ключи:" << endl;
    for (int r = 0; r < 11; r++) {
        cout << "Round " << r << ": ";
        for (int i = 0; i < 16; i++) {
            cout << hex << setw(2) << setfill('0') << (int)roundKeys[r][i] << " ";
        }
        cout << endl;
    }

    // 3. Преобразуем plaintext в State
    State state;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = plaintext[i + 4 * j];
        }
    }

    cout << "Начальный State:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // 4. Начальный раунд
    state = AddRoundKey(state, key);
    cout << "После AddRoundKey (round 0):" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // 5. 9 основных раундов
    for (int round = 1; round <= 9; round++) {
        cout << "\n=== Round " << round << " ===" << endl;

        state = SubBytes(state);
        cout << "После SubBytes:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }

        state = ShiftRows(state);
        cout << "После ShiftRows:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }

        state = MixColumns(state);
        cout << "После MixColumns:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }
        // Преобразование из плоского массива в Key (column-major)
        Key roundKey;
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                // roundKeys[round] хранится в column-major: [col0_row0, col0_row1, col0_row2, col0_row3, col1_row0, ...]
                roundKey[row][col] = roundKeys[round][col * 4 + row];
            }
        }

        cout << "Ключ раунда " << round << ":" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)roundKey[i][j] << " ";
            }
            cout << endl;
        }

        state = AddRoundKey(state, roundKey);
        cout << "После AddRoundKey:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }
    }

    // 6. Финальный раунд
    cout << "\n=== Final Round ===" << endl;
    state = SubBytes(state);
    cout << "После SubBytes:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    state = ShiftRows(state);
    cout << "После ShiftRows:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // Преобразуем последний раундовый ключ
    Key finalRoundKey;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            finalRoundKey[row][col] = roundKeys[10][col * 4 + row]; // column-major
        }
    }

    cout << "Финальный ключ:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)finalRoundKey[i][j] << " ";
        }
        cout << endl;
    }

    state = AddRoundKey(state, finalRoundKey);
    cout << "После Final AddRoundKey:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // 7. Преобразуем State обратно в байты
    unsigned char ciphertext[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ciphertext[i + 4 * j] = state[i][j]; // column-major
        }
    }

    // 8. Выводим результат
    cout << "\nЗашифрованный текст: ";
    for (int i = 0; i < 16; i++) {
        cout << hex << setw(2) << setfill('0') << (int)ciphertext[i] << " ";
    }
    cout << endl;

    return 0;
}