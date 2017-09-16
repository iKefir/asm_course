#include <iostream>
#include <ctime>

using namespace std;

void* m_memcpy_b(void* to, const void* from, size_t length) {
    asm("movq %0, %%rsi\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rcx\n"
        "rep movsb"
        :
        :"r"(from),"r"(to),"r"(length)
        :"%rsi","%rdi","%rcx"
        );
    return to;
}

const size_t A_SIZE = 1024*1024*1024;

void perf_test(void* (*tested)(void*, const void*, size_t), size_t length) {
    char *mas = new char[A_SIZE];// = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
    char *res = new char[A_SIZE];
    for (int i = 0; i < 11; ++i) mas[i] = 'q';
    mas[A_SIZE - 1] = 'r';
   
    clock_t start = clock();
    tested(res, mas, length);
    cout << clock() - start << endl;
    
    for (int i = 0; i < 11; ++i) cout << res[i];
    cout << endl << res[A_SIZE - 1] << endl;
    delete[] mas;
    delete[] res;
}

int main() {
    perf_test(memcpy, A_SIZE);
    perf_test(m_memcpy_b, A_SIZE);
    return 0;
}


