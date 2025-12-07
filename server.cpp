// #include<iostream>
// #include<cmath>
// #include<cstring>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <vector>
// #include<sstream>
// using namespace std;

// int create_listening_socket(){
//     // Создает слушающий сокет на порту 7129, биндит его и начинает прослушивание
//     const int port = 7129;
//     struct sockaddr_in server_addr;

//     // Создаем прослушивающий входящие соединения сокет
//     int listening_socket = socket(AF_INET, SOCK_STREAM, 0); // Тип адреса, тип пакетов, протокол

//     if (listening_socket < 0){
//         cerr << "Ошибка инициализации слушающего сокета" << endl;
//         return -1;
//     }
//     cout << "Слушающий сокет инициализирован успешно" << endl;

//     // Указываем адрес сервера
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
//     server_addr.sin_addr.s_addr = INADDR_ANY;

//     // Привязываем сокет к порту
//     int bind_result = ::bind(listening_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
//     if (bind_result < 0){ // сокет, ссылка на адрес, размер структуры адреса
//         cerr << "Ошибка при привязывании сокета к адресу" << endl;
//         return -1;
//     }
//     cout << "Сокет успешно привязан к адресу и порту" << endl;

//     // Начинаем прослушивать
//     int listen_result = listen(listening_socket, 128);
//     if (listen_result < 0){
//         cerr << "Ошибка при попытке начать прослушивание сокета" << endl;
//         return -1;
//     }

//     cout << "Сервер слушает на порту " << port << endl;
//     return listening_socket;
// }

// int accept_next_connection(int listening_socket){
// 	// Создает сокет для общения с пользователем
// 	struct sockaddr_in client_addr;
// 	socklen_t client_addr_len = sizeof(client_addr);
// 	char ip_str[INET_ADDRSTRLEN];

// 	int client_socket = accept(listening_socket, (sockaddr *)&client_addr, &client_addr_len);

// 	// Проверяем, получилось ли создать сокет для общения с конкретным пользователем
// 	if (client_socket < 0){
// 		cerr << "Ошибка при создании сокета для общения с клиентом" << endl;
// 		return -1;
// 	}
// 	cout << "Сокет для общения с клиентом создан" << endl;
// 	cout << "Принято соединение от " << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, INET_ADDRSTRLEN) << endl;
// 	return client_socket;
// }

// pair<bool, vector<float>> read_nums_from_socket(int socket){
// 	// Принимает сокет, в котором гарантированно записаны числа через пробел, возвращает вектор с этими символами
// 	pair<bool, vector<float>> result;
// 	string current_num = "";
// 	vector<float> nums;
// 	char chars_arr[50];
// 	ssize_t bytes_read;
// 	char c;
// 	string valid = "0123456789.+-eE";

// 	cout << "Считываем данные из сокета" << endl;
// 	while ((bytes_read = read(socket, &chars_arr, sizeof(chars_arr))) > 0){
// 		for (int i = 0; i <= bytes_read; i++){
// 			c = i == bytes_read ? ' ' : chars_arr[i];
// 			if (valid.find(c) != string::npos){current_num += c;} // если символ может находится в вещественном числе
// 			else if (!current_num.empty()){ // в ином случае если текущее значение числа не пустое пытаемся преобразовать его в float, затем обнуляем текуще значение
// 				try{nums.push_back(stof(current_num)); cout << "Считано число " << current_num << endl;}
// 				catch (const exception& e){cerr << "Невалидное число: " << current_num << endl;}
// 				current_num.clear();
// 			}
// 		}
// 	}
// 	if (bytes_read == 0){cout << "Клиент отключился" << endl;}
// 	else if (bytes_read < 0){
// 		cout << "Ошибка при чтении данных из сокета" << endl;
// 		result.first = false;
// 		result.second = nums;
// 		return result;
// 	}
// 	cout << "Числа считаны из сокета" << endl;
// 	result.first = true;
// 	result.second = nums;
// 	return result;
// }

// bool send_floats_vector(int socket, vector<float> floats){
// 	ostringstream oss;
// 	cout << "преобразовываем вещественный массив в символьный для отправки" << endl;
// 	for (size_t i = 0; i < floats.size(); i++) {oss << floats[i] << " ";}
// 	string data = oss.str();

// 	ssize_t totalSent = 0;
// 	ssize_t dataLen = data.size();
// 	const char* buffer = data.c_str();
// 	for (int i = 0; i < dataLen; i++){cout << (short)buffer[i] << " ";} // Вывод байт, которые мы запишем в сокет (проверка)
// 	cout << endl;

