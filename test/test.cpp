#include <Windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <chrono>
#include <random>

#include <mjs/runtime.h>
#include <mjs/context.h>

int main() {
    using namespace mjs;

    std::fstream file;
    file.open(R"(test.js)");
    auto content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());;
    file.close();

    Runtime rt;
    auto ctx = Context(&rt);
    ctx.Eval(content);

    ctx.ExecuteMicrotasks();

    auto start = std::chrono::high_resolution_clock::now();


    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Elapsed time: " << elapsed.count() << " ms\n";

    printf("%d", 100);
}
