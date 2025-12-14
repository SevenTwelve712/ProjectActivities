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
        }
    }
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
