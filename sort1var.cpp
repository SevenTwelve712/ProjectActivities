#include "utils.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

using namespace std;
using namespace std::chrono;

// ==================== Структура для статистики ====================
struct SortStats {
    long long comparisons = 0;
    long long swaps = 0;
    double time_mc = 0.0;
};

// ==================== Сортировка выбором ====================
vector<float> selectionSort(std::vector<float>& arr)
{
    SortStats stats;
    auto start = std::chrono::high_resolution_clock::now();

    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < n; j++) {
            stats.comparisons++;
            if (arr[j] < arr[minIndex]) {
                minIndex = j;
            }
            if (arr.size() <= 15) {
                print_fvec(arr);
                cout << " Количество сравнений: " << stats.comparisons << "; Количество перестановок: " << stats.swaps << endl;
            }
        }
        if (minIndex != i) {
            stats.swaps++;
            std::swap(arr[i], arr[minIndex]);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    stats.time_mc = duration_cast<microseconds>(end - start).count();
    cout << "Результат: ";
    if (arr.size() <= 15) {
        print_fvec(arr);
    }

    cout << " Количество сравнений: " << stats.comparisons << "; Количество перестановок: " << stats.swaps << endl;
    cout << "Время выполнения: " << stats.time_mc << " микросекунд" << endl;

    arr.insert(arr.begin(), stats.time_mc);
    arr.insert(arr.begin(), stats.swaps);
    arr.insert(arr.begin(), stats.comparisons);

    return arr;
}

// ==================== Быстрая сортировка ====================
// Функция разделения с подсчётом
int partition(std::vector<float>& arr, int low, int high, int& comparisons, int& swaps)
{
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        comparisons++;
        if (arr[j] <= pivot) {
            i++;
            if (i != j) {
                swaps++;
                std::swap(arr[i], arr[j]);
            }
            if (arr.size() < 15) {
                print_fvec(arr);
                cout << " Количество сравнений: " << comparisons << "; Количество перестановок: " << swaps << endl;
            }
        }
    }

    if (i + 1 != high) {
        swaps++;
        std::swap(arr[i + 1], arr[high]);
    }
    return i + 1;
}

// Рекурсивная быстрая сортировка
void quickSortRecursive(std::vector<float>& arr, int low, int high, int& comparisons, int& swaps)
{
    if (low < high) {
        int pi = partition(arr, low, high, comparisons, swaps);
        quickSortRecursive(arr, low, pi - 1, comparisons, swaps);
        quickSortRecursive(arr, pi + 1, high, comparisons, swaps);
    }
}

// Обёртка для быстрой сортировки
std::vector<float> quickSort(std::vector<float>& arr)
{
    int comparisons = 0;
    int swaps = 0;

    auto start = std::chrono::high_resolution_clock::now();

    quickSortRecursive(arr, 0, arr.size() - 1, comparisons, swaps);

    auto end = std::chrono::high_resolution_clock::now();
    double time_mc = duration_cast<microseconds>(end - start).count();

    cout << "Результат: ";
    if (arr.size() <= 15) {
        print_fvec(arr);
    }

    cout << " Количество сравнений: " << comparisons << "; Количество перестановок: " << swaps << endl;
    cout << "Время выполнения: " << time_mc << " микросекунд" << endl;

    arr.insert(arr.begin(), time_mc);
    arr.insert(arr.begin(), swaps);
    arr.insert(arr.begin(), comparisons);
    return arr;
}

// ==================== Вспомогательные функции ====================
// Генерация случайного массива
std::vector<float> generateRandomArray(int size, int minVal = 0, int maxVal = 1000)
{
    std::vector<float> arr(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(minVal, maxVal);

    for (int i = 0; i < size; i++) {
        arr[i] = distrib(gen);
    }
    return arr;
}
