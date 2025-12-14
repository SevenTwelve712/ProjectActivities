// #include <algorithm>
// #include <arpa/inet.h>
// #include <cmath>
// #include <cstring>
// #include <iostream>
// #include <netinet/in.h>
// #include <sstream>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <unistd.h>
// #include <vector>
// #include <algorithm>
// using namespace std;

// int create_listening_socket()
// {
//     // Создает слушающий сокет на порту 7129, биндит его и начинает прослушивание
//     const int port = 8081;
//     struct sockaddr_in server_addr;

//     // Создаем прослушивающий входящие соединения сокет
//     int listening_socket = socket(AF_INET, SOCK_STREAM, 0); // Тип адреса, тип пакетов, протокол

//     if (listening_socket < 0) {
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
//     int bind_result = ::bind(listening_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
//     if (bind_result < 0) { // сокет, ссылка на адрес, размер структуры адреса
//         cerr << "Ошибка при привязывании сокета к адресу" << endl;
//         return -1;
//     }
//     cout << "Сокет успешно привязан к адресу и порту" << endl;

//     // Начинаем прослушивать
//     int listen_result = listen(listening_socket, 128);
//     if (listen_result < 0) {
//         cerr << "Ошибка при попытке начать прослушивание сокета" << endl;
//         return -1;
//     }

//     cout << "Сервер слушает на порту " << port << endl;
//     return listening_socket;
// }

// int accept_next_connection(int listening_socket)
// {
//     // Создает сокет для общения с пользователем
//     struct sockaddr_in client_addr;
//     socklen_t client_addr_len = sizeof(client_addr);
//     char ip_str[INET_ADDRSTRLEN];

//     int client_socket = accept(listening_socket, (sockaddr*)&client_addr, &client_addr_len);

//     // Проверяем, получилось ли создать сокет для общения с конкретным пользователем
//     if (client_socket < 0) {
//         cerr << "Ошибка при создании сокета для общения с клиентом" << endl;
//         return -1;
//     }
//     cout << "Сокет для общения с клиентом создан" << endl;
//     cout << "Принято соединение от " << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, INET_ADDRSTRLEN) << endl;
//     return client_socket;
// }

// pair<bool, vector<float>> read_nums_from_socket(int socket)
// {
//     // Принимает сокет, в котором гарантированно записаны числа через пробел, возвращает вектор с этими символами
//     pair<bool, vector<float>> result;
//     string current_num = "";
//     vector<float> nums;
//     char chars_arr[50];
//     ssize_t bytes_read;
//     char c;
//     string valid = "0123456789.+-eE";

//     cout << "Считываем данные из сокета" << endl;
//     while ((bytes_read = read(socket, &chars_arr, sizeof(chars_arr))) > 0) {
//         for (int i = 0; i <= bytes_read; i++) {
//             c = i == bytes_read ? ' ' : chars_arr[i];
//             if (valid.find(c) != string::npos) {
//                 current_num += c;
//             } // если символ может находится в вещественном числе
//             else if (!current_num.empty()) { // в ином случае если текущее значение числа не пустое пытаемся преобразовать его в float, затем обнуляем текуще значение
//                 try {
//                     nums.push_back(stof(current_num));
//                     cout << "Считано число " << current_num << endl;
//                 } catch (const exception& e) {
//                     cerr << "Невалидное число: " << current_num << endl;
//                 }
//                 current_num.clear();
//             }
//         }
//     }
//     if (bytes_read == 0) {
//         cout << "Клиент отключился" << endl;
//     } else if (bytes_read < 0) {
//         cout << "Ошибка при чтении данных из сокета" << endl;
//         result.first = false;
//         result.second = nums;
//         return result;
//     }
//     cout << "Числа считаны из сокета" << endl;
//     result.first = true;
//     result.second = nums;
//     return result;
// }

// bool send_floats_vector(int socket, vector<float> floats)
// {
//     ostringstream oss;
//     cout << "преобразовываем вещественный массив в символьный для отправки" << endl;
//     for (size_t i = 0; i < floats.size(); i++) {
//         oss << floats[i] << " ";
//     }
//     string data = oss.str();

//     ssize_t totalSent = 0;
//     ssize_t dataLen = data.size();
//     const char* buffer = data.c_str();
//     for (int i = 0; i < dataLen; i++) {
//         cout << (short)buffer[i] << " ";
//     } // Вывод байт, которые мы запишем в сокет (проверка)
//     cout << endl;

