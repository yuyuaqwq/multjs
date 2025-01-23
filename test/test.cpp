#include <Windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <chrono>
#include <random>

#include "runtime.h"
#include "context.h"

//#include "lexer.h"
//#include "parser.h"
//#include "exp.h"
//#include "codegener.h"
//#include "vm.h"


int main() {
    using namespace mjs;

    std::fstream srcFile;
    srcFile.open(R"(test.js)");

    srcFile.seekg(0, std::ios::end);
    std::streampos length = srcFile.tellg();
    srcFile.seekg(0, std::ios::beg);
    std::vector<char> res(length);
    srcFile.read((char*)res.data(), length);

    Runtime rt;
    //rt.RegistryFunctionBridge("println",
    //    [](uint32_t parCount, StackFrame* stack) -> Value {
    //        for (int i = 0; i < parCount; i++) {
    //            auto val = stack->Pop();
    //            if (val.type() == ValueType::kString) {
    //                std::cout << val.string_u8();
    //            }
    //            else if (val.type() == ValueType::kNumber) {
    //                std::cout << val.number();
    //            }
    //            //else if (val.type() == ValueType::kU64) {
    //            //    std::cout << val.u64();
    //            //}
    //        }
    //        printf("\n");
    //        return Value();
    //    });

    auto ctx = Context(&rt);
    ctx.Eval(res.data());

    auto start = std::chrono::high_resolution_clock::now();


    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Elapsed time: " << elapsed.count() << " ms\n";

    printf("%d", 100);
}
