#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <err.h>

#include "tvm.h"

#define DEFAULT_MEMORY_SIZE 4096

static tvm_memory_address_int_t memory_size = DEFAULT_MEMORY_SIZE;

static void execute_file(const char* bytecode_file)
{
	bool is_failure = false;
	struct tvm_static_memory mem;
	struct tvm_bytecode_header header;
	FILE* file;

	tvm_create_static_memory(&mem, memory_size, NULL);

	if ((file = fopen(bytecode_file, "r")) == NULL) {
		warn("can't open bytecode file");
		goto fail;
	}

	if (tvm_load_bytecode(file, &header, (struct tvm_memory*) &mem) != TVM_OK) {
		warnx("can't load bytecode file");
		goto fail;
	}

	if (header.version != TVM_BYTECODE_VERSION) {
		warnx("incompatible bytecode version");
		goto fail;
	}

	fclose(file);

	for (int i = 0; i < header.ext_count; i++) {
		printf("loading %s...", header.exts[i]);

		switch (tvm_load_ext(header.exts[i])) {
		case TVM_BUILTIN:
			printf("builtin ok\n");
			break;

		default:
			warnx("can't load %s extension", header.exts[i]);
			goto fail;
		}
	}

	switch (tvm_execute((struct tvm_memory*) &mem, header.program_start_addr)) {
	case TVM_OK:
		goto done;

	case TVM_INVALID_COMMAND:	warnx("invalid command in bytecode");
		goto fail;
	case TVM_INVALID_EXT:		warnx("invalid extension in bytecode");
		goto fail;
	default:			warnx("unknown error");
		goto fail;
	}

fail:
	is_failure = true;

done:
	tvm_free_static_memory(&mem);

	if (is_failure)
		exit(EXIT_FAILURE);
}

#define USAGE_SMALL	"tvmi [-h] [-m] file\n"
#define USAGE		"	-h	print usage\n" \
			"	-m	set program memory size (default 4096)\n"

static void usage(FILE* file, bool small)
{
	fprintf(file, small ? USAGE_SMALL : USAGE_SMALL USAGE);
}

int main(int argc, char* argv[])
{
	const char* bytecode_file;

	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "hm:")) != -1) switch (c) {
	case 'm':
		memory_size = atoi(optarg);
		break;

	case 'h':
		usage(stdout, false);
		exit(EXIT_SUCCESS);

	case '?':
		warnx("invalid flag %c", optopt);
		usage(stderr, true);
		exit(EXIT_FAILURE);
	}

	if ((bytecode_file = argv[optind]) == NULL) {
		warnx("bytecode file unspecified");
		usage(stderr, false);
		exit(EXIT_FAILURE);
	}

	execute_file(bytecode_file);

	return 0;
}