//     while (totalSent < dataLen) {
//         ssize_t sent = send(socket, buffer + totalSent, dataLen - totalSent, 0);
//         if (sent == -1) {
//             cerr << "Отправка прервана" << endl;
//             return 0;
//         }
//         totalSent += sent;
//     }
//     cout << "Данные отправлены в сокет" << endl;
//     return 1;
// }

// int main()
// {
//     int listening_socket = create_listening_socket();
//     if (listening_socket < 0) {
//         return 1;
//     }
//     cout << "<------------->" << endl;
//     while (1) {
//         // Принимаем следующего пользователя
//         int client_socket = accept_next_connection(listening_socket);

//         // Если что то пошло не так при создании клиентского сокета брейкаем выполнение программы
//         if (client_socket < 0) {
//             return 1;
//         }

//         // Считываем числа из сокета
//         auto read_from_socket_res = read_nums_from_socket(client_socket);

//         // Если при считывании произошла ошибка флаг read_from_socket_res.first будет false
//         // В таком случае мы закрываем сокет и брейкаем программу
//         if (!read_from_socket_res.first) {
//             close(client_socket);
//             cout << "Сокет закрыт" << endl;
//             return 1;
//         }

//         // Разбиваем полученные числа на номер алгоритма и числа, которые надо преобразовать (вектор nums)
//         float algorithm = read_from_socket_res.second[0];
//         vector<float> nums(read_from_socket_res.second.begin() + 1, read_from_socket_res.second.end());

//         // Выбираем алгоритм
//         if (algorithm == 1) {
//             // Вызываем функцию поиска

//         } else if (algorithm == 2) {
//             // Вызываем функцию сортировки (1 вариант)
//         }

//         else {
//             // Вызываем функцию сортировки (6 вариант)
//         }

//         // Не забудьте вызвать send_floats_vector для отправки чисел в сокет

//         // Закрываем сокет, ждем следующего соединения
//         close(client_socket);
//         cout << "Сокет закрыт" << endl;
//         cout << "<------------->" << endl;
//     }
// }

// // <-------------->
// // Реализуйте алгоритмы здесь
// // 1. Алгоритм последовательного поиска
// vector<int> sequentialSearch(const vector<int>& arr, int key)
// {
//     vector<int> indices;
//     for (int i = 0; i < arr.size(); i++) {
//         if (arr[i] == key) {
//             indices.push_back(i);
//         }
//     }
//     return indices;
// }

// // 2. Алгоритм быстрого последовательного поиска (с барьером)
// vector<int> fastSequentialSearch(vector<int> arr, int key)
// {
//     vector<int> indices;
//     // Добавляем барьер в конец массива
//     arr.push_back(key);

//     int i = 0;
//     while (arr[i] != key) {
//         i++;
//     }

//     // Удаляем барьер
//     arr.pop_back();

//     // Если нашли не барьер, продолжаем поиск остальных вхождений
//     if (i < arr.size()) {
//         indices.push_back(i);
//         // Ищем остальные вхождения
//         for (int j = i + 1; j < arr.size(); j++) {
//             if (arr[j] == key) {
//                 indices.push_back(j);
//             }
//         }
//     }

//     return indices;
// }
// // // 3.1 Функция бинарного поиска первого вхождения (без изменения исходного массива)
// // int binarySearchFirstConst(const vector<int>& arr, int key) {
// //     // Создаем копию и сортируем ее
// //     vector<int> sortedArr = arr;
// //     sort(sortedArr.begin(), sortedArr.end());
    
// //     int left = 0;
// //     int right = sortedArr.size() - 1;
// //     int result = -1;
    
// //     while (left <= right) {
// //         int mid = left + (right - left) / 2;
        
// //         if (sortedArr[mid] == key) {
// //             result = mid;
// //             right = mid - 1;
// //         }
// //         else if (sortedArr[mid] > key) {
// //             right = mid - 1;
// //         }
// //         else {
// //             left = mid + 1;
// //         }
// //     }
    
// //     return result;
// // }
// // 3.2 Алгоритм бинарного поиска всех вхождений (без изменения исходного массива)
// vector<int> binarySearchAllConst(const vector<int>& arr, int key)
// {
//     // Создаем копию и сортируем ее
//     vector<int> sortedArr = arr;
//     sort(sortedArr.begin(), sortedArr.end());
    
