# MultJS Project Guide for AI Agents

## Project Overview

**MultJS** is a lightweight JavaScript engine implementation written in C++20. The project consists of two main components:

1. **JavaScript Engine Core**: A complete JS engine with lexer, parser, compiler, VM, and runtime
2. **JavaScript-to-C++ Transpiler (cpp_gen)**: A transpiler that converts JavaScript code to high-performance C++ code

### Key Features
- Full JavaScript parsing and execution pipeline (Lexer → Parser → AST → Bytecode → VM)
- ES6+ support including modules, classes, async/await, generators, and Promises
- JavaScript-to-C++ transpilation with type inference
- Garbage collection and memory management
- Microtask queue for Promise handling

## Technology Stack

- **Language**: C++20
- **Build System**: CMake 3.10+
- **Testing Framework**: GoogleTest (v1.13.0)
- **Project Type**: Static library with CLI tools

## Project Structure

```
multjs/
├── include/mjs/              # Public headers
│   ├── value.h               # JavaScript value type system
│   ├── context.h             # Execution context
│   ├── vm.h                  # Virtual machine
│   ├── runtime.h             # Runtime environment
│   ├── object.h              # Object system
│   ├── opcode.h              # Bytecode opcodes
│   └── object/          # Object implementations
│
├── src/                      # Source files
│   ├── compiler/             # Compiler components
│   │   ├── lexer.cpp/h       # Lexical analyzer
│   │   ├── parser.cpp/h      # Syntax analyzer (AST builder)
│   │   ├── code_generator.cpp/h  # Bytecode generator
│   │   ├── expression.cpp/h  # Expression AST nodes
│   │   ├── statement.cpp/h   # Statement AST nodes
│   │   ├── expression_impl/  # Expression implementations
│   │   └── statement_impl/   # Statement implementations
│   ├── object/          # Object implementations
│   ├── vm.cpp                # Virtual machine implementation
│   ├── context.cpp           # Context implementation
│   └── runtime.cpp           # Runtime implementation
│
├── cpp_gen/                  # JavaScript-to-C++ transpiler
│   ├── cpp_code_generator.h/cpp  # Main C++ code generator
│   ├── type_inference_engine.h/cpp  # Type inference
│   ├── cpp_type.h/cpp        # C++ type system
│   ├── code_emitter.h/cpp    # Code formatting
│   ├── name_mangler.h/cpp    # Name mangling for C++ keywords
│   └── mjs_runtime.h         # Runtime support library
│
├── tools/                    # CLI tools
│   └── js2cpp.cpp            # JavaScript to C++ transpiler CLI
│
├── tests/                    # Test suite
│   ├── unit/                 # Unit tests
│   ├── integration/          # Integration tests
│   └── cpp_gen/              # C++ generator tests
│
├── examples/                 # Example files
├── docs/                     # Documentation
├── CMakeLists.txt            # Main CMake configuration
└── build.bat                 # Windows build script
```

## Build Instructions

### Prerequisites
- CMake 3.10 or higher
- C++20 compatible compiler (MSVC, GCC, or Clang)
- Internet connection (for fetching GoogleTest)

### Build Commands

```bash
# Windows (using build.bat)
build.bat
cd build
cmake --build .

# Or manual build
mkdir build
cd build
cmake ..
cmake --build .
```

### Build Outputs
- `mjs` - Main JavaScript engine static library
- `mjs_cpp_gen` - C++ transpiler static library
- `js2cpp` - Command-line transpiler tool
- `unit_tests` - Unit test executable
- `integration_tests` - Integration test executable
- `cpp_gen_tests` - C++ generator test executable

## Testing

### Running Tests

```bash
cd build

# Run all tests
ctest

# Run specific test suites
./unit_tests
./integration_tests
./cpp_gen_tests

# Run with filters
./unit_tests --gtest_filter=LexerTest.*
./integration_tests --gtest_filter=BasicIntegrationTest.*

# Run with detailed output
./unit_tests --gtest_print_time=1 --gtest_color=yes
```

### Test Coverage Analysis

```bash
# Check test coverage mapping (requires bash)
bash check_coverage.sh
```

## Code Style Guidelines

### Doxygen Documentation
The project uses Doxygen for documentation. See `DOXYGEN_STYLE_GUIDE.md` for detailed conventions.

**Key conventions:**
- All public APIs must have Doxygen comments
- Comments are written in **Chinese**
- Technical terms remain in English
- Use `@brief`, `@param`, `@return`, `@throw`, `@note`, `@warning` tags
- File headers must include copyright and license

### Example Header Template:
```cpp
/**
 * @file filename.h
 * @brief Brief description
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * Detailed description...
 */
```

### Coding Standards
- C++20 features are encouraged
- Use `snake_case` for functions and variables
- Use `PascalCase` for class names
- Use `SCREAMING_SNAKE_CASE` for macros and constants
- Indentation: 4 spaces (no tabs)
- Maximum line length: 120 characters
- Always use `#pragma once` for header guards

## Architecture Overview

### JavaScript Execution Pipeline

```
JavaScript Source
       ↓
   Lexer (词法分析)
       ↓
   Tokens
       ↓
   Parser (语法分析)
       ↓
   AST (抽象语法树)
       ↓
   CodeGenerator (代码生成)
       ↓
   Bytecode (字节码)
       ↓
   VM (虚拟机执行)
       ↓
   Result
```

### Key Components

