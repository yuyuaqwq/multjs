﻿#include <Windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <chrono>
#include <random>

#include <mjs/runtime.h>
#include <mjs/context.h>

int main() {
    using namespace mjs;

    Runtime rt;
    auto ctx = Context(&rt);

    ctx.EvalByPath("arrow_function_test.js");

    ctx.ExecuteMicrotasks();

    // rt.const_pool().clear();

    rt.module_cache().clear();

    ctx.PrintObjectTree();

    ctx.GC();

    ctx.PrintObjectTree();

    auto start = std::chrono::high_resolution_clock::now();


    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Elapsed time: " << elapsed.count() << " ms\n";

    printf("%d", 100);
}
