// ============================================================
//  СЕРВЕР БОЛЬНИЦЫ
//  Обслуживает 3 клиентов по очереди.
//  Для каждого: принимает ФИО, вес, рост → считает ИМТ →
//  отправляет обратно категорию.
//  Формат пакета: BMI|ФИО|вес|рост
//  Порт: 65432
// ============================================================

#include <iostream>
#include <string>
#include <sstream>   // std::stringstream — разбор строки по разделителю

#include <winsock2.h>   // сокеты Windows
#include <ws2tcpip.h>   // inet_pton и прочее

#pragma comment(lib, "ws2_32.lib")   // подключаем библиотеку Winsock

#define PORT 65432          // порт сервера
#define BUFFER_SIZE 1024    // размер буфера приёма
#define CLIENT_COUNT 3      // сколько клиентов обслужить

int main()
{
    // Русский язык в консоли
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    // Инициализация Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Создаём TCP-сокет
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Настраиваем адрес: локальный хост, порт PORT
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);                     // host → network order
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // безопаснее inet_addr

    // Привязываем сокет к адресу и начинаем слушать
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);   // очередь подключений — до 5

    std::cout << "=== СЕРВЕР БОЛЬНИЦЫ ===\n";
    std::cout << "Ожидание " << CLIENT_COUNT << " клиентов...\n\n";

    // Цикл: обслуживаем 3 клиентов подряд
    for (int i = 0; i < CLIENT_COUNT; ++i)
    {
        std::cout << "--- Клиент " << (i + 1) << " ---\n";
        std::cout << "Ожидание подключения...\n";

        // accept ждёт, пока клиент подключится, и возвращает сокет этого клиента
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        std::cout << "Клиент подключился!\n";

        // Принимаем данные от клиента
        char buffer[BUFFER_SIZE];
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        // Если клиент отключился, не прислав данные — пропускаем
        if (bytesReceived <= 0)
        {
            std::cout << "Клиент отключился, не прислав данные.\n\n";
            closesocket(clientSocket);
            continue;
        }

        buffer[bytesReceived] = '\0';   // завершаем строку нулём

        // Ожидаемый формат: BMI|Иван Иванов|75|1.8
        std::string data(buffer);
        std::stringstream ss(data);

        std::string command, name, weightStr, heightStr;

        // Читаем поля по разделителю '|'.
        // Благодаря '|' ФИО может содержать пробелы.
        std::getline(ss, command,   '|');   // "BMI"
        std::getline(ss, name,      '|');   // "Иван Иванов"
        std::getline(ss, weightStr, '|');   // "75"
        std::getline(ss, heightStr, '|');   // "1.8"

        // Преобразуем строки в числа. При ошибке — try/catch.
        double weight = 0.0, height = 0.0;
        try
        {
            weight = std::stod(weightStr);
            height = std::stod(heightStr);
        }
        catch (...)
        {
            std::string err = "Ошибка: не удалось разобрать вес или рост.\n";
            send(clientSocket, err.c_str(), (int)err.length(), 0);
            closesocket(clientSocket);
            continue;
        }

        // Защита от деления на ноль
        if (height <= 0.0)
        {
            std::string err = "Ошибка: рост должен быть больше нуля.\n";
            send(clientSocket, err.c_str(), (int)err.length(), 0);
            closesocket(clientSocket);
            continue;
        }

        // ИМТ = вес / рост²
        double bmi = weight / (height * height);

        // Категория по порогам ВОЗ
        std::string category;
        if (bmi < 18.5)
            category = "Недостаточный вес";
        else if (bmi < 25)
            category = "Норма";
        else if (bmi < 30)
            category = "Избыточный вес";
        else
            category = "Ожирение";

        // Формируем ответ
        std::string response =
            "Пациент: " + name +
            "\nИМТ: " + std::to_string(bmi) +
            "\nКатегория: " + category + "\n";

        // Отправляем ответ клиенту
        send(clientSocket, response.c_str(), (int)response.length(), 0);

        std::cout << "Обработан пациент: " << name << "\n\n";

        // Закрываем соединение с ЭТИМ клиентом (сервер продолжает работать)
        closesocket(clientSocket);
    }

    std::cout << "Все клиенты обслужены. Сервер завершает работу.\n";

    // Закрываем серверный сокет и выгружаем Winsock
    closesocket(serverSocket);
    WSACleanup();

    std::cout << "Нажмите Enter для выхода...";
    std::cin.get();
    return 0;
}
