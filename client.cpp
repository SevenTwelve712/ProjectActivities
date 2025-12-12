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
// Подключаем статическую библиотеку WinSock
#pragma comment(lib, "ws2_32.lib")

using namespace std;
// Функция-помощник: проверяет, пуст ли файл
bool isFileEmpty(std::ifstream& pFile) {
    return pFile.peek() == std::ifstream::traits_type::eof();
}
int main(void)
{
	// Инициализация генератора случайных чисел (Mersenne Twister)
	std::random_device rd;
    std::mt19937 gen(rd());
	// Установка кодировки консоли CP1251 для корректного отображения кириллицы
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	// Ключевые константы 
	const char SERVER_IP[] = "10.35.153.128";		// IPv4 адрес сервера
	const short SERVER_PORT_NUM = 7129;				// Порт сервера
	const short BUFF_SIZE = 4096;					// Максимальный размер буфера для обмена данных
 
	// Ключевая переменная erStat
	int erStat;										// Проверка на ошибки и возвращение их номера

	// Преобразование IP адреса сервера в структуру in_addr
	in_addr ip_to_num;		
	if (inet_pton(AF_INET, SERVER_IP, &ip_to_num) <= 0) {
        cerr << "Ошибка преобразования IP адреса" << endl;
        return 1;
    }
	// Инициализация WinSock
	WSADATA wsData;
	// Запрашиваем версию WinSock 2.2
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

	ZeroMemory(&servInfo, sizeof(servInfo)); 	// Обнуляем память структуры

	servInfo.sin_family = AF_INET;		// Семейство адресов (IPv4)
	servInfo.sin_addr = ip_to_num;		// IP адрес (уже в бинарном виде)
	servInfo.sin_port = htons(SERVER_PORT_NUM);		// Порт (htons переводит порядок байт из хостового в сетевой)

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
	
	// --- ВЗАИМОДЕЙСТВИЕ С ПОЛЬЗОВАТЕЛЕМ (Генерация данных) ---
	int userdata = 0;						//Введенные пользователем желаемые данные
	while (userdata != 2)		// Цикл меню: 1 - Генерация чисел в файл, 2 - Пропуск генерации (сразу к отправке)
	{
		cout << "Выберите вариант, что вы хотите сделать?\n1. Очистить файл numbers.txt, сгенерировать n случайных чисел в диапазоне [a;b) (n, a, b вводятся вручную) и записать их в файл numbers.txt;\n2. Считать данные из файла в сокет." << endl;
		cin >> userdata;
		cout << "\n";
		if (userdata != 1&&userdata!=2)
		{
			cerr << "Ошибка! Введены некорректные данные!" << endl;
		} else if (userdata == 1)
		{
			// Блок генерации чисел
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
			ofstream file("numbers.txt", std::ios::out | std::ios::trunc); // Открытие файла с флагами: out (запись) и trunc (удалить содержимое перед записью)
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
	// --- ВЗАИМОДЕЙСТВИЕ С ПОЛЬЗОВАТЕЛЕМ (Выбор поиска) ---
	userdata=0;
	float usersearch;
	// Цикл выбора алгоритма поиска (1-4)
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
	// --- ФОРМИРОВАНИЕ ПАКЕТА ДЛЯ ОТПРАВКИ ---
    // 1. Формируем заголовок запроса: "ID_алгоритма Искомое_Число "
	std::ostringstream oss;
    oss << userdata << " " << usersearch << " "; 
    std::string searchstring = oss.str();
	// 2. Копируем заголовок в буфер отправки
	vector <char> clientBuff;	
	vector <char> servBuff(BUFF_SIZE);		// Буфер для получения данных
	for (char c : searchstring) {
            clientBuff.push_back(c);
    }
	short packet_size = 0;		// Размер отправленных/полученных данных в байтах
	// 3. Читаем числа из файла и добавляем их в буфер отправки
	ifstream file("numbers.txt"); 
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
    // Показываем ASCII коды первых 20 байт, которые уйдут на сервер
    for (char c : clientBuff) {
		if (counter < 20) {
        cout << static_cast<int>(c) << " ";
		} else break;
		counter++;
    }
    cout << "\n";
	// --- ОТПРАВКА ДАННЫХ ---
	counter = 0;
	bool isSent = false;
	// Пытаемся отправить данные (с повторными попытками при ошибке)
	while (!isSent) {
		packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0); //Отправка данных на сервер
		if (packet_size == -1) {
            cerr << "Ошибка отправки! Повторная попытка через 2 секунды..." << endl;      
            // Пауза перед повторной попыткой
            std::this_thread::sleep_for(std::chrono::seconds(2));
			counter++;
        } else {
            cout << "Данные успешно отправлены на сервер! Ожидаем ответа." << endl;
            isSent = true; // Флаг для выхода из цикла
        }
		// Лимит попыток отправки
		if (counter == 5) {
			cerr << "Слишком много неудачных попыток отправки, закрываем клиент!" << endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}
	}
	shutdown(ClientSock, 1); //Закрытие сокета для записи со стороны клиента
	// --- ПОЛУЧЕНИЕ ОТВЕТА ---
	int totalBytesReceived = 0;

    while (true)
    {
        // ПРОВЕРКА: Если буфер заполнен (или почти заполнен), увеличиваем его
        if (totalBytesReceived >= servBuff.size()) {
            // Увеличиваем размер в 2 раза
            servBuff.resize(servBuff.size() * 2);
            cout << "Буфер расширен до " << servBuff.size() << " байт." << endl;
        }

        // Читаем в свободное место
        // servBuff.data() + totalBytesReceived — это указатель на "хвост" записанных данных
        // servBuff.size() - totalBytesReceived — это сколько места осталось
        int bytesToRead = servBuff.size() - totalBytesReceived;
        
        packet_size = recv(ClientSock, servBuff.data() + totalBytesReceived, bytesToRead, 0);

        if (packet_size > 0) {
            totalBytesReceived += packet_size;
            cout << "Принято " << packet_size << " байт. Всего: " << totalBytesReceived << endl;
        } 
        else if (packet_size == 0) {
            cout << "Сервер закончил передачу." << endl;
            break;
        } 
        else {
            cerr << "Ошибка recv" << endl;
            break;
        }
    }

    // После цикла обрезаем вектор до реального размера данных, чтобы убрать лишние нули в конце
    servBuff.resize(totalBytesReceived);
    string dataString(servBuff.begin(), servBuff.end());
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
