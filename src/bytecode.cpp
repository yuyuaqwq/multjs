#include <mjs/bytecode.h>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/function_def.h>


namespace mjs {

std::unordered_map<OpcodeType, InstrInfo> g_instr_symbol{
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

    {OpcodeType::kPropertyLoad, {"property_load", {4}}},
    {OpcodeType::kPropertyStore, {"property_store", {4}}},

    {OpcodeType::kIndexedLoad, {"indexed_load", {}}},
    {OpcodeType::kIndexedStore, {"indexed_store", {}}},
    

    {OpcodeType::kPop, {"pop", {}}},
    {OpcodeType::kDump, {"dump", {}}},
    {OpcodeType::kSwap, {"swap", {}}},
    {OpcodeType::kUndefined, {"undefined", {}}},

    {OpcodeType::kAdd, {"add", {}}},
    {OpcodeType::kInc, {"inc", {}}},
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

    {OpcodeType::kFunctionCall, {"function_call", {}}},
    {OpcodeType::kGetThis, {"get_this", {}}},
    {OpcodeType::kSetThis, {"set_this", {}}},

    {OpcodeType::kYield, {"yield", {}}},
    {OpcodeType::kGeneratorReturn, {"generator_return", {}}},
    {OpcodeType::kAwait, {"await", {}}},
    {OpcodeType::kAsyncReturn, {"async_return", {}}},

    {OpcodeType::kNew, {"new", {}}},

    {OpcodeType::kTryBegin, {"try_begin", {}}},
    {OpcodeType::kThrow, {"throw", {}}},
    {OpcodeType::kTryEnd, {"try_end", {}}},
    {OpcodeType::kFinallyReturn, {"finally_return", {}}},
    {OpcodeType::kFinallyGoto, {"finally_goto", {2}}},

    {OpcodeType::kGetModule, {"get_module", {}}},
    {OpcodeType::kGetModuleAsync, {"get_module_async", {}}},
    {OpcodeType::kClosure, {"closure", {4}}},
    
};


uint8_t* ByteCode::GetPtr(Pc pc) {
	return bytes_.data() + pc;
}

const uint8_t* ByteCode::GetPtr(Pc pc) const {
    return bytes_.data() + pc;
}


OpcodeType ByteCode::GetOpcode(Pc pc) const {
	return (OpcodeType)bytes_[pc];
}


Pc ByteCode::GetPc(Pc* pc) const {
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
    return ConstIndex(GetI32(pc_));
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
		// 64bit？
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

void ByteCode::EmitClosure(ConstIndex idx) {
    EmitOpcode(OpcodeType::kClosure);
    EmitU32(idx);
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
    EmitU32(const_idx);
}

void ByteCode::EmitPropertyStore(ConstIndex const_idx) {
    EmitOpcode(OpcodeType::kPropertyStore);
    EmitU32(const_idx);
}


void ByteCode::EmitIndexedLoad() {
    EmitOpcode(OpcodeType::kIndexedLoad);
}

void ByteCode::EmitIndexedStore() {
    EmitOpcode(OpcodeType::kIndexedStore);
}

void ByteCode::EmitReturn(FunctionType func_type) {
    if (func_type == FunctionType::kGenerator) {
        EmitOpcode(OpcodeType::kGeneratorReturn);
    }
    else if (func_type == FunctionType::kAsync) {
        EmitOpcode(OpcodeType::kAsyncReturn);
    }
    else {
        EmitOpcode(OpcodeType::kReturn);
    }
}

void ByteCode::RepairOpcode(Pc opcode_pc, OpcodeType op) {
    // skip opcode
    *reinterpret_cast<OpcodeType*>(GetPtr(opcode_pc)) = op;
}


void ByteCode::RepairPc(Pc pc_from, Pc pc_to) {
	// skip opcode
	*reinterpret_cast<int16_t*>(GetPtr(pc_from) + 1) = int64_t(pc_to) - int64_t(pc_from);
}

Pc ByteCode::CalcPc(Pc cur_pc) const {
    // skip opcode
    return cur_pc + *reinterpret_cast<const int16_t*>(GetPtr(cur_pc) + 1);
}

std::string ByteCode::Disassembly(Context* context, Pc& pc, OpcodeType& opcode, uint32_t& par, const FunctionDef* func_def) const {
    std::string str;
    char buf[16] = { 0 };
    sprintf_s(buf, "%04d\t", pc);
    opcode = GetOpcode(pc++);
    const auto& info = g_instr_symbol.find(opcode);
    str += buf + info->second.str + "\t";
    auto last_par = 0;
    for (const auto& par_size : info->second.par_size_list) {
        if (par_size == 1) {
            par = GetU8(pc);
            str += std::to_string(par) + "\t";
        }
        else if (par_size == 2) {
            par = GetU16(pc);
            str += std::to_string(par) + "\t";
        }
        else if (par_size == 4) {
            par = GetU32(pc);
            str += std::to_string(par) + "\t";
        }

        last_par = par;

        pc += par_size;
    }

    if (info->second.par_size_list.empty()) {
        str += "\t";
    }

    if (opcode >= OpcodeType::kCLoad_0 && opcode <= OpcodeType::kCLoad_5) {
        auto idx = opcode - OpcodeType::kCLoad_0;
        const auto& val = context->runtime().const_pool().at(ConstIndex(idx));
        if (val.IsString()) {
            str += "\"";
        }
        str += val.ToString(context).string_view();
        if (val.IsString()) {
            str += "\"";
        }
        str += "\t";
    }
    else if (opcode == OpcodeType::kCLoad || opcode == OpcodeType::kPropertyLoad || opcode == OpcodeType::kPropertyStore) {
        auto idx = par;
        const auto& val = context->runtime().const_pool().at(ConstIndex(idx));
        if (val.IsString()) {
            str += "\"";
        }
        str += val.ToString(context).string_view();
        if (val.IsString()) {
            str += "\"";
        }
        str += "\t";
    }


    if (opcode >= OpcodeType::kVLoad_0 && opcode <= OpcodeType::kVLoad_3) {
        auto idx = opcode - OpcodeType::kVLoad_0;
        auto& info = func_def->GetVarInfo(idx);
        str += "$";
        str += info.name;
        str += "\t";
    }
    if (opcode == OpcodeType::kVLoad) {
        auto idx = par;
        auto& info = func_def->GetVarInfo(idx);
        str += "$";
        str += info.name;
        str += "\t";
    }

    if (opcode >= OpcodeType::kVStore_0 && opcode <= OpcodeType::kVStore_3) {
        auto idx = opcode - OpcodeType::kVStore_0;
        auto& info = func_def->GetVarInfo(idx);
        str += "$";
        str += info.name;
        str += "\t";
    }
    if (opcode == OpcodeType::kVStore) {
        auto idx = par;
        auto& info = func_def->GetVarInfo(idx);
        str += "$";
        str += info.name;
        str += "\t";
    }

    if (opcode == OpcodeType::kGoto || opcode == OpcodeType::kIfEq) {
        str += "To:";
        str += std::to_string(int16_t(pc - 3 + last_par));
        str += "\t";
    }

    return str;
}


int8_t ByteCode::GetI8(Pc pc) const {
    return *(int8_t*)&bytes_[pc];
}

uint8_t ByteCode::GetU8(Pc pc) const {
    return *(uint8_t*)&bytes_[pc];
}

int16_t ByteCode::GetI16(Pc pc) const {
    return *(int16_t*)&bytes_[pc];
}

uint16_t ByteCode::GetU16(Pc pc) const {
    return *(uint16_t*)&bytes_[pc];
}

int32_t ByteCode::GetI32(Pc pc) const {
    return *(int32_t*)&bytes_[pc];
}

Pc ByteCode::GetU32(Pc pc) const {
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