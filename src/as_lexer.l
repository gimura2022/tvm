%{
#include <stdlib.h>
#include <string.h>

#include "as_parser.h"
#include "tvm.h"

void yyerror(char*); 
%}

%%

"require"		return KEYWORD_REQUIRE;
"start"			return KEYWORD_START;
"ascii"			return KEYWORD_ASCII;
"int8"			return KEYWORD_INT8;
"int16"			return KEYWORD_INT16;
"int32"			return KEYWORD_INT32;
"int64"			return KEYWORD_INT64;
"alloc"			return KEYWORD_ALLOC;

"ext"			{ yylval.command_type = TVM_EXT; return COMMAND; }
"exit"			{ yylval.command_type = TVM_EXIT; return COMMAND; }
"add"			{ yylval.command_type = TVM_ADD; return COMMAND; }
"sub"			{ yylval.command_type = TVM_SUB; return COMMAND; }
"goto"			{ yylval.command_type = TVM_GOTO; return COMMAND; }
"if"			{ yylval.command_type = TVM_IF; return COMMAND; }

"ext_io_print"		{ yylval.ext = TVM_EXT_IO_PRINT; return EXT; }
"ext_io_println"	{ yylval.ext = TVM_EXT_IO_PRINTLN; return EXT; }
"ext_io_getline"	{ yylval.ext = TVM_EXT_IO_GETLINE; return EXT; }

"ext_runtime_info_name"	{ yylval.ext = TVM_EXT_RUNTIME_INFO_NAME; return EXT; }

[0-9]+			{
				yylval.integer = atoi(yytext);
				return INTEGER;
			}

[!;:@.$?&#^]		return *yytext;
[a-zA-Z0-9_-]+		{ yylval.str = yytext; return IDENTIFIER; }
\"(\\.|[^"\\])*\"	{ yylval.str = yytext; return STRING; }

\/\*(\n|[^*])*\*\/	;
[ \n\t]			;

.			yyerror("unknown character");

%%

int yywrap(void)
{
	return 1;
} 
