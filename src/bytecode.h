#pragma once

#include <map>
#include <vector>
#include <string>

#include <mjs/const_def.h>
#include <mjs/var_def.h>
#include <mjs/opcode.h>

namespace mjs {

inline static OpcodeType operator+(OpcodeType a, size_t b) {
	return static_cast<OpcodeType>(static_cast<size_t>(a) + b);
}

inline static size_t operator-(OpcodeType a, OpcodeType b) {
	return static_cast<size_t>(a) - static_cast<size_t>(b);
}

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
	void EmitPropertyStore();

	void EmitIndexedLoad();
	void EmitIndexedStore();

	void EmitReturn(bool is_generator);


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