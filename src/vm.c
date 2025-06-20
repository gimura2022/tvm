#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "tvm.h"
#include "utils.h"

#include "config.h"


static tvm_ext_callback_f exts[TVM_MAX_EXT_COUNT];
static size_t ext_count = 0;

static int io_ext(const struct tvm_command* command, struct tvm_memory* mem)
{
	switch (command->arg0) {
	case TVM_EXT_IO_PRINT:
		fputs((const char*) mem->get(mem, command->address), stdout);
		break;

	case TVM_EXT_IO_PRINTLN:
		fputs((const char*) mem->get(mem, command->address), stdout);
		fputc('\n', stdout);
		break;

	case TVM_EXT_IO_GETLINE:
		fgets((char*) mem->get(mem, command->address), command->arg1, stdin);
		break;

	default:
		return TVM_INVALID_EXT;
	}

	return TVM_OK;
}

static int runtime_info_ext(const struct tvm_command* command, struct tvm_memory* mem)
{
	switch (command->arg0) {
	case TVM_EXT_RUNTIME_INFO_NAME:
		strcpy((char*) mem->get(mem, command->address), PACKAGE_STRING);
		break;

	default:
		return TVM_INVALID_EXT;
	}

	return TVM_OK;
}

static int extmath_ext(const struct tvm_command* command, struct tvm_memory* mem)
{
	switch (command->arg0) {
	case TVM_EXT_EXTMATH_INT16_ADD:
		*((uint16_t*) mem->get(mem, command->address)) += command->arg1;
		break;

	case TVM_EXT_EXTMATH_INT16_SUB:
		*((uint16_t*) mem->get(mem, command->address)) -= command->arg1;
		break;

	case TVM_EXT_EXTMATH_INT32_ADD:
		*((uint32_t*) mem->get(mem, command->address)) += command->arg1;
		break;

	case TVM_EXT_EXTMATH_INT32_SUB:
		*((uint32_t*) mem->get(mem, command->address)) -= command->arg1;
		break;

	case TVM_EXT_EXTMATH_INT64_ADD:
		*((uint64_t*) mem->get(mem, command->address)) += command->arg1;
		break;

	case TVM_EXT_EXTMATH_INT64_SUB:
		*((uint64_t*) mem->get(mem, command->address)) -= command->arg1;
		break;

	default:
		return TVM_INVALID_EXT;
	}

	return TVM_OK;
}

static int execute_ext(const struct tvm_command* command, struct tvm_memory* mem)
{
	for (int i = 0; i < ext_count; i++) {
		int res = exts[i](command, mem);

		if (res != TVM_INVALID_EXT)
			return res;
	}

	return TVM_INVALID_EXT;
}

int tvm_execute(struct tvm_memory* mem, tvm_memory_address_int_t start_address)
{
	size_t call_stack_pos = 0;

	for (tvm_memory_address_int_t i = start_address; i < mem->end(mem);
			i += TVM_COMMAND_SIZE_IN_MEMORY) {
		const struct tvm_command* command = (struct tvm_command*) mem->get(mem, i);

		switch (command->command) {
		case TVM_ADD:
			*mem->get(mem, command->address) += command->arg0;
			break;

		case TVM_SUB:
			*mem->get(mem, command->address) -= command->arg0;
			break;

		case TVM_GOTO:
			i = command->address - TVM_COMMAND_SIZE_IN_MEMORY;
			break;

		case TVM_IF:
			if (mem->get(mem, command->address))
				break;

			i += 1;
			break;

		case TVM_EXIT:
			goto exit;

		case TVM_EXT:
			unwrap(execute_ext(command, mem));
			break;

		default:
			return TVM_INVALID_COMMAND;
		}
	}

exit:
	return TVM_OK;
}

int tvm_load_ext(const char* name)
{
	if (ext_count >= TVM_MAX_EXT_COUNT)
		return TVM_TOO_MANY_EXT;

	if (!strcmp(name, "ext_io")) {
		exts[ext_count++] = io_ext;
		return TVM_BUILTIN;
	} else if (!strcmp(name, "ext_runtime_info")) {
		exts[ext_count++] = runtime_info_ext;
		return TVM_BUILTIN;
	} else if (!strcmp(name, "ext_extmath")) {
		exts[ext_count++] = extmath_ext;
		return TVM_BUILTIN;
	} else {
		return TVM_UNSUPPORTED_EXT;
	}
}

int tvm_load_custom_ext(tvm_ext_callback_f callback)
{
	if (ext_count >= TVM_MAX_EXT_COUNT)
		return TVM_TOO_MANY_EXT;

	exts[ext_count++] = callback;

	return TVM_OK;
}
