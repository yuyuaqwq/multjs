#pragma once

#include <map>
#include <vector>
#include <string>

namespace mjs {

enum class OpcodeType {
	kIConst_0 = 0x03,
	kIConst_1 = 0x04,
	kIConst_2 = 0x05,
	kIConst_3 = 0x06,
	kIConst_4 = 0x07,
	kIConst_5 = 0x08,

	kLConst_0 = 0x09, // Long constant 0
	kLConst_1 = 0x0a, // Long constant 1
	kDConst_0 = 0x0e, // Double constant 0.0
	kDConst_1 = 0x0f, // Double constant 1.0

	kBIPush = 0x10,
	kSIPush = 0x11,
	kLdc = 0x12,
	kLdc_W = 0x13,
	kLdc2_W = 0x14, // Load long or double constant

	// Load instructions
	kILoad = 0x15,
	kLLoad = 0x16,
	kDLoad = 0x18,
	kILoad_0 = 0x1a,
	kILoad_1 = 0x1b,
	kILoad_2 = 0x1c,
	kILoad_3 = 0x1d,
	kLLoad_0 = 0x1e,
	kLLoad_1 = 0x1f,
	kLLoad_2 = 0x20,
	kLLoad_3 = 0x21,
	kDLoad_0 = 0x26,
	kDLoad_1 = 0x27,
	kDLoad_2 = 0x28,
	kDLoad_3 = 0x29,

	// Store instructions
	kIStore = 0x36,
	kLStore = 0x37,
	kDStore = 0x39,
	kIStore_0 = 0x3b,
	kIStore_1 = 0x3c,
	kIStore_2 = 0x3d,
	kIStore_3 = 0x3e,
	kLStore_0 = 0x3f,
	kLStore_1 = 0x40,
	kLStore_2 = 0x41,
	kLStore_3 = 0x42,
	kDStore_0 = 0x47,
	kDStore_1 = 0x48,
	kDStore_2 = 0x49,
	kDStore_3 = 0x4a,

	// Stack manipulation
	kPop = 0x57,
	kPop2 = 0x58,
	kDup = 0x59,
	kDup_X1 = 0x5a,
	kDup_X2 = 0x5b,
	kDup2 = 0x5c,       // Duplicate top two stack items
	kDup2_X1 = 0x5d,    // Duplicate two stack items and insert below one
	kDup2_X2 = 0x5e,    // Duplicate two stack items and insert below two
	kSwap = 0x5f,

	// Arithmetic operations
	kIAdd = 0x60,
	kLAdd = 0x61,
	kFAdd = 0x62,
	kDAdd = 0x63,
	kISub = 0x64,
	kLSub = 0x65,
	kFSub = 0x66,
	kDSub = 0x67,
	kIMul = 0x68,
	kLMul = 0x69,
	kFMul = 0x6a,
	kDMul = 0x6b,
	kIDiv = 0x6c,
	kLDiv = 0x6d,
	kFDiv = 0x6e,
	kDDiv = 0x6f,

	// Bitwise and shift operations
	kIShl = 0x78,
	kLShl = 0x79,
	kIShr = 0x7a,
	kLShr = 0x7b,
	kIUShr = 0x7c,
	kLUShr = 0x7d,

	// Negation
	kINeg = 0x74,
	kLNeg = 0x75,
	kFNeg = 0x76,
	kDNeg = 0x77,

	// Type conversions
	kI2L = 0x85,  // Convert int to long
	kI2D = 0x87,  // Convert int to double
	kL2I = 0x88,  // Convert long to int
	kL2D = 0x8a,  // Convert long to double
	kD2I = 0x8b,  // Convert double to int
	kD2L = 0x8d,  // Convert double to long

	// Comparisons
	kLCmp = 0x94, // Compare long
	kDCmpL = 0x98, // Compare double (less if NaN)
	kDCmpG = 0x97, // Compare double (greater if NaN)

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

	// Control flow
	kGoto = 0xa7,

	// Return instructions
	kIReturn = 0xac,
	kLReturn = 0xad,
	kFReturn = 0xae,
	kDReturn = 0xaf,
	kAReturn = 0xb0,
	kReturn = 0xb1,

	// Method invocation
	kInvokeStatic = 0xb8,

	// Object and array instructions
	kInstanceOf = 0xc1,
	kCheckCast = 0xc0,
	kANewArray = 0xbd,
	kArrayLength = 0xbe
};


inline static OpcodeType operator+(OpcodeType a, size_t b) {
	return static_cast<OpcodeType>(static_cast<size_t>(a) + b);
}

struct InstrInfo {
	std::string str;
	std::vector<char> par_size_list;
};

class ByteCode {
public:
	uint8_t* GetPtr(uint32_t pc);
	uint32_t GetPc() const noexcept;
	OpcodeType GetOpcode(uint32_t pc);
	uint8_t GetU8(uint32_t pc);
	uint16_t GetU16(uint32_t pc);
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

	void RepairPc(uint32_t pc_from, uint32_t pc_to);

public:
	std::vector<uint8_t> container;
};

extern std::map<OpcodeType, InstrInfo> g_instr_symbol;

} // namespace mjs