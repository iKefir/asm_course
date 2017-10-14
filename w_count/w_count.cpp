#include <iostream>
#include <ctime>
#include <wmmintrin.h>

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
	size_t aligned_ans = 0, unaligned_ans = 0, edge_case_ans = 0;
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

    size_t to_count, tmp, loop_var;
    __m128i spaces, w_part, next_part, big_temp;

	asm(
    "xor %0, %0\n"
    //create whitespace xmm register
    "mov $0x2020202020202020, %4\n"
    "movq %4, %[big_temp]\n"
    "movdqa %[big_temp], %[spaces]\n"
    "pslldq $0x8, %[spaces]\n"
    "xorpd %[big_temp], %[spaces]\n"
    "xorpd %[big_temp], %[big_temp]\n"

    //set counter for loop
    "movq %2, %[loop_var]\n"
    "and $0xf, %2\n"
    "add $0x10, %2\n"
    "shr $0x4, %[loop_var]\n"
    "jz final\n"

    "movdqa (%1), %[w_part]\n"
    "add $0x10, %1\n"
    "pcmpeqb %[spaces], %[w_part]\n"
    "dec %[loop_var]\n"

    "for_cycle:\n"
    "movdqa (%1), %[next_part]\n"
    "add $0x10, %1\n"
    "pcmpeqb %[spaces], %[next_part]\n"
    "movdqa %[next_part], %[big_temp]\n"
    "palignr $0x1, %[w_part], %[big_temp]\n"
    "xorpd %[w_part], %[big_temp]\n"
    //count spaces
    "pmovmskb %[big_temp], %[to_count]\n"
    "popcnt %[to_count], %[to_count]\n"
    "add %[to_count], %0\n"

    "movdqa %[next_part], %[w_part]\n"
    "dec %[loop_var]\n"
    "jnz for_cycle\n"
    "sub $0x10, %1\n"
        
    "final:\n"
    :"=r"(aligned_ans),"=r"(ptr),"=r"(length),[to_count]"=r"(to_count),"=r"(tmp),[loop_var]"=r"(loop_var),[spaces]"=x"(spaces),[w_part]"=x"(w_part),[next_part]"=x"(next_part),[big_temp]"=x"(big_temp)
    :"1"(ptr),"2"(length)
    :);

    if (length > 0) {
        unaligned_ans += stupid_count(ptr, length);

        char unaligned_first = *ptr;
        char aligned_last = *(ptr - 1);
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
