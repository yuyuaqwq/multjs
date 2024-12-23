#include "object.h"

namespace mjs {


FunctionBodyObject::FunctionBodyObject(uint32_t par_count) noexcept
	: par_count(par_count) {}

std::string FunctionBodyObject::Disassembly() {
	std::string str;
	for (int pc = 0; pc < byte_code.Size(); ) {
		char buf[16] = { 0 };
		sprintf_s(buf, "%04d\t", pc);
		const auto& info = g_instr_symbol.find(byte_code.GetOpcode(pc++));
		str += buf + info->second.str + "\t";
		for (const auto& par_size : info->second.par_size_list) {
			if (par_size == 1) {
				auto ki = byte_code.GetU8(pc);
				str += std::to_string(ki) + " ";
			}
			else if (par_size == 2) {
				auto ki = byte_code.GetU16(pc);
				str += std::to_string(ki) + " ";
			}
			else if (par_size == 4) {
				auto ki = byte_code.GetU32(pc);
				str += std::to_string(ki) + " ";
			}
			pc += par_size;
		}
		str += "\n";
	}
	return str;
}


UpValueObject::UpValueObject(uint32_t index, FunctionBodyObject* func_body) noexcept
	: index(index)
	, func_body(func_body) {}

} // namespace mjs