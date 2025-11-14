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
	int listening_socket;
	struct sockaddr_in server_addr;
	socklen_t client_addr_len;

	// Создаем прослушивающий входящие соединения сокет
	listening_socket = socket(AF_INET, SOCK_STREAM, 0); // Тип адреса, тип пакетов, протокол

	if (listening_socket < 0){
		cout << "Error while socket init" << endl;
		return -1;
	}

	// Указываем адрес сервера
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(7129);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Привязываем сокет к порту
	if ((bind(listening_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0){ // сокет, ссылка на адрес, размер структуры адреса
		cout << "Error while binding socket" << endl;
		return -1;
	}

	if ((listen(listening_socket, 128)) < 0){
		cout << "Error while tryuing to listen" << endl;
		return -1;
	}

	cout << "Server listening on 7129" << endl;
	return listening_socket;
}


int accept_next_connection(int listening_socket){
	/*
	 * Создает сокет для следующего пользователя
	 * */
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	char ip_str[INET_ADDRSTRLEN];
	int client_socket = accept(listening_socket, (sockaddr *)&client_addr, &client_addr_len);

	// Проверяем, получилось ли создать сокет для общения с конкретным пользователем
	if (client_socket < 0){
		cout << "Error while trying to create client socket" << endl;
		return -1;
	}
	cout << "Socket created" << endl;
	cout << "Accepted from " << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, INET_ADDRSTRLEN) << endl;
	return client_socket;
}

vector<float> read_nums_from_socket(int socket){
	cout << "Reading nums from socket" << endl;
	// Принимает сокет, в котором гарантированно записаны числа через пробел, возвращает вектор с этими символами

	string current_num = "";
	vector<float> nums;
	char chars_arr[1024];
	ssize_t bytes_read;
	char c;

	while ((bytes_read = read(socket, &chars_arr, sizeof(chars_arr))) > 0){
		for (int i = 0; i < bytes_read; i++){
			c = chars_arr[i];
			if (!isspace(c)){current_num += c;}
			else if (!current_num.empty()){
				try{
					nums.push_back(stof(current_num));
				}
				catch (const exception& e){
					cerr << "Invalid number: " << current_num << endl;
				}
				current_num.clear();
			}
		}
	}
	if (!current_num.empty()){nums.push_back(stof(current_num));}
	if (bytes_read == 0){cout << "Клиент отключился" << endl;}
	else if (bytes_read < 0){cout << "Ошибка при чтении данных из сокета" << endl;}
	cout << "Nums read from socket" << endl;
	return nums;
}

int send_nums_vector(int socket, vector<float> floats){
	ostringstream oss;
	for (size_t i = 0; i < floats.size(); i++) {oss << floats[i] << " ";}
	string data = oss.str();

	ssize_t totalSent = 0;
	ssize_t dataLen = data.size();
	const char* buffer = data.c_str();
	for (int i = 0; i < dataLen; i++){
		cout << (short)buffer[i] << " ";
	}
	cout << endl;

	while (totalSent < dataLen) {
		ssize_t sent = send(socket, buffer + totalSent, dataLen - totalSent, 0);
		if (sent == -1) {
			perror("send failed");
			return 1;
		}
		totalSent += sent;
	}
	cout << "Данные отправлены в сокет" << endl;
	return 0;
}


int main(){
	int listening_socket = create_listening_socket();
	while (1){
		int client_socket = accept_next_connection(listening_socket);
		if (client_socket < 0){return 1;}
		vector<float> nums = read_nums_from_socket(client_socket);
		send_nums_vector(client_socket, nums);
		close(client_socket);
		cout << "Сокет закрыт" << endl;
		cout << "<------------->" << endl;
	}
}
