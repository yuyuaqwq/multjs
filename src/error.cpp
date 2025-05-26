#include <mjs/error.h>

#include <mjs/context.h>
#include <mjs/function_def.h>

namespace mjs {

Value Error::Throw(Context* context, const StackFrame& stack, std::string&& string) {
    const mjs::StackFrame* js_stack = &stack;
    while (!js_stack->function_def()) {
        if (!js_stack->upper_stack_frame()) {
            break;
        }
        js_stack = js_stack->upper_stack_frame();
    }
    auto debug_info = js_stack->function_def()->debug_table().FindEntry(js_stack->pc());
    SourceLine line = kInvalidSourceLine;
    if (debug_info) {
        line = debug_info->source_line;
    }
    
	return Value(String::Format("[line:{}] {}", line, string)).SetException();
}

} // namespace msj