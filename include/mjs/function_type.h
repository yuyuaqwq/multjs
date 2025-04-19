#pragma once

namespace mjs {
enum class FunctionType {
	kNormal,
	kAsync,
	kGenerator,
	kModule,
};
} // namespace mjs