//     vector<int> indices;
//     int first = -1;
    
//     // Поиск первого вхождения
//     int left = 0;
//     int right = sortedArr.size() - 1;
//     while (left <= right) {
//         int mid = left + (right - left) / 2;
        
//         if (sortedArr[mid] == key) {
//             first = mid;
//             right = mid - 1;
//         }
//         else if (sortedArr[mid] > key) {
//             right = mid - 1;
//         }
//         else {
//             left = mid + 1;
//         }
//     }

//     if (first == -1) {
//         return indices; // Ключ не найден
//     }

//     // Находим все вхождения
//     // Влево
//     int i = first;
//     while (i >= 0 && sortedArr[i] == key) {
//         indices.push_back(i);
//         i--;
//     }

//     // Вправо
//     i = first + 1;
//     while (i < sortedArr.size() && sortedArr[i] == key) {
//         indices.push_back(i);
//         i++;
//     }

//     sort(indices.begin(), indices.end());
//     return indices;
// }

// // 4. Алгоритм последовательного поиска в упорядоченном массиве
// // Алгоритм последовательного поиска в упорядоченном массиве
// // Функция автоматически сортирует массив перед поиском
// vector<int> sequentialSearchSorted(vector<int>& arr, int key)
// {
//     vector<int> indices;
    
//     // 1. Сортировка массива по возрастанию
//     sort(arr.begin(), arr.end());
    
//     // 2. Последовательный поиск с досрочным прекращением
//     for (int i = 0; i < arr.size(); i++) {
//         if (arr[i] == key) {
//             indices.push_back(i);
//         } else if (arr[i] > key) {
//             // Поскольку массив отсортирован, дальше искать нет смысла
//             break;
//         }
//     }
    
//     return indices;
// }
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
#include <chrono>
using namespace std;
using namespace std::chrono;

int create_listening_socket()
{
    const int port = 8081;
    struct sockaddr_in server_addr;

    int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_socket < 0) {
        cerr << "Ошибка инициализации слушающего сокета" << endl;
        return -1;
    }
    cout << "Слушающий сокет инициализирован успешно" << endl;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int bind_result = ::bind(listening_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) {
        cerr << "Ошибка при привязывании сокета к адресу" << endl;
        return -1;
    }
    cout << "Сокет успешно привязан к адресу и порту" << endl;

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
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char ip_str[INET_ADDRSTRLEN];

    int client_socket = accept(listening_socket, (sockaddr*)&client_addr, &client_addr_len);

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
            }
            else if (!current_num.empty()) {
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
    cout << "Преобразовываем вещественный массив в символьный для отправки" << endl;
    for (size_t i = 0; i < floats.size(); i++) {
        oss << floats[i] << " ";
    }
    string data = oss.str();

    ssize_t totalSent = 0;
    ssize_t dataLen = data.size();
    const char* buffer = data.c_str();
    
    cout << "Отправляемые байты (" << dataLen << " байт): ";
    for (size_t i = 0; i < min((size_t)20, data.size()); i++) {
        cout << (int)(unsigned char)buffer[i] << " ";
    }
    if (dataLen > 20) cout << "...";
    cout << endl;

    while (totalSent < dataLen) {
        ssize_t sent = send(socket, buffer + totalSent, dataLen - totalSent, 0);
        if (sent == -1) {
            cerr << "Отправка прервана" << endl;
            return false;
        }
        totalSent += sent;
    }
    cout << "Данные отправлены в сокет" << endl;
    return true;
}

// Структура для хранения результатов поиска
struct SearchResult {
    vector<int> indices;
    double time_ms;
    int steps;
};

// 1. Алгоритм последовательного поиска
SearchResult sequentialSearch(const vector<int>& arr, int key)
{
    auto start = high_resolution_clock::now();
    vector<int> indices;
    int steps = 0;
    
    for (int i = 0; i < arr.size(); i++) {
        steps++; // Считаем сравнение
        if (arr[i] == key) {
            indices.push_back(i);
        }
    }
    
    auto end = high_resolution_clock::now();
    double duration = duration_cast<microseconds>(end - start).count() / 1000.0;
    
    return {indices, duration, steps};
}

