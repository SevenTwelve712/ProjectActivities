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

// Сетевые заголовки
#include <WinSock2.h>
#include <WS2tcpip.h>

// Линковка библиотеки
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Функция для проверки, является ли строка целым числом
bool isValidInteger(const string& s) {
    if (s.empty()) return false;
    
    size_t start = 0;
    // Если первый символ минус - пропускаем его, чтоб функция не ругалась на отрицательные числа
    if (s[0] == '-') {
        start = 1; 
        if (s.length() == 1) return false; // Строка состоит только из минуса "-", возвращаем false
    }

    // Проверяем каждый символ считанной строки
    for (size_t i = start; i < s.length(); ++i) {
        if (!isdigit(static_cast<unsigned char>(s[i]))) { 
            return false; // Нашли не цифру (букву, точку и т.д.) - возвращаем false
        }
    }
    return true;
}

// Функция для надежной отправки всех данных
bool SendAll(SOCKET s, const char* data, int totalBytes) {
    int bytesSent = 0;	//Количество отправленных байт
    while (bytesSent < totalBytes) { //Цикл для отправки до тех пор, пока количество отправленных байт не окажется равным количеству байт в векторе
        int res = send(s, data + bytesSent, totalBytes - bytesSent, 0); //res - количество отправленных байт, отправленных на сервер в этой итерации цикла
        if (res == SOCKET_ERROR) {
            return false; //Если ошибка отправки - функция возвращает False
        }
        bytesSent += res; //Прибавляем res к bytesDent, чтобы знать, сколько байт уже отправлено 
    }
    return true;
}

// Функция для безопасного ввода числа (защита от букв) т.к. при вводе некорректного значения cin может выдать ошибку и положить клиент, либо же обрезать некорректно введённые данные до целого числа 
template <typename T> //Задаём шаблон T - типа данных
T getSafeInput() {
    T value; //Присваиваем value тип данных T
    while (true) {
        if (cin >> value) {
            // Дополнительная проверка символа, если ввод оставил мусор, условно: если пользователь для int ввёл 12.5, cin считает 12 и оставит .5
            char nextChar;
            if (cin.get(nextChar)) {
                 if (isspace(nextChar)) {
                     return value; // Всё норм, после числа пробел или Enter
                 }
                 // Если после числа идут буквы, например 12abc, считаем ошибкой
                 cin.putback(nextChar); 
            } else {
                return value; // Всё нормально, возвращаем value
            }
        }
        cout << "Ошибка! Введите целое число: ";
        cin.clear(); //Сбрасываем флаги ошибок при некорректном вводе 

        cin.ignore(INT_MAX, '\n'); //Очищаем буфер ввода - пропускаем все символы до пробела, максимум можно пропустить INT_MAX символов
    }
}

