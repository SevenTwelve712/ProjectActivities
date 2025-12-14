#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

// ==================== Структура для статистики ====================
struct SortStats {
    long long comparisons = 0;
    long long swaps = 0;
    double time_ms = 0.0;
};

// ==================== Сортировка выбором ====================
SortStats selectionSort(std::vector<int>& arr)
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
        }
        if (minIndex != i) {
            stats.swaps++;
            std::swap(arr[i], arr[minIndex]);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    stats.time_ms = std::chrono::duration<double, std::milli>(end - start).count();

    return stats;
}

// ==================== Быстрая сортировка ====================
// Функция разделения с подсчётом
int partition(std::vector<int>& arr, int low, int high, SortStats& stats)
{
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        stats.comparisons++;
        if (arr[j] <= pivot) {
            i++;
            if (i != j) {
                stats.swaps++;
                std::swap(arr[i], arr[j]);
            }
        }
    }

    if (i + 1 != high) {
        stats.swaps++;
        std::swap(arr[i + 1], arr[high]);
    }
    return i + 1;
}

// Рекурсивная быстрая сортировка
void quickSortRecursive(std::vector<int>& arr, int low, int high, SortStats& stats)
{
    if (low < high) {
        int pi = partition(arr, low, high, stats);
        quickSortRecursive(arr, low, pi - 1, stats);
        quickSortRecursive(arr, pi + 1, high, stats);
    }
}

// Обёртка для быстрой сортировки
SortStats quickSort(std::vector<int>& arr)
{
    SortStats stats;
    auto start = std::chrono::high_resolution_clock::now();

    quickSortRecursive(arr, 0, arr.size() - 1, stats);

    auto end = std::chrono::high_resolution_clock::now();
    stats.time_ms = std::chrono::duration<double, std::milli>(end - start).count();

    return stats;
}

