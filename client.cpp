#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <inaddr.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <windows.h>
#include <random>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool isFileEmpty(std::ifstream& pFile) {
    return pFile.peek() == std::ifstream::traits_type::eof();
}
int main(void)
{
	//Настраиваем генератор случайных чисел
	std::random_device rd;
    std::mt19937 gen(rd());
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	// Ключевые константы 
	const char SERVER_IP[] = "10.35.153.128";		// IPv4 адрес сервера
	const short SERVER_PORT_NUM = 7129;				// Порт сервера
	// const short BUFF_SIZE = 4096;					// Максимальный размер буфера для обмена данных
 
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
	
	vector <char> clientBuff;	
	vector <char> servBuff;		// Буферы для отправки и получения данных
	short packet_size = 0;										// Размер отправленных/полученных данных в байтах
	int userdata = 0;						//Введенные пользователем желаемые данные
	while (userdata != 2)		//Спрашиваем у пользователя, что он хочет сделать в файле
	{
		cout << "Выберите вариант, что вы хотите сделать?\n1. Сгенерировать n случайных чисел в диапазоне [a;b) (n, a, b вводятся вручную) и записать их в файл numbers.txt;\n2. Считать данные из файла в сокет." << endl;
		cin >> userdata;
		cout << "\n";
		if (userdata != 1|userdata!=2)
		{
			cout << "Ошибка! Введены некорректные данные!" << endl;
		} else if (userdata == 1)
		{
			cout << "Сколько чисел вы хотите сгенерировать?" << endl;
			int n; 
			cin >> n;
			if (n<=0)
			{
				cout << "\nВведите корректный n!" << endl;
				continue;
			}
			cout << "\nВведите начальное значение интервала: " << endl;
			double interval_start, interval_finish;
			cin >> interval_start;
			cout << "\nВведите конечное значение интервала: " << endl;
			cin >> interval_finish;
			if (interval_start>interval_finish) { 
				std::swap(interval_start, interval_finish);	//Проверка на дурака - при некорректном интервале меняем значения друг с другом
			}
			cout << "Будет сгенерировано " << n << " случайных чисел в диапазоне от " << interval_start << " до " << interval_finish << " и записано в файл. Некоторое количество первых случайных чисел:" << endl;
			std::uniform_real_distribution<double> dist(interval_start, interval_finish); //Создаём объект распределения для генерации случайных чисел 
			ofstream file("numbers.txt"); //Открываем файл для записи
			if (!file.is_open()) {
        		cout << "Ошибка: Не удалось открыть файл для записи!" << endl;
				continue;
    		}	
			// Настройка формата записи 
   			file << std::fixed << std::setprecision(4); // 4 знака после запятой
			for (int i=0; i<n; i++) { //Цикл генерации n случайных чисел 
				double random_num = dist(gen);
				file << random_num << " ";	
				if (i<10) {
					cout << random_num << " " << endl;
				}
			}
			file.close();
			cout << "Готово! Числа записаны в файл!" << endl;
		}	
	}
	
	ifstream file("numbers.txt"); //Открываем файл для чтения
    if (!file.is_open()) {
        cout << "Ошибка открытия файла!" << endl;
		closesocket(ClientSock);
		WSACleanup();
        return 1;
    }
	if (isFileEmpty(file)) {
        std::cout << "Файл пустой!\n";
		closesocket(ClientSock);
		WSACleanup();
        return 1;
    }
	string bufferstring;		//Буферная строка
    while (getline(file, bufferstring)) {	//Считываем числа из файла
        // Проходим по текущей строке
        for (char currentchar : bufferstring) {
            // Добавляем символ в общий вектор
            clientBuff.push_back(currentchar);
        }
    }
    cout << "\nASCII коды части отправленных символов:\n";
	int counter = 0;
    // Вывод содержимого в формате ASCII
    for (char c : clientBuff) {
		if (counter < 20) {
        cout << static_cast<int>(c) << " ";
		} else break;
		counter++;
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
