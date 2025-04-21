#pragma once

#include <unordered_map>
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

	// Module
	kGetModule = 0x20,
	kGetModuleAsync = 0x21,

	// Store instructions
	kVStore = 0x36,
	kVStore_0 = 0x3b,
	kVStore_1 = 0x3c,
	kVStore_2 = 0x3d,
	kVStore_3 = 0x3e,

	kPropertyLoad = 0x40,
	kPropertyStore = 0x41,

	kIndexedLoad = 0x48,
	kIndexedStore = 0x49,

	// Stack manipulation
	kPop = 0x50,
	kDump = 0x51,
	kSwap = 0x52,
	kUndefined = 0x53,

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

	kIfEq = 0xa0,

	// Control flow
	kGoto = 0xa7,

	// Return instructions
	kReturn = 0xb1,

	// Method invocation
	kFunctionCall = 0xb8,
	kGetThis = 0xb9,
	kSetThis = 0xba,

	// async
	kYield = 0xc0,
	kGeneratorReturn = 0xc1,
	kAwait = 0xc2,
	kAsyncReturn = 0xc3,

	// new
	kNew = 0xc8,

	// error
	kTryBegin = 0xd0,
	kThrow = 0xd3,
	kTryEnd = 0xd4,
	kFinallyReturn = 0xd5,

	kFinallyGoto = 0xd6,

	// 0xf0 ~ 0xff 保留
};

struct InstrInfo {
	std::string str;
	std::vector<char> par_size_list;
};

using Pc = uint32_t;
using PcOffset = uint16_t;

constexpr Pc kInvalidPc = 0xffffffff;

extern std::unordered_map<OpcodeType, InstrInfo> g_instr_symbol;

} // namespace mjs