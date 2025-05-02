#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include <mjs/noncopyable.h>
#include <mjs/const_def.h>
#include <mjs/var_def.h>
#include <mjs/opcode.h>

namespace mjs {

enum class FunctionType {
	kNormal,
	kGenerator,
	kAsync,
	kModule,
};

inline static OpcodeType operator+(OpcodeType a, size_t b) {
	return static_cast<OpcodeType>(static_cast<size_t>(a) + b);
}

inline static size_t operator-(OpcodeType a, OpcodeType b) {
	return static_cast<size_t>(a) - static_cast<size_t>(b);
}

class Context;
class FunctionDef;
class ByteCode : public noncopyable {
public:
	OpcodeType GetOpcode(Pc pc) const;
	Pc GetPc(Pc* pc) const;
	VarIndex GetVarIndex(Pc* pc);
	ConstIndex GetConstIndex(Pc* pc);

	void EmitOpcode(OpcodeType opcode);
	// void EmitPc(Pc pc);
	void EmitPcOffset(PcOffset offset);
	void EmitVarIndex(VarIndex idx);
	void EmitConstIndex(ConstIndex idx);

	void EmitConstLoad(ConstIndex idx);

	void EmitClosure(ConstIndex idx);

	void EmitVarStore(VarIndex idx);
	void EmitVarLoad(VarIndex idx);

	void EmitGoto();

	void EmitPropertyLoad(ConstIndex const_idx);
	void EmitPropertyStore(ConstIndex const_idx);

	void EmitIndexedLoad();
	void EmitIndexedStore();

	void EmitReturn(FunctionType func_type);


	void RepairPc(Pc pc_from, Pc pc_to);
	Pc CalcPc(Pc cur_pc) const;

	std::string Disassembly(Context* context, Pc& pc, OpcodeType& opcode, uint32_t& par, const FunctionDef* func_def) const;

	Pc Size() const { return bytes_.size(); }


	int8_t GetI8(Pc pc) const;
	uint8_t GetU8(Pc pc) const;
	int16_t GetI16(Pc pc) const;
	uint16_t GetU16(Pc pc) const;
	int32_t GetI32(Pc pc) const;
	uint32_t GetU32(Pc pc) const;

private:
	uint8_t* GetPtr(Pc pc);
	const uint8_t* GetPtr(Pc pc) const;

	void EmitI8(int8_t val);
	void EmitU8(uint8_t val);
	void EmitI16(int16_t val);
	void EmitU16(uint16_t val);
	void EmitI32(uint32_t val);
	void EmitU32(uint32_t val);

private:
	std::vector<uint8_t> bytes_;
};

extern std::unordered_map<OpcodeType, InstrInfo> g_instr_symbol;

} // namespace mjs