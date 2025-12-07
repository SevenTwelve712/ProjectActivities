// #include <iostream>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <cstring>
// #include <vector>
// #include <fstream>
// #include <string>

// using namespace std;

// int main(void)
// {
//     // Ключевые константы
//     const char SERVER_IP[] = "127.0.0.1";        // IPv4 адрес сервера
//     const short SERVER_PORT_NUM = 7129;                // Порт сервера
//     const short BUFF_SIZE = 4096;                    // Максимальный размер буфера для обмена данных
 
//     // Ключевая переменная erStat
//     int erStat;                                        // Проверка на ошибки и возвращение их номера

//     // Преобразование IP адреса сервера
//     in_addr ip_to_num;        
//     inet_pton(AF_INET, SERVER_IP, &ip_to_num);

//     // Инициализация сокета (в Linux не требуется WSAStartup)
//     int ClientSock = socket(AF_INET, SOCK_STREAM, 0);

//     if (ClientSock == -1) {
//         cout << "Ошибка инициализации сокета" << endl;
//         return 1;
//     }
//     else 
//         cout << "Инициализация сокета клиента прошла успешно!" << endl;

//     // Задание структуры servInfo, в которой будут храниться тип IP адреса, сам адрес и порт сервера
//     sockaddr_in servInfo;

//     memset(&servInfo, 0, sizeof(servInfo));

//     servInfo.sin_family = AF_INET;
//     servInfo.sin_addr = ip_to_num;    
//     servInfo.sin_port = htons(SERVER_PORT_NUM);

//     // Подключение к серверу
//     erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
    
//     if (erStat != 0) {
//         cout << "Попытка подключения к серверу не удалась." << endl;
//         close(ClientSock);
//         return 1;
//     }
//     else { 
//         cout << "Соединение с сервером установлено успешно!" << endl;
//     }
    
//     vector <char> clientBuff;    
//     vector <char> servBuff(BUFF_SIZE);        // Буферы для отправки и получения данных
//     ssize_t packet_size = 0;                                        // Размер отправленных/полученных данных в байтах
    
//     ifstream file("numbers.txt");
//     string bufferstring;
    
//     if (!file.is_open()) {
//         cout << "Ошибка открытия файла!" << endl;
//         close(ClientSock);
//         return 1;
//     }

//     // Изменение: резервируем память заранее
//     clientBuff.reserve(BUFF_SIZE);
    
//     while (getline(file, bufferstring)) {
//         // Проходим по текущей строке
//         for (char currentchar : bufferstring) {
//             // Добавляем символ в общий вектор
//             clientBuff.push_back(currentchar);
//         }
//     }

//     // Добавляем нулевой терминатор для корректной отправки
//     clientBuff.push_back('\0');

//     cout << "\nASCII коды всех символов:\n";
//     // Вывод содержимого в формате ASCII
//     for (char c : clientBuff) {
//         cout << static_cast<int>(c) << " ";
//     }
//     cout << "\n";
    
//     // Отправка данных на сервер
//     send(ClientSock, clientBuff.data(), clientBuff.size(), 0);
    
//     // В Linux shutdown работает так же
//     shutdown(ClientSock, SHUT_WR); // Закрытие сокета для записи со стороны клиента
    
//     // Получение данных с сервера
//     packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
    
//     if (packet_size < 0) { 
//         cout << "Не удалось получить данные с сервера." << endl;
//         close(ClientSock);
//         return 1;
//     }
//     else {
//         // Добавляем нулевой терминатор для корректного вывода
//         servBuff[packet_size] = '\0';
//         cout << "Получено от сервера: " << servBuff.data() << endl;
//     }
    
//     close(ClientSock);
    
//     cout << "Для выхода нажмите Enter...";
//     cin.get();
    
//     return 0;
// }
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

// Функция для чтения данных из файла numbers.txt с проверкой на пустой файл
bool readNumbersFromFile(string& data, int& count) {
    ifstream file("numbers.txt");
    
    // Проверка 1: Существует ли файл?
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл numbers.txt!" << endl;
        cerr << "Убедитесь, что файл numbers.txt находится в той же папке, что и программа." << endl;
        return false;
    }
    
    // Проверка 2: Пустой ли файл?
    if (file.peek() == ifstream::traits_type::eof()) {
        cerr << "Ошибка: файл numbers.txt пуст!" << endl;
        cerr << "Добавьте целые числа в файл через пробел." << endl;
        file.close();
        return false;
    }
    
    // Читаем все числа из файла в строку
    stringstream buffer;
    int number;
    count = 0;
    bool first = true;
    bool hasValidData = false;
    
    while (file >> number) {
        hasValidData = true;
        if (!first) {
            buffer << " ";
        }
        buffer << number;
        first = false;
        count++;
    }
    
    file.close();
    
    // Проверка 3: Содержит ли файл хотя бы одно число?
    if (!hasValidData) {
        cerr << "Ошибка: файл numbers.txt не содержит чисел!" << endl;
        cerr << "Убедитесь, что файл содержит целые числа, разделенные пробелами." << endl;
        return false;
    }
    
    data = buffer.str();
    return true;
}

// Функция для форматирования данных для отправки
string formatDataForSending(int algorithmChoice, int searchKey, const string& data) {
    stringstream ss;
    ss << algorithmChoice << " " << searchKey << " " << data;
    return ss.str();
}

