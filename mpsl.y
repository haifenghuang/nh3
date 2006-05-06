%{
/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2006 Angel Ortega <angel@triptico.com>

    mpsl.y - Minimum Profit Scripting Language YACC parser

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    http://www.triptico.com

*/

#include <stdio.h>
#include <wchar.h>
#include "mpdm.h"
#include "mpsl.h"

/*******************
	Data
********************/

int yylex(void);
void yyerror(char * s);

/* the bytecode being generated */
static mpdm_t mpsl_bytecode = NULL;

/* cached value with the hash of opcodes */
static mpdm_t mpsl_opcodes = NULL;

/* pointer to source code being compiled */
extern wchar_t * mpsl_next_char;

/* line number */
extern int mpsl_line;

/* filename */
char * mpsl_filename = NULL;

/* pointer to file being compiled */
extern FILE * mpsl_file;

/* shortcut macros to insert instructions */

#define INS0(o)			do_ins(o, 0, NULL, NULL, NULL)
#define INS1(o,a1)		do_ins(o, 1, a1, NULL, NULL)
#define INS2(o,a1,a2)		do_ins(o, 2, a1, a2, NULL)
#define INS3(o,a1,a2,a3)	do_ins(o, 3, a1, a2, a3)

static mpdm_t do_ins(wchar_t * opcode, int args, mpdm_t a1, mpdm_t a2, mpdm_t a3);

/*******************
	Code
********************/

static mpdm_t mpsl_x(mpdm_t a1, mpdm_t a2)
{
	return(MPDM_X2(mpsl_exec_p,
		do_ins(L"SUBFRAME", a2 == NULL ? 1 : 2, a1, a2, NULL)));
}


%}

%union {
	mpdm_t v;	/* a simple value */
	mpdm_t ins;	/* an 'instruction': [ opcode, args ] */
};

%token <v> NULLV INTEGER REAL STRING SYMBOL LITERAL
%token WHILE IF SUB FOREACH LOCAL BREAK RETURN
%nonassoc IFI
%nonassoc ELSE

%left BOOLAND BOOLOR
%left INC DEC IADD ISUB IMUL IDIV IMOD
%left '!'
%left STRCAT STREQ NUMEQ STRNE NUMNE NUMGE NUMLE HASHPAIR RANGE '>''<'
%left '+' '-'
%left '*' '/' MOD
%nonassoc UMINUS

%type <ins> stmt expr sym_list stmt_list list hash compsym

%%

program:
	function		{ ; }
	;

function:
	function stmt_list	{
					mpsl_bytecode = $2;
				}
	| /* NULL */
	;

stmt:
	';'			{
					/* null instruction */
					$$ = INS0(L"MULTI");
				}
	| expr ';'		{
					/* expression, as is */
					$$ = $1;
				}

	| WHILE '(' expr ')' stmt
				{
					/* while loop */
					$$ = INS2(L"WHILE", $3, $5);
				}
	| IF '(' expr ')' stmt %prec IFI
				{
					/* if - then construction */
					$$ = INS2(L"IF", $3, $5);
				}
	| IF '(' expr ')' stmt ELSE stmt
				{
					/* if - then - else construction */
					$$ = INS3(L"IF", $3, $5, $7);
				}

	| SUB compsym '{' stmt_list '}'
				{
					/* subroutine definition,
					   without arguments */
					$$ = INS2(L"ASSIGN", $2,
						INS1(L"LITERAL",
							mpsl_x($4, NULL)));
				}

	| SUB compsym '(' ')' '{' stmt_list '}'
				{
					/* subroutine definition,
					   without arguments (second
					   syntax, including parens) */
					$$ = INS2(L"ASSIGN", $2,
						INS1(L"LITERAL",
							mpsl_x($6, NULL)));
				}

	| SUB compsym '(' sym_list ')' '{' stmt_list '}'
				{
					/* subroutine definition,
					   with arguments */
					$$ = INS2(L"ASSIGN", $2,
						INS1(L"LITERAL",
							mpsl_x($7, $4)));
				}

	| FOREACH '(' compsym ',' expr ')' stmt
				{
					/* foreach construction */
					$$ = INS3(L"FOREACH", $3, $5, $7);
				}

	| '{' stmt_list '}'	{
					/* block of instructions,
					   with local symbol table */
					$$ = INS1(L"BLKFRAME", $2);
				}

	| LOCAL sym_list ';'	{
					/* local symbol creation */
					$$ = INS1(L"LOCAL", $2);
				}
	| LOCAL SYMBOL '=' expr	';'
				{
					/* contraction; local symbol
					   creation and assignation */
					$$ = INS2(L"MULTI",
						INS1(L"LOCAL",
							INS1(L"LITERAL", $2)),
						INS2(L"ASSIGN",
							INS1(L"LITERAL", $2),$4)
						);
				}
	| BREAK ';'		{
					/* break (exit from loop) */
					$$ = INS0(L"BREAK");
				}
	| RETURN expr ';'	{
					/* return from subroutine */
					$$ = INS1(L"RETURN", $2);
				}
	| RETURN ';'		{
					/* return from subroutine (void) */
					$$ = INS0(L"RETURN");
				}
	;

