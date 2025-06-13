#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include <getopt.h>

#include "tvm.h"

static void disasseble(const char* input, const char* output)
{
	bool is_failure = false;
	struct tvm_static_memory mem;
	struct tvm_bytecode_header header;
	FILE* file;
	int i;

	tvm_create_static_memory(&mem, 4096, NULL);

	if ((file = fopen(input, "r")) == NULL) {
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

	if ((file = fopen(output, "w")) == NULL) {
		warn("can't open assember file");
		goto fail;
	}

	for (i = 0; i < header.ext_count; i++) {
		fprintf(file, "@require %s\n", header.exts[i]);
	}

	fprintf(file, "@start #%i\n", header.program_start_addr);

	for (i = 0; i < header.data_size; i++)
		fprintf(file, ".byte %i\n", *mem.get((struct tvm_memory*) &mem, i));

	goto done;

fail:
	is_failure = true;

done:
	tvm_free_static_memory(&mem);
	fclose(file);

	if (is_failure)
		exit(EXIT_FAILURE);
}

#define USAGE_SMALL	"tvm-dis [-h] input output\n"
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

	if ((bytecode_file = argv[optind]) == NULL) {
		warnx("bytecode file unspecified");
		usage(stderr, false);
		exit(EXIT_FAILURE);
	}

	if ((assember_file = argv[optind + 1]) == NULL) {
		warnx("assembler file unspecified");
		usage(stderr, false);
		exit(EXIT_FAILURE);
	}

	disasseble(bytecode_file, assember_file);

	return 0;
}
