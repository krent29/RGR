#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <dlfcn.h>
#include <stdexcept>
#include <algorithm>
#include "cipher_interface.h"
#include "auth.h"

using namespace std;

typedef int (*TransformFunc)(const char*, unsigned char*, size_t, bool);

string cleanPath(string path) {
    if (path.empty()) return path;
    if ((path.front() == '\'' && path.back() == '\'') || (path.front() == '"' && path.back() == '"')) {
        return path.substr(1, path.length() - 2);
    }
    return path;
}

string getCipherStatus(const string& libPath) {
    void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) return "[Не найден/Ошибка]";
    dlclose(handle);
    return "[Готов]";
}

string readKeyFromFile(const string& filePath) {
    ifstream file(cleanPath(filePath));
    if (!file.is_open()) {
        throw runtime_error("Не удалось открыть файл ключа: " + filePath);
    }
    string key;
    getline(file, key);
    if (key.empty()) {
        throw runtime_error("Файл ключа пуст: " + filePath);
    }
    return key;
}

void processCipher(const string& libPath, const string& cipherName) {
    void* libHandle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!libHandle) {
        throw runtime_error("Библиотека " + cipherName + " не загружена: " + string(dlerror()));
    }

    TransformFunc transform = (TransformFunc)dlsym(libHandle, "transform");
    const char* dlsymError = dlerror();
    if (dlsymError) {
        dlclose(libHandle);
        throw runtime_error("Ошибка dlsym: " + string(dlsymError));
    }

    cout << "\n" << cipherName << "\n";
    cout << "1. Обработать текст из консоли\n";
    cout << "2. Обработать файл\n";
    cout << "Ваш выбор: ";
    int mode;
    cin >> mode;
    cin.ignore();
    cout << "1. Зашифровать\n2. Расшифровать\nВыбор: ";
    int action;
    cin >> action;
    bool isEncrypt = (action == 1);
    cin.ignore();
    cout << "Источник ключа (1 - консоль, 2 - файл): ";
    int keySource;
    cin >> keySource;
    cin.ignore();
    string cipherKey;
    if (keySource == 1) {
        cout << "Введите ключ: ";
        getline(cin, cipherKey);
    } else {
        string keyPath;
        cout << "Путь к файлу ключа: ";
        getline(cin, keyPath);
        cipherKey = readKeyFromFile(keyPath);
    }
    vector<unsigned char> dataBuffer;
    string inputPath, outputPath;

    if (mode == 1) {
        cout << "Введите текст: ";
        string text;
        getline(cin, text);
        for (char c : text) dataBuffer.push_back((unsigned char)c);
    } else {
        cout << "Введите путь к входному файлу: ";
        getline(cin, inputPath);
        inputPath = cleanPath(inputPath);

        ifstream inFile(inputPath, ios::binary | ios::ate);
        if (!inFile.is_open()) {
            dlclose(libHandle);
            throw runtime_error("Не удалось открыть файл: " + inputPath);
        }
        streamsize fileSize = inFile.tellg();
        inFile.seekg(0, ios::beg);
        dataBuffer.resize(fileSize);
        inFile.read((char*)dataBuffer.data(), fileSize);
        inFile.close();
    }

    transform(cipherKey.c_str(), dataBuffer.data(), dataBuffer.size(), isEncrypt);
    if (mode == 1) {
        cout << "Результат: ";
        for (unsigned char b : dataBuffer) cout << (char)b;
        cout << endl;
    }
    cout << "Путь к выходному файлу (пусто для default): ";
    getline(cin, outputPath);
    outputPath = cleanPath(outputPath);
    if (outputPath.empty()) {
        outputPath = isEncrypt ? "encrypted_output.bin" : "decrypted_output.txt";
        cout << "Используется путь по умолчанию: " << outputPath << endl;
    }
    ofstream outFile(outputPath, ios::binary);
    if (!outFile.is_open()) {
        dlclose(libHandle);
        throw runtime_error("Не удалось сохранить в файл: " + outputPath);
    }
    outFile.write((const char*)dataBuffer.data(), dataBuffer.size());
    outFile.close();
    dlclose(libHandle);
    cout << "Готово!\n";
}

int main(int argc, char* argv[]) {
    try {
        if (!isRegistered()) {
            cout << "Первый запуск: необходимо установить пароль." << endl;
            registerUser();
        } else {
            login();
        }

        while (true) {
            cout << "\nМеню" << endl;
            cout << "1. Табличная перестановка " << getCipherStatus("./columnar.so") << endl;
            cout << "2. Квадрат Полибия " << getCipherStatus("./polybius.so") << endl;
            cout << "3. Зафиксированная перестановка " << getCipherStatus("./fixed.so") << endl;
            cout << "4. Сменить пароль" << endl;
            cout << "0. Выход" << endl;
            cout << "Выберите действие: ";
            string choice;
            cin >> choice;
            cin.ignore();

            if (choice == "0") break;
            if (choice == "4") {
                changePassword();
                continue;
            }
            string lib;
            string name;
            if (choice == "1") { lib = "./columnar.so"; name = "Табличная перестановка"; }
            else if (choice == "2") { lib = "./polybius.so"; name = "Квадрат Полибия"; }
            else if (choice == "3") { lib = "./fixed.so"; name = "Фиксированная перестановка"; }
            else {
                cout << "Неверный выбор. Попробуйте снова." << endl;
                continue;
            }
            try { processCipher(lib, name); }
            catch (const exception& e) { cerr << "Ошибка: " << e.what() << endl; }
        }
    } catch (...) { return 1; }
}
