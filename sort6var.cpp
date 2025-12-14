#include <chrono>
#include <iostream>
#include <time.h>
#include <vector>

using namespace std;
using namespace std::chrono;

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

vector<float> generateRandomVector(int n)
{
    vector<float> arr(n);
    for (int i = 0; i < n; i++) {
        arr[i] = (rand() % (2 * 10 * n) - 10 * n) / 10.0; // Числа от 0.0 до 99.9
    }
    return arr;
}

void print_fvec(const vector<float>& arr)
{
    cout << '{';
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i];
        if (i != arr.size() - 1) {
            cout << "; ";
        }
    }
    cout << '}';
}

vector<float> insertions_fvec_sort(vector<float> arr)
{
    float key = 0;
    unsigned long comp_count = 0l;
    unsigned long insertions_count = 0l;
    unsigned long shift_count = 0l;

    auto start = high_resolution_clock::now();
    for (int i = 1; i < arr.size(); i++) {
        int j = i - 1;
        key = arr[i];

        while (j >= 0) {
            if (arr.size() <= 15) { // Вывод малого массива
                print_fvec(arr);
                cout << " Количество сравнений: " << comp_count << "; Количество вставок: " << insertions_count << "; Количество сдвигов: " << shift_count << endl;
            }

            comp_count++;
            if (arr[j] > key) {
                arr[j + 1] = arr[j];
                shift_count++;
            } else {
                break;
            }
            j--;
        }
        if (j + 1 != i) {
            arr[j + 1] = key;
            insertions_count++;
        }
    }
    auto end = high_resolution_clock::now();
    cout << "Результат: ";
    if (arr.size() <= 15) {
        print_fvec(arr);
    }
    cout << " Количество сравнений: " << comp_count << "; Количество вставок: " << insertions_count << "; Количество сдвигов: " << shift_count << endl;
    cout << "Время выполнения: " << duration_cast<microseconds>(end - start).count() << " микросекунд" << endl;
    return arr;
}

vector<float> shell_fvec_sort(vector<float> arr)
{
    int ciura_seq[9] = { 1750, 701, 301, 132, 57, 23, 10, 4, 1 };
    float key = 0;
    unsigned long comp_count = 0l;
    unsigned long insertions_count = 0l;
    unsigned long shift_count = 0l;

    auto start = high_resolution_clock::now();

    for (int gap : ciura_seq) {
        if (gap >= arr.size()) {
            continue;
        }

        for (int i = gap; i < arr.size(); i++) {
            int j = i - gap;
            key = arr[i];

            while (j >= 0) {
                if (arr.size() <= 15) { // Вывод малого массива
                    print_fvec(arr);
                    cout << " Количество сравнений: " << comp_count << "; Количество вставок: " << insertions_count << "; Количество сдвигов: " << shift_count << endl;
                }

                comp_count++;
                if (arr[j] <= key)
                    break;
                arr[j + gap] = arr[j];
                shift_count++;
                j -= gap;
            }
            if (j + gap != i) {
                arr[j + gap] = key;
                insertions_count++;
            }
        }
    }
    auto end = high_resolution_clock::now();
    cout << "Результат: ";
    if (arr.size() <= 15) {
        print_fvec(arr);
    }
    cout << " Количество сравнений: " << comp_count << "; Количество вставок: " << insertions_count << "; Количество сдвигов: " << shift_count << endl;
    cout << "Время выполнения: " << duration_cast<microseconds>(end - start).count() << " микросекунд" << endl;
    return arr;
}

int main()
{
    srand(time(0));
    vector<float> arr = generateRandomVector(20000);
    cout << "Ваш массив: ";
    if (arr.size() <= 15) {
        print_fvec(arr);
    }
    cout << '\n';
    cout << "Сортировка шелла: " << endl;
    vector<float> sorted = shell_fvec_sort(arr);
    cout << "Сортировка вставками: " << endl;
    sorted = insertions_fvec_sort(arr);
    return 0;
}