// 2. Алгоритм быстрого последовательного поиска (с барьером)
SearchResult fastSequentialSearch(vector<int> arr, int key)
{
    auto start = high_resolution_clock::now();
    vector<int> indices;
    int steps = 0;
    
    arr.push_back(key);

    int i = 0;
    steps++; // Первое сравнение
    while (arr[i] != key) {
        i++;
        steps++; // Сравнение на каждой итерации
    }

    arr.pop_back();

    if (i < arr.size()) {
        indices.push_back(i);
        for (int j = i + 1; j < arr.size(); j++) {
            steps++; // Сравнение для каждого элемента
            if (arr[j] == key) {
                indices.push_back(j);
            }
        }
    }
    
    auto end = high_resolution_clock::now();
    double duration = duration_cast<microseconds>(end - start).count() / 1000.0;
    
    return {indices, duration, steps};
}

// 3. Алгоритм бинарного поиска всех вхождений
SearchResult binarySearchAll(const vector<int>& arr, int key)
{
    auto start = high_resolution_clock::now();
    vector<int> sortedArr = arr;
    sort(sortedArr.begin(), sortedArr.end());
    
    vector<int> indices;
    int steps = 0;
    int first = -1;
    
    int left = 0;
    int right = sortedArr.size() - 1;
    
    // Бинарный поиск первого вхождения
    while (left <= right) {
        steps++; // Считаем сравнение в цикле
        int mid = left + (right - left) / 2;
        
        steps++; // Сравнение arr[mid] == key
        if (sortedArr[mid] == key) {
            first = mid;
            right = mid - 1;
        }
        else if (sortedArr[mid] > key) {
            steps++; // Дополнительное сравнение
            right = mid - 1;
        }
        else {
            steps++; // Дополнительное сравнение
            left = mid + 1;
        }
    }

    if (first == -1) {
        auto end = high_resolution_clock::now();
        double duration = duration_cast<microseconds>(end - start).count() / 1000.0;
        return {indices, duration, steps};
    }

    // Поиск всех вхождений влево
    int i = first;
    while (i >= 0 && sortedArr[i] == key) {
        steps++; // Сравнение в цикле
        indices.push_back(i);
        i--;
    }

    // Поиск всех вхождений вправо
    i = first + 1;
    while (i < sortedArr.size() && sortedArr[i] == key) {
        steps++; // Сравнение в цикле
        indices.push_back(i);
        i++;
    }

    sort(indices.begin(), indices.end());
    
    auto end = high_resolution_clock::now();
    double duration = duration_cast<microseconds>(end - start).count() / 1000.0;
    
    return {indices, duration, steps};
}

