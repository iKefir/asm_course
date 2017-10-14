#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>

template<typename... Args>
struct count_args;

template<>
struct count_args<> {
    const static int INTEGER = 0;
    const static int SSE = 0;
};

template<typename First, typename... Other>
struct count_args<First, Other...> {
    const static int INTEGER = count_args<Other...>::INTEGER + 1;
    const static int SSE = count_args<Other...>::SSE;
};

template<typename... Other>
struct count_args<double, Other...> {
    const static int INTEGER = count_args<Other...>::INTEGER;
    const static int SSE = count_args<Other...>::SSE + 1;
};

template<typename... Other>
struct count_args<float, Other...> {
    const static int INTEGER= count_args<Other...>::INTEGER;
    const static int SSE = count_args<Other...>::SSE + 1;
};

class memory_list {

    static void** head_ptr;
    static const size_t PAGE = 4096;
    static const size_t SIZE = 128;

    static void create_list() {
        size_t i = 0;
        while (true) {
            i += SIZE;
            char* ptr = (char*)head_ptr + i;
            *(void**)(ptr - SIZE) = ptr;
            if (i >= PAGE - SIZE) {
                *(void**)ptr = nullptr;
                return;
            }
        }
    }

    static void* pick_node() {
        if (head_ptr == nullptr) {
            head_ptr = (void**)mmap(nullptr, SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            create_list();
        }
        return head_ptr;
    }

public:

    static void* get_node() {
        void* answer = pick_node();
        head_ptr = (void**)*head_ptr;
        return answer;
    }

    static void return_node(void* ptr) {
        *(void**)ptr = pick_node();
        head_ptr = (void**)ptr;
    }
};

void** memory_list::head_ptr = nullptr;

struct code_writer {
    char* pcode;

    code_writer(void* code) {
        pcode = (char*)code;
    }

    void write(const char* to_write) {
        while (*to_write) {
            *(pcode++) = *(to_write++);
        }
    }

    void write_int(int number) {
        *(int*)pcode = number;
        pcode += 4;
    }

    void write_from_ptr(void* ptr) {
        *(void**)pcode = ptr;
        pcode += 8;
    }

    void write_shifts(const int size) {
        switch (size) {
            case 5:
                write("\x4D\x89\xC1"); //mov r9, r8
            case 4:
                write("\x49\x89\xC8"); //mov r8, rcx
            case 3:
                write("\x48\x89\xD1"); //mov rcx, rdx
            case 2:
                write("\x48\x89\xF2"); //mov rdx, rsi
            case 1:
                write("\x48\x89\xFE"); //mov rsi, rdi
            default:
                return;
        }
    }
};

template <typename T>
struct trampoline {

    template <typename F>
    trampoline(F func)
    {}

    ~trampoline();

    T* get() const;
};

template <typename R, typename... Args>
struct trampoline<R (Args...)> {

    template <typename F>
    trampoline(F const& func)
            : func_obj(new F(func))
            , caller(&do_call<F>)
            , deleter(do_delete<F>) {

        code_writer writer = code_writer(trampoline_malloc());

        int arguments = count_args<Args ...>::INTEGER;
        if (arguments >= 6) {
        	int sse_size = count_args<Args ...>::SSE - 8;
        	if (sse_size < 0) {
        		sse_size = 0;
        	}
            int size = (count_args<Args ...>::INTEGER - 5 + sse_size) << 3;

            //mov r11, [rsp]
            writer.write("\x4C\x8B\x1C\x24");
            //push r9
            writer.write("\x41\x51");
            writer.write_shifts(5);
            //mov rax, rsp
            writer.write("\x48\x89\xE0");
            //add rax, size
            writer.write("\x48\x05"); writer.write_int(size);
            //add rsp, $8
            writer.write("\x48\x81\xC4"); writer.write_int(8);

            char* loop_label = writer.pcode;

            //cmp rax, rsp
            writer.write("\x48\x39\xE0");
            //je break_label
            writer.write("\x74");

            char* break_label = writer.pcode++;

            //add rsp, $8
            writer.write("\x48\x81\xC4"); writer.write_int(8);
            //mov rdi, [rsp]
            writer.write("\x48\x8B\x3C\x24");
            //mov [rsp - 8], rdi
            writer.write("\x48\x89\x7C\x24\xF8");
            //jmp loop_label
            writer.write("\xEB");

            *writer.pcode = loop_label - writer.pcode - 1;
            ++writer.pcode;
            *break_label = writer.pcode - break_label - 1;

            //mov [rsp], r11
            writer.write("\x4C\x89\x1C\x24");
            //sub rsp, size
            writer.write("\x48\x81\xEC"); writer.write_int(size);
            //mov rdi, imm
            writer.write("\x48\xBF"); writer.write_from_ptr(func_obj);
            //mov rax, imm
            writer.write("\x48\xB8"); writer.write_from_ptr((void*)&do_call<F>);
            //call rax
            writer.write("\xFF\xD0");
            //pop r9
            writer.write("\x41\x59");
            //mov r11, [rsp + size - 8]
            writer.write("\x4C\x8B\x9C\x24"); writer.write_int(size - 8);
            //mov [rsp], r11
            writer.write("\x4C\x89\x1C\x24");
            //ret
            writer.write("\xC3");
        } else {
            writer.write_shifts(count_args<Args ...>::INTEGER);
            //mov rdi, imm
            writer.write("\x48\xBF"); writer.write_from_ptr(func_obj);
            //mov rax, imm
            writer.write("\x48\xB8"); writer.write_from_ptr((void*)&do_call<F>);
            //jmp rax
            writer.write("\xFF\xE0");
        }
    }

    ~trampoline() {
        deleter(func_obj);
        trampoline_free();
    }

    trampoline(const trampoline &) = delete;

    trampoline(trampoline &&other) : code(other.code), deleter(other.deleter), func_obj(other.func_obj) {
        other.func_obj = nullptr;
    }

    trampoline& operator=(const trampoline& other) = delete;

    trampoline& operator=(trampoline&& other) {
        func_obj = other.func_obj;
        code = other.code;
        deleter = other.deleter;
        other.func_obj = nullptr;
        return *this;
    }

    R (*get() const)(Args ...args) {
        return (R (*)(Args ...args))code;
    }

private:

    template <typename F>
    static void do_delete(void* obj) {
        delete ((F*)obj);
    }

    template <typename F>
    static R do_call(void* obj, Args ...args) {
        return (*(F*)obj)(args...);
    }

    void* trampoline_malloc() {
        code = memory_list::get_node();
        return code;
    }

    void trampoline_free() {
        memory_list::return_node(code);
    }

    void* code;
    void* func_obj;
    R (*caller)(void* obj,  Args ...args);
    void (*deleter)(void*);
};

#endif // TRAMPOLINE_H
