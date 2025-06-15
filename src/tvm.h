#ifndef _tvm_h
#define _tvm_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define TVM_COMMAND_SIZE_IN_MEMORY 8
#define TVM_BYTECODE_VERSION 2
#define TVM_MAX_EXT_COUNT 1024
#define TVM_MAX_EXT_NAME 64

struct tvm_command {
	int address : 16;
	int command : 8;

	int arg0 : 16;
	int arg1 : 16;


	int _unused : 8;
};

enum {
	TVM_ADD = 0,
	TVM_SUB,
	TVM_GOTO,
	TVM_IF,
	TVM_EXT,
	TVM_EXIT,
};

enum {
	TVM_EXT_IO_PRINT = 100,
	TVM_EXT_IO_PRINTLN,
	TVM_EXT_IO_GETLINE,

	TVM_EXT_RUNTIME_INFO_NAME = 200,

	TVM_EXT_EXTMATH_INT16_ADD = 300,
	TVM_EXT_EXTMATH_INT16_SUB,

	TVM_EXT_EXTMATH_INT32_ADD,
	TVM_EXT_EXTMATH_INT32_SUB,

	TVM_EXT_EXTMATH_INT64_ADD,
	TVM_EXT_EXTMATH_INT64_SUB,
};

struct tvm_memory;

typedef uint8_t tvm_memory_cell_int_t;
typedef uint16_t tvm_memory_address_int_t;

typedef tvm_memory_cell_int_t* (*tvm_get_mem_f)(struct tvm_memory* mem, tvm_memory_address_int_t addr);
typedef tvm_memory_address_int_t (*tvm_end_mem_f)(struct tvm_memory* mem);

struct tvm_memory {
	tvm_get_mem_f get;
	tvm_end_mem_f end;
};

typedef void (*tvm_static_memory_error_callback_f)(const char* msg);

struct tvm_static_memory {
	tvm_get_mem_f get;
	tvm_end_mem_f end;

	tvm_static_memory_error_callback_f error_callback;

	tvm_memory_cell_int_t* mem;
	tvm_memory_address_int_t mem_size;
};

struct tvm_bytecode_header {
	uint8_t version;
	uint16_t data_size;
	uint16_t program_start_addr;

	uint16_t ext_count;
	char exts[TVM_MAX_EXT_COUNT][TVM_MAX_EXT_NAME];
};

enum {
	TVM_OK = 0,
	TVM_BUILTIN,

	TVM_INVALID_COMMAND,
	TVM_INVALID_EXT,

	TVM_UNSUPPORTED_EXT,
	TVM_UNEXCEPTED_BYTECODE_END,
	TVM_TOO_MANY_EXT,
};


int tvm_load_bytecode(FILE* file, struct tvm_bytecode_header* header, struct tvm_memory* mem);
int tvm_write_bytecode(FILE* file, const struct tvm_bytecode_header* header, struct tvm_memory* mem);

void tvm_create_static_memory(struct tvm_static_memory* mem, tvm_memory_address_int_t size,
		tvm_static_memory_error_callback_f error_callback);
void tvm_free_static_memory(struct tvm_static_memory* mem);

struct tvm_command* tvm_get_command(struct tvm_memory* mem, tvm_memory_address_int_t addr);

typedef int (*tvm_ext_callback_f)(const struct tvm_command* command, struct tvm_memory* mem);

int tvm_load_ext(const char* name);
int tvm_load_custom_ext(tvm_ext_callback_f callback);
int tvm_execute(struct tvm_memory* mem, tvm_memory_address_int_t start_address);

#endif
