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
#include <iomanip>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;

// Структура для хранения результатов поиска
struct SearchResult {
    int algorithm;
    string algorithmName;
    int index;
    int comparisons;
    double timeMs;
    bool found;
    bool error;
    string errorMsg;
};

// Мьютекс для синхронизации вывода
mutex coutMutex;

// Функция для чтения данных из файла numbers.txt с проверкой на пустой файл
bool readNumbersFromFile(string& data, int& count) {
    ifstream file("numbers.txt");
    
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл numbers.txt!" << endl;
        cerr << "Убедитесь, что файл numbers.txt находится в той же папке, что и программа." << endl;
        return false;
    }
    
    if (file.peek() == ifstream::traits_type::eof()) {
        cerr << "Ошибка: файл numbers.txt пуст!" << endl;
        cerr << "Добавьте целые числа в файл через пробел." << endl;
        file.close();
        return false;
    }
    
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

// Функция для выполнения поиска через сокет
SearchResult performSearch(int algorithm, const string& algorithmName, 
                          int searchKey, const string& data, 
                          const string& serverIp, int serverPort) {
    SearchResult result;
    result.algorithm = algorithm;
    result.algorithmName = algorithmName;
    result.error = false;
    
    auto startTime = chrono::high_resolution_clock::now();
    
    // Создание сокета
    int clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == -1) {
        result.error = true;
        result.errorMsg = "Ошибка создания сокета";
        return result;
    }
    
    // Преобразование IP адреса
    in_addr ip_to_num;
    if (inet_pton(AF_INET, serverIp.c_str(), &ip_to_num) <= 0) {
        result.error = true;
        result.errorMsg = "Неверный IP адрес";
        close(clientSock);
        return result;
    }
    
    // Настройка структуры сервера
    sockaddr_in servInfo;
    memset(&servInfo, 0, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(serverPort);
    
    // Подключение к серверу
    if (connect(clientSock, (sockaddr*)&servInfo, sizeof(servInfo)) != 0) {
        result.error = true;
        result.errorMsg = "Не удалось подключиться к серверу";
        close(clientSock);
        return result;
    }
    
    // Отправка запроса
    string request = formatDataForSending(algorithm, searchKey, data);
    if (send(clientSock, request.c_str(), request.size(), 0) <= 0) {
        result.error = true;
        result.errorMsg = "Ошибка отправки запроса";
        close(clientSock);
        return result;
    }
    
    // Закрытие сокета для записи
    shutdown(clientSock, SHUT_WR);
    
    // Получение ответа
    char buffer[4096];
    ssize_t bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        result.error = true;
        result.errorMsg = "Не получен ответ от сервера";
        close(clientSock);
        return result;
    }
    
    buffer[bytesReceived] = '\0';
    close(clientSock);
    
    auto endTime = chrono::high_resolution_clock::now();
    result.timeMs = chrono::duration<double, milli>(endTime - startTime).count();
    
    // Парсинг ответа от сервера
    string response(buffer);
    stringstream ss(response);
    string line;
    
    while (getline(ss, line)) {
        if (line.find("Найден на позиции:") != string::npos) {
            size_t pos = line.find(":");
            if (pos != string::npos) {
                result.index = stoi(line.substr(pos + 2));
                result.found = true;
            }
        } else if (line.find("не найден") != string::npos) {
            result.found = false;
            result.index = -1;
        } else if (line.find("Количество сравнений:") != string::npos) {
            size_t pos = line.find(":");
            if (pos != string::npos) {
                result.comparisons = stoi(line.substr(pos + 2));
            }
        }
    }
    
    return result;
}

