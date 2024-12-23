#include "instr.h"

namespace mjs {

    std::map<OpcodeType, InstrInfo> g_instr_symbol{
        {OpcodeType::kIConst_0, {"iconst_0", {}}},
        {OpcodeType::kIConst_1, {"iconst_1", {}}},
        {OpcodeType::kIConst_2, {"iconst_2", {}}},
        {OpcodeType::kIConst_3, {"iconst_3", {}}},
        {OpcodeType::kIConst_4, {"iconst_4", {}}},
        {OpcodeType::kIConst_5, {"iconst_5", {}}},

        {OpcodeType::kLConst_0, {"lconst_0", {}}},
        {OpcodeType::kLConst_1, {"lconst_1", {}}},
        {OpcodeType::kDConst_0, {"dconst_0", {}}},
        {OpcodeType::kDConst_1, {"dconst_1", {}}},

        {OpcodeType::kBIPush, {"bipush", {1}}},
        {OpcodeType::kSIPush, {"sipush", {2}}},
        {OpcodeType::kLdc, {"ldc", {1}}},
        {OpcodeType::kLdc_W, {"ldc_w", {2}}},
        {OpcodeType::kLdc2_W, {"ldc2_w", {2}}},

        // Load instructions
        {OpcodeType::kILoad, {"iload", {1}}},
        {OpcodeType::kLLoad, {"lload", {1}}},
        {OpcodeType::kDLoad, {"dload", {1}}},
        {OpcodeType::kILoad_0, {"iload_0", {}}},
        {OpcodeType::kILoad_1, {"iload_1", {}}},
        {OpcodeType::kILoad_2, {"iload_2", {}}},
        {OpcodeType::kILoad_3, {"iload_3", {}}},
        {OpcodeType::kLLoad_0, {"lload_0", {}}},
        {OpcodeType::kLLoad_1, {"lload_1", {}}},
        {OpcodeType::kLLoad_2, {"lload_2", {}}},
        {OpcodeType::kLLoad_3, {"lload_3", {}}},
        {OpcodeType::kDLoad_0, {"dload_0", {}}},
        {OpcodeType::kDLoad_1, {"dload_1", {}}},
        {OpcodeType::kDLoad_2, {"dload_2", {}}},
        {OpcodeType::kDLoad_3, {"dload_3", {}}},

        // Store instructions
        {OpcodeType::kIStore, {"istore", {1}}},
        {OpcodeType::kLStore, {"lstore", {1}}},
        {OpcodeType::kDStore, {"dstore", {1}}},
        {OpcodeType::kIStore_0, {"istore_0", {}}},
        {OpcodeType::kIStore_1, {"istore_1", {}}},
        {OpcodeType::kIStore_2, {"istore_2", {}}},
        {OpcodeType::kIStore_3, {"istore_3", {}}},
        {OpcodeType::kLStore_0, {"lstore_0", {}}},
        {OpcodeType::kLStore_1, {"lstore_1", {}}},
        {OpcodeType::kLStore_2, {"lstore_2", {}}},
        {OpcodeType::kLStore_3, {"lstore_3", {}}},
        {OpcodeType::kDStore_0, {"dstore_0", {}}},
        {OpcodeType::kDStore_1, {"dstore_1", {}}},
        {OpcodeType::kDStore_2, {"dstore_2", {}}},
        {OpcodeType::kDStore_3, {"dstore_3", {}}},

        // Stack manipulation
        {OpcodeType::kPop, {"pop", {}}},
        {OpcodeType::kPop2, {"pop2", {}}},
        {OpcodeType::kDup, {"dup", {}}},
        {OpcodeType::kDup_X1, {"dup_x1", {}}},
        {OpcodeType::kDup_X2, {"dup_x2", {}}},
        {OpcodeType::kDup2, {"dup2", {}}},
        {OpcodeType::kDup2_X1, {"dup2_x1", {}}},
        {OpcodeType::kDup2_X2, {"dup2_x2", {}}},
        {OpcodeType::kSwap, {"swap", {}}},

        // Arithmetic operations
        {OpcodeType::kIAdd, {"iadd", {}}},
        {OpcodeType::kLAdd, {"ladd", {}}},
        {OpcodeType::kFAdd, {"fadd", {}}},
        {OpcodeType::kDAdd, {"dadd", {}}},
        {OpcodeType::kISub, {"isub", {}}},
        {OpcodeType::kLSub, {"lsub", {}}},
        {OpcodeType::kFSub, {"fsub", {}}},
        {OpcodeType::kDSub, {"dsub", {}}},
        {OpcodeType::kIMul, {"imul", {}}},
        {OpcodeType::kLMul, {"lmul", {}}},
        {OpcodeType::kFMul, {"fmul", {}}},
        {OpcodeType::kDMul, {"dmul", {}}},
        {OpcodeType::kIDiv, {"idiv", {}}},
        {OpcodeType::kLDiv, {"ldiv", {}}},
        {OpcodeType::kFDiv, {"fdiv", {}}},
        {OpcodeType::kDDiv, {"ddiv", {}}},

        // Bitwise and shift operations
        {OpcodeType::kIShl, {"ishl", {}}},
        {OpcodeType::kLShl, {"lshl", {}}},
        {OpcodeType::kIShr, {"ishr", {}}},
        {OpcodeType::kLShr, {"lshr", {}}},
        {OpcodeType::kIUShr, {"iushr", {}}},
        {OpcodeType::kLUShr, {"lushr", {}}},

        // Negation
        {OpcodeType::kINeg, {"ineg", {}}},
        {OpcodeType::kLNeg, {"lneg", {}}},
        {OpcodeType::kFNeg, {"fneg", {}}},
        {OpcodeType::kDNeg, {"dneg", {}}},

        // Type conversions
        {OpcodeType::kI2L, {"i2l", {}}},
        {OpcodeType::kI2D, {"i2d", {}}},
        {OpcodeType::kL2I, {"l2i", {}}},
        {OpcodeType::kL2D, {"l2d", {}}},
        {OpcodeType::kD2I, {"d2i", {}}},
        {OpcodeType::kD2L, {"d2l", {}}},

        // Comparisons
        {OpcodeType::kLCmp, {"lcmp", {}}},
        {OpcodeType::kDCmpL, {"dcmp_l", {}}},
        {OpcodeType::kDCmpG, {"dcmp_g", {}}},

        {OpcodeType::kIfEq, {"ifeq", {2}}},
        {OpcodeType::kIfNe, {"ifne", {2}}},
        {OpcodeType::kIfLt, {"iflt", {2}}},
        {OpcodeType::kIfGe, {"ifge", {2}}},
        {OpcodeType::kIfGt, {"ifgt", {2}}},
        {OpcodeType::kIfLe, {"ifle", {2}}},
        {OpcodeType::kIfICmpEq, {"if_icmpeq", {2}}},
        {OpcodeType::kIfICmpNe, {"if_icmpne", {2}}},
        {OpcodeType::kIfICmpLt, {"if_icmplt", {2}}},
        {OpcodeType::kIfICmpGe, {"if_icmpge", {2}}},
        {OpcodeType::kIfICmpGt, {"if_icmpgt", {2}}},
        {OpcodeType::kIfICmpLe, {"if_icmple", {2}}},

        // Control flow
        {OpcodeType::kGoto, {"goto", {2}}},

        // Return instructions
        {OpcodeType::kIReturn, {"ireturn", {}}},
        {OpcodeType::kLReturn, {"lreturn", {}}},
        {OpcodeType::kFReturn, {"freturn", {}}},
        {OpcodeType::kDReturn, {"dreturn", {}}},
        {OpcodeType::kAReturn, {"areturn", {}}},
        {OpcodeType::kReturn, {"return", {}}},

        // Method invocation
        {OpcodeType::kInvokeStatic, {"invokestatic", {2}}},

        // Object and array instructions
        {OpcodeType::kInstanceOf, {"instanceof", {2}}},
        {OpcodeType::kCheckCast, {"checkcast", {2}}},
        {OpcodeType::kANewArray, {"anewarray", {2}}},
        {OpcodeType::kArrayLength, {"arraylength", {}}}
    };





uint8_t* ByteCode::GetPtr(uint32_t pc) {
	return container.data() + pc;
}

uint32_t ByteCode::GetPc() const noexcept {
	return container.size();
}

OpcodeType ByteCode::GetOpcode(uint32_t pc) {
	return (OpcodeType)container[pc];
}

uint8_t ByteCode::GetU8(uint32_t pc) {
	return *(uint8_t*)&container[pc];
}

uint16_t ByteCode::GetU16(uint32_t pc) {
	return *(uint16_t*)&container[pc];
}

uint32_t ByteCode::GetU32(uint32_t pc) {
	return *(uint32_t*)&container[pc];
}

void ByteCode::EmitOpcode(OpcodeType opcode) {
	container.push_back(static_cast<uint8_t>(opcode));
}

void ByteCode::EmitI8(int8_t val) {
	container.push_back(static_cast<uint8_t>(val));
}

void ByteCode::EmitU8(uint8_t val) {
	container.push_back(val);
}

void ByteCode::EmitI16(int16_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
}

void ByteCode::EmitU16(uint16_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
}

void ByteCode::EmitI32(uint32_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
	container.push_back(static_cast<uint8_t>(val >> 16));
	container.push_back(static_cast<uint8_t>(val >> 24));
}

void ByteCode::EmitU32(uint32_t val) {
	container.push_back(static_cast<uint8_t>(val & 0xff));
	container.push_back(static_cast<uint8_t>(val >> 8));
	container.push_back(static_cast<uint8_t>(val >> 16));
	container.push_back(static_cast<uint8_t>(val >> 24));
}

void ByteCode::EmitLdc(uint32_t idx) {
	if (idx <= 0xff) {
		// ÔÊÐí¼ÓÔØ64bit£¿
		EmitOpcode(OpcodeType::kLdc);
		EmitU8(idx);
	}
	else if (idx <= 0xffff) {
		EmitOpcode(OpcodeType::kLdc2_W);
		EmitU16(idx);
	}
	else {
		// err
	}
}

void ByteCode::EmitIStore(uint32_t idx) {
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

void ByteCode::EmitILoad(uint32_t idx) {
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

void ByteCode::RepairPc(uint32_t pc_from, uint32_t pc_to) {
	// skip opcode
	*reinterpret_cast<uint16_t*>(GetPtr(pc_from) + 1) = pc_to - pc_from;
}


} // namespace mjs