#include <Windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <chrono>
#include <random>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/object_impl/cpp_module_object.h>



int main() {
    {
        using namespace mjs;

        Runtime rt;
        auto ctx = Context(&rt);

        ctx.EvalByPath("object.js");

        ctx.ExecuteMicrotasks();

        rt.module_manager().ClearModuleCache();
        // rt.const_pool().clear();

        rt.stack().resize(0);

        std::cout << "Context GC begin..." << std::endl;
        ctx.gc_manager().PrintObjectTree(&ctx);

        std::cout << "Context GC..." << std::endl;
        ctx.gc_manager().GC(&ctx);

        ctx.gc_manager().PrintObjectTree(&ctx);
        std::cout << "Context GC end..." << std::endl;


        //std::cout << "Runtime GC begin..." << std::endl;
        //rt.gc_manager().PrintObjectTree(nullptr);

        //std::cout << "Runtime GC..." << std::endl;
        //rt.gc_manager().GC(nullptr);

        //std::cout << "Runtime GC end..." << std::endl;
        //rt.gc_manager().PrintObjectTree(nullptr);

        auto start = std::chrono::high_resolution_clock::now();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        std::cout << "Elapsed time: " << elapsed.count() << " ms\n";

        printf("%d", 100);

    }

    return 0;
}