1. **Value System** (`include/mjs/value.h`)
   - Unified JavaScript value representation
   - Supports all JS types: undefined, null, boolean, number, string, symbol, object
   - Reference counting for memory management

2. **Context** (`include/mjs/context.h`)
   - Execution context manager
   - Module compilation and execution
   - Microtask queue management
   - Garbage collection coordination

3. **VM** (`include/mjs/vm.h`)
   - Bytecode interpreter
   - Function call management
   - Stack frame handling
   - Generator and async support

4. **Object System** (`include/mjs/object.h` and `object/`)
   - Object, Array, Function objects
   - Promise and Generator objects
   - Module and Class support

### Bytecode System

The VM executes bytecode defined in `include/mjs/opcode.h`. Key opcode categories:
- **Constants**: `kCLoad`, `kCLoad_0` ~ `kCLoad_5`
- **Variables**: `kVLoad`, `kVStore`, `kGetGlobal`
- **Arithmetic**: `kAdd`, `kSub`, `kMul`, `kDiv`, `kMod`
- **Comparison**: `kEq`, `kNe`, `kLt`, `kGt`, `kLe`, `kGe`
- **Control Flow**: `kGoto`, `kIfEq`, `kReturn`
- **Functions**: `kFunctionCall`, `kClosure`
- **Async**: `kAwait`, `kYield`, `kAsyncReturn`
- **Exceptions**: `kTryBegin`, `kThrow`, `kTryEnd`

## JavaScript-to-C++ Transpiler (cpp_gen)

### Purpose
Converts JavaScript code to high-performance C++ code using static type inference.

### Architecture
```
JavaScript Source
       ↓
   Parser (existing AST)
       ↓
   TypeInferenceEngine
       ↓
   CppCodeGenerator
       ↓
   C++ Source Code
```

### Key Files
- `cpp_gen/cpp_code_generator.h/cpp` - Main generator
- `cpp_gen/type_inference_engine.h/cpp` - Type inference
- `cpp_gen/cpp_type.h/cpp` - C++ type system
- `cpp_gen/code_emitter.h/cpp` - Code formatting
- `cpp_gen/name_mangler.h/cpp` - Handle C++ keyword conflicts
- `cpp_gen/mjs_runtime.h` - Runtime support (JSValue, JSObject, JSArray)

### CLI Tool Usage
```bash
# Basic usage
./js2cpp input.js -o output.cpp

# With options
./js2cpp input.js --namespace game_logic --indent 4 -o output.cpp

# Disable type inference
./js2cpp input.js --no-type-inference -o output.cpp
```

### API Usage
```cpp
#include "cpp_gen/cpp_code_generator.h"
#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"

mjs::compiler::Lexer lexer(js_code);
mjs::compiler::Parser parser(&lexer);
parser.ParseProgram();

mjs::compiler::cpp_gen::CppCodeGenerator::Config config;
config.namespace_name = "game";
config.enable_type_inference = true;

mjs::compiler::cpp_gen::CppCodeGenerator generator(config);
std::string cpp_code = generator.Generate(parser);
```

## Module System

The engine supports ES6-style modules:
- `import` declarations
- `export` declarations (named and default)
- Module loading via `ModuleManager`
- C++ module bindings via `CppModuleObject`

### Example C++ Module Binding
```cpp
// Create a C++ module
auto module = context->CreateCppModule("my_module");
module->AddFunction("my_func", [](Context* ctx, uint32_t count, const StackFrame& stack) {
    // Implementation
    return Value(42);
});
```

## Testing Strategy

### Unit Tests
- Test individual classes/functions
- Located in `tests/unit/`
- Named `{component}_test.cpp`

### Integration Tests
- End-to-end JavaScript execution tests
- Located in `tests/integration/`
- Test fixtures in `tests/integration/fixtures/`

### Test Categories
1. **Basic Integration**: Variables, types, operators
2. **Function Integration**: Functions, closures, generators
3. **Class Integration**: Classes, inheritance, prototypes
4. **Async Integration**: Promises, async/await, microtasks
5. **Exception Integration**: try/catch/finally, error objects
6. **Performance Integration**: Memory, GC, large-scale operations

## Development Workflow

### Adding New Features
1. Implement in appropriate `src/` directory
2. Add unit tests in `tests/unit/`
3. Add integration tests if needed
4. Update documentation (Doxygen comments)
5. Run full test suite before committing

### Debugging Tips
- Use `VM` friend test classes for debugging
- Enable debug output in VM with appropriate flags
- Use `stack_frame.Dump()` for stack inspection
- Check `examples/` for usage patterns

## Important Notes

1. **Language**: All comments and documentation are in **Chinese**
2. **Thread Safety**: Context and Runtime are NOT thread-safe
3. **Memory Management**: Uses reference counting + GC for objects
4. **New Object Types**: Must be added to `Value::IsObject()` to prevent memory leaks
5. **Exception Handling**: Uses exception marking on Value objects, not C++ exceptions

## References

- `README.md` - Basic project info
- `PROGRESS_SUMMARY.md` - C++ transpiler progress
- `CPP_TRANSLATOR_PLAN.md` - C++ transpiler design
- `INTEGRATION_TEST_PLAN.md` - Testing strategy
- `DOXYGEN_STYLE_GUIDE.md` - Documentation conventions
- `examples/` - Usage examples

## License

MIT License - Copyright (c) 2025 yuyuaqwq
