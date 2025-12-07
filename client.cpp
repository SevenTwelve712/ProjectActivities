#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <inaddr.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;


int main(void)
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	// Ключевые константы
	const char SERVER_IP[] = "10.35.153.128";		// IPv4 адрес сервера
	const short SERVER_PORT_NUM = 7129;				// Порт сервера
	const short BUFF_SIZE = 4096;					// Максимальный размер буфера для обмена данных
 
	// Ключевая переменная erStat
	int erStat;										// Проверка на ошибки и возвращение их номера

	// Преобразование IP адреса сервера
	in_addr ip_to_num;		
	inet_pton(AF_INET, SERVER_IP, &ip_to_num);


	// Инициализация WinSock
	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2,2), &wsData);

	if (erStat != 0) {
		cout << "Ошибка инициализации версии WinSock #";
		cout << WSAGetLastError();
		return 1;
	}
	else 
		cout << "Инициализация WinSock прошла успешно!" << endl;
	
	// Инициализация сокета
	SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);

	if (ClientSock == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета #" << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
	}
	else 
		cout << "Инициализация сокета клиента прошла успешно!" << endl;

	// Задание структуры servInfo, в которой будут храниться тип IP адреса, сам адрес и порт сервера
	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;	
	servInfo.sin_port = htons(SERVER_PORT_NUM);

	//Подключение к серверу

	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
	
	if (erStat != 0) {
		cout << "Попытка подключения к серверу не удалась. Ошибка #" << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
		return 1;
	}
	else { 
		cout << "Соединение с сервером установлено успешно!" << endl;
	}
	
	vector <char> clientBuff(BUFF_SIZE);	
	vector <char> servBuff(BUFF_SIZE);		// Буферы для отправки и получения данных
	short packet_size = 0;										// Размер отправленных/полученных данных в байтах
	ifstream file("numbers.txt");
    string bufferstring;
    if (!file.is_open()) {
        cout << "Ошибка открытия файла!" << endl;
		closesocket(ClientSock);
		WSACleanup();
        return 1;
    }

    while (getline(file, bufferstring)) {
        // Проходим по текущей строке
        for (char currentchar : bufferstring) {
            // Добавляем символ в общий вектор
            clientBuff.push_back(currentchar);
        }
    }

    cout << "\nASCII коды всех символов:\n";
    // Вывод содержимого в формате ASCII
    for (char c : clientBuff) {
        cout << static_cast<int>(c) << " ";
    }
    cout << "\n";
	send(ClientSock, clientBuff.data(), clientBuff.size(), 0); //Отправка данных на сервер
	shutdown(ClientSock, 1); //Закрытие сокета для записи со стороны клиента
	packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0); //Получение данных с сервера
	if (packet_size == SOCKET_ERROR) { 
			cout << "Не удалось получить данные с сервера. Ошибка # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			system("pause");
			return 1;
		}
		else
			cout << clientBuff.data() << endl;
	closesocket(ClientSock);
	WSACleanup();
	system("pause");
	return 0;

}