// Функция для отображения сводной таблицы
void displaySummaryTable(const vector<SearchResult>& results, int searchKey, int dataSize) {
    cout << "\n" << string(80, '=') << endl;
    cout << "                      СВОДНАЯ ТАБЛИЦА РЕЗУЛЬТАТОВ ПОИСКА" << endl;
    cout << string(80, '=') << endl;
    cout << "Ключ поиска: " << searchKey << " | Размер массива: " << dataSize << " элементов\n" << endl;
    
    cout << left << setw(40) << "АЛГОРИТМ ПОИСКА" 
         << setw(15) << "РЕЗУЛЬТАТ" 
         << setw(12) << "ПОЗИЦИЯ" 
         << setw(15) << "СРАВНЕНИЯ" 
         << setw(12) << "ВРЕМЯ (мс)" 
         << endl;
    cout << string(80, '-') << endl;
    
    for (const auto& result : results) {
        cout << left << setw(40) << result.algorithmName;
        
        if (result.error) {
            cout << setw(15) << "ОШИБКА" 
                 << setw(12) << "N/A" 
                 << setw(15) << "N/A" 
                 << setw(12) << "N/A" 
                 << endl;
            cout << "  ↳ " << result.errorMsg << endl;
        } else {
            string resultStr = result.found ? "НАЙДЕН" : "НЕ НАЙДЕН";
            string position = result.found ? to_string(result.index) : "N/A";
            
            cout << setw(15) << resultStr
                 << setw(12) << position
                 << setw(15) << result.comparisons
                 << fixed << setprecision(3) << setw(12) << result.timeMs
                 << endl;
        }
    }
    
    cout << string(80, '=') << endl;
    
    // Анализ результатов
    cout << "\nАНАЛИЗ РЕЗУЛЬТАТОВ:" << endl;
    
    // Находим самый быстрый алгоритм
    double minTime = numeric_limits<double>::max();
    int fastestIndex = -1;
    
    for (size_t i = 0; i < results.size(); i++) {
        if (!results[i].error && results[i].found) {
            if (results[i].timeMs < minTime) {
                minTime = results[i].timeMs;
                fastestIndex = i;
            }
        }
    }
    
    if (fastestIndex != -1) {
        cout << "✓ Самый быстрый алгоритм: " << results[fastestIndex].algorithmName 
             << " (" << minTime << " мс)" << endl;
    }
    
    // Находим алгоритм с наименьшим числом сравнений
    int minComparisons = numeric_limits<int>::max();
    int efficientIndex = -1;
    
    for (size_t i = 0; i < results.size(); i++) {
        if (!results[i].error && results[i].found) {
            if (results[i].comparisons < minComparisons) {
                minComparisons = results[i].comparisons;
                efficientIndex = i;
            }
        }
    }
    
    if (efficientIndex != -1) {
        cout << "✓ Самый эффективный (меньше сравнений): " << results[efficientIndex].algorithmName 
             << " (" << minComparisons << " сравнений)" << endl;
    }
    
    // Проверка согласованности результатов
    bool allFoundSame = true;
    int firstFoundIndex = -1;
    
    for (size_t i = 0; i < results.size(); i++) {
        if (!results[i].error && results[i].found) {
            if (firstFoundIndex == -1) {
                firstFoundIndex = results[i].index;
            } else if (results[i].index != firstFoundIndex) {
                allFoundSame = false;
                break;
            }
        }
    }
    
    if (allFoundSame && firstFoundIndex != -1) {
        cout << "✓ Все алгоритмы дали одинаковый результат" << endl;
    }
}

// Функция для выполнения всех поисков параллельно
vector<SearchResult> performAllSearches(int searchKey, const string& data, int dataSize) {
    const string SERVER_IP = "127.0.0.1";
    const int SERVER_PORT = 7129;
    
    vector<pair<int, string>> algorithms = {
        {1, "1. Последовательный поиск"},
        {2, "2. Быстрый последовательный поиск"},
        {3, "3. Бинарный поиск"},
        {4, "4. Поиск в упорядоченном массиве"}
    };
    
    vector<SearchResult> results;
    vector<thread> threads;
    
    cout << "\n" << string(60, '=') << endl;
    cout << "           ЗАПУСК ВСЕХ АЛГОРИТМОВ ПОИСКА ОДНОВРЕМЕННО" << endl;
    cout << string(60, '=') << endl;
    cout << "Запуск " << algorithms.size() << " алгоритмов поиска для ключа: " << searchKey << "\n" << endl;
    
    // Запускаем каждый поиск в отдельном потоке
    for (const auto& algo : algorithms) {
        threads.emplace_back([&results, algo, searchKey, &data, SERVER_IP, SERVER_PORT]() {
            auto result = performSearch(algo.first, algo.second, searchKey, data, SERVER_IP, SERVER_PORT);
            
            lock_guard<mutex> lock(coutMutex);
            results.push_back(result);
            
            string status = result.error ? "✗ ОШИБКА" : "✓ ЗАВЕРШЕН";
            cout << left << setw(40) << algo.second << status << endl;
        });
    }
    
    // Ожидаем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    
    cout << "\n" << string(60, '=') << endl;
    cout << "Все алгоритмы завершили выполнение!" << endl;
    cout << string(60, '=') << endl;
    
    return results;
}

