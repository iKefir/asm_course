#include <iostream>

#include "trampoline.hpp"

using namespace std;

int main() {

    int b = 123;
    trampoline<int (int)> tr_1([&](int a) { return printf("%d %d\n", a, b); });
    auto p_1 = tr_1.get();
    p_1(5);
    b = 124;
    p_1(6);

    trampoline<int (int)> tr_1c([&](int a) { return printf("%d\n", a); });
    auto p_1c = tr_1c.get();
    p_1c(6);

    tr_1c = move(tr_1);
    p_1c = tr_1c.get();
    p_1c(6); 

    trampoline<double (double, double, double, double, double, double, double, double, double, double)> tr_2([&](double a, double b, double c, double d, double e, double f, double g, double h, double i, double j) { return a + b + c + d + e + f + g + h + i + j; });
    auto p_2 = tr_2.get();
    cout << p_2(1,2,3,4,5,6,7,8,9,10) << endl;
    cout << p_2(1.5,2.5,3.5,4.5,5.5,6.5,7.5,8.5,9.5,10.6) << endl;

    trampoline<int (int, int, int, int, int)> tr_3([&](int a, int b, int c, int d, int e) { return a + b - c + d - e; });
    auto p_3 = tr_3.get();
    cout << p_3(1, 2, 3, 4, 5) << endl;

    trampoline<int (int, int, int, int, int, int)> tr_4([&](int a, int b, int c, int d, int e, int f) { return a + b - c + d - e + f; });
    auto p_4 = tr_4.get();
    cout << p_4(1, 2, 3, 4, 5, 6) << endl;

    trampoline<double (int, double, int, double, int, double, int, double, int, double, int, double, double, double, double)> tr_5([&](int a, double b, int c, double d, int e, double f, int g, double h, int i, double j, int k, double l, double m, double n, double o) { return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o; });
    auto p_5 = tr_5.get();
    cout << p_5(1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7.5, 8.5, 9.6) << endl;

    trampoline<double (int, double, int, double, int, double, int, double, int, double, int, double, double, double, double)> tr_5c(move(tr_5));
    auto p_5c = tr_5c.get();
    cout << p_5c(1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7.5, 8.5, 9.6) << endl;

    return 0;
}