#include <mjs/key_const_index_table.h>

namespace mjs {

KeyConstIndexTable::KeyConstIndexTable(GlobalConstPool* global_const_pool) {
	prototype_const_index_ = global_const_pool->insert(Value("prototype"));
	constructor_const_index_ = global_const_pool->insert(Value("constructor"));
}

KeyConstIndexTable::~KeyConstIndexTable() = default;

} // namespace mjs