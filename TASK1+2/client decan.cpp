// ============================================================
//  КЛИЕНТ ДЕКАНАТА
//  Вводит ФИО студента (можно с пробелами) и 4 оценки,
//  отправляет серверу, печатает ответ.
//  Порт: 65433
// ============================================================

#include <iostream>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 65433
#define BUFFER_SIZE 1024

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    // Инициализация Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Создаём сокет
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Адрес сервера деканата
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Подключаемся
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cout << "Не удалось подключиться к серверу деканата.\n";
        std::cout << "Сначала запустите сервер!\n";
        system("pause");
        return 1;
    }

    std::cout << "=== КЛИЕНТ ДЕКАНАТА ===\n\n";

    std::string fullName;
    int g1 = 0, g2 = 0, g3 = 0, g4 = 0;

    // ФИО читаем через getline — можно вводить с пробелами
    std::cout << "Введите ФИО студента: ";
    std::getline(std::cin, fullName);

    // 4 оценки: защита от неверного ввода
    std::cout << "Введите 4 оценки через пробел (например: 5 4 5 5): ";
    while (!(std::cin >> g1 >> g2 >> g3 >> g4))
    {
        std::cout << "Некорректный ввод. Введите 4 числа через пробел: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    // Формируем пакет: DEANERY|ФИО|g1|g2|g3|g4
    std::string message =
        "DEANERY|" + fullName + "|" +
        std::to_string(g1) + "|" +
        std::to_string(g2) + "|" +
        std::to_string(g3) + "|" +
        std::to_string(g4);

    // Отправляем
    send(clientSocket, message.c_str(), (int)message.length(), 0);

    std::cout << "\nДанные отправлены.\n";

    // Принимаем ответ
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytesReceived] = '\0';

    std::cout << "\n=== ОТВЕТ СЕРВЕРА ===\n" << buffer;

    // Закрываем
    closesocket(clientSocket);
    WSACleanup();

    std::cout << "\nНажмите Enter для выхода...";
    std::cin.ignore(10000, '\n');
    std::cin.get();
    return 0;
}
