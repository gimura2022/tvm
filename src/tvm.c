#include <string.h>

#include "tvm.h"
#include "utils.h"

struct tvm_command* tvm_get_command(struct tvm_memory* mem, tvm_memory_address_int_t addr)
{
	return (struct tvm_command*) mem->get(mem, addr);
}
