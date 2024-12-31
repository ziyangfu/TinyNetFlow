/*!
 * ubuntu20.04 core i5 8代
 * output：
        std::string time: 0.0679161 s
        std::vector<char> time: 0.0596953 s
 * */
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>

void benchmark_string() {
    std::string str;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(33, 126);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        str += static_cast<char>(dis(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "std::string time: " << elapsed.count() << " s\n";
}

void benchmark_vector() {
    std::vector<char> vec;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(33, 126);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        vec.push_back(static_cast<char>(dis(gen)));
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "std::vector<char> time: " << elapsed.count() << " s\n";
}

int main() {
    benchmark_string();
    benchmark_vector();
    return 0;
}
