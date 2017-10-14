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

void* m_memcpy_w(void* to, const void* from, size_t length) {
    asm("movq %0, %%rsi\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rcx\n"
        "movq %%rcx, %%rax\n"
        "and $0x1, %%rax\n"
        "shr $0x1, %%rcx\n"
        "rep movsw\n"
        "test $0x1, %%rax\n"
        "jz end_symb\n"
        "movq %%rax, %%rcx\n"
        "rep movsb\n"
        "end_symb:\n"
        :
        :"r"(from),"r"(to),"r"(length)
        :"%rsi","%rdi","%rcx","%rax");
    return to;
}

const size_t A_SIZE = 1024*1024;
char *mas = new char[A_SIZE];
char *res = new char[A_SIZE];

void perf_prep() {
    for (size_t i = 0; i < A_SIZE; ++i) {
        mas[i] = 0;
    }
    for (int i = 0; i < 11; ++i) mas[i] = 'q';
    mas[A_SIZE - 1] = 'r';
}

void perf_test(void* (*tested)(void*, const void*, size_t), size_t length) {
    clock_t start = clock();
    tested(res, mas, length);
    cout << clock() - start << endl;
    
    for (int i = 0; i < 11; ++i) cout << res[i];
    cout << endl << res[A_SIZE - 1] << endl;
}

int main() {
    perf_prep();
    perf_test(m_memcpy_b, A_SIZE);
    perf_test(m_memcpy_w, A_SIZE);
    perf_test(memcpy, A_SIZE);
    return 0;
}


