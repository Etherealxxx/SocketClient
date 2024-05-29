#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main() {
    WSADATA wsaData; // Структура для хранения информации о реализации WinSock
    ADDRINFO hints; // Структура для хранения информации о желаемом типе адресов
    ADDRINFO* addrResult; // Указатель на результирующую структуру адресов
    SOCKET ListenSocket = INVALID_SOCKET; // Переменная для хранения сокета для прослушивания
    SOCKET ConnectSocket = INVALID_SOCKET; // Переменная для хранения сокета подключения
    char recvBuffer[512]; // Буфер для приема данных

    const char* sendBuffer = "Hello from server"; // Сообщение для отправки клиенту

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Настройка структуры hints
    ZeroMemory(&hints, sizeof(hints)); // Обнуление памяти для структуры
    hints.ai_family = AF_INET; // Использование IPv4
    hints.ai_socktype = SOCK_STREAM; // Установка типа сокета на потоковый (TCP)
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP
    hints.ai_flags = AI_PASSIVE; // Установка флага для использования адреса сервера

    // Получение адреса и порта сервера
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return 1;
    }

    // Создание сокета для прослушивания
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для адресной информации
        WSACleanup(); // Очистка ресурсов Winsock
        return 1;
    }

    // Привязка сокета к адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для адресной информации
        WSACleanup(); // Очистка ресурсов Winsock
        return 1;
    }

    // Прослушивание входящих соединений
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для адресной информации
        WSACleanup(); // Очистка ресурсов Winsock
        return 1;
    }

    // Принятие входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для адресной информации
        WSACleanup(); // Очистка ресурсов Winsock
        return 1;
    }

    // Слушающий сокет больше не нужен
    closesocket(ListenSocket);

    // Получение данных до тех пор, пока клиент не закроет соединение
    do {
        ZeroMemory(recvBuffer, 512); // Обнуление буфера приема
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка ответа клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << WSAGetLastError() << endl;
                closesocket(ConnectSocket); // Закрытие сокета
                WSACleanup(); // Очистка ресурсов Winsock
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl;
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket); // Закрытие сокета
            WSACleanup(); // Очистка ресурсов Winsock
            return 1;
        }
    } while (result > 0);

    // Завершение соединения, так как работа завершена
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket); // Закрытие сокета
        WSACleanup(); // Очистка ресурсов Winsock
        return 1;
    }

    // Очистка ресурсов
    closesocket(ConnectSocket); // Закрытие сокета
    WSACleanup(); // Очистка ресурсов Winsock
    return 0;
}
