#include <cctype>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Подключение библиотеки сокетов (Windows)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

// --- ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ---

// Проверка: является ли строка корректным целым числом
bool isValidInteger(const string& s)
{
    if (s.empty())
        return false; // Если строка пустая - возвращаем false
    size_t start = 0;
    if (s[0] == '-') { // Если число отрицательное - пропускаем минус и начинаем со следующего символа
        start = 1;
        if (s.length() == 1)
            return false; // Если строка состоит только из минуса - возвращаем false
    }
    for (size_t i = start; i < s.length(); ++i) {
        if (!isdigit(static_cast<unsigned char>(s[i]))) {
            return false; // Проверяем строку посимвольно: если встречается некорректный символ - возвращаем false
        }
    }
    return true; // Всё верно - возвращаем true
}

// Надежная отправка данных
bool SendAll(int s, const char* data, int totalBytes)
{
    int bytesSent = 0; // Переменная, показывающая сколько байт отправлено на данный момент
    while (bytesSent < totalBytes) {
        int res = send(s, data + bytesSent, totalBytes - bytesSent, 0); // Отправляем в сокет данные, если отправляется несколько раз - начинаем с момента конца предыдущей отправки
        if (res < 0) {
            return false; // Если возникает ошибка отправки - возвращаем false
        }
        bytesSent += res; // Добавляем количество отправленных в данной итерации цикла отправки байт к bytesSent
    }
    return true; // Всё верно - возвращаем true
}

// Шаблонная функция для безопасного ввода
template <typename T> // Задаём шаблон Т - типа данных
T getSafeInput()
{ // Запускаем фунцию в зависимости от типа данных
    T value; // Переменная с типом данных T
    while (true) {
        if (cin >> value) { // Ввод числа
            char nextChar;
            if (cin.get(nextChar) && !isspace(nextChar)) { // Если можно считать символ и он не пробел - ошибка
                cin.putback(nextChar); // Возвращаем считанный символ обратно в поток
            } else {
                return value; // Если всё нормально - возвращаем введённое значение
            }
        }
        cout << "Некорректный ввод! Пожалуйста, введите целое число: ";
        cin.clear(); // Сброс флагов ошибок
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Очистка буфера до конца строки, максимальный пропуск - INT_MAX
    }
}

// --- MAIN ---