// 	while (totalSent < dataLen) {
// 		ssize_t sent = send(socket, buffer + totalSent, dataLen - totalSent, 0);
// 		if (sent == -1) {
// 			cerr << "Отправка прервана" << endl;
// 			return 0;
// 		}
// 		totalSent += sent;
// 	}
// 	cout << "Данные отправлены в сокет" << endl;
// 	return 1;
// }


// int main(){
// 	int listening_socket = create_listening_socket();
// 	if (listening_socket < 0){return 1;}
// 	cout << "<------------->" << endl;
// 	while (1){
// 		int client_socket = accept_next_connection(listening_socket);
// 		if (client_socket < 0){return 1;}
// 		auto read_from_socket_res = read_nums_from_socket(client_socket);
// 		if (!read_from_socket_res.first) close(client_socket);
// 		send_floats_vector(client_socket, read_from_socket_res.second);
// 		close(client_socket);
// 		cout << "Сокет закрыт" << endl;
// 		cout << "<------------->" << endl;
// 	}
// }
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

// 1. Алгоритм последовательного поиска
vector<int> sequentialSearch(const vector<int>& arr, int key) {
    vector<int> indices;
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i] == key) {
            indices.push_back(i);
        }
    }
    return indices;
}

// 2. Алгоритм быстрого последовательного поиска (с барьером)
vector<int> fastSequentialSearch(vector<int> arr, int key) {
    vector<int> indices;
    // Добавляем барьер в конец массива
    arr.push_back(key);
    
    int i = 0;
    while (arr[i] != key) {
        i++;
    }
    
    // Удаляем барьер
    arr.pop_back();
    
    // Если нашли не барьер, продолжаем поиск остальных вхождений
    if (i < arr.size()) {
        indices.push_back(i);
        // Ищем остальные вхождения
        for (int j = i + 1; j < arr.size(); j++) {
            if (arr[j] == key) {
                indices.push_back(j);
            }
        }
    }
    
    return indices;
}

// 3. Функция бинарного поиска первого вхождения
int binarySearchFirst(const vector<int>& arr, int key) {
    int left = 0;
    int right = arr.size() - 1;
    int result = -1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == key) {
            result = mid;
            right = mid - 1; // Продолжаем искать в левой части
        }
        else if (arr[mid] < key) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }
    
    return result;
}

// Алгоритм бинарного поиска (все вхождения)
vector<int> binarySearchAll(const vector<int>& arr, int key) {
    vector<int> indices;
    int first = binarySearchFirst(arr, key);
    
    if (first == -1) {
        return indices; // Ключ не найден
    }
    
    // Находим все вхождения
    // Влево
    int i = first;
    while (i >= 0 && arr[i] == key) {
        indices.push_back(i);
        i--;
    }
    
    // Вправо
    i = first + 1;
    while (i < arr.size() && arr[i] == key) {
        indices.push_back(i);
        i++;
    }
    
    // Сортируем индексы по возрастанию
    sort(indices.begin(), indices.end());
    return indices;
}

// 4. Алгоритм последовательного поиска в упорядоченном массиве
vector<int> sequentialSearchSorted(const vector<int>& arr, int key) {
    vector<int> indices;
    
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i] == key) {
            indices.push_back(i);
        }
        else if (arr[i] > key) {
            // Поскольку массив отсортирован, дальше искать нет смысла
            break;
        }
    }
    
    return indices;
}

// Функция для парсинга строки в вектор чисел
vector<int> parseStringToVector(const string& str) {
    vector<int> result;
    stringstream ss(str);
    int num;
    
    while (ss >> num) {
        result.push_back(num);
    }
    
    return result;
}

// Функция для форматирования вектора индексов в строку
string formatIndices(const vector<int>& indices) {
    if (indices.empty()) {
        return "Элемент не найден";
    }
    
    stringstream ss;
    ss << "Найдено вхождений: " << indices.size() << "\n";
    ss << "Индексы: ";
    
    for (size_t i = 0; i < indices.size(); i++) {
        ss << indices[i];
        if (i != indices.size() - 1) {
            ss << ", ";
        }
    }
    
    return ss.str();
}

