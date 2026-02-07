#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#pragma once

using namespace std;
namespace fs = std::filesystem;

class Backup {
private:
	fs::path targetFolder;
    vector<pair<fs::path, fs::file_time_type>> filesWithDates;

    string generateUniqueName(const string& original_name) {
        auto now = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(now);

        tm local_tm;
        #ifdef _WIN32
        localtime_s(&local_tm, &tt);
        #else
        localtime_r(&tt, &local_tm);
        #endif

        // Формат: backup_ГГГГ-ММ-ДД_ЧЧ-ММ-СС_случайное.bak
        char time_buffer[32];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H-%M-%S", &local_tm);

        mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> dist(1000, 9999);

        return string("backup_") + time_buffer + "_" +
            to_string(dist(rng)) + ".bak";
    }

    void getSortedFilesByDate() {
        filesWithDates.clear();

        // Собираем все файлы
        vector<fs::path> all_files;
        for (const auto& entry : fs::directory_iterator(targetFolder)) {
            if (entry.is_regular_file()) {
                all_files.push_back(entry.path());
            }
        }

        // Сортируем по имени файла
        sort(all_files.begin(), all_files.end(),
            [](const fs::path& a, const fs::path& b) {
                return a.filename().string() < b.filename().string();
            });

        // Заполняем filesWithDates
        for (const auto& filepath : all_files) {
            filesWithDates.emplace_back(filepath, fs::last_write_time(filepath));
        }
    }

    void cleanupOldFiles(int maxFiles) {
        getSortedFilesByDate();

        if (filesWithDates.size() > maxFiles) {
            size_t filesToDelete = filesWithDates.size() - maxFiles;
            for (size_t i = 0; i < filesToDelete; ++i) {
                cout << "Удаляю старый бэкап: " << filesWithDates[i].first.filename() << endl;
                fs::remove(filesWithDates[i].first);
            }
        }
    }

public:
    Backup(const fs::path& folderPath) : targetFolder(folderPath) {
        if (!fs::exists(targetFolder)) {
            fs::create_directory(targetFolder);
        }
    }

    bool copyFile(const string& sourcePath) {
        try {
            // Получаем имя файла из пути
            string unique_name = generateUniqueName(sourcePath);

            // Путь для копии в текущей папке
            fs::path destination = targetFolder / unique_name;

            // Проверяем, существует ли исходный файл
            if (!fs::exists(sourcePath)) {
                std::cerr << "Исходный файл не существует: " << sourcePath << std::endl;
                return false;
            }

            // Копируем файл
            fs::copy_file(sourcePath, destination, fs::copy_options::overwrite_existing);
            cout << "Создан бэкап: " << unique_name << endl;
            
            // Удаляем старые
            cleanupOldFiles(3);
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Ошибка копирования: " << e.what() << std::endl;
            return false;
        }
    }

    string getLastCopiedPath() {
        getSortedFilesByDate();
        if (filesWithDates.empty()) {
            return {};
        }
        return filesWithDates.back().first.string();
    }
};

class FileName {
private:
    static inline const std::string SUFFIX = ".UnRob4ik";

    static bool hasSuffix(const std::string& filename) {
        return filename.size() > SUFFIX.size() &&
            filename.substr(filename.size() - SUFFIX.size()) == SUFFIX;
    }

public:
    static string addSuffix(const string& original) {
        return original + SUFFIX;
    }

    static string removeSuffix(const string& masked) {
        if (!hasSuffix(masked)) {
            throw std::runtime_error("Not a masked file");
        }
        return masked.substr(0, masked.length() - SUFFIX.length());
    }
};