#include "auth.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

using namespace std;

const string AUTH_FILE = "auth.bin";
string hashPassword(const string& password, const string& salt) {
    unsigned long hash = 5381;
    string combined = salt + password + salt;

    for (char c : combined) {
        hash = ((hash << 5) + hash) + c;
    }        
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

string generateSalt() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 15);
    stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << hex << dis(gen);
    }
    return ss.str();
}

void saveAuth(const string& hash, const string& salt) {
    ofstream file(AUTH_FILE, ios::binary);
    if (!file) {
        throw runtime_error("Ошибка: Не удалось сохранить данные аутентификации!");
    }
    size_t saltLen = salt.length();
    size_t hashLen = hash.length();
    file.write(reinterpret_cast<const char*>(&saltLen), sizeof(saltLen));
    file.write(salt.c_str(), saltLen);
    file.write(reinterpret_cast<const char*>(&hashLen), sizeof(hashLen));
    file.write(hash.c_str(), hashLen);
}

bool loadAuth(string& hash_out, string& salt_out) {
    ifstream file(AUTH_FILE, ios::binary);
    if (!file) return false;
    size_t saltLen;
    if (!file.read(reinterpret_cast<char*>(&saltLen), sizeof(saltLen))) return false;
    vector<char> saltBuf(saltLen);
    file.read(saltBuf.data(), saltLen);
    salt_out.assign(saltBuf.begin(), saltBuf.end());
    size_t hashLen;
    file.read(reinterpret_cast<char*>(&hashLen), sizeof(hashLen));
    vector<char> hash_buf(hashLen);
    file.read(hash_buf.data(), hashLen);
    hash_out.assign(hash_buf.begin(), hash_buf.end());

    return true;
}

bool isRegistered() {
    ifstream file(AUTH_FILE);
    return file.good();
}

void registerUser() {
    cout << "Регистрация" << endl;
    string pass1, pass2;
    while (true) {
        cout << "Введите новый пароль: ";
        getline(cin, pass1);
        if (pass1.empty()) {
            cout << "Пароль не может быть пустым!" << endl;
            continue;
        }
        cout << "Повторите пароль: ";
        getline(cin, pass2);
        if (pass1 == pass2) {
            string salt = generateSalt();
            string hash = hashPassword(pass1, salt);
            saveAuth(hash, salt);
            cout << "Пароль успешно установлен!" << endl;
            break;
        } else {
            cout << "Пароли не совпадают. Попробуйте снова." << endl;
        }
    }
}

bool login() {
    string storedHash, salt;
    if (!loadAuth(storedHash, salt)) {
        throw runtime_error("Ошибка: Файл авторизации поврежден или отсутствует. Требуется повторная регистрация.");
    }
    int attempts = 0;
    while (true) {
        string input_pass;
        cout << "Введите пароль для входа: ";
        getline(cin, input_pass);
        if (hashPassword(input_pass, salt) == storedHash) {
            cout << "Вход выполнен успешно." << endl;
            return true;
        } else {
            attempts++;
            int waitTime = attempts;
            cout << "Неверный пароль! Подождите " << waitTime << " сек..." << endl;
            this_thread::sleep_for(chrono::seconds(waitTime));
        }
    }
}

void changePassword() {
    string storedHash, salt;
    if (!loadAuth(storedHash, salt)) {
        cout << "Ошибка чтения данных аутентификации." << endl;
        return;
    }

    cout << "Смена пароля" << endl;
    int attempts = 0;
    while (true) {
        string oldPass;
        cout << "Введите текущий пароль (или 'cancel' для отмены): ";
        getline(cin, oldPass);

        if (oldPass == "cancel") return;
        if (hashPassword(oldPass, salt) == storedHash) {
            break;
        } else {
            attempts++;
            cout << "Неверный текущий пароль. Задержка " << attempts << " сек..." << endl;
            this_thread::sleep_for(chrono::seconds(attempts));
        }
    }

    string pass1, pass2;
    while (true) {
        cout << "Введите новый пароль: ";
        getline(cin, pass1);
            if (pass1.empty()) {
            cout << "Пароль не может быть пустым!" << endl;
            continue;
        }
        cout << "Повторите новый пароль: ";
        getline(cin, pass2);

        if (pass1 == pass2) {
            string newSalt = generateSalt();
            string newHash = hashPassword(pass1, newSalt);
            saveAuth(newHash, newSalt);
            cout << "Пароль успешно изменен!" << endl;
            return;
        } else {
            cout << "Пароли не совпадают." << endl;
        }
    }
}
