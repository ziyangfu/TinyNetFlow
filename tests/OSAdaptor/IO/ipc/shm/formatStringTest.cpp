/*!
 * <= C++17   gcc-9.4
 *      g++ --std=c++17 formatStringTest -o formatStringTest
 * >= C++20   clang-16 && clang-18
 *      clang++-18 --std=c++20 --stdlib=libc++ ./formatStringTest.cpp -o formatStringTestCpp20
 *      */


#include <format>
#include <iostream>
#include <string>
#include <string_view>

template <typename... Args>
std::string dyna_print(std::string_view rt_fmt_str, Args&&... args) {
    return std::vformat(rt_fmt_str, std::make_format_args(args...));
}

int main() {
    std::cout << std::format("Hello {}!\n", "world");

    std::string fmt;
    for (int i{}; i != 3; ++i) {
        fmt += "{} "; // 构造格式化字符串
        std::cout << fmt << " : ";
        std::cout << dyna_print(fmt, "alpha", 'Z', 3.14, "unused");
        std::cout << '\n';
    }

    std::string myformat = "I love {}. It has {} arguments {}, {},kkk,{}";
    std::cout << dyna_print(myformat, "formatting", 2,34,54,"fzy") << std::endl;
}