// Главное меню
void printMenu() {
    cout << "\n" << string(60, '=') << endl;
    cout << "           ГЛАВНОЕ МЕНЮ КЛИЕНТА ПОИСКА" << endl;
    cout << string(60, '=') << endl;
    cout << "1. Выполнить все алгоритмы поиска одновременно" << endl;
    cout << "2. Выполнить конкретный алгоритм поиска" << endl;
    cout << "3. Показать информацию о данных" << endl;
    cout << "0. Выход" << endl;
    cout << string(60, '-') << endl;
    cout << "Выберите действие (0-3): ";
}

int main() {
    cout << "=== КЛИЕНТ ПАРАЛЛЕЛЬНОГО ПОИСКА В МАССИВЕ ===" << endl;
    cout << "Данные загружаются из файла numbers.txt" << endl;
    
    // Загрузка данных из файла
    string data;
    int count = 0;
    
    if (!readNumbersFromFile(data, count)) {
        cout << "\nДля выхода нажмите Enter...";
        cin.ignore();
        cin.get();
        return 1;
    }
    
    cout << "\n✓ Файл numbers.txt успешно загружен!" << endl;
    cout << "Загружено " << count << " чисел" << endl;
    
    // Главный цикл программы
    while (true) {
        printMenu();
        
        int choice;
        cin >> choice;
        
        if (choice == 0) {
            cout << "Выход из программы." << endl;
            break;
        }
        
        if (choice == 3) {
            // Показать информацию о данных
            cout << "\n=== ИНФОРМАЦИЯ О ДАННЫХ ===" << endl;
            cout << "Количество чисел: " << count << endl;
            
            stringstream ss(data);
            vector<int> numbers;
            int num;
            while (ss >> num) {
                numbers.push_back(num);
            }
            
            if (!numbers.empty()) {
                // Проверяем, отсортирован ли массив
                bool isSorted = true;
                for (size_t i = 1; i < numbers.size(); i++) {
                    if (numbers[i] < numbers[i-1]) {
                        isSorted = false;
                        break;
                    }
                }
                
                cout << "Массив " << (isSorted ? "отсортирован" : "не отсортирован") << endl;
                cout << "Минимальное значение: " << *min_element(numbers.begin(), numbers.end()) << endl;
                cout << "Максимальное значение: " << *max_element(numbers.begin(), numbers.end()) << endl;
                
                // Показать первые и последние 5 чисел
                cout << "\nПервые 5 чисел: ";
                for (int i = 0; i < min(5, count); i++) {
                    cout << numbers[i] << " ";
                }
                
                cout << "\nПоследние 5 чисел: ";
                for (int i = max(0, count - 5); i < count; i++) {
                    cout << numbers[i] << " ";
                }
                cout << endl;
            }
            continue;
        }
        
        // Ввод ключа поиска для операций 1 и 2
        int searchKey;
        cout << "Введите ключ для поиска: ";
        cin >> searchKey;
        
        if (choice == 1) {
            // Выполнить все алгоритмы одновременно
            auto results = performAllSearches(searchKey, data, count);
            displaySummaryTable(results, searchKey, count);
        }
        else if (choice == 2) {
            // Выполнить конкретный алгоритм
            cout << "\nВыберите алгоритм:" << endl;
            cout << "1. Последовательный поиск" << endl;
            cout << "2. Быстрый последовательный поиск" << endl;
            cout << "3. Бинарный поиск" << endl;
            cout << "4. Поиск в упорядоченном массиве" << endl;
            cout << "Выбор (1-4): ";
            
            int algoChoice;
            cin >> algoChoice;
            
            if (algoChoice >= 1 && algoChoice <= 4) {
                vector<pair<int, string>> algorithms = {
                    {1, "1. Последовательный поиск"},
                    {2, "2. Быстрый последовательный поиск"},
                    {3, "3. Бинарный поиск"},
                    {4, "4. Поиск в упорядоченном массиве"}
                };
                
                auto result = performSearch(algoChoice, algorithms[algoChoice-1].second, 
                                           searchKey, data, "127.0.0.1", 7129);
                
                vector<SearchResult> singleResult = {result};
                displaySummaryTable(singleResult, searchKey, count);
            } else {
                cout << "Неверный выбор алгоритма!" << endl;
            }
        }
        
        // Предложение продолжить
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