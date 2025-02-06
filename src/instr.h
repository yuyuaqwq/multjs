#pragma once

#include <map>
#include <vector>
#include <string>

#include <mjs/const_def.h>
#include <mjs/var_def.h>

namespace mjs {

enum class OpcodeType {
	// kStop = 0x00,

	kCLoad_0 = 0x03,
	kCLoad_1 = 0x04,
	kCLoad_2 = 0x05,
	kCLoad_3 = 0x06,
	kCLoad_4 = 0x07,
	kCLoad_5 = 0x08,

	kCLoad = 0x12,
	kCLoadW = 0x13,
	kCLoadD = 0x14,

	// Load instructions
	kVLoad = 0x15,
	kVLoad_0 = 0x1a,
	kVLoad_1 = 0x1b,
	kVLoad_2 = 0x1c,
	kVLoad_3 = 0x1d,

	// Store instructions
	kVStore = 0x36,
	kVStore_0 = 0x3b,
	kVStore_1 = 0x3c,
	kVStore_2 = 0x3d,
	kVStore_3 = 0x3e,

	kPropertyLoad = 0x40,
	kPropertyCall = 0x41,
	kPropertyStore = 0x42,
	kVPropertyStore = 0x43,

	kIndexedLoad = 0x48,
	kIndexedStore = 0x49,

	// Stack manipulation
	kPop = 0x57,

	// Arithmetic operations
	kAdd = 0x60,
	kSub = 0x64,
	kMul = 0x68,
	kDiv = 0x6c,

	// Bitwise and shift operations
	kShl = 0x78,
	kShr = 0x7a,

	// Negation
	kNeg = 0x74,

	// Comparisons
	kEq = 0x99,
	kNe = 0x9a,
	kLt = 0x9b,
	kGe = 0x9c,
	kGt = 0x9d,
	kLe = 0x9e,

	kIfEq = 0xa0,	// Õ»¶¥Îª0ÔòÌø×ª

	// Control flow
	kGoto = 0xa7,

	// Return instructions
	kReturn = 0xb1,

	// Method invocation
	kFunctionCall = 0xb8,
};


inline static OpcodeType operator+(OpcodeType a, size_t b) {
	return static_cast<OpcodeType>(static_cast<size_t>(a) + b);
}

inline static size_t operator-(OpcodeType a, OpcodeType b) {
	return static_cast<size_t>(a) - static_cast<size_t>(b);
}

struct InstrInfo {
	std::string str;
	std::vector<char> par_size_list;
};

using Pc = uint32_t;
using PcOffset = uint16_t;

class ByteCode {
public:
	OpcodeType GetOpcode(Pc pc);
	Pc GetPc(Pc* pc);
	VarIndex GetVarIndex(Pc* pc);
	ConstIndex GetConstIndex(Pc* pc);

	void EmitOpcode(OpcodeType opcode);
	// void EmitPc(Pc pc);
	void EmitPcOffset(PcOffset offset);
	void EmitVarIndex(VarIndex idx);
	void EmitConstIndex(ConstIndex idx);

	void EmitConstLoad(ConstIndex idx);

	void EmitVarStore(VarIndex idx);
	void EmitVarLoad(VarIndex idx);

	void EmitGoto();

	void EmitPropertyLoad();
	void EmitPropertyCall();
	void EmitPropertyStore();
	void EmitVPropertyStore(VarIndex var_idx);

	void EmitIndexedLoad();
	void EmitIndexedStore();

	void RepairPc(Pc pc_from, Pc pc_to);
	Pc CalcPc(Pc cur_pc);

	std::string Disassembly(Pc& pc);

	Pc Size() { return bytes_.size(); }


	int8_t GetI8(Pc pc);
	uint8_t GetU8(Pc pc);
	int16_t GetI16(Pc pc);
	uint16_t GetU16(Pc pc);
	int32_t GetI32(Pc pc);
	uint32_t GetU32(Pc pc);



private:
	uint8_t* GetPtr(Pc pc);

	void EmitI8(int8_t val);
	void EmitU8(uint8_t val);
	void EmitI16(int16_t val);
	void EmitU16(uint16_t val);
	void EmitI32(uint32_t val);
	void EmitU32(uint32_t val);

private:
	std::vector<uint8_t> bytes_;
};

extern std::map<OpcodeType, InstrInfo> g_instr_symbol;

} // namespace mjs