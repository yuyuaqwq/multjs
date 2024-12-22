#include "instr.h"

namespace mjs {

std::map<OpcodeType, InstrInfo> g_instr_symbol {
	//{OpcodeType::kStop, {"stop", {}}},
	//{OpcodeType::kNop, {"nop", {}}},
	//{OpcodeType::kPushK, {"pushk", {4}}},
	//{OpcodeType::kPushV, {"pushv", {4}}},
	//{OpcodeType::kPop, {"pop", {}}},
	//{OpcodeType::kPopV, {"popv", {4}}},
	//{OpcodeType::kAdd, {"add", {}}},
	//{OpcodeType::kSub, {"sub", {}}},
	//{OpcodeType::kMul, {"mul", {}}},
	//{OpcodeType::kDiv, {"div", {}}},
	//{OpcodeType::kCall, {"call", {4}}},
	//{OpcodeType::kRet, {"ret", {}}},
	//{OpcodeType::kNe, {"ne", {}}},
	//{OpcodeType::kEq, {"eq", {}}},
	//{OpcodeType::kGt, {"gt", {}}},
	//{OpcodeType::kGe, {"ge", {}}},
	//{OpcodeType::kLt, {"lt", {}}},
	//{OpcodeType::kLe, {"le", {}}},
	//{OpcodeType::kJcf, {"jcf", {4}}},
	//{OpcodeType::kJmp, {"jmp", {4}}},


};


uint8_t* InstrSection::GetPtr(uint32_t pc) {
	return container.data() + pc;
}

uint32_t InstrSection::GetPc() const noexcept {
	return container.size();
}

OpcodeType InstrSection::GetOpcode(uint32_t pc) {
	return (OpcodeType)container[pc];
}

uint8_t InstrSection::GetU8(uint32_t pc) {
	return *(uint8_t*)&container[pc];
}

uint32_t InstrSection::GetU32(uint32_t pc) {
	return *(uint32_t*)&container[pc];
}

void InstrSection::EmitOpcode(OpcodeType opcode) {
	container.push_back(static_cast<uint8_t>(opcode));
}

void InstrSection::EmitI8(int8_t val) {
	container.push_back(static_cast<uint8_t>(val));
}

void InstrSection::EmitU8(uint8_t val) {
	container.push_back(val);
}

void InstrSection::EmitI16(int16_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
}

void InstrSection::EmitU16(uint16_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
}

void InstrSection::EmitI32(uint32_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
	container.push_back(static_cast<uint8_t>(val >> 16));
	container.push_back(static_cast<uint8_t>(val >> 24));
}

void InstrSection::EmitU32(uint32_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
	container.push_back(static_cast<uint8_t>(val >> 16));
	container.push_back(static_cast<uint8_t>(val >> 24));
}

void InstrSection::EmitLdc(uint32_t idx) {
	if (idx <= 0xff) {
		EmitOpcode(OpcodeType::kLdc);
		EmitU8(idx);
	}
	else if (idx <= 0xffff) {
		EmitOpcode(OpcodeType::kLdcW);
		EmitU16(idx);
	}
	else {
		// err
	}
}

void InstrSection::EmitIStore(uint32_t idx) {
	if (idx >= 0 && idx <= 3) {
		EmitOpcode(OpcodeType::kIStore_0  + idx);
	}
	else if (idx <= 0xff) {
		EmitOpcode(OpcodeType::kIStore);
		EmitU8(idx);
	}
	else {
		// err
	}
}

void InstrSection::EmitILoad(uint32_t idx) {
	if (idx >= 0 && idx <= 3) {
		EmitOpcode(OpcodeType::kILoad_0 + idx);
	}
	else if (idx <= 0xff) {
		EmitOpcode(OpcodeType::kILoad);
		EmitU8(idx);
	}
	else {
		// err
	}
}


} // namespace mjs