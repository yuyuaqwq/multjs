#pragma once

#include <map>
#include <vector>
#include <string>

namespace mjs {

enum class OpcodeType {
	/*kStop = 0,
	kNop,
	kPushK,
	kPushV,
	kPop,
	kPopV,
	kAdd,
	kSub,
	kMul,
	kDiv,
	kCall,
	kRet,
	kNe,
	kEq,
	kGt,
	kGe,
	kLt,
	kLe,
	kJcf,
	kJmp,*/

	kIConst_0 = 0x03,
	kIConst_1 = 0x04,
	kIConst_2 = 0x05,
	kIConst_3 = 0x06,
	kIConst_4 = 0x07,
	kIConst_5 = 0x08,

	kBIPush = 0x10,
	kSIPush = 0x11,
	kLdc = 0x12,
	kLdcW = 0x13,
	kLdc2W = 0x14,

	kILoad = 0x15,
	kILoad_0 = 0x1a,
	kILoad_1 = 0x1b,
	kILoad_2 = 0x1c,
	kILoad_3 = 0x1d,
	kIStore = 0x36,
	kIStore_0 = 0x3b,
	kIStore_1 = 0x3b,
	kIStore_2 = 0x3c,
	kIStore_3 = 0x3d,

	kIAdd = 0x60,
	kISub = 0x64,
	kIMul = 0x68,
	kIDiv = 0x6c,

	kIInc = 0x84,

	kICmp = 0x94,
	kIfEq = 0x99,
	kIfNe = 0x9a,
	kIfLt = 0x9b,
	kIfGe = 0x9c,
	kIfGt = 0x9d,
	kIfLe = 0x9e,

	kIfICmpEq = 0x9f,
	kIfICmpNe = 0xa0,
	kIfICmpLt = 0xa1,
	kIfICmpGe = 0xa2,
	kIfICmpGt = 0xa3,
	kIfICmpLe = 0xa4,
	kGoto = 0xa7,

	KIReturn = 0xac,			// 带返回值
	KReturn = 0xb1,				// 不带返回值
	kInvokeStatic = 0xb8,		// 调用函数
};

OpcodeType operator+(OpcodeType a, size_t b) {
	return static_cast<OpcodeType>(static_cast<size_t>(a) + b);
}

struct InstrInfo {
	std::string str;
	std::vector<char> par_size_list;
};

class InstrSection {
public:
	uint8_t* GetPtr(uint32_t pc);
	uint32_t GetPc() const noexcept;
	OpcodeType GetOpcode(uint32_t pc);
	uint8_t GetU8(uint32_t pc);
	uint32_t GetU32(uint32_t pc);

	void EmitOpcode(OpcodeType opcode);
	void EmitI8(int8_t val);
	void EmitU8(uint8_t val);
	void EmitI16(int16_t val);
	void EmitU16(uint16_t val);
	void EmitI32(uint32_t val);
	void EmitU32(uint32_t val);
	void EmitLdc(uint32_t idx);
	void EmitIStore(uint32_t idx);
	void EmitILoad(uint32_t idx);

public:
	std::vector<uint8_t> container;
};

extern std::map<OpcodeType, InstrInfo> g_instr_symbol;

} // namespace mjs