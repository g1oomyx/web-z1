// ============================================================
//  СЕРВЕР ДЕКАНАТА
//  Обслуживает 3 клиентов по очереди.
//  Принимает ФИО и 4 оценки → считает средний балл,
//  определяет категорию и наличие стипендии.
//  Формат пакета: DEANERY|ФИО|g1|g2|g3|g4
//  Порт: 65433
// ============================================================

#include <iostream>
#include <string>
#include <sstream>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 65433
#define BUFFER_SIZE 1024
#define CLIENT_COUNT 3

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "=== СЕРВЕР ДЕКАНАТА ===\n";
    std::cout << "Ожидание " << CLIENT_COUNT << " клиентов...\n\n";

    // Обслуживаем 3 клиентов подряд
    for (int i = 0; i < CLIENT_COUNT; ++i)
    {
        std::cout << "--- Клиент " << (i + 1) << " ---\n";
        std::cout << "Ожидание подключения...\n";

        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        std::cout << "Клиент подключился!\n";

        // Принимаем данные
        char buffer[BUFFER_SIZE];
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (bytesReceived <= 0)
        {
            std::cout << "Клиент отключился, не прислав данные.\n\n";
            closesocket(clientSocket);
            continue;
        }

        buffer[bytesReceived] = '\0';

        // Ожидаемый формат: DEANERY|Иван Иванов|5|4|5|5
        std::string data(buffer);
        std::stringstream ss(data);

        std::string command, name;
        std::string s1, s2, s3, s4;

        // Читаем поля по разделителю '|'
        std::getline(ss, command, '|');   // "DEANERY"
        std::getline(ss, name,    '|');   // "Иван Иванов"
        std::getline(ss, s1,      '|');   // "5"
        std::getline(ss, s2,      '|');   // "4"
        std::getline(ss, s3,      '|');   // "5"
        std::getline(ss, s4,      '|');   // "5"

        // Преобразуем оценки в int
        int g1 = 0, g2 = 0, g3 = 0, g4 = 0;
        try
        {
            g1 = std::stoi(s1);
            g2 = std::stoi(s2);
            g3 = std::stoi(s3);
            g4 = std::stoi(s4);
        }
        catch (...)
        {
            std::string err = "Ошибка: не удалось разобрать оценки.\n";
            send(clientSocket, err.c_str(), (int)err.length(), 0);
            closesocket(clientSocket);
            continue;
        }

        // Средний балл
        double average = (g1 + g2 + g3 + g4) / 4.0;

        // Определяем категорию студента
        std::string category;
        bool hasTwo   = (g1 == 2 || g2 == 2 || g3 == 2 || g4 == 2);
        bool hasThree = (g1 == 3 || g2 == 3 || g3 == 3 || g4 == 3);
        bool allFive  = (g1 == 5 && g2 == 5 && g3 == 5 && g4 == 5);

        if (hasTwo)
            category = "Неуспевающий";
        else if (hasThree)
            category = "Троечник";
        else if (allFive)
            category = "Отличник";
        else
            category = "Хорошист";

        // Стипендия: да, если нет ни троек, ни двоек
        std::string scholarship = (hasTwo || hasThree) ? "Нет" : "Да";

        // Формируем ответ
        std::string response =
            "Студент: " + name +
            "\nСредний балл: " + std::to_string(average) +
            "\nКатегория: " + category +
            "\nСтипендия: " + scholarship + "\n";

        // Отправляем
        send(clientSocket, response.c_str(), (int)response.length(), 0);

        std::cout << "Обработан студент: " << name << "\n\n";

        // Закрываем соединение с этим клиентом
        closesocket(clientSocket);
    }

    std::cout << "Все клиенты обслужены. Сервер завершает работу.\n";

    closesocket(serverSocket);
    WSACleanup();

    std::cout << "Нажмите Enter для выхода...";
    std::cin.get();
    return 0;
}
