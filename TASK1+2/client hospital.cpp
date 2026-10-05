// ============================================================
//  КЛИЕНТ БОЛЬНИЦЫ
//  Вводит ФИО (можно с пробелами), вес, рост,
//  отправляет серверу, печатает ответ.
//  Порт: 65432
// ============================================================

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

    // Инициализация Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Создаём сокет клиента
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Адрес сервера — тот же компьютер, порт PORT
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Подключаемся. Если сервер не запущен — выходим.
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cout << "Не удалось подключиться к серверу.\n";
        std::cout << "Сначала запустите сервер!\n";
        system("pause");   // чтобы окно не закрылось
        return 1;
    }

    std::cout << "=== КЛИЕНТ БОЛЬНИЦЫ ===\n\n";

    std::string fullName;
    double weight = 0.0, height = 0.0;

    // ФИО читаем через getline — можно вводить с пробелами
    std::cout << "Введите ФИО пациента: ";
    std::getline(std::cin, fullName);

    // Вес: защита от букв и прочего мусора
    std::cout << "Введите вес (кг): ";
    while (!(std::cin >> weight))
    {
        std::cout << "Некорректный ввод. Введите число: ";
        std::cin.clear();                // сбрасываем флаг ошибки
        std::cin.ignore(10000, '\n');    // выкидываем остаток строки
    }

    // Рост: аналогичная защита
    std::cout << "Введите рост (м, например 1.80): ";
    while (!(std::cin >> height))
    {
        std::cout << "Некорректный ввод. Введите число: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    // Если ввели рост в сантиметрах (180) — переводим в метры (1.80)
    if (height > 3.0)
        height = height / 100.0;

    // Формируем пакет: BMI|ФИО|вес|рост
    // Разделитель '|' — чтобы ФИО с пробелами не ломало разбор
    std::string message =
        "BMI|" + fullName + "|" +
        std::to_string(weight) + "|" +
        std::to_string(height);

    // Отправляем серверу
    send(clientSocket, message.c_str(), (int)message.length(), 0);

    std::cout << "\nДанные отправлены.\n";

    // Принимаем ответ
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytesReceived] = '\0';

    std::cout << "\n=== ОТВЕТ СЕРВЕРА ===\n" << buffer;

    // Закрываем соединение и выгружаем Winsock
    closesocket(clientSocket);
    WSACleanup();

    std::cout << "\nНажмите Enter для выхода...";
    std::cin.ignore(10000, '\n');   // съедаем остаток от последнего cin >>
    std::cin.get();
    return 0;
}