int main(void) {
    // Настройка локали Win1251 для корректного вывода в консоли
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
	
	//Настройка генерации псевдослучаёных чисел
    std::random_device rd; //Берём из системы случайное зерно
    std::mt19937 gen(rd()); //Исходя из взятого зерна инициализируем генератор случайных чисел, генерируем с помощью вихря Мерсена, используем агоритм MT19937 с периодом 2^19937 - 1

	//Ключевые константы - IPv4-адрес сервера и его порт
    const char SERVER_IP[] = "172.20.10.4"; //IP-адрес выглядит таким образом, т.к используется диапазон Класса А - от 10.0.0.0 до 10.255.255.255
    const short SERVER_PORT_NUM = 7129;
    
    // Инициализация WinSock
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) { //Используем последнюю версию WinSock - 2.2
        cerr << "Ошибка инициализации WinSock: " << WSAGetLastError() << endl; 
        return 1;
    }

    // Создание сокета
    SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSock == INVALID_SOCKET) {
        cerr << "Ошибка сокета: " << WSAGetLastError() << endl;
        WSACleanup(); //Очищаем WSA
        return 1;
    }

    sockaddr_in servInfo; //Создаём переменную servInfo типа struct sockaddr_in (специальная структура для хранения IPv4 адресов)
    ZeroMemory(&servInfo, sizeof(servInfo)); //Очищаем структуру, т.к там может лежать мусор
    servInfo.sin_family = AF_INET; //Испоьзуем семейство адресов IPv4
    
    if (inet_pton(AF_INET, SERVER_IP, &servInfo.sin_addr) <= 0) { //Используем inet_pton для преобразования IPv4 адреса в понятный компьютеру вид и кладём его в часть структуры servInfo, где хранится IP-адрес
        cerr << "Неверный IP адрес" << endl; //Если IP-адрес неверный, говорим об этом пользователю
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }
    servInfo.sin_port = htons(SERVER_PORT_NUM); //Преобразуем номер порта сервера в понятный для компьютера вид

    cout << "Подключение к серверу..." << endl;
	int attempts = 5; // Количество попыток подключения
    int delay = 2;    // Задержка между попытками
    bool isConnected = false; //Флаг для того, чтоб знать: подключились ли к серверу или нет
    for (int i = 1; i <= attempts; ++i) {
        // Попытка подключения
        if (connect(ClientSock, reinterpret_cast<sockaddr*>(&servInfo), sizeof(servInfo)) != 0) { //Используем reinterpret_cast, т.к функция connect ожидает указатель на общую структуру sockaddr, а мы спользуем sockaddr_in из-за IPv4. Говорим компилятору, чтоб он трактовал эту структуру нормально
            // Если ошибка
            cerr << "Попытка #" << i << " не удалась. Ошибка #" << WSAGetLastError() << endl;
            
            if (i < attempts) {
                cout << "Повторная попытка через " << delay << " сек..." << endl;
                // Пауза, чтобы не дудосить сервер
                std::this_thread::sleep_for(std::chrono::seconds(delay));
            }
        } else {
            // Если успех (вернул 0)
            cout << "Соединение успешно установлено!" << endl;
            isConnected = true;
            break; // Выходим из цикла, мы подключились!
        }
    }

    // Если после всех попыток так и не подключились
    if (!isConnected) {
        cerr << "Не удалось подключиться к серверу после " << attempts << " попыток." << endl;
        closesocket(ClientSock);
        WSACleanup();
        system("pause");
        return 1;
    }

    // --- МЕНЮ ГЕНЕРАЦИИ ---
    int userdata = 0; //Переменная, отвечающая за выбор пользователя
    while (userdata != 1 && userdata != 2) {
        cout << "1. Сгенерировать числа в файл\n2. Пропустить генерацию\nВаш выбор: " << endl;
        userdata = getSafeInput<int>(); 

        if (userdata == 1) {
            cout << "Количество чисел (n): ";
            int n = getSafeInput<int>();
            
            cout << "Начало интервала: ";
            int start = getSafeInput<int>();
            
            cout << "Конец интервала: ";
            int end = getSafeInput<int>();

            if (start > end) std::swap(start, end);

            std::uniform_int_distribution<int> dist(start, end); //Генерируем целые числа в интервале, заданном пользователем
            ofstream file("numbers.txt", std::ios::out | std::ios::trunc); //Начинаем запись в файл numbers.txt, предварительно его очистив
            
            if (!file.is_open()) {
                cerr << "Ошибка открытия файла!" << endl;
                continue; // Возврат в меню
			}
            for (int i = 0; i < n; i++) {
                file << dist(gen) << (i == n - 1 ? "" : " "); // Избегаем лишнего пробела в конце: если генерируется последнее число - после него пробел не ставим
            }
            file.close(); //Закрываем файл
            cout << "Числа сгенерированы.\n";
        }
    }

    // --- МЕНЮ ВЫБОРА СОРТИРОВКИ ---
    int algoChoice = 0; //Переменная, отвечающая за выбранный алгоритмм сортировки 
    while (true) {
        cout << "\nВыберите алгоритм сортировки (1-4):\n1. Сортировка по убыванию вставкой;\n2. Сортировка по убыванию Шелла;\n3. Сортировка по возрастанию выбором;\n4. Сортировка по возрастанию быстрая.\nВаш выбор:" << endl;
        algoChoice = getSafeInput<int>();
        if (algoChoice >= 1 && algoChoice <= 4) break;
        cout << "Ошибка! Число должно быть от 1 до 4." << endl;
    }

    cout << "Введите искомое число: ";
    int usersearch = getSafeInput<int>(); //Вводим число, которое нужно будет искать в файле

	// --- ВАЛИДАЦИЯ И ЧТЕНИЕ ФАЙЛА ---
    ifstream file("numbers.txt"); //Открываем файл для чтения
    if (!file.is_open()) {
        cerr << "Файл не найден!" << endl; 
        return 1;
    }

    string tempStr; //Временная строка, в которой будет храниться проверяемое число
    string validDataString; // Строка, куда будут записаны проверенные числа
    bool fileIsEmpty = true; //Флаг, отвечающий за проверку наполненности файла

    // Читаем файл по словам (числам), разделенным пробелами
    while (file >> tempStr) {
        fileIsEmpty = false; //Если файл не пустой - меняем флаг на false 
        // Проверка на целочисленность
        if (!isValidInteger(tempStr)) {
            cerr << "\nОшибка! В файле найдено некорректное значение: '" << tempStr << "'" << endl;
            cerr << "Файл должен содержать только целые числа." << endl;
            file.close();
            closesocket(ClientSock);
            WSACleanup();
            system("pause");
            return 1;
        }
        // Если проверка прошла, добавляем в строку отправки
        validDataString += tempStr + " ";
    }
    file.close(); //Закрываем файл

    if (fileIsEmpty) { //Если флаг fileIsEmpty не изменился на false - значит файл пустой
        cerr << "Ошибка: Файл пуст!" << endl;
        return 1;
    }

    // Убираем последний лишний пробел для красоты
    if (!validDataString.empty()) {
        validDataString.pop_back();
    }

    // --- ФОРМИРОВАНИЕ ПАКЕТА ---
    std::ostringstream headerOss; //Поток ввода заголовка
    headerOss << algoChoice << " " << usersearch << " "; //Записываем нужные нам данные: выбранный алгоритм и искомое число
    string header = headerOss.str(); //Преобразуем поток ввода 

    // Собираем всё вместе: Заголовок + Валидированные данные
    string fullPacket = header + validDataString;

    // --- ОТПРАВКА ---
    cout << "Отправка пакета (" << fullPacket.size() << " байт)..." << endl;
    if (!SendAll(ClientSock, fullPacket.c_str(), fullPacket.size())) {
        cerr << "Ошибка отправки!" << endl;
        return 1;
    }
    shutdown(ClientSock, SD_SEND); //Закрываем отправку со стороны клиента и уведомляем об этом сервер
    // --- ПРИЕМ ОТВЕТА ---
    vector<char> recvBuffer(4096); //Буфер для приёма данных с сервера
    int totalReceived = 0;	//Общее количество полученных от сервера байт
    int receivedNow = 0; //Количество байт, полученных на данный момент

    while (true) {
        // Если буфер заполнен, расширяем
        if (totalReceived == recvBuffer.size()) {
            recvBuffer.resize(recvBuffer.size() * 2); //Увеличиваем размер буфера в 2 раза
        }

        receivedNow = recv(ClientSock, recvBuffer.data() + totalReceived, recvBuffer.size() - totalReceived, 0); //Принимаем данные от сервера, если это уже не первая итерация - то дописываем в расширенный буфер, начиная с момента прошлой записи
        
        if (receivedNow > 0) {
            totalReceived += receivedNow; //Если сервер пришлёт какое-то количество данных, то receivedNow вернёт значение больше 0
        } else if (receivedNow == 0) {
            cout << "Cервер закрыл соединение" << endl; //Если сервер закончит передачу, то receivedNow вернёт 0
            break;
        } else {
            cerr << "Ошибка приема данных!" << endl; //Если произошла ошибка приёма данных, то receivedNow вернёт -1, .е ошибку
            break;
        }
    }

    // Обработка результата
    if (totalReceived > 0) {
        // Формируем строку только из полученных данных, чтоб не забивать её бесполезными данными
        string response(recvBuffer.begin(), recvBuffer.begin() + totalReceived);
		cout << "\nASCII коды первых нескольких байт ответа:" << endl;
        int count = 0;
        for (char c : response) {
            // нужно, чтобы коды 128-255 не были отрицательными, поэтому используем unsigned char
            cout << static_cast<int>(static_cast<unsigned char>(c)) << " ";
            count++;
            if (count >= 20) break; // Ограничиваем вывод 20-ю значениями
        }
        cout << "\n" << endl;
        stringstream ss(response); //Открываем строковый поток
        int resultIndex;
        
        if (ss >> resultIndex) {
            if (resultIndex != 0) {
                 cout << "Число найдено под номером: " << resultIndex << endl;
            } else {
                 cout << "Число не найдено (Сервер вернул 0)." << endl;
            }
        // Оставшиеся данные - отсортированный массив, так что записываем его в файл
            ofstream outFile("numbers.txt", std::ios::out | std::ios::trunc); //Открываем файл для записи, предварительно его очистив
            if (outFile.is_open()) {
                int arrayElement;
                bool isFirst = true;
                // Цикл while (ss >> arrayElement) будет читать числа, пока они есть в потоке.
                // Так как первое число (индекс) мы уже считали выше, то цикл начнет читать сразу со второго числа (начало массива).
                while (ss >> arrayElement) {
                    outFile << arrayElement << " ";
                    isFirst = false;
                }

                outFile.close(); //Закрываем файл
                if (!isFirst) {
                    cout << "Остальной массив успешно записан в файл 'numbers.txt'." << endl;
                } else {
                    cout << "Массив пуст (сервер прислал только индекс)." << endl;
                }
            } else {
                cerr << "Ошибка: Не удалось открыть файл для записи массива!" << endl;
            }

        } else {
            cout << "Ошибка парсинга: Сервер прислал не числовые данные." << endl;
        }

    } else {
        cout << "Сервер ничего не прислал." << endl;
    }

    closesocket(ClientSock);
    WSACleanup();
    system("pause");
    return 0;
}
