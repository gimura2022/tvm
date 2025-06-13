#include <stdio.h>

#include "tvm.h"
#include "utils.h"

#define header_structure(file, header) \
	do { \
		field(file, &header->version); \
		field(file, &header->data_size); \
		field(file, &header->program_start_addr); \
		field(file, &header->ext_count); \
		arr(file, header->exts, header->ext_count); \
	} while (0);

#define read_arr(file, x, n) \
	if (fread(x, sizeof(*x), n, file) != n) \
		return TVM_UNEXCEPTED_BYTECODE_END; \

#define read_field(file, x) read_arr(file, x, 1)

static int load_header(FILE* file, struct tvm_bytecode_header* header)
{
#	define field read_field
#	define arr read_arr

	header_structure(file, header);

#	undef field
#	undef arr

	return TVM_OK;
}

int tvm_load_bytecode(FILE* file, struct tvm_bytecode_header* header, struct tvm_memory* mem)
{
	int i;

	unwrap(load_header(file, header));

	for (i = 0; i < header->data_size; i++)
		read_field(file, mem->get(mem, i));

	return TVM_OK;
}

#define write_arr(file, x, n) \
	if (fwrite(x, sizeof(*x), n, file) != n) \
		return TVM_UNEXCEPTED_BYTECODE_END; \

#define write_field(file, x) write_arr(file, x, 1)

static int write_header(FILE* file, const struct tvm_bytecode_header* header)
{
#	define field write_field
#	define arr write_arr

	header_structure(file, header);

#	undef field
#	undef arr

	return TVM_OK;
}

int tvm_write_bytecode(FILE* file, const struct tvm_bytecode_header* header, struct tvm_memory* mem)
{
	int i;

	unwrap(write_header(file, header));

	for (i = 0; i < header->data_size; i++)
		write_field(file, mem->get(mem, i));

	return TVM_OK;
}
