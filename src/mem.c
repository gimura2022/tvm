#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <err.h>

#include "tvm.h"

static tvm_memory_cell_int_t* static_memory_get(struct tvm_static_memory* mem, tvm_memory_address_int_t addr)
{
	if (addr >= mem->mem_size)
		mem->error_callback("invalid memory address");

	return &mem->mem[addr];
}

static tvm_memory_address_int_t static_memory_end(struct tvm_static_memory* mem)
{
	return mem->mem_size;
}

static void static_memory_default_error_callback(const char* msg)
{
	errx(EXIT_FAILURE, "tvm static memory error: %s\n", msg); 
}

void tvm_create_static_memory(struct tvm_static_memory* mem, tvm_memory_address_int_t size,
		tvm_static_memory_error_callback_f error_callback)
{
	mem->mem_size       = size;
	mem->mem            = malloc(size);
	mem->error_callback = error_callback == NULL ? static_memory_default_error_callback : error_callback;

	memset(mem->mem, 0, size);

	mem->get = (tvm_get_mem_f) static_memory_get;
	mem->end = (tvm_end_mem_f) static_memory_end;
}

void tvm_free_static_memory(struct tvm_static_memory* mem)
{
	free(mem->mem);
}
