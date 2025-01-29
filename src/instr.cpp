#include "instr.h"

namespace mjs {

std::map<OpcodeType, InstrInfo> g_instr_symbol{
    // {OpcodeType::kStop, {"stop", {}}},

    {OpcodeType::kCLoad_0, {"cload_0", {}}},
    {OpcodeType::kCLoad_1, {"cload_1", {}}},
    {OpcodeType::kCLoad_2, {"cload_2", {}}},
    {OpcodeType::kCLoad_3, {"cload_3", {}}},
    {OpcodeType::kCLoad_4, {"cload_4", {}}},
    {OpcodeType::kCLoad_5, {"cload_5", {}}},

    {OpcodeType::kCLoad, {"cload", {1}}},
    {OpcodeType::kCLoadW, {"cload_w", {2}}},
    {OpcodeType::kCLoadD, {"cload_d", {4}}},
    

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

    {OpcodeType::kPropertyLoad, {"propertyload", {4}}},
    {OpcodeType::kPropertyCall, {"propertycall", {4}}},
    {OpcodeType::kPropertyStore, {"propertystore", {4}}},
    {OpcodeType::kVPropertyStore, {"vpropertystore", {2, 4}}},

    {OpcodeType::kIndexedLoad, {"indexedload", {}}},
    {OpcodeType::kIndexedStore, {"indexedstore", {}}},

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

    {OpcodeType::kFunctionCall, {"functioncall", {2}}},
};


uint8_t* ByteCode::GetPtr(Pc pc) {
	return bytes_.data() + pc;
}


OpcodeType ByteCode::GetOpcode(Pc pc) {
	return (OpcodeType)bytes_[pc];
}


Pc ByteCode::GetPc(Pc* pc)  {
    auto pc_ = *pc;
    *pc += sizeof(Pc);
    return GetU32(pc_);
}

VarIndex ByteCode::GetVarIndex(Pc* pc) {
    auto pc_ = *pc;
    *pc += sizeof(VarIndex);
    return GetU16(pc_);
}

ConstIndex ByteCode::GetConstIndex(Pc* pc) {
    auto pc_ = *pc;
    *pc += sizeof(ConstIndex);
    return GetI32(pc_);
}


void ByteCode::EmitOpcode(OpcodeType opcode) {
	bytes_.push_back(static_cast<uint8_t>(opcode));
}



void ByteCode::EmitPcOffset(PcOffset offset) {
    EmitU16(offset);
}

void ByteCode::EmitVarIndex(VarIndex idx) {
    EmitU16(idx);
}

void ByteCode::EmitConstIndex(ConstIndex idx) {
    EmitI32(idx);
}

void ByteCode::EmitConstLoad(ConstIndex idx) {
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
        EmitOpcode(OpcodeType::kCLoadD);
        EmitU32(idx);
	}
}

void ByteCode::EmitVarStore(VarIndex idx) {
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

void ByteCode::EmitVarLoad(VarIndex idx) {
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

void ByteCode::EmitPropertyLoad(ConstIndex const_idx) {
    EmitOpcode(OpcodeType::kPropertyLoad);
    EmitConstIndex(const_idx);
}

void ByteCode::EmitPropertyCall(ConstIndex const_idx) {
    EmitOpcode(OpcodeType::kPropertyCall);
    EmitConstIndex(const_idx);
}

void ByteCode::EmitPropertyStore(ConstIndex const_idx) {
    EmitOpcode(OpcodeType::kPropertyStore);
    EmitConstIndex(const_idx);
}

void ByteCode::EmitVPropertyStore(VarIndex var_idx, ConstIndex const_idx) {
    EmitOpcode(OpcodeType::kVPropertyStore);
    EmitVarIndex(var_idx);
    EmitConstIndex(const_idx); 
}


void ByteCode::EmitIndexedLoad() {
    EmitOpcode(OpcodeType::kIndexedLoad);
}


void ByteCode::EmitIndexedStore() {
    EmitOpcode(OpcodeType::kIndexedStore);
}


void ByteCode::RepairPc(Pc pc_from, Pc pc_to) {
	// skip opcode
	*reinterpret_cast<int16_t*>(GetPtr(pc_from) + 1) = int64_t(pc_to) - int64_t(pc_from);
}

Pc ByteCode::CalcPc(Pc cur_pc) {
    // skip opcode
    return cur_pc + *reinterpret_cast<int16_t*>(GetPtr(cur_pc) + 1);
}

std::string ByteCode::Disassembly(Pc& pc) {
    std::string str;
    char buf[16] = { 0 };
    sprintf_s(buf, "%04d\t", pc);
    const auto& info = g_instr_symbol.find(GetOpcode(pc++));
    str += buf + info->second.str + "\t";
    for (const auto& par_size : info->second.par_size_list) {
        if (par_size == 1) {
            auto ki = GetU8(pc);
            str += std::to_string(ki) + " ";
        }
        else if (par_size == 2) {
            auto ki = GetU16(pc);
            str += std::to_string(ki) + " ";
        }
        else if (par_size == 4) {
            auto ki = GetU32(pc);
            str += std::to_string(ki) + " ";
        }
        pc += par_size;
    }
    return str;
}


int8_t ByteCode::GetI8(Pc pc) {
    return *(int8_t*)&bytes_[pc];
}

uint8_t ByteCode::GetU8(Pc pc) {
    return *(uint8_t*)&bytes_[pc];
}

int16_t ByteCode::GetI16(Pc pc) {
    return *(int16_t*)&bytes_[pc];
}

uint16_t ByteCode::GetU16(Pc pc) {
    return *(uint16_t*)&bytes_[pc];
}

int32_t ByteCode::GetI32(Pc pc) {
    return *(int32_t*)&bytes_[pc];
}

Pc ByteCode::GetU32(Pc pc) {
    return *(Pc*)&bytes_[pc];
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


} // namespace mjs