// Функция обработки запроса клиента
string processRequest(const string& request) {
    stringstream ss(request);
    int algorithmChoice, searchKey;
    string dataStr;
    
    // Парсим запрос: выбор алгоритма, ключ, данные
    ss >> algorithmChoice >> searchKey;
    getline(ss, dataStr); // Читаем остаток строки (данные)
    
    // Убираем начальный пробел если есть
    if (!dataStr.empty() && dataStr[0] == ' ') {
        dataStr = dataStr.substr(1);
    }
    
    vector<int> data = parseStringToVector(dataStr);
    
    vector<int> result;
    string algorithmName;
    
    switch (algorithmChoice) {
        case 1: // Последовательный поиск
            result = sequentialSearch(data, searchKey);
            algorithmName = "Последовательный поиск";
            break;
            
        case 2: // Быстрый последовательный поиск
            result = fastSequentialSearch(data, searchKey);
            algorithmName = "Быстрый последовательный поиск";
            break;
            
        case 3: // Бинарный поиск
            {
                vector<int> sortedData = data;
                sort(sortedData.begin(), sortedData.end());
                result = binarySearchAll(sortedData, searchKey);
                algorithmName = "Бинарный поиск (массив отсортирован автоматически)";
            }
            break;
            
        case 4: // Последовательный поиск в упорядоченном массиве
            {
                vector<int> sortedData = data;
                sort(sortedData.begin(), sortedData.end());
                result = sequentialSearchSorted(sortedData, searchKey);
                algorithmName = "Последовательный поиск в упорядоченном массиве";
            }
            break;
            
        default:
            return "Ошибка: неверный выбор алгоритма";
    }
    
    stringstream response;
    response << "Алгоритм: " << algorithmName << "\n";
    response << "Ключ поиска: " << searchKey << "\n";
    response << "Размер массива: " << data.size() << "\n";
    response << formatIndices(result);
    
    return response.str();
}

int main() {
    // Константы сервера
    const char SERVER_IP[] = "127.0.0.1";
    const short SERVER_PORT_NUM = 7129;
    const short BUFF_SIZE = 4096;
    
    int erStat;  // Объявляем переменную один раз в начале main()
    
    // Создание сокета
    int ServerSock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (ServerSock == -1) {
        cout << "Ошибка инициализации сокета" << endl;
        return 1;
    }
    cout << "Инициализация сокета сервера прошла успешно!" << endl;
    
    // Настройка адреса сервера
    sockaddr_in servInfo;
    memset(&servInfo, 0, sizeof(servInfo));
    
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr.s_addr = inet_addr(SERVER_IP);
    servInfo.sin_port = htons(SERVER_PORT_NUM);
    
    // Привязка сокета к адресу
    erStat = ::bind(ServerSock, (sockaddr*)&servInfo, sizeof(servInfo));
    
    if (erStat != 0) {
        cout << "Ошибка привязки сокета к адресу" << endl;
        close(ServerSock);
        return 1;
    }
    cout << "Привязка сокета к адресу прошла успешно!" << endl;
    
    // Прослушивание порта
    erStat = listen(ServerSock, SOMAXCONN); // УБРАНО int перед erStat
    
    if (erStat != 0) {
        cout << "Не удалось начать прослушивание порта" << endl;
        close(ServerSock);
        return 1;
    }
    cout << "Ожидание подключений на порту " << SERVER_PORT_NUM << "..." << endl;
    
    // Принятие подключений в бесконечном цикле
    while (true) {
        sockaddr_in clientInfo;
        socklen_t clientInfoSize = sizeof(clientInfo);
        
        // Принятие входящего подключения
        int ClientConn = accept(ServerSock, (sockaddr*)&clientInfo, &clientInfoSize);
        
        if (ClientConn == -1) {
            cout << "Клиент найден, но подключиться не удалось" << endl;
            continue;
        }
        
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "\nКлиент подключился с IP: " << clientIP << endl;
        
        // Буфер для получения данных
        vector<char> clientBuff(BUFF_SIZE);
        ssize_t packet_size = recv(ClientConn, clientBuff.data(), clientBuff.size(), 0);
        
        if (packet_size <= 0) {
            cout << "Не удалось получить данные от клиента" << endl;
            close(ClientConn);
            continue;
        }
        
        // Преобразование полученных данных в строку
        clientBuff[packet_size] = '\0';
        string request(clientBuff.data());
        
        cout << "Получен запрос от клиента" << endl;
        cout << "Размер данных: " << packet_size << " байт" << endl;
        
        // Обработка запроса
        string response = processRequest(request);
        
        // Отправка ответа клиенту
        send(ClientConn, response.c_str(), response.size(), 0);
        
        cout << "Ответ отправлен клиенту" << endl;
        
        // Закрытие соединения с текущим клиентом
        close(ClientConn);
        cout << "Соединение с клиентом закрыто" << endl;
    }
    
    close(ServerSock);
    
    return 0;
}