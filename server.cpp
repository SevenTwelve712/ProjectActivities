#include<iostream>
#include<cmath>
#include<cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include<sstream>
using namespace std;

int create_listening_socket(){
	// Создает слушающий сокет на порту 7129, биндит его и начинает прослушивание
	const int port = 7129;
	struct sockaddr_in server_addr;

	// Создаем прослушивающий входящие соединения сокет
	int listening_socket = socket(AF_INET, SOCK_STREAM, 0); // Тип адреса, тип пакетов, протокол

	if (listening_socket < 0){
		cerr << "Ошибка инициализации слушающего сокета" << endl;
		return -1;
	}
	cout << "Слушающий сокет инициализирован успешно" << endl;

	// Указываем адрес сервера
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Привязываем сокет к порту
	if ((bind(listening_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0){ // сокет, ссылка на адрес, размер структуры адреса
		cerr << "Ошибка при привязывании сокета к адресу" << endl;
		return -1;
	}
	cout << "Сокет успешно привязан к адресу и порту" << endl;

	// Начинаем прослушивать
	if ((listen(listening_socket, 128)) < 0){
		cerr << "Ошибка при попытке начать прослушивание сокета" << endl;
		return -1;
	}

	cout << "Сервер слушает на порту " << port << endl;
	return listening_socket;
}


int accept_next_connection(int listening_socket){
	// Создает сокет для общения с пользователем
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	char ip_str[INET_ADDRSTRLEN];

	int client_socket = accept(listening_socket, (sockaddr *)&client_addr, &client_addr_len);

	// Проверяем, получилось ли создать сокет для общения с конкретным пользователем
	if (client_socket < 0){
		cerr << "Ошибка при создании сокета для общения с клиентом" << endl;
		return -1;
	}
	cout << "Сокет для общения с клиентом создан" << endl;
	cout << "Принято соединение от " << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, INET_ADDRSTRLEN) << endl;
	return client_socket;
}

pair<bool, vector<float>> read_nums_from_socket(int socket){
	// Принимает сокет, в котором гарантированно записаны числа через пробел, возвращает вектор с этими символами
	pair<bool, vector<float>> result;
	string current_num = "";
	vector<float> nums;
	char chars_arr[50];
	ssize_t bytes_read;
	char c;
	string valid = "0123456789.+-eE";

	cout << "Считываем данные из сокета" << endl;
	while ((bytes_read = read(socket, &chars_arr, sizeof(chars_arr))) > 0){
		for (int i = 0; i <= bytes_read; i++){
			c = i == bytes_read ? ' ' : chars_arr[i];
			if (valid.find(c) != string::npos){current_num += c;} // если символ может находится в вещественном числе
			else if (!current_num.empty()){ // в ином случае если текущее значение числа не пустое пытаемся преобразовать его в float, затем обнуляем текуще значение
				try{nums.push_back(stof(current_num)); cout << "Считано число " << current_num << endl;}
				catch (const exception& e){cerr << "Невалидное число: " << current_num << endl;}
				current_num.clear();
			}
		}
	}
	if (bytes_read == 0){cout << "Клиент отключился" << endl;}
	else if (bytes_read < 0){
		cout << "Ошибка при чтении данных из сокета" << endl;
		result.first = false;
		result.second = nums;
		return result;
	}
	cout << "Числа считаны из сокета" << endl;
	result.first = true;
	result.second = nums;
	return result;
}

bool send_floats_vector(int socket, vector<float> floats){
	ostringstream oss;
	cout << "преобразовываем вещественный массив в символьный для отправки" << endl;
	for (size_t i = 0; i < floats.size(); i++) {oss << floats[i] << " ";}
	string data = oss.str();

	ssize_t totalSent = 0;
	ssize_t dataLen = data.size();
	const char* buffer = data.c_str();
	for (int i = 0; i < dataLen; i++){cout << (short)buffer[i] << " ";} // Вывод байт, которые мы запишем в сокет (проверка)
	cout << endl;

	while (totalSent < dataLen) {
		ssize_t sent = send(socket, buffer + totalSent, dataLen - totalSent, 0);
		if (sent == -1) {
			cerr << "Отправка прервана" << endl;
			return 0;
		}
		totalSent += sent;
	}
	cout << "Данные отправлены в сокет" << endl;
	return 1;
}


int main(){
	int listening_socket = create_listening_socket();
	if (listening_socket < 0){return 1;}
	cout << "<------------->" << endl;
	while (1){
		int client_socket = accept_next_connection(listening_socket);
		if (client_socket < 0){return 1;}
		auto read_from_socket_res = read_nums_from_socket(client_socket);
		if (!read_from_socket_res.first) close(client_socket);
		send_floats_vector(client_socket, read_from_socket_res.second);
		close(client_socket);
		cout << "Сокет закрыт" << endl;
		cout << "<------------->" << endl;
	}
}