int main(void)
{

    // 2. Инициализация генератора случайных чисел
    std::random_device rd; // Берём случайное зерно из системы
    std::mt19937 gen(rd()); // Используем вихрь Мерсена с периодом 2^19937-1, зерно для генератора взяли из системы

    const char SERVER_IP[] = "127.0.0.1"; // IPv4-адрес сервера класса А
    const short SERVER_PORT_NUM = 8081; // порт сервера

    // 4. Создание сокета
    int ClientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSock < 0) {
        cerr << "Error creating socket: " << endl;
        return 1;
    }

    sockaddr_in servInfo; // Переменная servInfo типа sockaddr_in для IPv4
    memset(&servInfo, 0, sizeof(servInfo));
    servInfo.sin_family = AF_INET; // Используем семейство IPv4 адресов

    if (inet_pton(AF_INET, SERVER_IP, &servInfo.sin_addr) <= 0) { // Преобразуем IP-адрес в понятный для компьютера вид - строку sin.addr
        cerr << "Invalid IP address provided." << endl;
        close(ClientSock);
        return 1;
    }
    servInfo.sin_port = htons(SERVER_PORT_NUM); // Преобразуем номер хоста в понятный для компьютера вид

    // 5. Подключение
    cout << "Попытка подключения к серверу..." << endl;

    const int attempts = 5; // Количество попыток подключения
    const int delay = 2; // Задержка между подключениями
    bool isConnected = false; // Флаг подключения

    for (int i = 1; i <= attempts; ++i) {
        if (connect(ClientSock, reinterpret_cast<sockaddr*>(&servInfo), sizeof(servInfo)) != 0) { // Используем reinterpret_cast, чтоб компилятор трактовал sockaddr_in как просто sockaddr, подключаемся к серверу
            cerr << "[Попытка " << i << "] Неудачно." << endl;
            if (i < attempts) {
                std::this_thread::sleep_for(std::chrono::seconds(delay)); // Ставим задержку между попытками, чтоб не дудосить сервер
            }
        } else {
            cout << "Соединение успешно установлено!" << endl;
            isConnected = true; // Переключаем флаг
            break;
        }
    }

    if (!isConnected) {
        cerr << "Не удалось подключиться к серверу." << endl;
        close(ClientSock);
        system("pause");
        return 1;
    }

    // --- ГЕНЕРАЦИЯ ---
    int userdata = 0; // Переменная, отвечающая за выбор пользователя
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

            if (start > end)
                std::swap(start, end); // Проверка на дурака: если начальное значение интервала больше конечного - меняем их местами
            std::uniform_int_distribution<int> dist(start, end); // Задаём правило для равномерного распределения

            ofstream file("numbers.txt", std::ios::out | std::ios::trunc); // Открываем файл для записи, предварительно очистив его
            if (!file.is_open()) {
                cerr << "Ошибка файла!" << endl;
                continue;
            }
            for (int i = 0; i < n; i++) {
                file << dist(gen) << (i == n - 1 ? "" : " "); // Генерируем случаное число, если оно последнее - после него пробел не ставим
            }
            file.close(); // Закрываем файл
            cout << "Файл обновлен.\n";
        }
    }

    // --- ВЫБОР ДЕЙСТВИЯ ---
    int algoChoice = 0; // Выбор алгоритма
    while (true) {
        cout << "\n1. Поиск числа\n2. Сортировка (Шелла и вставками)\n3. Сортировка (быстрая и выбором)\nВаш выбор: ";
        algoChoice = getSafeInput<int>();
        if (algoChoice >= 1 && algoChoice <= 3)
            break;
    }

    int usersearch = 0; // Если выбрали поиск - вводим искомое число
    if (algoChoice == 1) {
        cout << "Введите искомое число: ";
        usersearch = getSafeInput<int>();
    }

    // --- ЧТЕНИЕ ФАЙЛА ---
    ifstream file("numbers.txt"); // Открываем файл для чтения
    if (!file.is_open()) {
        cerr << "Файл numbers.txt не найден!" << endl;
        return 1;
    }

    string tempStr, validDataString; // Переменные, отвечающие за временную строку, где будут храниться считанные числа, и за строку с уже валидированными числами
    bool fileIsEmpty = true; // Флаг пустоты файла

    while (file >> tempStr) {
        fileIsEmpty = false; // Переключаем флаг
        if (!isValidInteger(tempStr)) {
            cerr << "Ошибка: Некорректные данные в файле." << endl;
            return 1;
        }
        validDataString += tempStr + " "; // Прибавляем проверенные данные из временной строки в строку с валидированными данными
    }
    file.close(); // Закрываем файл
    if (!validDataString.empty())
        validDataString.pop_back(); // Убираем лишний пробел в конце

    if (fileIsEmpty) {
        cerr << "Файл пуст!" << endl;
        return 1;
    }

    // --- ОТПРАВКА ---
    std::ostringstream packetStream; // Создаём поток
    packetStream << algoChoice << " "; // Записываем выбранный пользователем алгоритм, если выбран поиск - записываем искомое число
    if (algoChoice == 1)
        packetStream << usersearch << " ";
    packetStream << validDataString; // Записываем считанные из файла числа

    string fullPacket = packetStream.str(); // Записываем данные из потока в строку fullPacket

    // === ВЫВОД ОТПРАВЛЯЕМЫХ ДАННЫХ ЗНАЧЕНИЯМИ ASCII ===
    cout << "\nОтправляемые данные в формате ASCII-кодов (первые 50 байт):" << endl;
    int printCount = 0; // Счётчик напечатанных кодов
    for (char c : fullPacket) {
        // Приводим к unsigned char, затем к int, чтобы видеть коды 0-255
        cout << static_cast<int>(static_cast<unsigned char>(c)) << " ";
        if (++printCount >= 50)
            break; // Ограничитель
    }
    cout << "\n"
         << endl;
    // ========================================

    cout << "Отправка пакета (" << fullPacket.size() << " байт)..." << endl;
    if (!SendAll(ClientSock, fullPacket.c_str(), (int)fullPacket.size())) {
        cerr << "Ошибка отправки!" << endl;
        return 1;
    }
    shutdown(ClientSock, SHUT_WR); // Заканчиваем отправку данных и говорим об этом серверу

    // --- ПРИЕМ ДАННЫХ ОТ СЕРВЕРА ---
    vector<char> recvBuffer(4096); // Буфер для приёма данных от сервера
    int totalReceived = 0; // Общее количество полученных байт

    cout << "Ожидание ответа..." << endl;
    while (true) {
        if (totalReceived == recvBuffer.size()) {
            recvBuffer.resize(recvBuffer.size() * 2); // Если количество полученных байт больше размера буфера - увеличиваем его размер в 2 раза
        }
        int receivedNow = recv(ClientSock, recvBuffer.data() + totalReceived, (int)(recvBuffer.size() - totalReceived), 0); // Получаем данные от сервера, если итерация не первая - начинаем с момента окончания предыдущей записи

        if (receivedNow > 0)
            totalReceived += receivedNow; // Увеличиваем totalReceived, если получили какие-то данные, если нет - выходим из цикла получения
        else
            break;
    }

    // --- ОБРАБОТКА ОТВЕТА ОТ СЕРВЕРА ---
    if (totalReceived > 0) {
        string response(recvBuffer.begin(), recvBuffer.begin() + totalReceived); // Переводим полученные данные от сервера в строку размером, равным общему количеству полученных байт

        // === ВЫВОД ПОЛУЧЕННЫХ ЗНАЧЕНИЙ В ФОРМАТЕ ASCII ===
        cout << "\nПолученные данные в виде ASCII-кодов (первые 50 байт):" << endl;
        printCount = 0; // Счётчик напечатанных кодов
        for (char c : response) {
            cout << static_cast<int>(static_cast<unsigned char>(c)) << " ";
            if (++printCount >= 50)
                break;
        }
        cout << "\n"
             << endl;
        // ======================================

        stringstream ss(response); // Открываем поток

        if (algoChoice == 1) { // Поиск
            int foundCount;
            if (ss >> foundCount) {
                if (foundCount == 0)
                    cout << "Число не найдено." << endl;
                else {
                    const char* searchNames[] = { "Последовательный", "Быстрый", "Бинарный", "В упорядоченном" };
                    for (int i = 0; i < 4; i++) {
                        if (i > 0 && !(ss >> foundCount))
                            break;
                        cout << "--- " << searchNames[i] << " ---" << endl;
                        cout << "Найдено: " << foundCount << ". Индексы: ";
                        int idx;
                        for (int k = 0; k < foundCount; k++)
                            if (ss >> idx)
                                cout << idx << " ";

                        float t;
                        long s;
                        ss >> t >> s;
                        cout << "\nВремя: " << t << " | Шаги: " << s << "\n"
                             << endl;
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

            ofstream outFile("numbers.txt", std::ios::out | std::ios::trunc); // Открываем файл для записи, предварительно его очистив
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
    return 0;
}
