%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "tvm.h"

int yylex(void);
void yyerror(const char*);

extern struct tvm_memory* mem;
extern struct tvm_bytecode_header header;

extern tvm_memory_address_int_t crnt_addr;
extern size_t label_count;

struct label {
	tvm_memory_address_int_t addr;
	char name[64];
};

extern struct label* labels;

extern tvm_memory_address_int_t get_addr_by_label(const char* name);
extern void compile(const char* input);

extern FILE* yyin;
%}

%token INTEGER
%token STRING
%token CHAR

%token KEYWORD_REQUIRE
%token KEYWORD_START
%token KEYWORD_LABEL
%token KEYWORD_IMPORT
%token KEYWORD_ASCII
%token KEYWORD_INT8
%token KEYWORD_INT16
%token KEYWORD_INT32
%token KEYWORD_INT64
%token KEYWORD_ALLOC

%token COMMAND
%token EXT

%code requires {
#include <stdint.h>

#include "tvm.h"
}

%union {
	tvm_memory_address_int_t addr;
	tvm_memory_cell_int_t cell;
	int integer;
	struct tvm_command command;
	uint16_t ext;
	uint8_t command_type;
	const char* str;
};

%type <integer> INTEGER
%type <str> STRING
%type <integer> CHAR

%type <ext> EXT
%type <command_type> COMMAND

%type <command> command
%type <integer> data

%type <addr> address
%type <ext> ext

%type <cell> mem_cell

%%

program:
       | program expr ';'
       ;

expr: '@' meta
    | '$' command	{ *tvm_get_command(mem, crnt_addr) = $2; crnt_addr += TVM_COMMAND_SIZE_IN_MEMORY; }
    | '.' data
    ;

meta: KEYWORD_REQUIRE STRING		{ memcpy(header.exts[header.ext_count++], $2 + 1, strlen($2) - 2); }
    | KEYWORD_START address		{ header.program_start_addr = $2; }
    | KEYWORD_LABEL STRING 		{
       						labels[label_count].addr = crnt_addr;
						strcpy(labels[label_count].name, $2);
						label_count++;
       					}
    | KEYWORD_IMPORT STRING		{
    						FILE* old_yyin = yyin;
						compile($2);
						yyin = old_yyin;
    					}
    ;

command: COMMAND address ext		{ $$.command = $1; $$.address = $2; $$.arg0 = $3; }
       | COMMAND address ext INTEGER	{ $$.command = $1; $$.address = $2; $$.arg0 = $3; $$.arg1 = $4; }
       | COMMAND address		{ $$.command = $1; $$.address = $2; }
       | COMMAND			{ $$.command = $1; }
       ;

data: KEYWORD_ASCII STRING		{ memcpy(mem->get(mem, crnt_addr), $2 + 1, strlen($2) - 2);
    						crnt_addr += strlen($2); }
    | KEYWORD_INT8 mem_cell		{ *mem->get(mem, crnt_addr) = (uint8_t) $2; crnt_addr += 1; }
    | KEYWORD_INT16 INTEGER		{ *mem->get(mem, crnt_addr) = (uint16_t) $2; crnt_addr += 2; }
    | KEYWORD_INT32 INTEGER		{ *mem->get(mem, crnt_addr) = (uint32_t) $2; crnt_addr += 4; }
    | KEYWORD_INT64 INTEGER		{ *mem->get(mem, crnt_addr) = (uint64_t) $2; crnt_addr += 8; }
    | KEYWORD_ALLOC INTEGER mem_cell	{ memset(mem->get(mem, crnt_addr), $3, $2); crnt_addr += $2; }
    ;

address: '#' INTEGER	{ $$ = $2; }
       | '?' STRING	{ $$ = get_addr_by_label($2); }
       ;

ext: '&' EXT		{ $$ = $2; }
   | '^' INTEGER	{ $$ = $2; }
   ;

mem_cell: INTEGER	{ $$ = $1; }
	| CHAR		{ $$ = $1; }
	;

%%

void yyerror(const char* msg)
{
	errx(EXIT_FAILURE, "%s", msg);
} 
