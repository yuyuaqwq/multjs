# MultJS

MultJS是一个轻量级的JavaScript引擎实现。

## 待完成
- 虚拟机(VM)重构
- 运行时(Runtime)重构
- 对象系统重构
- 内存管理优化
- 更多单元测试

## 构建与运行

### 依赖项
- CMake 3.10+
- C++20兼容的编译器

### 构建步骤
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 运行测试
```bash
cd build
ctest
```