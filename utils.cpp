#include <iostream>
#include <vector>
using namespace std;

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
