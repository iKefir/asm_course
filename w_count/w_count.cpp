#include <iostream>
#include <ctime>

using namespace std;

size_t stupid_count(const char* ptr, size_t length) {
        size_t answer = 0;
        bool in_word = false;
        char ch;
        for (size_t i = 0; i < length; ++i) {
                ch = *(ptr + i);
                if (ch == ' ') {
                         if (in_word) {
                                ++answer;
                                in_word = false;                                       
                        }
                } else {
                        in_word = true;
                }   
        }
        if (in_word) {
                ++answer;
        }
        return answer;
}

size_t w_count(const char* ptr, size_t length) {
	size_t aligned_ans = 0, ret_len = 0, unaligned_ans = 0, edge_case_ans = 0;
        char* ret_ptr;
        char ch;

        size_t shift = (unsigned long long)ptr % 16;
        bool in_word = false;
        if (shift != 0) {
                unaligned_ans += stupid_count(ptr, 16 - shift);

                ptr += 16 - shift;
                length -= shift;

                char unaligned_last = *(ptr - 1);
                char aligned_first = *ptr;

                if (unaligned_last != ' ' && aligned_first != ' ') {
                        --edge_case_ans;
                } else if (unaligned_last != ' ') {
                        --edge_case_ans;
                } else if (aligned_first != ' ') {
                        ++edge_case_ans;
                }
        } else {
                ++edge_case_ans;
        }

	asm(
        "mov %3, %%r11\n"
        "mov %4, %%r12\n"
        "mov %%r13, %%r13\n"

        //create whitespace xmm register
        "mov $0x2020202020202020, %%r8\n"
        "movq %%r8, %%xmm4\n"
        "movdqa %%xmm4, %%xmm5\n"
        "pslldq $0x8, %%xmm5\n"
        "xorpd %%xmm4, %%xmm5\n"
        "xorpd %%xmm4, %%xmm4\n"

        //set counter for loop
        "movq %%r12, %%rcx\n"
        "and $0xf, %%r12\n"
        "add $0x10, %%r12\n"
        "shr $0x4, %%rcx\n"
        "jz final\n"

        "movdqa (%%r11), %%xmm1\n"
        "add $0x10, %%r11\n"
        "pcmpeqb %%xmm5, %%xmm1\n"
        "sub $0x1, %%rcx\n"

        "for_cycle:\n"
        "movdqa (%%r11), %%xmm2\n"
        "add $0x10, %%r11\n"
        "pcmpeqb %%xmm5, %%xmm2\n"
        "movdqa %%xmm2, %%xmm4\n"
        "palignr $0x1, %%xmm1, %%xmm4\n"
        "xorpd %%xmm1, %%xmm4\n"
        //count spaces
        "pmovmskb %%xmm4, %%eax\n"
        "popcnt %%eax, %%ebx\n"
        "add %%rbx, %%r13\n"

        "movdqa %%xmm2, %%xmm1\n"
        "loop for_cycle\n"
        "sub $0x10, %%r11\n"
        
        "final:\n"
        "mov %%r13, %0\n"
        "mov %%r11, %1\n"
        "mov %%r12, %2\n"
	:"=r"(aligned_ans),"=r"(ret_ptr),"=r"(ret_len)
	:"r"(ptr),"r"(length)
	:"%rax","%rbx","%rcx","%r8","%r9","r10","r11","r12","r13","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5");

        if (ret_len > 0) {
                unaligned_ans += stupid_count(ret_ptr, ret_len);

                char unaligned_first = *ret_ptr;
                char aligned_last = *(ret_ptr - 1);
                if (unaligned_first != ' ' && aligned_last != ' ') {
                        --edge_case_ans;
                } else if (unaligned_first != ' ') {
                        --edge_case_ans;
                } else if (aligned_last != ' ') {
                        ++edge_case_ans;
                }
        } else {
                ++edge_case_ans;
        }

        return (aligned_ans + edge_case_ans) / 2 + unaligned_ans;
}

void perform_test(string sample) {
        const char * c_arr = sample.c_str();
        clock_t start;

        cout << "w_count start:" << endl;
        start = clock();
        unsigned int ans = w_count(c_arr, sample.size());
        cout << "w_count time: " << clock() - start << endl;

        cout << "stupid_count start:" << endl;
        start = clock();
        unsigned int st_ans = stupid_count(c_arr, sample.size());
        cout << "stupid_count time: " << clock() - start << endl;

        if (ans == st_ans) {
                cout << "YEAH!" << endl;
        } else {
                cout << "NO!!!" << endl;
                cout << "asm_ans: " << ans << endl;
                cout << "stupid_ans: " << st_ans << endl;  
        }  
}


int main() {
	string s_1 = "Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string Sample string";
        perform_test(s_1);
        string s_2 = "Sample    string";
        for (int i = 0; i < 100 - 1; ++i) {
                s_2 += "Sample    string";
        }
        perform_test(s_2);
        string s_3 = s_2;
        for (int i = 0; i < 100 - 1; ++i) {
                s_3 += s_2;
        }
        perform_test(s_3);
        string s_4 = s_3;
        for (int i = 0; i < 100 - 1; ++i) {
                s_4 += s_3;
        }
        perform_test(s_4);
        string s_5 = s_4;
        for (int i = 0; i < 100 - 1; ++i) {
                s_5 += s_4;
        }
        perform_test(s_5);
}
