#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>
#include <getopt.h>

#include "tvm.h"

#define MAX_LABEL_COUNT 1024
#define DEFAULT_MEMORY_SIZE 4096

static tvm_memory_address_int_t memory_size = DEFAULT_MEMORY_SIZE;

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
	char name[64];
};

static struct label labels_arr[MAX_LABEL_COUNT];
struct label* labels = labels_arr;

extern FILE* yyin;
extern void yyparse(void);

tvm_memory_address_int_t get_addr_by_label(const char* name)
{
	int i;

	for (i = 0; i < label_count; i++)
		if (!strcmp(name, labels[i].name))
			return labels[i].addr;

	errx(EXIT_FAILURE, "undefined label %s", name);
}

void compile(const char* input)
{
	bool is_failure = false;

	yyin = fopen(input, "r");

	yyparse();

	header.data_size = crnt_addr;	

	goto done;

fail:
	is_failure = true;

done:
	fclose(yyin);

	if (is_failure)
		exit(EXIT_FAILURE);
}

#define USAGE_SMALL	"tvm-as [-h] [-m] input output\n"
#define USAGE		"	-h	print usage\n" \
			"	-m	set program memory size (default 4096)\n"

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
	while ((c = getopt(argc, argv, "hm:")) != -1) switch (c) {
	case 'h':
		usage(stdout, false);
		exit(EXIT_SUCCESS);

	case 'm':
		memory_size = atoi(optarg);
		break;

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

	struct tvm_static_memory static_mem;
	FILE* file;
	int exit_code = 0;

	tvm_create_static_memory(&static_mem, memory_size, NULL);
	mem = (struct tvm_memory*) &static_mem;

	compile(assember_file);

	if ((file = fopen(bytecode_file, "w")) == NULL) {
		warn("can't open output file");
		exit_code = 1;
		goto done;
	}

	tvm_write_bytecode(file, &header, mem);

done:
	fclose(file);
	tvm_free_static_memory(&static_mem);

	return exit_code;
}
