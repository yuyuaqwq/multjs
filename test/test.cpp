#include <Windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <chrono>
#include <random>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/object_impl/cpp_module_object.h>

class ConsoleObject : public mjs::Object {
public:
    ConsoleObject(mjs::Runtime* rt) 
        : Object(rt)
    {
        auto log_const_index = rt->const_pool().insert(mjs::Value("log"));
        SetProperty(nullptr, log_const_index, mjs::Value([](mjs::Context* context, uint32_t par_count, const mjs::StackFrame& stack) -> mjs::Value {
            for (size_t i = 0; i < par_count; i++) {
                auto val = stack.get(i);
                try {
                    std::cout << val.ToString(context).string_view();
                }
                catch (const std::exception&)
                {
                    std::cout << "unknown";
                }
            }
            printf("\n");
            return mjs::Value();
        }));
    }
};

int main() {
    using namespace mjs;

    Runtime rt;
    auto const_idx = rt.const_pool().insert(Value("globalThis"));
    auto globalThis = rt.global_this();
    rt.global_this().object().SetProperty(nullptr, const_idx, std::move(globalThis));
    const_idx = rt.const_pool().insert(Value("console"));
    rt.global_this().object().SetProperty(nullptr, const_idx, Value(new ConsoleObject(&rt)));

    auto ctx = Context(&rt);

    ctx.EvalByPath("module1.js");

    ctx.ExecuteMicrotasks();

    // rt.const_pool().clear();

    rt.module_mgr().ClearModuleCache();

    ctx.PrintObjectTree();

    std::cout << "GC..." << std::endl;
    ctx.GC();

    ctx.PrintObjectTree();

    auto start = std::chrono::high_resolution_clock::now();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Elapsed time: " << elapsed.count() << " ms\n";

    printf("%d", 100);
}
