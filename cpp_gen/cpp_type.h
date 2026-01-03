/**
 * @file cpp_type.h
 * @brief C++类型系统，表示JavaScript类型到C++类型的映射
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace mjs {
namespace compiler {
namespace cpp_gen {

// 前向声明
class CppType;

/**
 * @struct ObjectPropertyType
 * @brief 对象属性类型定义
 */
struct ObjectPropertyType {
    std::string name;                              ///< 属性名
    std::shared_ptr<CppType> type;                ///< 属性类型
};

/**
 * @enum CppTypeCategory
 * @brief C++类型类别
 */
enum class CppTypeCategory {
    kPrimitive,    // 基本类型：int64_t, double, bool, std::string
    kValue,        // 动态类型：mjs::Value (回退选项)
    kArray,        // 数组类型：std::vector<T>
    kOptional,     // 可选类型：std::optional<T>
    kUnion,        // 联合类型：std::variant<Ts...>
    kFunction,     // 函数类型
    kVoid,         // void类型
    kObject        // 对象结构体类型
};

/**
 * @class CppType
 * @brief C++类型表示类
 */
class CppType {
public:
    /**
     * @brief 创建int64_t类型
     */
    static CppType Int64();

    /**
     * @brief 创建double类型
     */
    static CppType Float64();

    /**
     * @brief 创建bool类型
     */
    static CppType Boolean();

    /**
     * @brief 创建string类型
     */
    static CppType String();

    /**
     * @brief 创建void类型
     */
    static CppType Void();

    /**
     * @brief 创建动态Value类型
     */
    static CppType Value();

    /**
     * @brief 创建数组类型
     * @param element 元素类型
     */
    static CppType Array(std::shared_ptr<CppType> element);

    /**
     * @brief 创建可选类型
     * @param inner 内部类型
     */
    static CppType Optional(std::shared_ptr<CppType> inner);

    /**
     * @brief 创建联合类型
     * @param alternatives 可能的类型列表
     */
    static CppType Union(const std::vector<std::shared_ptr<CppType>>& alternatives);

    /**
     * @brief 创建函数类型
     * @param params 参数类型列表
     * @param return_type 返回值类型
     */
    static CppType Function(const std::vector<std::shared_ptr<CppType>>& params,
                           std::shared_ptr<CppType> return_type);

    /**
     * @brief 创建对象结构体类型
     * @param struct_name 结构体名称
     * @param properties 属性列表
     */
    static CppType Object(const std::string& struct_name,
                         const std::vector<ObjectPropertyType>& properties);

    /**
     * @brief 获取类型类别
     */
    CppTypeCategory category() const { return category_; }

    /**
     * @brief 生成C++类型名称字符串
     */
    std::string ToString() const;

    /**
     * @brief 合并两个类型（用于类型推断）
     * @param other 另一个类型
     * @return 合并后的类型
     */
    CppType Merge(const CppType& other) const;

    /**
     * @brief 检查是否为基本类型
     */
    bool IsPrimitive() const { return category_ == CppTypeCategory::kPrimitive; }

    /**
     * @brief 检查是否为动态类型
     */
    bool IsValue() const { return category_ == CppTypeCategory::kValue; }

    /**
     * @brief 检查是否为数组类型
     */
    bool IsArray() const { return category_ == CppTypeCategory::kArray; }

    /**
     * @brief 检查是否为对象结构体类型
     */
    bool IsObject() const { return category_ == CppTypeCategory::kObject; }

    /**
     * @brief 获取数组元素类型
     */
    const std::shared_ptr<CppType>& GetElementType() const;

    /**
     * @brief 获取可选类型的内部类型
     */
    const std::shared_ptr<CppType>& GetOptionalType() const;

    /**
     * @brief 获取联合类型的所有选项
     */
    const std::vector<std::shared_ptr<CppType>>& GetUnionAlternatives() const;

    /**
     * @brief 获取函数参数类型列表
     */
    const std::vector<std::shared_ptr<CppType>>& GetParameterTypes() const;

    /**
     * @brief 获取函数返回值类型
     */
    const std::shared_ptr<CppType>& GetReturnType() const;

    /**
     * @brief 获取对象结构体名称
     */
    const std::string& GetStructName() const;

    /**
     * @brief 获取对象结构体属性列表
     */
    const std::vector<ObjectPropertyType>& GetObjectProperties() const;

    /**
     * @brief 比较两个类型是否相等
     */
    bool Equals(const CppType& other) const;

private:
    /**
     * @brief 私有构造函数
     */
    CppType(CppTypeCategory category, const std::string& name);

    CppType(CppTypeCategory category,
            std::shared_ptr<CppType> inner_type);

    CppType(CppTypeCategory category,
            const std::vector<std::shared_ptr<CppType>>& types);

    CppType(CppTypeCategory category,
            const std::string& struct_name,
            const std::vector<ObjectPropertyType>& properties);

    CppTypeCategory category_;
    std::string name_;  // 用于基本类型和结构体名称

    // 复杂类型的子类型
    std::vector<std::shared_ptr<CppType>> sub_types_;

    // 对象类型的属性信息
    std::vector<ObjectPropertyType> object_properties_;
};

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs
