/*

    mpsl - Minimum Profit Scripting Language
    Copyright (C) 2003/2005 Angel Ortega <angel@triptico.com>

    stress.c - Stress tests for mpsl.

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
#include <string.h>
#include <wchar.h>

#include "mpdm.h"
#include "mpsl.h"

/* total number of tests and oks */
int tests=0;
int oks=0;

/* failed tests messages */
char * _failed_msgs[5000];
int _i_failed_msgs=0;

/*******************
	Code
********************/

void _test(char * str, int ok)
{
	char tmp[1024];

	sprintf(tmp, "%s: %s\n", str, ok ? "OK!" : "*** Failed ***");
	printf(tmp);

	tests++;

	if(ok)
		oks++;
	else
		_failed_msgs[_i_failed_msgs ++]=strdup(tmp);
}


/* tests */

mpdm_v _test_mpsl(char * code)
{
	mpdm_v v=mpsl_compile(MPDM_MBS(code));

	printf("Compile: ");
	_test(code, v != NULL);
	return(v);
}


mpdm_v _test_mpsl_file(char * file)
{
	mpdm_v v=mpsl_compile_file(MPDM_MBS(file));

	printf("Compile file: ");
	_test(file, v != NULL);
	return(v);
}


void test_mpsl(void)
{
	mpdm_v v;

	printf("mpsl (Minimum Profit Scripting Language)\n\n");

	v=mpsl_compile(MPDM_LS(L"a=1;"));
	mpdm_exec(v, NULL);

	printf("mpsl compilation tests-------------\n");

	_test_mpsl("a=1;");
	_test_mpsl("a.b.c=1;");
	_test_mpsl("a.b.c=d;");
	_test_mpsl("a.b.c=d.e.f;");
	_test_mpsl("a[\"b\"]=1;");
	_test_mpsl("a[\"b\"].c=1;");
	_test_mpsl("a[\"b\"][\"c\"]=1;");
	_test_mpsl("/* empty hash */ days={};");
	_test_mpsl("days.lunes=\"monday\";");
	_test_mpsl("days[\"martes\"]=\"tuesday\";");

	_test_mpsl("1 + ((3 - 5) * 8);");
	_test_mpsl("1.5 + ((3.1 - 5.8) * 8.0);");
	_test_mpsl("a=1 + ((3 - 5) * 8);");
	_test_mpsl("2 + 3 * 4;");
	_test_mpsl("2 * 3 + 4;");

	_test_mpsl("/* hash */ y={ 'enero' => 'january', 'febrero' => 'february' };");
	_test_mpsl("/* array */ a=[\"this\", \"one\", \"is\", 666, \"cool\"];");

	_test_mpsl("/* greatest common divisor (Euclid's algorithm) */ sub gcd(m, n) { while (m > 0) { if(n > m) { local t = m; m = n; n = t; } m -= n; } n; }");

	_test_mpsl("/* range assign */ a = [ 1 .. 1000 ];");

	_test_mpsl("a.b.c ++;");
	_test_mpsl("a.b.c --;");
	_test_mpsl("a.b.c += 100;");

	_test_mpsl("foreach (a, [ 1 .. 1000 ]) { print(a); }");

	_test_mpsl("local a;");
	_test_mpsl("local a, b, c;");
	_test_mpsl("local a = 666;");
	_test_mpsl("local a; a = 666;");

	_test_mpsl("a > b - 1;");
	_test_mpsl("a > b - 1 && a < c + 1;");

	_test_mpsl("a = NULL;");
	_test_mpsl("a = 100; b = 200;c = 300;");
	_test_mpsl("sub test {a = 100; b = 200;c = 300;}");
	_test_mpsl("sub test (d, e) {a = 100; b = 200;c = b;}");
	_test_mpsl("a();");
	_test_mpsl("! 1 > 2;");
	_test_mpsl("! (1 > 2);");
	_test_mpsl("1 != 2;");
	_test_mpsl("\"hello\" ne \"goodbye\";");

	mpdm_dump(_test_mpsl("sub test(a, b) { c=1; }"));
	mpdm_dump(_test_mpsl("sub test(a, b) { c=1; d=2; }"));
	mpdm_dump(_test_mpsl("sub test(a, b) { c=1; d=2; e=3; }"));
}


void test_mpsl2(void)
{
	mpdm_v v;
	mpdm_v w;

	/* execution tests */
	v=_test_mpsl("666;");
	mpdm_dump(v);
	v=mpdm_exec(v, NULL);
	_test("literal number", mpdm_ival(v) == 666);

	v=_test_mpsl("\"goodbye\";");
	v=mpdm_exec(v, NULL);
	_test("literal string", mpdm_cmp(v, MPDM_LS(L"goodbye")) == 0);

	v=_test_mpsl("1 + 3 + 5;");
	v=mpdm_exec(v, NULL);
	_test("mpsl calculator 1", mpdm_rval(v) == 9.0);

	v=_test_mpsl("1 + ((3 - 5) * 8);");
	v=mpdm_exec(v, NULL);
	_test("mpsl calculator 2", mpdm_rval(v) == -15.0);

	/* next value cannot be tested as an exact equality,
	   as rounding errors will manifest */
	v=_test_mpsl("1.5 + ((3.1 - 5.8) * 8.0);");
	v=mpdm_exec(v, NULL);
	_test("mpsl calculator 3",
		mpdm_rval(v) < -20.0 && mpdm_rval(v) > -21.0);

	v=_test_mpsl("2 + 3 * 4;");
	v=mpdm_exec(v, NULL);
	_test("mpsl calculator 4", mpdm_rval(v) == 14.0);

	v=_test_mpsl("2 * 3 + 4;");
	v=mpdm_exec(v, NULL);
	_test("mpsl calculator 5", mpdm_rval(v) == 10.0);

	v=mpdm_exec(_test_mpsl("2 + 3 * 4;"), NULL);
	w=mpdm_exec(_test_mpsl("2 + (3 * 4);"), NULL);
	_test("mpsl calculator 6 (operator precedence)", mpdm_rval(v) == mpdm_rval(w));

	v=mpdm_exec(_test_mpsl("2 + 3 * 4;"), NULL);
	w=mpdm_exec(_test_mpsl("(2 + 3) * 4;"), NULL);
	_test("mpsl calculator 7 (operator precedence)", mpdm_rval(v) != mpdm_rval(w));

	v=_test_mpsl("/* array */ [\"this\", \"one\", \"is\", 666, \"cool\"];");
	v=mpdm_exec(v, NULL);
	mpdm_dump(v);
	_test("mpsl array", mpdm_ival(mpdm_aget(v, 3)) == 666);

	v=_test_mpsl("/* hash */ { \"enero\" => \"january\", \"febrero\" => \"february\" };");
	v=mpdm_exec(v, NULL);
	mpdm_dump(v);
	_test("mpsl hash", mpdm_cmp(mpdm_hget(v,
		MPDM_LS(L"febrero")), MPDM_LS(L"february")) == 0);

	v=_test_mpsl("! 1;");
	v=mpdm_exec(v, NULL);
	_test("boolean not 1", v == NULL);
	v=_test_mpsl("! 0;");
	v=mpdm_exec(v, NULL);
	_test("boolean not 2", v != NULL);

	v=_test_mpsl("1 && 3;");
	v=mpdm_exec(v, NULL);
	_test("boolean and 1", mpdm_ival(v) == 3);
	v=_test_mpsl("1 && 0;");
	v=mpdm_exec(v, NULL);
	_test("boolean and 2", !mpsl_is_true(v));
	v=_test_mpsl("0 && 1;");
	v=mpdm_exec(v, NULL);
	_test("boolean and 3", !mpsl_is_true(v));

	v=_test_mpsl("1 || 3;");
	v=mpdm_exec(v, NULL);
	_test("boolean or 1", mpdm_ival(v) == 1);
	v=_test_mpsl("2 || 0;");
	v=mpdm_exec(v, NULL);
	_test("boolean or 2", mpdm_ival(v) == 2);
	v=_test_mpsl("0 || 3;");
	v=mpdm_exec(v, NULL);
	_test("boolean or 3", mpdm_ival(v) == 3);

	v=_test_mpsl("6 == 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric == 1", v != NULL);
	v=_test_mpsl("8.0 == 8.0;");
	v=mpdm_exec(v, NULL);
	_test("numeric == 2", v != NULL);
	v=_test_mpsl("6 == 8;");
	v=mpdm_exec(v, NULL);
	_test("numeric == 3", v == NULL);

	v=_test_mpsl("6 != 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric != 1", v == NULL);
	v=_test_mpsl("8.0 != 8.0;");
	v=mpdm_exec(v, NULL);
	_test("numeric != 2", v == NULL);
	v=_test_mpsl("6 != 8;");
	v=mpdm_exec(v, NULL);
	_test("numeric != 3", v != NULL);

	v=_test_mpsl("6 < 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric < 1", v == NULL);
	v=_test_mpsl("8 < 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric < 2", v == NULL);
	v=_test_mpsl("5 < 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric < 3", v != NULL);

	v=_test_mpsl("6 > 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric > 1", v == NULL);
	v=_test_mpsl("8 > 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric > 2", v != NULL);
	v=_test_mpsl("5 > 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric > 3", v == NULL);

	v=_test_mpsl("6 <= 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric <= 1", v != NULL);
	v=_test_mpsl("8 <= 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric <= 2", v == NULL);
	v=_test_mpsl("5 <= 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric <= 3", v != NULL);

	v=_test_mpsl("6 >= 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric >= 1", v != NULL);
	v=_test_mpsl("8 >= 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric >= 2", v != NULL);
	v=_test_mpsl("5 >= 6;");
	v=mpdm_exec(v, NULL);
	_test("numeric >= 3", v == NULL);

	v=_test_mpsl("11 % 6;");
	v=mpdm_exec(v, NULL);
	_test("modulo", mpdm_ival(v) == 5);

	v=_test_mpsl("variable=16384;");
	mpdm_dump(v);
	v=mpdm_exec(v, NULL);
	_test("assign 1", mpdm_ival(v) == 16384);

	v=_test_mpsl("array=[10, 20, 30, 40];");
	v=mpdm_exec(v, NULL);
	_test("assign 2", mpdm_ival(mpdm_aget(v, 2)) == 30);

	v=_test_mpsl("a=1; b=2; c=3;");
	mpdm_dump(v);
	v=mpdm_exec(v, NULL);

	v=_test_mpsl("CACHE={}; CACHE.regex=[]; CACHE.regex[0]=12345;");
	v=mpdm_exec(v, NULL);

	v=_test_mpsl("variable;");
	v=mpdm_exec(v, NULL);
	_test("symval 1", mpdm_ival(v) == 16384);

	v=_test_mpsl("variable2=1 + ((3 - 5) * 8); variable2;");
	v=mpdm_exec(v, NULL);
	_test("symval 2", mpdm_rval(v) == -15);

	v=_test_mpsl("variable3=variable2 * 2;");
	v=mpdm_exec(v, NULL);
	_test("symval 3", mpdm_ival(v) == -30);

	v=_test_mpsl("sub mysum(a, b) { a + b; }");
	mpdm_dump(v);
	v=mpdm_exec(v, NULL);
	_test("sub 1", v != NULL);

	v=_test_mpsl("sub pi() { 3.1416; }");
	mpdm_dump(v);
	v=mpdm_exec(v, NULL);
	_test("sub 2", v != NULL);

	v=_test_mpsl("var10=pi();");
	v=mpdm_exec(v, NULL);
	_test("exec 1", mpdm_rval(v) == 3.1416);

	v=_test_mpsl("var11=pi() * 10000; var11;");
	v=mpdm_exec(v, NULL);
	_test("exec 2", mpdm_rval(v) == 31416);

	v=_test_mpsl("mysum(100, 20);");
	v=mpdm_exec(v, NULL);
	_test("exec 3", mpdm_rval(v) == 120.0);

	v=_test_mpsl("a = NULL;");
	v=mpdm_exec(v, NULL);
	_test("NULL 1", v == NULL);

	v=_test_mpsl("a == NULL;");
	v=mpdm_exec(v, NULL);
	_test("NULL 2", mpdm_ival(v) == 1);

	v=_test_mpsl("local a, b; a = 1; b = 2;");
	v=mpdm_exec(v, NULL);

	v=_test_mpsl("a == NULL;");
	v=mpdm_exec(v, NULL);
	_test("local 1", mpdm_ival(v) == 1);

	v=_test_mpsl("66 * -1;");
	v=mpdm_exec(v, NULL);
	_test("uminus", mpdm_ival(v) == -66);

	v=_test_mpsl("\"test\" eq \"test\";");
	v=mpdm_exec(v, NULL);
	_test("streq 1", mpsl_is_true(v));

	v=_test_mpsl("\"test\" eq \"prueba\";");
	v=mpdm_exec(v, NULL);
	_test("streq 1", ! mpsl_is_true(v));

	v=_test_mpsl("a = 6; ++ a;");
	v=mpdm_exec(v, NULL);
	_test("pinc", mpdm_ival(v) == 7);

	v=_test_mpsl("a++;");
	v=mpdm_exec(v, NULL);
	_test("sinc", mpdm_ival(v) == 7);

	v=_test_mpsl("a += 10;");
	v=mpdm_exec(v, NULL);
	_test("iadd", mpdm_ival(v) == 18);

	v=_test_mpsl("local a, b, c; a=1; b=2; c=3; if(a == b) c=1000; c;");
	v=mpdm_exec(v, NULL);
	_test("if 1", mpdm_ival(v) == 3);

	v=_test_mpsl("local a, b, c; a=1; b=2; c=3; if(a <= b) c=1000; c;");
	v=mpdm_exec(v, NULL);
	_test("if 2", mpdm_ival(v) == 1000);

	v=_test_mpsl("local a, b, c; a=1; b=2; if(a == b) c=1000; else c=2000; c;");
	v=mpdm_exec(v, NULL);
	_test("ifelse", mpdm_ival(v) == 2000);

	v=_test_mpsl("local a; a = 0; while(a < 100) { a++; } a;");
	v=mpdm_exec(v, NULL);
	_test("ifelse", mpdm_ival(v) == 100);

	v=_test_mpsl("a=mysum(100, 50); a;");
	v=mpdm_exec(v, NULL);
	_test("mysum 1", mpdm_ival(v) == 150);

	v=_test_mpsl("a=mysum(2000, 500); a;");
	v=mpdm_exec(v, NULL);
	_test("mysum 2", mpdm_ival(v) == 2500);

	w=mpdm_ref(MPDM_A(2));
	mpdm_aset(w, MPDM_I(100), 0);
	mpdm_aset(w, MPDM_I(50), 1);

	/* asks for the value of the mysum symbol (the code) */
	v=_test_mpsl("mysum;");
	/* executes, so mysum() itself is being returned */
	v=mpdm_exec(v, NULL);
	mpdm_dump(v);
	_test("mysum 3", mpdm_ival(mpdm_exec(v, w)) == 150);

	mpdm_aset(w, MPDM_I(75), 1);
	_test("mysum 4", mpdm_ival(mpdm_exec(v, w)) == 175);

	/* compiles (and executes) the definition of gcd() */
	v=_test_mpsl("/* greatest common divisor (Euclid's algorithm) */ sub gcd(m, n) { while (m > 0) { if(n > m) { local t = m; m = n; n = t; } m -= n; } n; }");
	mpdm_exec(v, NULL);

	/* gets a pointer to gcd() */
	v=mpdm_exec(_test_mpsl("gcd;"), NULL);
	mpdm_dump(v);

	/* executes gcd(100, 50); */
	mpdm_aset(w, MPDM_I(50), 1);
	_test("gcd() 1", mpdm_ival(mpdm_exec(v, w)) == 50);

	/* executes gcd(100, 75); */
	mpdm_aset(w, MPDM_I(75), 1);
	_test("gcd() 2", mpdm_ival(mpdm_exec(v, w)) == 25);

	mpdm_unref(w);

	/* string concatenation */
	v=_test_mpsl("\"big\" ~ \" lebowski\";");
	_test("~ (strcat 1)", mpdm_cmp(mpdm_exec(v, NULL), MPDM_LS(L"big lebowski")) == 0);

	v=_test_mpsl("\"big\" ~ \" \" ~ \"lebowski\";");
	_test("~ (strcat 2)", mpdm_cmp(mpdm_exec(v, NULL), MPDM_LS(L"big lebowski")) == 0);
}


void test_mpsl3(void)
{
	mpdm_v v;

	v=_test_mpsl("v=[10,20]; w=v[0]; w;");
	mpdm_dump(v);
	v=mpdm_exec(v, NULL);
	mpdm_dump(v);

	/* library functions tests */
	v=_test_mpsl("dump( [1, 2, 3, 4, 5] );");
	mpdm_exec(v, NULL);

	v=_test_mpsl("if(size([2, 3, 4]) == 3) { dump(\"YES\"); } else { dump(\"NO\"); }");
	mpdm_exec(v, NULL);

	v=_test_mpsl("is_array(1);");
	_test("is_array 1", mpdm_exec(v, NULL) == NULL);
	v=_test_mpsl("is_array([]);");
	_test("is_array 2", mpdm_exec(v, NULL) != NULL);
	v=_test_mpsl("is_array({});");
	_test("is_array 3", mpdm_exec(v, NULL) != NULL);

	v=_test_mpsl("is_hash(1);");
	_test("is_hash 1", mpdm_exec(v, NULL) == NULL);
	v=_test_mpsl("is_hash([]);");
	_test("is_hash 2", mpdm_exec(v, NULL) == NULL);
	v=_test_mpsl("is_hash({});");
	_test("is_hash 3", mpdm_exec(v, NULL) != NULL);

	v=_test_mpsl("v=splice(\"inventions of life\", NULL, 0, 10); v[1];");
	v=mpdm_exec(v, NULL);
	_test("splice 1", mpdm_cmp(v, MPDM_LS(L"inventions")) == 0);

	v=_test_mpsl("v[0];");
	v=mpdm_exec(v, NULL);
	_test("splice 2", mpdm_cmp(v, MPDM_LS(L" of life")) == 0);

	v=_test_mpsl("sub func() { if(1 == 1) { return(6); 24; } 12; }");
	v=mpdm_exec(v, NULL);
	v=_test_mpsl("a=func();");
	v=mpdm_exec(v, NULL);
	_test("return 1", mpdm_ival(v) == 6);

	v=_test_mpsl("a=func(); 500;");
	v=mpdm_exec(v, NULL);
	_test("return 2", mpdm_ival(v) == 500);

	v=_test_mpsl("a=1; while(a < 10) { a++; } a;");
	v=mpdm_exec(v, NULL);
	_test("while", mpdm_ival(v) == 10);

	v=_test_mpsl("a=1; while(a < 10) { if(a == 5) break; a++; } a;");
	v=mpdm_exec(v, NULL);
	_test("break", mpdm_ival(v) == 5);

	v=_test_mpsl("b=0; foreach(a, [1, 2, 3, 4]) { dump(a); b += a; } return(b);");
	v=mpdm_exec(v, NULL);
	_test("foreach", mpdm_ival(v) == 10);

	v=_test_mpsl("b=0; foreach(a, [1 .. 20]) { dump(a); b += a; } return(b);");
	v=mpdm_exec(v, NULL);
	_test("foreach+range 1", mpdm_ival(v) == 210);

	v=_test_mpsl("b=0; foreach(a, [20 .. 1]) { dump(a); b += a; } return(b);");
	v=mpdm_exec(v, NULL);
	_test("foreach+range 2", mpdm_ival(v) == 210);

	v=_test_mpsl("print(\"print: \", 1, 2, [1, 2, 3], func(), 4);");
	v=mpdm_exec(v, NULL);

	v=_test_mpsl("'This is\\na string.';");
	v=mpdm_exec(v, NULL);
	mpdm_dump(v);

	v=_test_mpsl("\"This is\\na string.\";");
	v=mpdm_exec(v, NULL);
	mpdm_dump(v);

	v=_test_mpsl("sub t(h) { h.x; } H={}; H.x=5; t(h);");
	v=mpdm_exec(v, NULL);
	_test("Accesing a hash's component passed as argument", mpdm_ival(v) == 5);

/*	mpdm_dump(mpdm_root());*/
}


void test_mpsl_file(void)
{
	mpdm_v v;

	_mpsl_trace=0;

	v=_test_mpsl_file("test.mpsl");
	v=mpdm_exec(v, NULL);
}


int main(void)
{
	mpdm_startup();

	_mpsl_trace=1;
	test_mpsl();
	test_mpsl2();
	test_mpsl3();
	test_mpsl_file();

	mpdm_sweep(-1);
	mpdm_sweep(-1);
	mpdm_sweep(-1);
	mpdm_sweep(-1);
	mpdm_sweep(-1);
	mpdm_sweep(-1);
	mpdm_sweep(-1);
	mpdm_sweep(-1);
	mpdm_sweep(-1);

	mpdm_shutdown();

	printf("\n*** Total tests passed: %d/%d\n", oks, tests);

	if(oks == tests)
		printf("*** ALL TESTS PASSED\n");
	else
	{
		int n;

		printf("*** %d %s\n", tests - oks, "TESTS ---FAILED---");

		printf("\nFailed tests:\n\n");
		for(n=0;n < _i_failed_msgs;n++)
			printf("%s", _failed_msgs[n]);
	}

	return(0);
}