// Функция для вывода меню выбора алгоритма
void printMenu() {
    cout << "\n=== ВЫБОР АЛГОРИТМА ПОИСКА ===" << endl;
    cout << "1. Алгоритм последовательного поиска" << endl;
    cout << "2. Алгоритм быстрого последовательного поиска" << endl;
    cout << "3. Алгоритм бинарного поиска" << endl;
    cout << "4. Алгоритм последовательного поиска в упорядоченном массиве" << endl;
    cout << "0. Выход" << endl;
    cout << "===============================" << endl;
    cout << "Выберите алгоритм (0-4): ";
}

int main() {
    // Ключевые константы
    const char SERVER_IP[] = "127.0.0.1";
    const short SERVER_PORT_NUM = 7129;
    const short BUFF_SIZE = 4096;
    
    cout << "=== КЛИЕНТ ПОИСКА В МАССИВЕ ===" << endl;
    cout << "Данные загружаются из файла numbers.txt" << endl;
    
    // Загрузка данных из файла с проверкой
    string data;
    int count = 0;
    
    if (!readNumbersFromFile(data, count)) {
        // Если файл пуст или не найден, завершаем программу
        cout << "\nДля выхода нажмите Enter...";
        cin.ignore();
        cin.get();
        return 1;
    }
    
    // Вывод информации о загруженных данных
    cout << "\n✓ Файл numbers.txt успешно загружен!" << endl;
    cout << "Загружено " << count << " чисел" << endl;
    
    // Показ первых 20 чисел для наглядности
    cout << "Первые 20 чисел: ";
    stringstream preview(data);
    int temp;
    int previewCount = 0;
    
    while (preview >> temp && previewCount < 20) {
        cout << temp << " ";
        previewCount++;
    }
    if (count > 20) {
        cout << "...";
    }
    cout << endl;
    
    // Главный цикл выбора алгоритма
    while (true) {
        printMenu();
        
        int algorithmChoice;
        cin >> algorithmChoice;
        
        if (algorithmChoice == 0) {
            cout << "Выход из программы." << endl;
            break;
        }
        
        if (algorithmChoice < 1 || algorithmChoice > 4) {
            cout << "Неверный выбор алгоритма! Пожалуйста, выберите от 0 до 4." << endl;
            continue;
        }
        
        // Ввод ключа поиска
        int searchKey;
        cout << "Введите ключ для поиска: ";
        cin >> searchKey;
        
        // Форматирование запроса
        string request = formatDataForSending(algorithmChoice, searchKey, data);
        
        // Преобразование IP адреса сервера
        in_addr ip_to_num;        
        inet_pton(AF_INET, SERVER_IP, &ip_to_num);
        
        // Инициализация сокета
        int ClientSock = socket(AF_INET, SOCK_STREAM, 0);
        
        if (ClientSock == -1) {
            cout << "Ошибка инициализации сокета" << endl;
            return 1;
        }
        
        // Задание структуры servInfo
        sockaddr_in servInfo;
        memset(&servInfo, 0, sizeof(servInfo));
        
        servInfo.sin_family = AF_INET;
        servInfo.sin_addr = ip_to_num;    
        servInfo.sin_port = htons(SERVER_PORT_NUM);
        
        // Подключение к серверу
        cout << "\nПодключение к серверу " << SERVER_IP << ":" << SERVER_PORT_NUM << "..." << endl;
        int erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
        
        if (erStat != 0) {
            cout << "✗ Ошибка подключения к серверу!" << endl;
            cout << "  Убедитесь, что сервер запущен на порту " << SERVER_PORT_NUM << endl;
            close(ClientSock);
            
            // Предлагаем продолжить или выйти
            cout << "\nПопробовать снова? (1 - да, 0 - нет): ";
            int retry;
            cin >> retry;
            if (retry == 0) {
                break;
            }
            continue;
        }
        
        cout << "✓ Соединение с сервером установлено!" << endl;
        
        // Отправка данных на сервер
        cout << "Отправка запроса на сервер..." << endl;
        cout << "Размер запроса: " << request.size() << " байт" << endl;
        
        send(ClientSock, request.c_str(), request.size(), 0);
        
        // Закрытие сокета для записи
        shutdown(ClientSock, SHUT_WR);
        
        // Получение ответа от сервера
        vector<char> servBuff(BUFF_SIZE);
        ssize_t packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
        
        if (packet_size < 0) {
            cout << "Не удалось получить данные с сервера." << endl;
        } else {
            servBuff[packet_size] = '\0';
            cout << "\n=== РЕЗУЛЬТАТЫ ПОИСКА ===" << endl;
            cout << servBuff.data() << endl;
        }
        
        // Закрытие соединения
        close(ClientSock);
        cout << "\nСоединение с сервером закрыто." << endl;
        
        // Предложение продолжить поиск
        cout << "\nХотите выполнить еще один поиск? (1 - да, 0 - нет): ";
        int continueSearch;
        cin >> continueSearch;
        if (continueSearch == 0) {
            cout << "Выход из программы." << endl;
            break;
        }
    }
    
    cout << "\nДля выхода нажмите Enter...";
    cin.ignore();
    cin.get();
    
    return 0;
}