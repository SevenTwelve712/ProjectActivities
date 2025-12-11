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
#include <thread> // Для паузы (sleep)
#include <chrono> // Для задания времени паузы
#include <sstream>

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
	const short BUFF_SIZE = 4096;					// Максимальный размер буфера для обмена данных
 
	// Ключевая переменная erStat
	int erStat;										// Проверка на ошибки и возвращение их номера

	// Преобразование IP адреса сервера
	in_addr ip_to_num;		
	if (inet_pton(AF_INET, SERVER_IP, &ip_to_num) <= 0) {
        cerr << "Ошибка преобразования IP адреса" << endl;
        return 1;
    }
	// Инициализация WinSock
	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2,2), &wsData);

	if (erStat != 0) {
		cerr << "Ошибка инициализации версии WinSock #";
		cout << WSAGetLastError();
		return 1;
	}
	else 
		cout << "Инициализация WinSock прошла успешно!" << endl;
	// Инициализация сокета
	SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);

	if (ClientSock == INVALID_SOCKET) {
		cerr << "Ошибка инициализации сокета #" << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
		return 0;
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
	cout << "Подключение к серверу..." << endl;
	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
	
	if (erStat != 0) {
		cerr << "Попытка подключения к серверу не удалась. Ошибка #" << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
		return 0;
		}
	else { 
		cout << "Соединение с сервером установлено успешно!" << endl;
	}
	
	
	int userdata = 0;						//Введенные пользователем желаемые данные
	while (userdata != 2)		//Спрашиваем у пользователя, что он хочет сделать в файле
	{
		cout << "Выберите вариант, что вы хотите сделать?\n1. Очистить файл numbers.txt, сгенерировать n случайных чисел в диапазоне [a;b) (n, a, b вводятся вручную) и записать их в файл numbers.txt;\n2. Считать данные из файла в сокет." << endl;
		cin >> userdata;
		cout << "\n";
		if (userdata != 1||userdata!=2)
		{
			cerr << "Ошибка! Введены некорректные данные!" << endl;
		} else if (userdata == 1)
		{
			cout << "Сколько чисел вы хотите сгенерировать?" << endl;
			int n; 
			cin >> n;
			if (n<=0)
			{
				cerr << "\nВведите корректный n!" << endl;
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
			ofstream file("numbers.txt", std::ios::out | std::ios::trunc); //Открываем файл для записи
			if (!file.is_open()) {
        		cerr << "Ошибка: Не удалось открыть файл для записи!" << endl;
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
	userdata=0;
	float usersearch;
	while (true)
	{
		cout << "Выберите алгоритм поиска:\n1. Алгоритм последовательного поиска;\n2. Алгоритм быстрого последовательного поиска; \n3. Алгоритм бинарного поиска; \n4. Алгоритм последовательного поиска в упорядоченном массиве." << endl;
		cin >> userdata;
		if (userdata >= 1 && userdata <= 4)
		{
			cout << "\nВведите число, которое нужно найти в файле:" << endl;
			cin >> usersearch;
			break;
		} else {
			cerr << "\nОшибка! Введите корректные данные!" << endl;
		}
	}
	std::ostringstream oss;
    oss << userdata << " " << usersearch << " "; 
    std::string searchstring = oss.str();
	vector <char> clientBuff;	
	vector <char> servBuff(BUFF_SIZE);		// Буферы для отправки и получения данных
	for (char c : searchstring) {
            clientBuff.push_back(c);
    }
	short packet_size = 0;		// Размер отправленных/полученных данных в байтах
	ifstream file("numbers.txt"); //Открываем файл для чтения
    if (!file.is_open()) {
        cerr << "Ошибка открытия файла!" << endl;
		closesocket(ClientSock);
		WSACleanup();
        return 1;
    }
	if (isFileEmpty(file)) {
        cerr << "Файл пустой!\n";
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
	counter = 0;
	bool isSent = false;
	while (!isSent) {
		packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0); //Отправка данных на сервер
		if (packet_size == -1) {
            cerr << "Ошибка отправки! Повторная попытка через 2 секунды..." << endl;      
            // Делаем паузу, чтобы не нагружать процессор бесконечным циклом
            std::this_thread::sleep_for(std::chrono::seconds(2));
			counter++;
        } else {
            cout << "Данные успешно отправлены на сервер! Ожидаем ответа." << endl;
            isSent = true; // Флаг для выхода из цикла
        }
		if (counter == 5) {
			cerr << "Слишком много неудачных попыток отправки, закрываем клиент!" << endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}
	}
	shutdown(ClientSock, 1); //Закрытие сокета для записи со стороны клиента
	while (true)
	{
		packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0); //Получение данных с сервера	
		if (packet_size >0) {
			cout << "Сервер отправил " << packet_size << " байт." << endl;
		} else if (packet_size == 0)
		{
			cout << "Сервер закончил отправку." << endl;
			break;
		} else {	
			cerr << "Не удалось получить данные!" << endl;
			closesocket(ClientSock);
			WSACleanup();
			break;
		}
	}
	std::string dataString(servBuff.begin(), servBuff.end());
	std::stringstream ss(dataString);
	int firstNumber;
	if (ss >> firstNumber && firstNumber != 0) {
        cout << "Искомое число в файле находится под порядковым номером: " << firstNumber << endl;
    } else if (firstNumber == 0)
	{
		cout << "Не удалось найти искомое число!" << endl;	
	} else {
        cout << "В полученном векторе не оказалось чисел." << endl;
    }
	closesocket(ClientSock);
	WSACleanup();

	system("pause");
	return 0;

}
