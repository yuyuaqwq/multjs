#include "instr.h"

namespace mjs {

std::map<OpcodeType, InstrInfo> g_instr_symbol{
    {OpcodeType::kCLoad_0, {"cload_0", {}}},
    {OpcodeType::kCLoad_1, {"cload_1", {}}},
    {OpcodeType::kCLoad_2, {"cload_2", {}}},
    {OpcodeType::kCLoad_3, {"cload_3", {}}},
    {OpcodeType::kCLoad_4, {"cload_4", {}}},
    {OpcodeType::kCLoad_5, {"cload_5", {}}},

    {OpcodeType::kCLoad, {"cload", {1}}},
    {OpcodeType::kCLoadW, {"cload_w", {2}}},

    {OpcodeType::kVLoad, {"vload", {1}}},
    {OpcodeType::kVLoad_0, {"vload_0", {}}},
    {OpcodeType::kVLoad_1, {"vload_1", {}}},
    {OpcodeType::kVLoad_2, {"vload_2", {}}},
    {OpcodeType::kVLoad_3, {"vload_3", {}}},

    {OpcodeType::kVStore, {"vstore", {1}}},
    {OpcodeType::kVStore_0, {"vstore_0", {}}},
    {OpcodeType::kVStore_1, {"vstore_1", {}}},
    {OpcodeType::kVStore_2, {"vstore_2", {}}},
    {OpcodeType::kVStore_3, {"vstore_3", {}}},

    {OpcodeType::kPop, {"pop", {}}},

    {OpcodeType::kAdd, {"add", {}}},
    {OpcodeType::kSub, {"sub", {}}},
    {OpcodeType::kMul, {"mul", {}}},
    {OpcodeType::kDiv, {"div", {}}},

    {OpcodeType::kShl, {"shl", {}}},
    {OpcodeType::kShr, {"shr", {}}},

    {OpcodeType::kNeg, {"neg", {}}},

    {OpcodeType::kEq, {"eq", {}}},
    {OpcodeType::kNe, {"ne", {}}},
    {OpcodeType::kLt, {"lt", {}}},
    {OpcodeType::kGe, {"ge", {}}},
    {OpcodeType::kGt, {"gt", {}}},
    {OpcodeType::kLe, {"le", {}}},

    {OpcodeType::kIfEq, {"ifeq", {2}}},

    {OpcodeType::kGoto, {"goto", {2}}},

    {OpcodeType::kReturn, {"return", {}}},

    {OpcodeType::kInvokeStatic, {"invokestatic", {2}}},
};


uint8_t* ByteCode::GetPtr(uint32_t pc) {
	return bytes_.data() + pc;
}

uint32_t ByteCode::GetPc() const noexcept {
	return bytes_.size();
}

OpcodeType ByteCode::GetOpcode(uint32_t pc) {
	return (OpcodeType)bytes_[pc];
}

uint8_t ByteCode::GetU8(uint32_t pc) {
	return *(uint8_t*)&bytes_[pc];
}

uint16_t ByteCode::GetU16(uint32_t pc) {
	return *(uint16_t*)&bytes_[pc];
}

uint32_t ByteCode::GetU32(uint32_t pc) {
	return *(uint32_t*)&bytes_[pc];
}

void ByteCode::EmitOpcode(OpcodeType opcode) {
	bytes_.push_back(static_cast<uint8_t>(opcode));
}

void ByteCode::EmitI8(int8_t val) {
	bytes_.push_back(static_cast<uint8_t>(val));
}

void ByteCode::EmitU8(uint8_t val) {
	bytes_.push_back(val);
}

void ByteCode::EmitI16(int16_t val) {
	bytes_.push_back(static_cast<uint8_t>(val & 0xff));
	bytes_.push_back(static_cast<uint8_t>(val >> 8));
}

void ByteCode::EmitU16(uint16_t val) {
	bytes_.push_back(static_cast<uint8_t>(val & 0xff));
	bytes_.push_back(static_cast<uint8_t>(val >> 8));
}

void ByteCode::EmitI32(uint32_t val) {
	bytes_.push_back(static_cast<uint8_t>(val & 0xff));
	bytes_.push_back(static_cast<uint8_t>(val >> 8));
	bytes_.push_back(static_cast<uint8_t>(val >> 16));
	bytes_.push_back(static_cast<uint8_t>(val >> 24));
}

void ByteCode::EmitU32(uint32_t val) {
	bytes_.push_back(static_cast<uint8_t>(val & 0xff));
	bytes_.push_back(static_cast<uint8_t>(val >> 8));
	bytes_.push_back(static_cast<uint8_t>(val >> 16));
	bytes_.push_back(static_cast<uint8_t>(val >> 24));
}

void ByteCode::EmitConstLoad(uint32_t idx) {
    if (idx <= 5) {
        EmitOpcode(OpcodeType::kCLoad_0 + idx);
    }
	else if (idx <= 0xff) {
		// ÔÊÐí¼ÓÔØ64bit£¿
		EmitOpcode(OpcodeType::kCLoad);
		EmitU8(idx);
	}
	else if (idx <= 0xffff) {
		EmitOpcode(OpcodeType::kCLoadW);
		EmitU16(idx);
	}
	else {
		// err
	}
}

void ByteCode::EmitVarStore(uint32_t idx) {
    if (idx >= 0 && idx <= 3) {
        EmitOpcode(OpcodeType::kVStore_0 + idx);
    }
    else if (idx <= 0xff) {
        EmitOpcode(OpcodeType::kVStore);
        EmitU8(idx);
    }
    else {
        // err
    }
}

void ByteCode::EmitVarLoad(uint32_t idx) {
    if (idx >= 0 && idx <= 3) {
        EmitOpcode(OpcodeType::kVLoad_0 + idx);
    }
    else if (idx <= 0xff) {
        EmitOpcode(OpcodeType::kVLoad);
        EmitU8(idx);
    }
    else {
        // err
    }
}


void ByteCode::RepairPc(uint32_t pc_from, uint32_t pc_to) {
	// skip opcode
	*reinterpret_cast<uint16_t*>(GetPtr(pc_from) + 1) = pc_to - pc_from;
}


} // namespace mjs