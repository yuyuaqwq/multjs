#include "value.h"

namespace mjs {

bool Value::operator<(const Value& value) const {
	if (GetType() == value.GetType()) {
		switch (GetType()) {
		case ValueType::kNull: {
			return false;
		}
		case ValueType::kBool: {
			auto a = static_cast<const BoolValue*>(this);
			auto b = static_cast<const BoolValue*>(&value);
			return a->val < b->val;
		}
		case ValueType::kNumber: {
			auto a = static_cast<const NumberValue*>(this);
			auto b = static_cast<const NumberValue*>(&value);
			return a->val < b->val;
		}
		case ValueType::kString: {
			auto a = static_cast<const StringValue*>(this);
			auto b = static_cast<const StringValue*>(&value);
			return a->val < b->val;
		}
		case ValueType::kFunctionProto: {
			auto a = static_cast<const FunctionProtoValue*>(this);
			auto b = static_cast<const FunctionProtoValue*>(&value);
			return a->val < b->val;
		}
		case ValueType::kFunctionBridge:
		case ValueType::kFunctionBody:
		case ValueType::kUp: {
			return false;
		}
		}
	}
	return GetType() < value.GetType();
}

bool Value::operator<=(const Value& value) const {
	if (GetType() == value.GetType()) {
		switch (GetType()) {
		case ValueType::kNull: {
			return true;
		}
		case ValueType::kBool: {
			auto a = static_cast<const BoolValue*>(this);
			auto b = static_cast<const BoolValue*>(&value);
			return a->val <= b->val;
		}
		case ValueType::kNumber: {
			auto a = static_cast<const NumberValue*>(this);
			auto b = static_cast<const NumberValue*>(&value);
			return a->val <= b->val;
		}
		case ValueType::kString: {
			auto a = static_cast<const StringValue*>(this);
			auto b = static_cast<const StringValue*>(&value);
			return a->val <= b->val;
		}
		case ValueType::kFunctionProto: {
			auto a = static_cast<const FunctionProtoValue*>(this);
			auto b = static_cast<const FunctionProtoValue*>(&value);
			return a->val <= b->val;
		}
									  
		case ValueType::kFunctionBridge:
		case ValueType::kFunctionBody: 
		case ValueType::kUp: {
			return false;
		}
		}
	}
	return GetType() < value.GetType();
}

bool Value::operator==(const Value& value) const {
	if (GetType() == value.GetType()) {
		switch (GetType()) {
		case ValueType::kNull: {
			return true;
		}
		case ValueType::kBool: {
			auto a = static_cast<const BoolValue*>(this);
			auto b = static_cast<const BoolValue*>(&value);
			return a->val && b->val;
		}
		case ValueType::kNumber: {
			auto a = static_cast<const NumberValue*>(this);
			auto b = static_cast<const NumberValue*>(&value);
			return a->val == b->val;
		}
		case ValueType::kString: {
			auto a = static_cast<const StringValue*>(this);
			auto b = static_cast<const StringValue*>(&value);
			return a->val == b->val;
		}
		case ValueType::kFunctionProto: {
			auto a = static_cast<const FunctionProtoValue*>(this);
			auto b = static_cast<const FunctionProtoValue*>(&value);
			return a->val == b->val;
		}
		case ValueType::kFunctionBridge:
		case ValueType::kFunctionBody:
		case ValueType::kUp: {
			return false;
		}
		}
	}
	return GetType() == value.GetType();
}


BoolValue* Value::GetBool() noexcept {
	return static_cast<BoolValue*>(this);
}

NumberValue* Value::GetNumber() noexcept {
	return static_cast<NumberValue*>(this);
}

StringValue* Value::GetString() noexcept {
	return static_cast<StringValue*>(this);
}

FunctionProtoValue* Value::GetFunctionProto() noexcept {
	return static_cast<FunctionProtoValue*>(this);
}

FunctionBodyValue* Value::GetFunctionBody() noexcept {
	return static_cast<FunctionBodyValue*>(this);
}

FunctionBridgeValue* Value::GetFunctionBirdge() noexcept {
	return static_cast<FunctionBridgeValue*>(this);
}

UpValue* Value::GetUp() noexcept {
	return static_cast<UpValue*>(this);
}


ValueType NullValue::GetType() const noexcept {
	return ValueType::kNull;
}

std::unique_ptr<Value> NullValue::Copy() const {
	return std::make_unique<NullValue>();
}


ValueType BoolValue::GetType() const noexcept {
	return ValueType::kBool;
}

std::unique_ptr<Value> BoolValue::Copy() const {
	return std::make_unique<BoolValue>(val);
}

BoolValue::BoolValue(bool val) noexcept
	: val(val) {}


ValueType NumberValue::GetType() const noexcept {
	return ValueType::kNumber;
}

std::unique_ptr<Value> NumberValue::Copy() const {
	return std::make_unique<NumberValue>(val);
}

NumberValue::NumberValue(uint64_t val) noexcept
	: val(val) {}


ValueType StringValue::GetType() const noexcept {
	return ValueType::kString;
}

std::unique_ptr<Value> StringValue::Copy() const {
	return std::make_unique<StringValue>(val);
}

StringValue::StringValue(const std::string& t_val)
	: val(t_val) {}


ValueType FunctionBridgeValue::GetType() const noexcept {
	return ValueType::kFunctionBridge;
}

std::unique_ptr<Value> FunctionBridgeValue::Copy() const {
	return std::make_unique<FunctionProtoValue>(const_cast<FunctionBridgeValue*>(this));
}

FunctionBridgeValue::FunctionBridgeValue(FunctionBridgeCall func_addr) noexcept
	: func_addr(func_addr) {}


ValueType FunctionBodyValue::GetType() const noexcept {
	return ValueType::kFunctionBody;
}

std::unique_ptr<Value> FunctionBodyValue::Copy() const {
	return std::make_unique<FunctionProtoValue>(const_cast<FunctionBodyValue*>(this));
}

FunctionBodyValue::FunctionBodyValue(uint32_t par_count) noexcept
	: par_count(par_count) {}

std::string FunctionBodyValue::Disassembly() {
	std::string str;
	for (int pc = 0; pc < instr_sect.container.size(); ) {
		char buf[16] = { 0 };
		sprintf_s(buf, "%04d    ", pc);
		const auto& info = g_instr_symbol.find(instr_sect.GetOpcode(pc++));
		str += buf + info->second.str + "    ";
		for (const auto& parSize : info->second.par_size_list) {
			if (parSize == 4) {
				auto ki = instr_sect.GetU32(pc);
				str += std::to_string(ki) + " ";
			}
			pc += parSize;

		}
		str += "\n";
	}
	return str;
}


ValueType FunctionProtoValue::GetType() const noexcept {
	return ValueType::kFunctionProto;
}

std::unique_ptr<Value> FunctionProtoValue::Copy() const {
	return std::make_unique<FunctionProtoValue>(body_val);
}

FunctionProtoValue::FunctionProtoValue(FunctionBodyValue* val) noexcept
	: body_val(val) {}


FunctionProtoValue::FunctionProtoValue(FunctionBridgeValue* val) noexcept
	: bridge_val(val) {}

ValueType UpValue::GetType() const noexcept {
	return ValueType::kUp;
}

std::unique_ptr<Value> UpValue::UpValue::Copy() const {
	return std::make_unique<UpValue>(index, func_proto);
}

UpValue::UpValue(uint32_t index, FunctionBodyValue* func_proto) noexcept
	: index(index)
	, func_proto(func_proto) {}

} // namespace mjs