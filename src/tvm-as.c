#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>
#include <getopt.h>

#include "tvm.h"

#define MAX_LABEL_COUNT 1024
#define MEMORY_SIZE 4096

struct tvm_memory* mem;
struct tvm_bytecode_header header = {
	.version            = TVM_BYTECODE_VERSION,
	.ext_count          = 0,
	.program_start_addr = 0
};

tvm_memory_address_int_t crnt_addr = 0;
size_t label_count                 = 0;

struct label {
	tvm_memory_address_int_t addr;
	uint32_t id;
};

static struct label labels_arr[MAX_LABEL_COUNT];
struct label* labels = labels_arr;

extern FILE* yyin;
extern void yyparse(void);

tvm_memory_address_int_t get_addr_by_label(uint32_t id)
{
	int i;

	for (i = 0; i < label_count; i++)
		if (labels[i].id == id)
			return labels[i].addr;

	errx(EXIT_FAILURE, "undefined label %i", id);
}

static void compile(const char* input, const char* output)
{
	struct tvm_static_memory static_mem;
	FILE* file;
	bool is_failure = false;

	tvm_create_static_memory(&static_mem, MEMORY_SIZE, NULL);

	mem = (struct tvm_memory*) &static_mem;

	file = fopen(output, "w");
	yyin = fopen(input, "r");

	yyparse();

	header.data_size = crnt_addr;	

	tvm_write_bytecode(file, &header, mem);

	goto done;

fail:
	is_failure = true;

done:
	tvm_free_static_memory(&static_mem);

	fclose(file);
	fclose(yyin);

	if (is_failure)
		exit(EXIT_FAILURE);
}

#define USAGE_SMALL	"tvm-as [-h] input output\n"
#define USAGE		"	-h	print usage\n" \

static void usage(FILE* file, bool small)
{
	fprintf(file, small ? USAGE_SMALL : USAGE_SMALL USAGE);
}

int main(int argc, char* argv[])
{
	const char* bytecode_file;
	const char* assember_file;

	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "h")) != -1) switch (c) {
	case 'h':
		usage(stdout, false);
		exit(EXIT_SUCCESS);

	case '?':
		warnx("invalid flag %c", optopt);
		usage(stderr, true);
		exit(EXIT_FAILURE);
	}

	if ((assember_file = argv[optind]) == NULL) {
		warnx("assembler file unspecified");
		usage(stderr, false);
		exit(EXIT_FAILURE);
	}

	if ((bytecode_file = argv[optind + 1]) == NULL) {
		warnx("bytecode file unspecified");
		usage(stderr, false);
		exit(EXIT_FAILURE);
	}

	compile(assember_file, bytecode_file);

	return 0;
}