// ==================== Вспомогательные функции ====================
// Генерация случайного массива
std::vector<int> generateRandomArray(int size, int minVal = 0, int maxVal = 1000)
{
    std::vector<int> arr(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(minVal, maxVal);

    for (int i = 0; i < size; i++) {
        arr[i] = distrib(gen);
    }
    return arr;
}

// Печать статистики
void printStats(const std::string& algorithm, const SortStats& stats, int size)
{
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "[" << algorithm << " для n=" << size << "]\n";
    std::cout << "  Time: " << stats.time_ms << " ms\n";
    std::cout << "  Comparisons: " << stats.comparisons << "\n";
    std::cout << "  Swaps: " << stats.swaps << "\n";
}

// Печать массива (для маленьких размеров)
void printArray(const std::vector<int>& arr, const std::string& label = "")
{
    if (!label.empty()) {
        std::cout << label << ": ";
    }
    for (size_t i = 0; i < std::min(arr.size(), (size_t)20); i++) {
        std::cout << arr[i] << " ";
    }
    if (arr.size() > 20) {
        std::cout << "... (total " << arr.size() << " elements)";
    }
    std::cout << std::endl;
}

// Проверка отсортированности
bool isSorted(const std::vector<int>& arr, bool ascending = true)
{
    for (size_t i = 1; i < arr.size(); i++) {
        if (ascending && arr[i] < arr[i - 1])
            return false;
        if (!ascending && arr[i] > arr[i - 1])
            return false;
    }
    return true;
}

// ==================== Основная функция тестирования ====================
int main()
{
    std::cout << "========================================================\n";
    std::cout << "        TESTING OF SORTING ALGORITHMS\n";
    std::cout << "        (Ascending order: easy and fast)\n";
    std::cout << "========================================================\n\n";

    // Маленький массив для демонстрации
    {
        std::cout << "1. WORK DEMONSTRATION (small array):\n";
        std::vector<int> smallArray = { 64, 25, 12, 22, 11, 90, 45, 33, 18, 7 };

        std::vector<int> arr1 = smallArray;
        std::vector<int> arr2 = smallArray;

        printArray(arr1, "Original array");

        // Сортировка выбором
        SortStats stats1 = selectionSort(arr1);
        printArray(arr1, "After sorting by choice");
        printStats("Sorting by choice", stats1, arr1.size());
        std::cout << "  Correctness: " << (isSorted(arr1) ? "OK" : "ERROR") << "\n\n";

        // Быстрая сортировка
        SortStats stats2 = quickSort(arr2);
        printArray(arr2, "After a quick sort");
        printStats("Quick sorting", stats2, arr2.size());
        std::cout << "  Correctness: " << (isSorted(arr2) ? "OK" : "ERROR") << "\n\n";
    }

    // Тестирование на разных размерах
    std::cout << "\n2. TESTING ON DIFFERENT ARRAY SIZES:\n";
    std::cout << "===========================================\n";

    std::vector<int> testSizes = { 10, 100, 1000 };

    for (int size : testSizes) {
        std::cout << "\n--- Size of the array: " << size << " ---\n";

        // Генерируем массивы
        std::vector<int> randomArray = generateRandomArray(size);
        std::vector<int> arr1 = randomArray;
        std::vector<int> arr2 = randomArray;
        std::vector<int> arr3 = randomArray; // для std::sort

        // Сортировка выбором
        std::cout << "\nа) Random array:\n";
        SortStats stats1 = selectionSort(arr1);
        printStats("  Sorting by choice", stats1, size);

        // Быстрая сортировка
        SortStats stats2 = quickSort(arr2);
        printStats("  Quick sorting", stats2, size);

        // Стандартная сортировка для сравнения
        auto start = std::chrono::high_resolution_clock::now();
        std::sort(arr3.begin(), arr3.end());
        auto end = std::chrono::high_resolution_clock::now();
        double time_std = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "  std::sort time: " << time_std << " ms\n";

        // Проверка корректности
        bool correct1 = isSorted(arr1);
        bool correct2 = isSorted(arr2);
        std::cout << "  Check: choice=" << (correct1 ? "OK" : "ERROR")
                  << ", quick=" << (correct2 ? "OK" : "ERROR") << "\n";
    }

    // Таблица сравнения для больших массивов
    std::cout << "\n\n3. COMPARISON TABLE:\n";
    std::cout << "================================================================================\n";
    std::cout << std::setw(8) << "Size"
              << std::setw(20) << "Algorithm"
              << std::setw(12) << "Time(ms)"
              << std::setw(15) << "Comparisons"
              << std::setw(15) << "Swaps"
              << std::setw(10) << "C/n"
              << std::setw(10) << "S/n" << "\n";
    std::cout << "================================================================================\n";

    std::vector<int> finalSizes = { 100, 1000, 10000 };
    for (int size : finalSizes) {
        std::vector<int> arr = generateRandomArray(size);
        std::vector<int> arr1 = arr;
        std::vector<int> arr2 = arr;

        // Сортировка выбором
        SortStats stats1 = selectionSort(arr1);
        std::cout << std::setw(8) << size
                  << std::setw(20) << "Choice"
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats1.time_ms
                  << std::setw(15) << stats1.comparisons
                  << std::setw(15) << stats1.swaps
                  << std::setw(10) << std::setprecision(1) << (double)stats1.comparisons / size
                  << std::setw(10) << std::setprecision(1) << (double)stats1.swaps / size << "\n";

        // Быстрая сортировка
        SortStats stats2 = quickSort(arr2);
        std::cout << std::setw(8) << size
                  << std::setw(20) << "Quick"
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats2.time_ms
                  << std::setw(15) << stats2.comparisons
                  << std::setw(15) << stats2.swaps
                  << std::setw(10) << std::setprecision(1) << (double)stats2.comparisons / size
                  << std::setw(10) << std::setprecision(1) << (double)stats2.swaps / size << "\n";

        std::cout << "--------------------------------------------------------------------------------\n";
    }

    std::cout << "\nThe program is complete. Press Enter to exit...";
    std::cin.get();

    return 0;
}
