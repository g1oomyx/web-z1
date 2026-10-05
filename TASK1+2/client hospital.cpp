// Клиент больницы.
// Позволяет ввести ФИО с пробелами.
// Порт: 65432

#include <iostream>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 65432
#define BUFFER_SIZE 1024

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cout << "Не удалось подключиться к серверу.\n";
        std::cout << "Сначала запустите сервер!\n";
        system("pause");
        return 1;
    }

    std::cout << "=== КЛИЕНТ БОЛЬНИЦЫ ===\n\n";

    std::string fullName;
    double weight = 0.0, height = 0.0;

    // ФИО читаем через getline — можно с пробелами
    std::cout << "Введите ФИО пациента: ";
    std::getline(std::cin, fullName);

    // Вес
    std::cout << "Введите вес (кг): ";
    while (!(std::cin >> weight))
    {
        std::cout << "Некорректный ввод. Введите число: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    // Рост
    std::cout << "Введите рост (м, например 1.80): ";
    while (!(std::cin >> height))
    {
        std::cout << "Некорректный ввод. Введите число: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    // Если рост в см — переводим в метры
    if (height > 3.0)
        height = height / 100.0;

    // Формируем пакет: BMI|ФИО|вес|рост
    std::string message =
        "BMI|" + fullName + "|" +
        std::to_string(weight) + "|" +
        std::to_string(height);

    send(clientSocket, message.c_str(), (int)message.length(), 0);

    std::cout << "\nДанные отправлены.\n";

    // Ответ
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytesReceived] = '\0';

    std::cout << "\n=== ОТВЕТ СЕРВЕРА ===\n" << buffer;

    closesocket(clientSocket);
    WSACleanup();

    std::cout << "\nНажмите Enter для выхода...";
    std::cin.ignore(10000, '\n');
    std::cin.get();
    return 0;
}
