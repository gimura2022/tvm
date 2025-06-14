%{
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
	uint32_t id;
};

extern struct label* labels;

extern tvm_memory_address_int_t get_addr_by_label(uint32_t id);
%}

%token INTEGER
%token IDENTIFIER
%token STRING

%token KEYWORD_REQUIRE
%token KEYWORD_START
%token KEYWORD_ASCII
%token KEYWORD_BYTE
%token KEYWORD_ALLOC

%token COMMAND
%token EXT

%code requires {
#include <stdint.h>

#include "tvm.h"
}

%union {
	tvm_memory_address_int_t addr;
	int integer;
	struct tvm_command command;
	uint16_t ext;
	uint8_t command_type;
	const char* str;
};

%type <integer> INTEGER
%type <str> IDENTIFIER
%type <str> STRING

%type <ext> EXT
%type <command_type> COMMAND

%type <command> command
%type <integer> data

%type <addr> address
%type <ext> ext

%%

program:
       | program '!' INTEGER ':'	{
       						labels[label_count].addr = crnt_addr;
						labels[label_count].id = $3;
						label_count++;
       					}
       | program expr ';'
       ;

expr: '@' meta
    | '$' command	{ *tvm_get_command(mem, crnt_addr) = $2; crnt_addr += TVM_COMMAND_SIZE_IN_MEMORY; }
    | '.' data
    ;

meta: KEYWORD_REQUIRE IDENTIFIER	{ strcpy(header.exts[header.ext_count++], $2); }
    | KEYWORD_START address		{ header.program_start_addr = $2; }
    ;

command: COMMAND address ext		{ $$.command = $1; $$.address = $2; $$.arg0 = $3; }
       | COMMAND address ext INTEGER	{ $$.command = $1; $$.address = $2; $$.arg0 = $3; $$.arg1 = $4; }
       | COMMAND address		{ $$.command = $1; $$.address = $2; }
       | COMMAND			{ $$.command = $1; }
       ;

data: KEYWORD_ASCII STRING		{ memcpy(mem->get(mem, crnt_addr), $2 + 1, strlen($2) - 2);
    						crnt_addr += strlen($2); }
    | KEYWORD_BYTE INTEGER		{ *mem->get(mem, crnt_addr) = $2; crnt_addr += 1; }
    | KEYWORD_ALLOC INTEGER INTEGER	{ memset(mem->get(mem, crnt_addr), $3, $2); crnt_addr += $2; }
    ;

address: '#' INTEGER	{ $$ = $2; }
       | '?' INTEGER	{ $$ = get_addr_by_label($2); }
       ;

ext: '&' EXT		{ $$ = $2; }
   | '^' INTEGER	{ $$ = $2; }
   ;

%%

void yyerror(const char* msg)
{
	errx(EXIT_FAILURE, "%s", msg);
} 
