#include <iostream>

using namespace std;

unsigned int w_count(const char* ptr, size_t length) {
	unsigned int answer = 0;
	asm("movss 0x20202020, %%xmm1\n"
		:"=r"(answer)
		:"r"(ptr),"r"(length)
		:);
	return answer;
}

int main() {
	string sample = "Sample string";
	const char * c_arr = sample.c_str();
	if (w_count(c_arr, sample.size()) == 2) {
		cout << "YEAH!" << endl;
	} else {
		cout << "NO!!!" << endl;
	}
}