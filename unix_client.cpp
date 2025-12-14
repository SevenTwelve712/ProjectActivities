<<<<<<< HEAD
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
=======
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <iomanip>
#include <thread>
#include <chrono>
#include <sstream>
#include <cctype>
#include <limits>
#include <cstring> // Для memset

// --- UNIX-специфичные библиотеки ---
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // Для close()
#include <errno.h>  // Для вывода ошибок

using namespace std;

// --- ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ---
// Мьютекс для синхронизации вывода
mutex coutMutex;

// Функция для чтения данных из файла numbers.txt с проверкой на пустой файл
bool readNumbersFromFile(string& data, int& count)
{
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
=======
// Проверка: является ли строка корректным целым числом
bool isValidInteger(const string& s) {
    if (s.empty()) return false;
    size_t start = 0;
    if (s[0] == '-') {
        start = 1;
        if (s.length() == 1) return false;
    }
    for (size_t i = start; i < s.length(); ++i) {
        if (!isdigit(static_cast<unsigned char>(s[i]))) {
            return false;

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
string formatDataForSending(int algorithmChoice, int searchKey, const string& data)
{
    stringstream ss;
    ss << algorithmChoice << " " << searchKey << " " << data;
    return ss.str();
}

// Функция для выполнения поиска через сокет
SearchResult performSearch(int algorithm, const string& algorithmName,
    int searchKey, const string& data,
    const string& serverIp, int serverPort)
{
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
void displaySummaryTable(const vector<SearchResult>& results, int searchKey, int dataSize)
{
    cout << "\n"
         << string(80, '=') << endl;
    cout << "                      СВОДНАЯ ТАБЛИЦА РЕЗУЛЬТАТОВ ПОИСКА" << endl;
    cout << string(80, '=') << endl;
    cout << "Ключ поиска: " << searchKey << " | Размер массива: " << dataSize << " элементов\n"
         << endl;

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
vector<SearchResult> performAllSearches(int searchKey, const string& data, int dataSize)
{
    const string SERVER_IP = "127.0.0.1";
    const int SERVER_PORT = 7129;

    vector<pair<int, string>> algorithms = {
        { 1, "1. Последовательный поиск" },
        { 2, "2. Быстрый последовательный поиск" },
        { 3, "3. Бинарный поиск" },
        { 4, "4. Поиск в упорядоченном массиве" }
    };

    vector<SearchResult> results;
    vector<thread> threads;

    cout << "\n"
         << string(60, '=') << endl;
    cout << "           ЗАПУСК ВСЕХ АЛГОРИТМОВ ПОИСКА ОДНОВРЕМЕННО" << endl;
    cout << string(60, '=') << endl;
    cout << "Запуск " << algorithms.size() << " алгоритмов поиска для ключа: " << searchKey << "\n"
         << endl;

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

    cout << "\n"
         << string(60, '=') << endl;
    cout << "Все алгоритмы завершили выполнение!" << endl;
    cout << string(60, '=') << endl;

    return results;
}

// Главное меню
void printMenu()
{
    cout << "\n"
         << string(60, '=') << endl;
    cout << "           ГЛАВНОЕ МЕНЮ КЛИЕНТА ПОИСКА" << endl;
    cout << string(60, '=') << endl;
    cout << "1. Выполнить все алгоритмы поиска одновременно" << endl;
    cout << "2. Выполнить конкретный алгоритм поиска" << endl;
    cout << "3. Показать информацию о данных" << endl;
    cout << "0. Выход" << endl;
    cout << string(60, '-') << endl;
    cout << "Выберите действие (0-3): ";
}

int main()
{
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
                    if (numbers[i] < numbers[i - 1]) {
                        isSorted = false;
                        break;
                    }
                }

                // cout << "Массив " << (isSorted ? "отсортирован" : "не отсортирован") << endl;
                // cout << "Минимальное значение: " << *min_element(numbers.begin(), numbers.end()) << endl;
                // cout << "Максимальное значение: " << *max_element(numbers.begin(), numbers.end()) << endl;

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
        } else if (choice == 2) {
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
                    { 1, "1. Последовательный поиск" },
                    { 2, "2. Быстрый последовательный поиск" },
                    { 3, "3. Бинарный поиск" },
                    { 4, "4. Поиск в упорядоченном массиве" }
                };

                auto result = performSearch(algoChoice, algorithms[algoChoice - 1].second,
                    searchKey, data, "127.0.0.1", 7129);

                vector<SearchResult> singleResult = { result };
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

=======
    return true;
}

// Надежная отправка данных
// В UNIX сокет — это int
bool SendAll(int s, const char* data, int totalBytes) {
    int bytesSent = 0;
    while (bytesSent < totalBytes) {
        // В UNIX при ошибке send возвращает -1
        ssize_t res = send(s, data + bytesSent, totalBytes - bytesSent, 0);
        if (res < 0) {
            return false;
        }
        bytesSent += res;
    }
    return true;
}

// Шаблонная функция для безопасного ввода
template <typename T>
T getSafeInput() {
    T value;
    while (true) {
        if (cin >> value) {
            char nextChar;
            // Исправлена опечатка оригинала (убрана лишняя 'д')
            if (cin.get(nextChar) && !isspace(nextChar)) {
                cin.putback(nextChar);
            } else {
                return value;
            }
        }
        cout << "Некорректный ввод! Пожалуйста, введите целое число: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// --- MAIN ---

int main(void) {
    // 1. Локаль. В UNIX обычно используется UTF-8, системная локаль
    setlocale(LC_ALL, ""); 
    
    // 2. Инициализация генератора случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());

    const char SERVER_IP[] = "10.77.129.244"; 
    const short SERVER_PORT_NUM = 8081; 
    
    // 3. Инициализация WinSock НЕ НУЖНА в UNIX

    // 4. Создание сокета
    int ClientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSock < 0) {
        cerr << "Error creating socket: " << strerror(errno) << endl;
        return 1;
    }

    sockaddr_in servInfo;
    memset(&servInfo, 0, sizeof(servInfo)); // ZeroMemory -> memset
    servInfo.sin_family = AF_INET;
    
    if (inet_pton(AF_INET, SERVER_IP, &servInfo.sin_addr) <= 0) {
        cerr << "Invalid IP address provided." << endl;
        close(ClientSock);
        return 1;
    }
    servInfo.sin_port = htons(SERVER_PORT_NUM);

    // 5. Подключение
    cout << "Попытка подключения к серверу..." << endl;
    
    const int attempts = 5;
    const int delay = 2;
    bool isConnected = false;

    for (int i = 1; i <= attempts; ++i) {
        if (connect(ClientSock, reinterpret_cast<sockaddr*>(&servInfo), sizeof(servInfo)) != 0) {
            cerr << "[Попытка " << i << "] Неудачно." << endl;
            if (i < attempts) {
                std::this_thread::sleep_for(std::chrono::seconds(delay));
            }
        } else {
            cout << "Соединение успешно установлено!" << endl;
            isConnected = true;
            break; 
        }
    }

    if (!isConnected) {
        cerr << "Не удалось подключиться к серверу." << endl;
        close(ClientSock);
        // system("pause") удален, так как это команда Windows
        return 1;
    }

    // --- ГЕНЕРАЦИЯ ---
    int userdata = 0;
    while (userdata != 1 && userdata != 2) {
        cout << "\n1. Сгенерировать числа\n2. Использовать существующий файл\nВаш выбор: ";
        userdata = getSafeInput<int>(); 

        if (userdata == 1) {
            cout << "Количество чисел (n): ";
            int n = getSafeInput<int>();
            cout << "Начало интервала: ";
            int start = getSafeInput<int>();
            cout << "Конец интервала: ";
            int end = getSafeInput<int>();

            if (start > end) std::swap(start, end);
            std::uniform_int_distribution<int> dist(start, end);
            
            ofstream file("numbers.txt", std::ios::out | std::ios::trunc);
            if (!file.is_open()) {
                cerr << "Ошибка файла!" << endl;
                continue; 
            }
            for (int i = 0; i < n; i++) {
                file << dist(gen) << (i == n - 1 ? "" : " ");
            }
            file.close();
            cout << "Файл обновлен.\n";
        }
    }

    // --- ВЫБОР ДЕЙСТВИЯ ---
    int algoChoice = 0;
    while (true) {
        cout << "\n1. Поиск числа\n2. Сортировка (возрастание)\n3. Сортировка (убывание)\nВаш выбор: ";
        algoChoice = getSafeInput<int>();
        if (algoChoice >= 1 && algoChoice <= 3) break;
    }

    int usersearch = 0;
    if (algoChoice == 1) {
        cout << "Введите искомое число: ";
        usersearch = getSafeInput<int>();
    }

    // --- ЧТЕНИЕ ФАЙЛА ---
    ifstream file("numbers.txt");
    if (!file.is_open()) {
        cerr << "Файл numbers.txt не найден!" << endl; 
        close(ClientSock);
        return 1;
    }

    string tempStr, validDataString;
    bool fileIsEmpty = true;

    while (file >> tempStr) {
        fileIsEmpty = false;
        if (!isValidInteger(tempStr)) {
            cerr << "Ошибка: Некорректные данные в файле." << endl;
            close(ClientSock);
            return 1;
        }
        validDataString += tempStr + " ";
    }
    file.close();
    if (!validDataString.empty()) validDataString.pop_back();

    if (fileIsEmpty) {
        cerr << "Файл пуст!" << endl;
        close(ClientSock);
        return 1;
    }

    // --- ОТПРАВКА ---
    std::ostringstream packetStream;
    packetStream << algoChoice << " ";
    if (algoChoice == 1) packetStream << usersearch << " ";
    packetStream << validDataString;

    string fullPacket = packetStream.str();

    // === ВЫВОД ОТПРАВЛЯЕМЫХ ДАННЫХ ===
    cout << "\nОтправляемые данные в формате ASCII-кодов (первые 50 байт):" << endl;
    int printCount = 0;
    for (char c : fullPacket) {
        cout << static_cast<int>(static_cast<unsigned char>(c)) << " ";
        if (++printCount >= 50) break;
    }
    cout << "\n" << endl;
    // ========================================

    cout << "Отправка пакета (" << fullPacket.size() << " байт)..." << endl;
    if (!SendAll(ClientSock, fullPacket.c_str(), (int)fullPacket.size())) {
        cerr << "Ошибка отправки!" << endl;
        close(ClientSock);
        return 1;
    }
    // SD_SEND -> SHUT_WR
    shutdown(ClientSock, SHUT_WR); 

    // --- ПРИЕМ ДАННЫХ ОТ СЕРВЕРА ---
    vector<char> recvBuffer(4096);
    int totalReceived = 0;
    
    cout << "Ожидание ответа..." << endl;
    while (true) {
        if (totalReceived == recvBuffer.size()) {
            recvBuffer.resize(recvBuffer.size() * 2);
        }
        // recv возвращает ssize_t
        ssize_t receivedNow = recv(ClientSock, recvBuffer.data() + totalReceived, (int)(recvBuffer.size() - totalReceived), 0);
        
        if (receivedNow > 0) totalReceived += receivedNow;
        else break;
    }

    // --- ОБРАБОТКА ОТВЕТА ОТ СЕРВЕРА ---
    if (totalReceived > 0) {
        string response(recvBuffer.begin(), recvBuffer.begin() + totalReceived);
        
        cout << "\nПолученные данные в виде ASCII-кодов (первые 50 байт):" << endl;
        printCount = 0;
        for (char c : response) {
            cout << static_cast<int>(static_cast<unsigned char>(c)) << " ";
            if (++printCount >= 50) break;
        }
        cout << "\n" << endl;

        stringstream ss(response);

        if (algoChoice == 1) { // Поиск
            int foundCount;
            if (ss >> foundCount) {
                if (foundCount == 0) cout << "Число не найдено." << endl;
                else {
                    const char* searchNames[] = { "Последовательный", "Быстрый", "Бинарный", "В упорядоченном" };
                    for (int i = 0; i < 4; i++) {
                        if (i > 0 && !(ss >> foundCount)) break;
                        cout << "--- " << searchNames[i] << " ---" << endl;
                        cout << "Найдено: " << foundCount << ". Индексы: ";
                        int idx;
                        for(int k=0; k<foundCount; k++) if(ss >> idx) cout << idx << " ";
                        
                        float t; long s;
                        ss >> t >> s;
                        cout << "\nВремя: " << t << " | Шаги: " << s << "\n" << endl;
                    }
                }
            }
        } else { // Сортировка
            const char* sortNamesShell[] = { "Сортировка Шелла", "Сортировка Вставкой" }; 
            const char* sortNamesQuick[] = { "Сортировка Выбором", "Быстрая сортировка" }; 
            
            long comparisons = 0, swaps = 0;
            float timeSpent = 0.0f;

            cout << "\n--- Статистика ---\n";
            for (int i = 0; i < 2; i++) {
                const char* name = (algoChoice == 2) ? sortNamesShell[i] : sortNamesQuick[i];
                if (ss >> comparisons >> swaps >> timeSpent) {
                    cout << name << ": Сравнений: " << comparisons << " | Перестановок: " << swaps << " | Время: " << timeSpent << endl;
                }
            }

            ofstream outFile("numbers.txt", std::ios::out | std::ios::trunc);
            if (outFile.is_open()) {
                int val;
                bool first = true;
                while (ss >> val) {
                    outFile << val << (first ? "" : " ");
                    first = false;
                }
                outFile.close();
                cout << "\nМассив перезаписан в файл." << endl;
            }
        }
    } else {
        cout << "Пустой ответ от сервера." << endl;
    }

    close(ClientSock);
    // system("pause") не используется в UNIX, программа просто завершится
    return 0;
}
