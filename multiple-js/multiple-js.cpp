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
    printf("%d, %lld", i, GetTickCount64() - t);


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

    auto constSect = std::make_unique<ValueSection>();
    CodeGener cg(constSect.get());

    cg.RegistryFunctionBridge("println",
        [](uint32_t parCount, ValueSection* stack) -> std::unique_ptr<Value> {       // Toy_Println
            for (int i = 0; i < parCount; i++) {
                auto val = stack->Pop();
                if (val->GetType() == ValueType::kString) {
                    printf("%s", val->GetString()->val.c_str());
                }
                else if (val->GetType() == ValueType::kNumber) {
                    printf("%lld", val->GetNumber()->val);
                }
            }
            printf("\n");
            return std::make_unique<NullValue>();
        }
    );

    cg.RegistryFunctionBridge("tick",
        [](uint32_t parCount, ValueSection* stack)->std::unique_ptr<Value> {       // Toy_Tick
            return std::make_unique<NumberValue>(GetTickCount64());
        }
    );


    // printf("%s\n", vvm.Disassembly().c_str());

    cg.Generate(src.get(), constSect.get());

    VM vvm(constSect.get());

    std::cout << vvm.Disassembly() << std::endl;

    vvm.Run();





    //auto exp = parser.ParseExp();

    //int res = CalculationExp(exp.get());

    // printf("%d", res);
}