// 4. Алгоритм последовательного поиска в упорядоченном массиве
SearchResult sequentialSearchSorted(vector<int>& arr, int key)
{
    auto start = high_resolution_clock::now();
    vector<int> sortedArr = arr;
    sort(sortedArr.begin(), sortedArr.end());
    
    vector<int> indices;
    int steps = 0;
    
    for (int i = 0; i < sortedArr.size(); i++) {
        steps++; // Сравнение
        if (sortedArr[i] == key) {
            indices.push_back(i);
        } else if (sortedArr[i] > key) {
            steps++; // Дополнительное сравнение
            break;
        }
    }
    
    auto end = high_resolution_clock::now();
    double duration = duration_cast<microseconds>(end - start).count() / 1000.0;
    
    return {indices, duration, steps};
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
        if (!read_from_socket_res.first) {
            close(client_socket);
            cout << "Сокет закрыт" << endl;
            continue;
        }

        // Разбиваем полученные числа на номер алгоритма и числа
        float algorithm = read_from_socket_res.second[0];
        vector<float> nums(read_from_socket_res.second.begin() + 1, read_from_socket_res.second.end());
        
        // Преобразуем float в int для алгоритмов поиска
        vector<int> intArr;
        for (float num : nums) {
            intArr.push_back(static_cast<int>(num));
        }
        
        // Инициализируем результат
        vector<float> result;
        
        // Выбираем алгоритм
        if (algorithm == 1) {
            cout << "Выбран алгоритм поиска (1)" << endl;
            
            if (nums.size() < 2) {
                cerr << "Ошибка: для поиска нужно как минимум 2 числа (ключ и массив)" << endl;
                vector<float> errorResult = {0.0f};
                send_floats_vector(client_socket, errorResult);
                close(client_socket);
                continue;
            }
            
            // Первое число после алгоритма - это ключ поиска
            int searchKey = static_cast<int>(nums[0]);
            
            // Вектор для поиска (все числа после ключа)
            vector<int> searchArr(intArr.begin() + 1, intArr.end());
            
            cout << "Ключ поиска: " << searchKey << endl;
            cout << "Размер массива для поиска: " << searchArr.size() << endl;
            
            // Выполняем все 4 алгоритма поиска с измерением времени и подсчетом шагов
            
            // 1. Последовательный поиск
            auto result1 = sequentialSearch(searchArr, searchKey);
            cout << "1. Последовательный поиск: найдено " << result1.indices.size() 
                 << " вхождений, время: " << result1.time_ms << " мс, шагов: " << result1.steps << endl;
            
            // 2. Быстрый последовательный поиск (с барьером)
            auto result2 = fastSequentialSearch(searchArr, searchKey);
            cout << "2. Быстрый последовательный поиск: найдено " << result2.indices.size() 
                 << " вхождений, время: " << result2.time_ms << " мс, шагов: " << result2.steps << endl;
            
            // 3. Бинарный поиск всех вхождений
            auto result3 = binarySearchAll(searchArr, searchKey);
            cout << "3. Бинарный поиск: найдено " << result3.indices.size() 
                 << " вхождений, время: " << result3.time_ms << " мс, шагов: " << result3.steps << endl;
            
            // 4. Последовательный поиск в упорядоченном массиве
            auto result4 = sequentialSearchSorted(searchArr, searchKey);
            cout << "4. Поиск в упорядоченном массиве: найдено " << result4.indices.size() 
                 << " вхождений, время: " << result4.time_ms << " мс, шагов: " << result4.steps << endl;
            
            // Формируем результат для отправки клиенту
            // Новый формат: [размер_результата1] [индексы1] [время1] [шаги1]
            //               [размер_результата2] [индексы2] [время2] [шаги2] ...
            
            // Алгоритм 1
            result.push_back(static_cast<float>(result1.indices.size()));
            for (int idx : result1.indices) {
                result.push_back(static_cast<float>(idx + 1)); // +1 для соответствия с примером клиента
            }
            result.push_back(static_cast<float>(result1.time_ms)); // Время выполнения
            result.push_back(static_cast<float>(result1.steps));   // Количество шагов
            
            // Алгоритм 2
            result.push_back(static_cast<float>(result2.indices.size()));
            for (int idx : result2.indices) {
                result.push_back(static_cast<float>(idx + 1));
            }
            result.push_back(static_cast<float>(result2.time_ms)); // Время выполнения
            result.push_back(static_cast<float>(result2.steps));   // Количество шагов
            
            // Алгоритм 3
            result.push_back(static_cast<float>(result3.indices.size()));
            for (int idx : result3.indices) {
                result.push_back(static_cast<float>(idx + 1));
            }
            result.push_back(static_cast<float>(result3.time_ms)); // Время выполнения
            result.push_back(static_cast<float>(result3.steps));   // Количество шагов
            
            // Алгоритм 4
            result.push_back(static_cast<float>(result4.indices.size()));
            for (int idx : result4.indices) {
                result.push_back(static_cast<float>(idx + 1));
            }
            result.push_back(static_cast<float>(result4.time_ms)); // Время выполнения
            result.push_back(static_cast<float>(result4.steps));   // Количество шагов
            
        } else if (algorithm == 2) {
            // Вызываем функцию сортировки (1 вариант)
            cout << "Выбран алгоритм сортировки (2)" << endl;
            // TODO: Реализовать сортировку
            result.push_back(0.0f); // Заглушка
        }
        else if (algorithm == 3) {
            // Вызываем функцию сортировки (6 вариант)
            cout << "Выбран алгоритм сортировки (6)" << endl;
            // TODO: Реализовать сортировку
            result.push_back(0.0f); // Заглушка
        }
        else {
            cout << "Неизвестный алгоритм: " << algorithm << endl;
            result.push_back(0.0f);
        }

        // Отправляем результат клиенту
        send_floats_vector(client_socket, result);

        // Закрываем сокет, ждем следующего соединения
        close(client_socket);
        cout << "Сокет закрыт" << endl;
        cout << "<------------->" << endl;
    }
    
    return 0;
}