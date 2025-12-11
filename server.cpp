#include <algorithm>
#include <arpa/inet.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
using namespace std;

int create_listening_socket()
{
    // Создает слушающий сокет на порту 7129, биндит его и начинает прослушивание
    const int port = 7129;
    struct sockaddr_in server_addr;

    // Создаем прослушивающий входящие соединения сокет
    int listening_socket = socket(AF_INET, SOCK_STREAM, 0); // Тип адреса, тип пакетов, протокол

    if (listening_socket < 0) {
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
    int bind_result = ::bind(listening_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) { // сокет, ссылка на адрес, размер структуры адреса
        cerr << "Ошибка при привязывании сокета к адресу" << endl;
        return -1;
    }
    cout << "Сокет успешно привязан к адресу и порту" << endl;

    // Начинаем прослушивать
    int listen_result = listen(listening_socket, 128);
    if (listen_result < 0) {
        cerr << "Ошибка при попытке начать прослушивание сокета" << endl;
        return -1;
    }

    cout << "Сервер слушает на порту " << port << endl;
    return listening_socket;
}

int accept_next_connection(int listening_socket)
{
    // Создает сокет для общения с пользователем
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char ip_str[INET_ADDRSTRLEN];

    int client_socket = accept(listening_socket, (sockaddr*)&client_addr, &client_addr_len);

    // Проверяем, получилось ли создать сокет для общения с конкретным пользователем
    if (client_socket < 0) {
        cerr << "Ошибка при создании сокета для общения с клиентом" << endl;
        return -1;
    }
    cout << "Сокет для общения с клиентом создан" << endl;
    cout << "Принято соединение от " << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, INET_ADDRSTRLEN) << endl;
    return client_socket;
}

pair<bool, vector<float>> read_nums_from_socket(int socket)
{
    // Принимает сокет, в котором гарантированно записаны числа через пробел, возвращает вектор с этими символами
    pair<bool, vector<float>> result;
    string current_num = "";
    vector<float> nums;
    char chars_arr[50];
    ssize_t bytes_read;
    char c;
    string valid = "0123456789.+-eE";

    cout << "Считываем данные из сокета" << endl;
    while ((bytes_read = read(socket, &chars_arr, sizeof(chars_arr))) > 0) {
        for (int i = 0; i <= bytes_read; i++) {
            c = i == bytes_read ? ' ' : chars_arr[i];
            if (valid.find(c) != string::npos) {
                current_num += c;
            } // если символ может находится в вещественном числе
            else if (!current_num.empty()) { // в ином случае если текущее значение числа не пустое пытаемся преобразовать его в float, затем обнуляем текуще значение
                try {
                    nums.push_back(stof(current_num));
                    cout << "Считано число " << current_num << endl;
                } catch (const exception& e) {
                    cerr << "Невалидное число: " << current_num << endl;
                }
                current_num.clear();
            }
        }
    }
    if (bytes_read == 0) {
        cout << "Клиент отключился" << endl;
    } else if (bytes_read < 0) {
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

bool send_floats_vector(int socket, vector<float> floats)
{
    ostringstream oss;
    cout << "преобразовываем вещественный массив в символьный для отправки" << endl;
    for (size_t i = 0; i < floats.size(); i++) {
        oss << floats[i] << " ";
    }
    string data = oss.str();

    ssize_t totalSent = 0;
    ssize_t dataLen = data.size();
    const char* buffer = data.c_str();
    for (int i = 0; i < dataLen; i++) {
        cout << (short)buffer[i] << " ";
    } // Вывод байт, которые мы запишем в сокет (проверка)
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

int main()
{
    int listening_socket = create_listening_socket();
    if (listening_socket < 0) {
        return 1;
    }
    cout << "<------------->" << endl;
    while (1) {
        // Принимаем следующего пользователя
        int client_socket = accept_next_connection(listening_socket);

        // Если что то пошло не так при создании клиентского сокета брейкаем выполнение программы
        if (client_socket < 0) {
            return 1;
        }

        // Считываем числа из сокета
        auto read_from_socket_res = read_nums_from_socket(client_socket);

        // Если при считывании произошла ошибка флаг read_from_socket_res.first будет false
        // В таком случае мы закрываем сокет и брейкаем программу
        if (!read_from_socket_res.first) {
            close(client_socket);
            cout << "Сокет закрыт" << endl;
            return 1;
        }

        // Разбиваем полученные числа на номер алгоритма и числа, которые надо преобразовать (вектор nums)
        float algorithm = read_from_socket_res.second[0];
        vector<float> nums(read_from_socket_res.second.begin() + 1, read_from_socket_res.second.end());

        // Выбираем алгоритм
        if (algorithm == 1) {
            // Вызываем функцию поиска
        } else if (algorithm == 2) {
            // Вызываем функцию сортировки (1 вариант)
        }

        else {
            // Вызываем функцию сортировки (6 вариант)
        }

        // Не забудьте вызвать send_floats_vector для отправки чисел в сокет

        // Закрываем сокет, ждем следующего соединения
        close(client_socket);
        cout << "Сокет закрыт" << endl;
        cout << "<------------->" << endl;
    }
}

// <-------------->
// Реализуйте алгоритмы здесь
vector<int> sequentialSearch(const vector<int>& arr, int key)
{
    vector<int> indices;
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i] == key) {
            indices.push_back(i);
        }
    }
    return indices;
}

// 2. Алгоритм быстрого последовательного поиска (с барьером)
vector<int> fastSequentialSearch(vector<int> arr, int key)
{
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
int binarySearchFirst(const vector<int>& arr, int key)
{
    int left = 0;
    int right = arr.size() - 1;
    int result = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] == key) {
            result = mid;
            right = mid - 1; // Продолжаем искать в левой части
        } else if (arr[mid] < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return result;
}

// Алгоритм бинарного поиска (все вхождения)
vector<int> binarySearchAll(const vector<int>& arr, int key)
{
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
vector<int> sequentialSearchSorted(const vector<int>& arr, int key)
{
    vector<int> indices;

    for (int i = 0; i < arr.size(); i++) {
        if (arr[i] == key) {
            indices.push_back(i);
        } else if (arr[i] > key) {
            // Поскольку массив отсортирован, дальше искать нет смысла
            break;
        }
    }

    return indices;
}
