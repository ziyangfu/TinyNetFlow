/*!
 * <= C++17   gcc-9.4
 *      g++ --std=c++17 formatStringTest.cpp -o formatStringTest
 * >= C++20   clang-16 && clang-18
 *      clang++-18 --std=c++20 --stdlib=libc++ ./formatStringTest.cpp -o formatStringTestCpp20
 *
fzy@fzy-Lenovo:TinyNetFlow/tests/OSAdaptor/IO/ipc/shm$ ./formatStringTest
/tmp/osadaptor_shm_domain_10_port_20
/tmp/osadaptor_shm_domain_100_port_200_pid_18790_count_4

**/

#if __cplusplus >= 202002L
#include <format>
#endif

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <stdarg.h>

std::string formatStringImpl(const std::string &format, va_list args);

#if __cplusplus >= 202002L
template <typename... Args>
std::string dyna_print(std::string_view rt_fmt_str, Args&&... args) {
    return std::vformat(rt_fmt_str, std::make_format_args(args...));
}
#endif

/*!
 * \brief 根据format格式，填入参数，返回完整的字符串
 * \details
 *      - 低于C++20，且没有fmt库 则使用vsnprintf 函数
 *      - va_list 是 C 语言中用来处理可变参数列表的一个特殊类型。当你需要编写一个函数，
 *          且该函数接受数量不确定的参数时，va_list 就非常有用
 * */
std::string formatString(const std::string &format, ...) {
    std::vector<char> buffer(100);
    std::string result;
    va_list args;
    va_start(args, format.c_str());   /** format之后即可变参数列表 */
    int ret = vsnprintf(buffer.data(), buffer.size(), format.c_str(), args);
    if (ret < 0) {
        throw std::runtime_error("vsnprintf failed");
    }
    va_end(args);
    if (static_cast<size_t>(ret) < buffer.size()) {
        result = std::string(buffer.data(), ret);
    }
    return result;
}

int main() {
    const std::string kShmPathFormat {"%s/osadaptor_shm_domain_%u_port_%u"};
    const std::string kClientFormat {"%s/osadaptor_shm_domain_%u_port_%u_pid_%u_count_%u"};
#if __cplusplus < 202002L
    std::string shmPath_1 = formatString(kShmPathFormat, "/tmp", 10, 20);
    std::cout << shmPath_1 << std::endl;
    std::string shmClientPath_1 = formatString(kClientFormat, "/tmp", 100, 200, 18790, 4);
    std::cout << shmClientPath_1 << std::endl;
#endif

#if __cplusplus >= 202002L
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
#endif

    return 0;
}