stmt_list:
	stmt			{ $$ = $1; }
	| stmt_list stmt	{
					/* sequence of instructions */
					$$ = INS2(L"MULTI", $1, $2);
				}
	;

list:
	expr			{
					$$ = INS1(L"LIST", $1);
				}
	| list ',' expr		{
					/* build list from list of
					   instructions */
					mpdm_push($1, $3); $$ = $1;
				}
	;

sym_list:
	SYMBOL			{
					$$ = INS1(L"LIST",
						INS1(L"LITERAL", $1));
				}
	| sym_list ',' SYMBOL	{
					/* comma-separated list of symbols */
					mpdm_push($1,
						INS1(L"LITERAL", $3));
					$$ = $1;
				}
	;

hash:
	expr HASHPAIR expr	{
					$$ = INS2(L"HASH", $1, $3);
				}
	| hash ',' expr HASHPAIR expr
				{
					/* build hash from list of
					   instructions */
					mpdm_push($1, $3);
					mpdm_push($1, $5);
					$$ = $1;
				}
	;

compsym:
	SYMBOL			{
					$$ = INS1(L"LIST",
						INS1(L"LITERAL", $1));
				}
	| compsym '.' INTEGER	{
					/* a.5 compound symbol */
					mpdm_push($1,
				  		INS1(L"LITERAL", $3));
				  	$$ = $1;
				}
	| compsym '.' SYMBOL	{
					/* a.b compound symbol */
					mpdm_push($1,
				  		INS1(L"LITERAL", $3));
				  	$$ = $1;
				}
	| compsym '[' expr ']'	{
					/* a["b"] or a[5] compound symbol */
					mpdm_push($1, $3);
					$$ = $1;
				}
	;

expr:
	INTEGER			{
					/* literal integer */
					$$ = INS1(L"LITERAL", $1);
				}
	| STRING		{
					/* literal string */
					$$ = INS1(L"LITERAL", $1);
				}
	| REAL			{
					/* literal real number */
					$$ = INS1(L"LITERAL", $1);
				}
	| compsym		{
					/* compound symbol */
					$$ = INS1(L"SYMVAL", $1);
				}
	| NULLV			{
					/* NULL value */
					$$ = INS1(L"LITERAL", NULL);
				}

	| '-' expr %prec UMINUS	{
					/* unary minus */
					$$ = INS1(L"UMINUS", $2);
				}

				/* math operations */
	| expr '+' expr		{ $$ = INS2(L"ADD", $1, $3); }
	| expr '-' expr		{ $$ = INS2(L"SUB", $1, $3); }
	| expr '*' expr		{ $$ = INS2(L"MUL", $1, $3); }
	| expr '/' expr		{ $$ = INS2(L"DIV", $1, $3); }
	| expr MOD expr		{ $$ = INS2(L"MOD", $1, $3); }

				/* immediate math operations */
	| INC compsym		{ $$ = INS1(L"PINC", $2); }
	| compsym INC		{ $$ = INS1(L"SINC", $1); }
	| DEC compsym		{ $$ = INS1(L"PDEC", $2); }
	| compsym DEC		{ $$ = INS1(L"SDEC", $1); }
	| compsym IADD expr	{ $$ = INS2(L"IADD", $1, $3); }
	| compsym ISUB expr	{ $$ = INS2(L"ISUB", $1, $3); }
	| compsym IMUL expr	{ $$ = INS2(L"IMUL", $1, $3); }
	| compsym IDIV expr	{ $$ = INS2(L"IDIV", $1, $3); }
	| compsym IMOD expr	{ $$ = INS2(L"IMOD", $1, $3); }

	| '!' expr		{
					/* boolean not */
					$$ = INS1(L"NOT", $2);
				}
	| expr '<' expr		{
					/* bool less than */
					$$ = INS2(L"NUMLT", $1, $3);
				}
	| expr '>' expr		{
					/* bool greater than */
					$$ = INS2(L"NUMGT", $1, $3);
				}
	| expr NUMLE expr	{
					/* bool less or equal than */
					$$ = INS2(L"NUMLE", $1, $3);
				}
	| expr NUMGE expr	{
					/* bool greater or equal than */
					$$ = INS2(L"NUMGE", $1, $3);
				}
	| expr NUMEQ expr       {
					/* bool numeric equal */
					$$ = INS2(L"NUMEQ", $1, $3);
				}
	| expr NUMNE expr	{
					/* bool numeric non-equal */
					$$ = INS1(L"NOT",
						INS2(L"NUMEQ", $1, $3));
				}

	| expr STRCAT expr	{
					/* string concatenation */
					$$ = INS2(L"STRCAT", $1, $3);
				}
	| expr STREQ expr       {
					/* bool string equal */
					$$ = INS2(L"STREQ", $1, $3);
				}
	| expr STRNE expr	{
					/* bool string non-equal */
					$$ = INS1(L"NOT",
						INS2(L"STREQ", $1, $3));
				}

	| expr BOOLAND expr	{
					/* boolean and */
					$$ = INS2(L"AND", $1, $3);
				}
	| expr BOOLOR expr	{
					/* boolean or */
					$$ = INS2(L"OR", $1, $3);
				}
 
	| SUB '{' stmt_list '}'	{
					/* anonymous subroutine (without args) */
					$$ = INS1(L"LITERAL", mpsl_x($3, NULL));
				}

	| SUB '(' sym_list ')' '{' stmt_list '}'
				{
					/* anonymous subroutine (with args) */
					$$ = INS1(L"LITERAL", mpsl_x($6, $3));
				}

	| '(' expr ')'		{
					/* parenthesized expression */
					$$ = $2;
				}

	| '[' ']'		{
					/* empty list */
					$$ = INS0(L"LIST");
				}
	| '[' list ']'		{
					/* non-empty list */
					$$ = $2;
				}
	| '[' expr RANGE expr ']'
				{
					/* build range from expressions */
					$$ = INS2(L"RANGE", $2, $4);
				}

	| '{' '}'		{
					/* empty hash */
					$$ = INS0(L"HASH");
				}
	| '{' hash '}'		{
					/* non-empty hash */
					$$ = $2;
				}

	| compsym '(' ')'	{
					/* function call (without args) */
					$$ = INS1(L"EXEC", $1);
				}
	| compsym '(' list ')'	{
					/* function call (with args) */
					$$ = INS2(L"EXEC", $1, $3);
				}
	| compsym '=' expr	{
					/* simple assignation */
					$$ = INS2(L"ASSIGN", $1, $3);
				}

	;

%%

void yyerror(char * s)
{
	char tmp[1024];

	snprintf(tmp, sizeof(tmp), "%s in %s, line %d",
		s, mpsl_filename, mpsl_line + 1);

	mpsl_error(MPDM_MBS(tmp));
}


static mpdm_t do_ins(wchar_t * opcode, int args, mpdm_t a1, mpdm_t a2, mpdm_t a3)
/* adds an instruction */
{
	mpdm_t v;

	v = MPDM_A(args + 1);

	/* inserts the opcode */
	mpdm_aset(v, mpdm_hget_s(mpsl_opcodes, opcode), 0);

	if(args > 0) mpdm_aset(v, a1, 1);
	if(args > 1) mpdm_aset(v, a2, 2);
	if(args > 2) mpdm_aset(v, a3, 3);

	return(v);
}


static mpdm_t do_parse(void)
/* calls yyparse() after doing some initialisations, and returns
   the compiled code as an executable value */
{
	mpdm_t v;
	mpdm_t x = NULL;

	/* first line */
	mpsl_line = 0;

	/* reset last bytecode */
	mpsl_bytecode = NULL;

	/* cache the opcodes */
	v = mpdm_hget_s(mpdm_root(), L"MPSL");
	mpsl_opcodes = mpdm_hget_s(v, L"opcodes");

	/* compile! */
	if(yyparse() == 0 && mpsl_bytecode != NULL)
		x = mpsl_x(mpsl_bytecode, NULL);

	return(x);
}


/**
 * mpsl_compile - Compiles a string of MPSL code.
 * @code: A value containing a string of MPSL code
 *
 * Compiles a string of MPSL code and returns an mpdm value executable
 * by mpdm_exec(). If there is a syntax (or other type) error, NULL
 * is returned instead.
 */
mpdm_t mpsl_compile(mpdm_t code)
{
	mpdm_t x = NULL;

	/* point to code */
	mpsl_next_char = (wchar_t *) code->data;

	/* no real filename */
	mpsl_filename = "<INLINE>";

	mpdm_ref(code);
	x = do_parse();
	mpdm_unref(code);

	return(x);
}


static FILE * inc_fopen(char * filename)
/* loads filename, searching in INC if not directly accesible */
{
	mpdm_t a;
	FILE * f = NULL;
	char tmp[1024];

	if((a = mpsl_get_symbol(MPDM_LS(L"INC"))) != NULL)
	{
		int n;

		/* loop through INC, prepending each path
		   to the filename */
		for (n = 0;n < mpdm_size(a);n++)
		{
			mpdm_t v = mpdm_aget(a, n);

			v = MPDM_2MBS(v->data);
			snprintf(tmp, sizeof(tmp), "%s/%s",
				(char *)v->data, filename);

			if ((f = fopen(tmp, "r")) != NULL)
				break;
		}
	}

	return(f);
}


/**
 * mpsl_compile_file - Compiles a file of MPSL code.
 * @file: File stream or file name.
 *
 * Compiles a source file of MPSL code and returns an mpdm value
 * executable by mpdm_exec(). If @file is an MPSL file descriptor,
 * it's read as is and compiled; otherwise, it's assumed to be a
 * file name, that will be searched for in any of the paths defined
 * in the INC MPSL global array (take note that the current
 * directory is NOT searched by default). If the file cannot be found
 * or there is any other error, NULL is returned instead.
 */
mpdm_t mpsl_compile_file(mpdm_t file)
{
	mpdm_t x = NULL;

	if(file->flags & MPDM_FILE)
	{
		FILE ** f;

		/* it's an open file; just store the stream */
		/* FIXME: this is a hack; there should exist
		   a way to retrieve the FILE handle */
		mpsl_filename = "<FILE>";
		f = file->data;
		mpsl_file = *f;
	}
	else
	{
		/* it's a filename; open it */
		file = MPDM_2MBS(file->data);

		mpsl_filename = file->data;

		if((mpsl_file = inc_fopen(mpsl_filename)) == NULL)
		{
			char tmp[128];

			snprintf(tmp, sizeof(tmp) - 1,
				"File '%s' not found in INC",
				mpsl_filename);
			mpsl_error(MPDM_MBS(tmp));

			return(NULL);
		}
	}

	x = do_parse();

	fclose(mpsl_file);
	mpsl_file = NULL;

	return(x);
}


/**
 * mpsl_eval - Evaluates MSPL code.
 * @code: A value containing a string of MPSL code, or executable code
 * @args: optional arguments for @code
 *
 * Evaluates a piece of code. The @code can be a string containing MPSL source
 * code (that will be compiled) or a direct executable value. If the compilation
 * or the execution gives an error, the ERROR variable will be set to a printable
 * value and NULL returned. Otherwise, the exit value from the code is returned
 * and ERROR set to NULL. The abort flag is reset on exit.
 */
mpdm_t mpsl_eval(mpdm_t code, mpdm_t args)
{
	mpdm_t v = NULL;

	/* reset error */
	mpsl_error(NULL);
	mpsl_abort = 0;

	/* if code is not executable, try to compile */
	if(!MPDM_IS_EXEC(code))
		code = mpsl_compile(code);

	/* execute, if possible */
	if(MPDM_IS_EXEC(code))
		v = mpdm_exec(code, args);

	/* reset the abort flag */
	mpsl_abort = 0;

	return(v);
}
