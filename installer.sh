#!/bin/bash

echo "Начало сборки и установки..."
if ! command -v make &> /dev/null || ! command -v g++ &> /dev/null; then
    echo "Требуются make и g++ для сборки."
    exit 1
fi

make clean
make
if [ $? -ne 0 ]; then
    echo "Ошибка при сборке проекта."
    exit 1
fi
echo "Для установки в систему могут потребоваться права sudo."
sudo make install
if [ $? -eq 0 ]; then
    echo "Приложение 'cipher_app' успешно установлено!"
    echo "Важно: запускайте программу из папки проекта,"
    echo "так как она ищет библиотеки (.so) в текущей директории."
else
    echo "Ошибка при установке."
    exit 1
fi
