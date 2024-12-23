#include <Windows.h>

#include <iostream>
#include <fstream>

#include "lexer.h"
#include "parser.h"
#include "exp.h"
#include "codegener.h"
#include "vm.h"


int main() {
    using namespace mjs;

    auto t = GetTickCount64();
    int i = 0;
    for (; i < 100000000; i++) {
        ++i;
    }
    printf("%d, %lld\n", i, GetTickCount64() - t);


    std::fstream srcFile;
    srcFile.open(R"(test.js)");

    srcFile.seekg(0, std::ios::end);
    std::streampos length = srcFile.tellg();
    srcFile.seekg(0, std::ios::beg);
    std::vector<char> res(length);
    srcFile.read((char*)res.data(), length);

    Lexer lexer{ res.data() };
    Parser parser(&lexer);

    auto src = parser.ParseSource();

    auto const_pool = std::make_unique<ConstPool>();
    CodeGener cg(const_pool.get());

    cg.RegistryFunctionBridge("println",
        [](uint32_t parCount, StackFrame* stack) -> Value {
            for (int i = 0; i < parCount; i++) {
                auto val = stack->Pop();
                if (val.type() == ValueType::kString) {
                    std::cout << val.string_u8();
                }
                else if (val.type() == ValueType::kNumber) {
                    std::cout << val.number();
                }
                else if (val.type() == ValueType::kU64) {
                    std::cout << val.u64();
                }
            }
            printf("\n");
            return Value();
        }
    );

    cg.RegistryFunctionBridge("tick",
        [](uint32_t par_count, StackFrame* stack) -> Value {
            return Value(GetTickCount64());
        }
    );


    // printf("%s\n", vvm.Disassembly().c_str());

    cg.Generate(src.get());

    VM vvm(const_pool.get());

    std::cout << vvm.Disassembly() << std::endl;

    vvm.Run();





    //auto exp = parser.ParseExp();

    //int res = CalculationExp(exp.get());

    // printf("%d", res);
}
