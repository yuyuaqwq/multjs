/**
 * @file cpp_type.cpp
 * @brief CppType实现
 */

#include "cpp_gen/cpp_type.h"
#include <algorithm>
#include <stdexcept>

namespace mjs {
namespace compiler {
namespace cpp_gen {

namespace {

// C++关键字集合，用于名称修饰
const std::vector<std::string> kCppKeywords = {
    "class", "int", "float", "double", "bool", "void", "char", "wchar_t",
    "if", "else", "for", "while", "do", "switch", "case", "break", "continue",
    "return", "goto", "default", "sizeof", "typeid", "typename", "auto",
    "const", "volatile", "static", "extern", "register", "mutable", "inline",
    "virtual", "explicit", "friend", "public", "protected", "private",
    "template", "namespace", "using", "typedef", "struct", "union", "enum",
    "class", "operator", "this", "super", "new", "delete", "true", "false",
    "nullptr", "nullptr_t", "throw", "try", "catch", "and", "or", "not",
    "xor", "bitand", "bitor", "compl", "and_eq", "or_eq", "xor_eq", "not_eq"
};

} // anonymous namespace

// ========== 静态工厂方法 ==========

CppType CppType::Int64() {
    return CppType(CppTypeCategory::kPrimitive, "int64_t");
}

CppType CppType::Float64() {
    return CppType(CppTypeCategory::kPrimitive, "double");
}

CppType CppType::Boolean() {
    return CppType(CppTypeCategory::kPrimitive, "bool");
}

CppType CppType::String() {
    return CppType(CppTypeCategory::kPrimitive, "std::string");
}

CppType CppType::Void() {
    return CppType(CppTypeCategory::kVoid, "void");
}

CppType CppType::Value() {
    return CppType(CppTypeCategory::kValue, "mjs::generated::JSValue");
}

CppType CppType::Array(std::shared_ptr<CppType> element) {
    if (!element) {
        throw std::invalid_argument("Array element type cannot be null");
    }
    return CppType(CppTypeCategory::kArray, element);
}

CppType CppType::Optional(std::shared_ptr<CppType> inner) {
    if (!inner) {
        throw std::invalid_argument("Optional inner type cannot be null");
    }
    return CppType(CppTypeCategory::kOptional, inner);
}

CppType CppType::Union(const std::vector<std::shared_ptr<CppType>>& alternatives) {
    if (alternatives.empty()) {
        throw std::invalid_argument("Union type must have at least one alternative");
    }
    return CppType(CppTypeCategory::kUnion, alternatives);
}

CppType CppType::Function(const std::vector<std::shared_ptr<CppType>>& params,
                         std::shared_ptr<CppType> return_type) {
    std::vector<std::shared_ptr<CppType>> types = params;
    types.push_back(return_type);
    return CppType(CppTypeCategory::kFunction, types);
}

CppType CppType::Object(const std::string& struct_name,
                       const std::vector<ObjectPropertyType>& properties) {
    return CppType(CppTypeCategory::kObject, struct_name, properties);
}

// ========== 私有构造函数 ==========

CppType::CppType(CppTypeCategory category, const std::string& name)
    : category_(category), name_(name) {}

CppType::CppType(CppTypeCategory category, std::shared_ptr<CppType> inner_type)
    : category_(category), name_("") {
    sub_types_.push_back(std::move(inner_type));
}

CppType::CppType(CppTypeCategory category,
                const std::vector<std::shared_ptr<CppType>>& types)
    : category_(category), name_(""), sub_types_(types) {}

CppType::CppType(CppTypeCategory category,
                const std::string& struct_name,
                const std::vector<ObjectPropertyType>& properties)
    : category_(category), name_(struct_name), object_properties_(properties) {}

// ========== 公共方法 ==========

std::string CppType::ToString() const {
    switch (category_) {
        case CppTypeCategory::kPrimitive:
            return name_;

        case CppTypeCategory::kVoid:
            return "void";

        case CppTypeCategory::kValue:
            return "mjs::generated::JSValue";

        case CppTypeCategory::kArray:
            // 数组类型使用 mjs::Value 包装 mjs::ArrayObject
            return "mjs::Value";

        case CppTypeCategory::kOptional:
            if (sub_types_.empty()) {
                throw std::runtime_error("Optional type missing inner type");
            }
            return "std::optional<" + sub_types_[0]->ToString() + ">";

        case CppTypeCategory::kUnion: {
            if (sub_types_.empty()) {
                throw std::runtime_error("Union type missing alternatives");
            }
            std::string result = "std::variant<";
            for (size_t i = 0; i < sub_types_.size(); ++i) {
                if (i > 0) result += ", ";
                result += sub_types_[i]->ToString();
            }
            result += ">";
            return result;
        }

        case CppTypeCategory::kFunction:
            // 函数类型通常不需要直接转换成字符串
            // 返回签名格式：ReturnType(Param1, Param2, ...)
            {
                if (sub_types_.empty()) {
                    throw std::runtime_error("Function type missing return type");
                }
                auto return_type = sub_types_.back();
                std::string result = return_type->ToString() + "(";
                for (size_t i = 0; i < sub_types_.size() - 1; ++i) {
                    if (i > 0) result += ", ";
                    result += sub_types_[i]->ToString();
                }
                result += ")";
                return result;
            }

        case CppTypeCategory::kObject:
            // 对象类型使用 mjs::Value 包装，符合 mjs GC 集成要求
            // mjs::Value 通过引用计数管理对象生命周期
            return "mjs::Value";

        default:
            return "/* unknown type */";
    }
}

CppType CppType::Merge(const CppType& other) const {
    // 如果类型完全相同，直接返回
    if (Equals(other)) {
        return *this;
    }

    // 如果其中一个是Value，结果总是Value
    if (IsValue() || other.IsValue()) {
        return Value();
    }

    // int64 + double = double
    if (category_ == CppTypeCategory::kPrimitive &&
        other.category_ == CppTypeCategory::kPrimitive) {
        if (name_ == "int64_t" && other.name_ == "double") {
            return Float64();
        }
        if (name_ == "double" && other.name_ == "int64_t") {
            return Float64();
        }
    }

    // 其他情况回退到联合类型或Value
    // 简化处理：直接回退到Value
    return Value();
}

bool CppType::Equals(const CppType& other) const {
    if (category_ != other.category_) {
        return false;
    }

    switch (category_) {
        case CppTypeCategory::kPrimitive:
        case CppTypeCategory::kVoid:
            return name_ == other.name_;

        case CppTypeCategory::kValue:
            return true;

        case CppTypeCategory::kArray:
        case CppTypeCategory::kOptional:
            if (sub_types_.size() != other.sub_types_.size()) {
                return false;
            }
            return sub_types_[0]->Equals(*other.sub_types_[0]);

        case CppTypeCategory::kUnion:
        case CppTypeCategory::kFunction:
            if (sub_types_.size() != other.sub_types_.size()) {
                return false;
            }
            for (size_t i = 0; i < sub_types_.size(); ++i) {
                if (!sub_types_[i]->Equals(*other.sub_types_[i])) {
                    return false;
                }
            }
            return true;

        case CppTypeCategory::kObject:
            // 对象类型比较名称和属性
            if (name_ != other.name_) {
                return false;
            }
            if (object_properties_.size() != other.object_properties_.size()) {
                return false;
            }
            for (size_t i = 0; i < object_properties_.size(); ++i) {
                if (object_properties_[i].name != other.object_properties_[i].name) {
                    return false;
                }
                if (!object_properties_[i].type->Equals(*other.object_properties_[i].type)) {
                    return false;
                }
            }
            return true;

        default:
            return false;
    }
}

const std::shared_ptr<CppType>& CppType::GetElementType() const {
    if (category_ != CppTypeCategory::kArray) {
        throw std::runtime_error("Type is not an array");
    }
    return sub_types_[0];
}

const std::shared_ptr<CppType>& CppType::GetOptionalType() const {
    if (category_ != CppTypeCategory::kOptional) {
        throw std::runtime_error("Type is not optional");
    }
    return sub_types_[0];
}

const std::vector<std::shared_ptr<CppType>>& CppType::GetUnionAlternatives() const {
    if (category_ != CppTypeCategory::kUnion) {
        throw std::runtime_error("Type is not a union");
    }
    return sub_types_;
}

const std::vector<std::shared_ptr<CppType>>& CppType::GetParameterTypes() const {
    if (category_ != CppTypeCategory::kFunction) {
        throw std::runtime_error("Type is not a function");
    }
    // 返回除最后一个（返回类型）之外的所有类型
    static thread_local std::vector<std::shared_ptr<CppType>> params;
    params.clear();
    for (size_t i = 0; i < sub_types_.size() - 1; ++i) {
        params.push_back(sub_types_[i]);
    }
    return params;
}

const std::shared_ptr<CppType>& CppType::GetReturnType() const {
    if (category_ != CppTypeCategory::kFunction) {
        throw std::runtime_error("Type is not a function");
    }
    return sub_types_.back();
}

const std::string& CppType::GetStructName() const {
    if (category_ != CppTypeCategory::kObject) {
        throw std::runtime_error("Type is not an object");
    }
    return name_;
}

const std::vector<ObjectPropertyType>& CppType::GetObjectProperties() const {
    if (category_ != CppTypeCategory::kObject) {
        throw std::runtime_error("Type is not an object");
    }
    return object_properties_;
}

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs
