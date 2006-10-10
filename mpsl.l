%{
/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2006 Angel Ortega <angel@triptico.com>

    mpsl.l - Minimum Profit Scripting Language [F]lexer

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

#include <stdlib.h>
#include "y.tab.h"

void yyerror(char * s);
int yy_input_for_flex(char * buf, int max);

/* redefinition of input function for GNU Flex */
#undef YY_INPUT
#define YY_INPUT(b,r,m) (r = yy_input_for_flex(b,m))

/* internal pointer to next character in code */
wchar_t * mpsl_next_char = NULL;

/* file stream for compiling from file */
FILE * mpsl_file = NULL;

/* line number */
int mpsl_line = 0;

/* cached value MPSL.LC */
mpdm_t mpsl_lc = NULL;

/* dynamic string manipulation macros */
#ifndef ds_init
struct ds { wchar_t * d; int p; int s; };
#define ds_init(x) do { x.d = (wchar_t *)0; x.p = x.s = 0; } while(0)
#define ds_rewind(x) x.p = 0;
#define ds_free(x) do { if(x.d) free(x.d); ds_init(x); } while(0)
#define ds_redim(x) do { if(x.p >= x.s) x.d = realloc(x.d, ++x.s * sizeof(wchar_t)); } while(0)
#define ds_poke(x,c) do { ds_redim(x); x.d[x.p++] = c; } while(0)
#define ds_pokes(x,t) do { wchar_t *p = t; while(*p) ds_poke(x, *p++); } while(0)
#endif /* ds_init */

/* a dynamic string */
struct ds ds_v;

static wchar_t * s_mbstowcs(char * str)
/* converts from mbs to wcs, using a static buffer */
{
	static wchar_t * wc = NULL;
	static int wc_s = 0;
	int n;

	/* allocs wchar_t space, if needed */
	if((n = mbstowcs(NULL, str, 0)) > wc_s)
	{
		wc_s = n;
		wc = realloc(wc, (wc_s + 1) * sizeof(wchar_t));
	}

	/* really converts */
	mbstowcs(wc, str, n);
	wc[n] = L'\0';

	return(wc);
}


static mpdm_t literal_cache(char * ptr, wchar_t * wptr)
/* the cache of literal values */
{
	mpdm_t v;

	/* convert to wchar_t */
	if(ptr != NULL)
		wptr = s_mbstowcs(ptr);

	/* search in cache */
	if((v = mpdm_hget_s(mpsl_lc, wptr)) == NULL)
	{
		v = MPDM_S(wptr);
		mpdm_hset(mpsl_lc, v, v);
	}

	return(v);
}


%}

DIGIT		[0-9]
LETTER		[a-zA-Z_]
WSPACE		[ \t\r]+
OCTDIGIT	[0-7]
HEXDIGIT	[0-9a-fA-F]

DECINTEGER	{DIGIT}+
OCTINTEGER	0{OCTDIGIT}+
HEXINTEGER	0[xX]{HEXDIGIT}+

REAL		{DIGIT}*[\.]?{DIGIT}+
SYMBOL		{LETTER}({LETTER}|{DIGIT})*
INTEGER		({DECINTEGER}|{OCTINTEGER}|{HEXINTEGER})

%x REM
%x STR

%%

{INTEGER}	{
			/* integers */
			yylval.v = literal_cache(yytext, NULL);
			return(INTEGER);
		}

{REAL}		{
			/* real numbers */
			yylval.v = literal_cache(yytext, NULL);
			return(REAL);
		}

\'[^']*\'	{
			/* single quoted string; return as is */
			yytext[yyleng - 1] = '\0';
			yylval.v = literal_cache(yytext + 1, NULL);
			return(STRING);
		}

"NULL"		return NULLV;
"while"		return WHILE;
"if"		return IF;
"else"		return ELSE;
"sub"		return SUB;
"foreach"	return FOREACH;
"local"		return LOCAL;
"break"		return BREAK;
"return"	return RETURN;

"=="		return NUMEQ;
"!="		return NUMNE;
">="		return NUMGE;
"<="		return NUMLE;
"&&"		return BOOLAND;
"||"		return BOOLOR;
"=>"		return HASHPAIR;
".."		return RANGE;
"&"		return BITAND;
"|"		return BITOR;
"^"		return BITXOR;

"eq"		return STREQ;
"ne"		return STRNE;
"~"		return STRCAT;

"++"		return INC;
"--"		return DEC;
"+="		return IADD;
"-="		return ISUB;
"*="		return IMUL;
"/="		return IDIV;
"%"		return MOD;
"%="		return IMOD;

{SYMBOL}	{
			/* symbol name */
			yylval.v = literal_cache(yytext, NULL);
			return(SYMBOL);
		}

{WSPACE}	;	/* ignore spaces */

\n		{ mpsl_line++; }

\/\*		{ BEGIN REM; /* C-like comments */ }
<REM>\*\/	{ BEGIN 0; }
<REM>\n		{ mpsl_line++; }
<REM>.		;

\"		{ BEGIN STR; ds_rewind(ds_v); }
<STR>\n		{ ds_poke(ds_v, L'\n'); mpsl_line++; }
<STR>\\n	{ ds_poke(ds_v, L'\n'); }
<STR>\\t	{ ds_poke(ds_v, L'\t'); }
<STR>\\r	{ ds_poke(ds_v, L'\r'); }
<STR>\\e	{ ds_poke(ds_v, 27); }
<STR>\\\"	{ ds_poke(ds_v, L'\"'); }
<STR>\"		{
			ds_poke(ds_v, L'\0');
			yylval.v = literal_cache(NULL, ds_v.d);
			BEGIN 0;
			return(STRING);
		}
<STR>.		{ wchar_t wc; mbtowc(&wc, yytext, 1); ds_poke(ds_v, wc); }

.		{ return(*yytext); }

%%

int yywrap(void) { return(1); }

int yy_input_for_flex(char * buf, int max)
{
	int n = 0;

	if(mpsl_file != NULL)
	{
		while(n < max)
		{
			int c;

			if((c = fgetc(mpsl_file)) == EOF)
			{
				mpsl_file = NULL;
				break;
			}

			buf[n++] = c;
		}
	}
	else
	if(mpsl_next_char != NULL)
	{
		for(;;)
		{
			char tmp[MB_CUR_MAX + 1];
			int c, i;

			if(*mpsl_next_char == L'\0' ||
			   (c = wctomb(tmp, *mpsl_next_char)) < 0)
			{
				mpsl_next_char = NULL;
				break;
			}

			/* no room? try next time */
			if(n + c >= max)
				break;

			mpsl_next_char++;

			/* transfer */
			for(i = 0;i < c;i++)
				buf[n++] = tmp[i];
		}
	}

	return(n);
}
