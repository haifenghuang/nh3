/*

    MPSL - Minimum Profit Scripting Language 3.x
    Copyright (C) 2003/2012 Angel Ortega <angel@triptico.com>

    stress.c - Stress tests.

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

#include "mpsl.h"

void mpsl_disasm(mpdm_t prg);
mpdm_t mpsl_asm(mpdm_t code);

/* total number of tests and oks */
int tests = 0;
int oks = 0;

/* also print good tests */
int verbose = 0;


/* failed tests messages */
char *failed_msgs[5000];
int i_failed_msgs = 0;


#define do_test(s, o) _do_test(s, o ,__LINE__)

void do_disasm(char *prg)
{
    mpsl_disasm(mpdm_aget(mpsl_compile(MPDM_MBS(prg)), 1));
}


void _do_test(char *prg, mpdm_t t_value, int line)
{
    mpdm_t v;
	char tmp[1024];
    int ok = 0;

    mpdm_ref(t_value);

    mpdm_hset_s(mpdm_root(), L"ERROR", NULL);

    v = mpdm_ref(mpsl_compile(MPDM_MBS(prg)));

    if (v != NULL) {
        int i = mpdm_ival(mpdm_exec(v, NULL, NULL));

        if (i == VM_IDLE) {
            if (t_value) {
                if (mpdm_cmp(mpdm_hget_s(mpdm_root(), L"T"), t_value) == 0)
                    ok = 1;
            }
            else
                ok = 1;
        }
    }

	sprintf(tmp, "stress.c:%d: error: test #%d \"%s\" (line %d): %s\n", line, tests + 1, prg, line, ok ? "OK!" : "*** Failed ***");

    if (verbose)
	   printf("%s", tmp);

    if (!ok) {
        printf("ERROR:\n");
        mpdm_dump(mpdm_hget_s(mpdm_root(), L"ERROR"));
        printf("T:\n");
        mpdm_dump(mpdm_hget_s(mpdm_root(), L"T"));
        printf("Disasm:\n");
        mpsl_disasm(mpdm_aget(v, 1));
    }

	tests++;

	if (ok)
		oks++;
	else
		failed_msgs[i_failed_msgs++] = strdup(tmp);

    mpdm_unref(v);
    mpdm_unref(t_value);
}


void test_summary(void)
{
	printf("\n*** Total tests passed: %d/%d\n", oks, tests);

	if (oks == tests)
		printf("*** ALL TESTS PASSED\n");
	else {
		int n;

		printf("*** %d %s\n", tests - oks, "TESTS ---FAILED---");

		printf("\nFailed tests:\n\n");
		for (n = 0; n < i_failed_msgs; n++)
			printf("%s", failed_msgs[n]);
	}
}


int main(int argc, char *argv[])
{
    mpdm_t v, w;

    mpsl_startup(argc, argv);

    if (argc > 1 && strcmp(argv[1], "-v") == 0)
        verbose = 1;

    /* create the T global variable */
    mpdm_hset_s(mpdm_root(), L"T", NULL);
    mpdm_hset_s(mpdm_root(), L"TT", NULL);
    mpdm_hset_s(mpdm_root(), L"L", NULL);

    do_test("1;", NULL);
    do_test("1 + 2;", NULL);
    do_test("T = 1;", MPDM_I(1));
    do_test("T = 3.14;", MPDM_R(3.14));
    do_test("T = 1 + 3;", MPDM_I(1 + 3));
    do_test("T = 1 + 3 * 5;", MPDM_I(1 + 3 * 5));
    do_test("T = (1 + 3) * 5;", MPDM_I((1 + 3) * 5));
    do_test("T = 1 + (3 * 5);", MPDM_I(1 + (3 * 5)));

    v = mpdm_ref(MPDM_A(0));
    mpdm_push(v, MPDM_I(1));
    mpdm_push(v, MPDM_I(2));
    mpdm_push(v, MPDM_I(3));

    do_test("T = [1, 2, 3];", v);

    w = mpdm_push(v, MPDM_A(0));
    mpdm_push(w, MPDM_LS(L"a"));
    mpdm_push(w, MPDM_LS(L"b"));

    do_test("T = [1, 2, 3, ['a', 'b']];", v);

    mpdm_unref(v);

    v = mpdm_ref(MPDM_H(0));
    mpdm_hset_s(v, L"ones", MPDM_I(111));
    mpdm_hset_s(v, L"twos", MPDM_I(222));

    do_test("T = { 'ones': 111, 'twos': 222 };", v);

    w = mpdm_hset_s(v, L"array", MPDM_A(0));
    mpdm_push(w, MPDM_LS(L"name"));
    mpdm_push(w, MPDM_LS(L"surname"));

    do_test("T = { 'ones': 111, 'twos': 222, 'array': [ 'name', 'surname'] };", v);

    mpdm_hset_s(v, L"twos", MPDM_I(2 + 3 * 5));

    do_test("T = { 'ones': 111, 'twos': 2 + 3 * 5, 'array': [ 'name', 'surname'] };", v);

    mpdm_unref(v);

    do_test("var addr = { name: 'angel', email: 'angel@triptico.com' };", NULL);

    do_test("var tt = 1234; T = tt;", MPDM_I(1234));
    do_test("TT = { 'name': 'me', 'host': 'varhost', 'one': 1, 'two': 2, 'german': { 'ein': 1, 'zwei': 2 } }; T = 1;", MPDM_I(1));
    do_test("T = TT.name;", MPDM_LS(L"me"));
    do_test("T = TT.one + TT.two;", MPDM_I(3));
    do_test("T = 1 + TT.two;", MPDM_I(3));
    do_test("T = TT.one + 2;", MPDM_I(3));
    do_test("TT.one = 111; T = TT.one;", MPDM_I(111));
    do_test("TT.german.ein = 222; T = TT.german.ein;", MPDM_I(222));
    do_test("L = [10, 20, 30];", NULL);
    do_test("T = L[1];", MPDM_I(20));
    do_test("L[2] = 1000; T = L[2];", MPDM_I(1000));
    do_test("L = [10, 20, 30, [1, 2, 3]];", NULL);
    do_test("L[3][0] = 123; T = L[3][0];", MPDM_I(123));
    do_test("T = TT['host'];", MPDM_LS(L"varhost"));
    do_test("T = L[1 + 1];", MPDM_I(30));
    do_test("TT['three'] = 3; T = TT.three;", MPDM_I(3));

    do_test("var n = 0; while (n < 10) n = n + 1; T = n;", MPDM_I(10));
    do_test("var n = 123; if (n < 200) { T = 1; } else T = 2;", MPDM_I(1));
    do_test("var n = 123; if (n > 200) { T = 1; } else T = 2;", MPDM_I(2));

    do_test("var pi; sub pi { return 3.14; } T = pi();", MPDM_R(3.14));
    do_test("var pi; sub pi() { return 3.14; } T = pi();", MPDM_R(3.14));
    do_test("var sum; sub sum(a, b) { return a + b; } T = sum(5, 6);", MPDM_I(11));

    do_test("sub pi { return 3.14; } T = pi();", MPDM_R(3.14));
    do_test("sub pi() { return 3.14; } T = pi();", MPDM_R(3.14));
    do_test("sub sum(a, b) { return a + b; } T = sum(5, 6);", MPDM_I(11));

    do_test("T = (1 == 1);", MPDM_I(1));
    do_test("T = (1 == 2);", MPDM_I(0));
    do_test("T = (1 == 1.0);", MPDM_I(1));
    do_test("T = (1 > 2);", MPDM_I(0));
    do_test("T = (1 < 2);", MPDM_I(1));
    do_test("T = (1 == 1.0 && 2 == 2.000);", MPDM_I(1));
    do_test("T = (1 == 2 || 2 == 2.000);", MPDM_I(1));
    do_test("var t = NULL; T = (t == NULL);", MPDM_I(1));
    do_test("var t = NULL; T = (t != NULL);", MPDM_I(0));
    do_test("T = 0; T = T == 0 && 1 || 2;", MPDM_I(1));
    do_test("T = 1; T = T == 0 && 1 || 2;", MPDM_I(2));
    do_test("T = 0; T = T < sys.time() && 1 || 2;", MPDM_I(1));

//    do_disasm("global obj = { x: 1234, get_x: NULL }; sub obj.get_x { return x; } T = obj.get_x();");
/*    do_test("global obj = { x: 1234, get_x: NULL }; sub obj.get_x { return x; } T = obj.get_x();", MPDM_I(1234));
    do_test("global obj = { x: 2, y: 3, size: NULL }; sub obj.size { return x * y; } T = obj.size();", MPDM_I(6));
    do_test("global obj = { x: 1234, get_x: NULL }; sub obj.get_x { return this.x; } T = obj.get_x();", MPDM_I(1234));
    do_test("global obj = { x: 2, y: 3, size: NULL }; sub obj.size { return this.x * this.y; } T = obj.size();", MPDM_I(6));
*/
    do_test("var pi = sub () { return 3.14; }; T = pi();", MPDM_R(3.14));
    do_test("var obj = { x: 1234, get_x: sub { return this.x; }}; T = obj.get_x();", MPDM_I(1234));
    do_test("var obj = { x: 1234, get_x: sub () { return this.x; }}; T = obj.get_x();", MPDM_I(1234));
    do_test("var obj = { x: 1234, get_fx: sub (f) { return f * this.x; }}; T = obj.get_fx(10);", MPDM_I(12340));

//    do_test("print(1234, 5678, \"--hello\\n\");", NULL);

    do_test("var n = 0; while (n < 10) n += 1; T = n;", MPDM_I(10));
    do_test("var n = 3; n *= 7; T = n;", MPDM_I(3 * 7));
    do_test("var n = 12345678; n &= 0xff; T = n;", MPDM_I(12345678 & 0xff));

    do_test("/* comment */ T = 1;", MPDM_I(1));
    do_test("T /* comment */ = 1;", MPDM_I(1));
    do_test("T =/* comment */ 1;", MPDM_I(1));
    do_test("T = /* comment */ 1;", MPDM_I(1));
    do_test("T = /* comment */1;", MPDM_I(1));
    do_test("T = 1 /* comment */;", MPDM_I(1));
    do_test("T = 1; /* comment */", MPDM_I(1));
    do_test("T /** comment **/ = 1;", MPDM_I(1));
    do_test("T = 2; // C++ style comment\nT = 1;", MPDM_I(1));

    do_test("var n = 1; ++n; T = n;", MPDM_I(2));
    do_test("var n = 10; T = ++n;", MPDM_I(11));
    do_test("var n = 1; --n; T = n;", MPDM_I(0));
    do_test("var n = 10; T = --n;", MPDM_I(9));

    do_test("T = 'a' ~ 'b';", MPDM_LS(L"ab"));
    do_test("T = [1, 2, 3] ~ ':';", MPDM_LS(L"1:2:3"));
    do_test("T = ([1, 2, 3] ~ [4, 5, 6]) ~ ':';", MPDM_LS(L"1:2:3:4:5:6"));
    do_test("T = [1, 2, 3] ~ [4, 5, 6] ~ ':';", MPDM_LS(L"1:2:3:4:5:6"));
    do_test("T = ({a: 1, b: 2} ~ '=') ~ ',';", MPDM_LS(L"a=1,b=2"));
    do_test("T = {a: 1, b: 2} ~ '=' ~ ',';", MPDM_LS(L"a=1,b=2"));

    do_test("T = 0; foreach 10 ++T;", MPDM_I(10));
    do_test("T = 0; foreach [1, 3, 7, 'a', 9] { ++T; }", MPDM_I(5));
    do_test("T = 0; foreach { 'a': 1, 'b': 2 } ++T;", MPDM_I(2));

    /* nested subroutines */
    do_test("sub circlen(r) { sub pi() { return 3.14; } return 2 * pi() * r; } T = circlen(2);", MPDM_R(6.28 * 2));

    do_test("T = 0; sub six { return 6; } T = six();", MPDM_I(6));
    do_test("T = 0; var a = {six: sub { return 6; }}; T = a.six();", MPDM_I(6));
    do_test("T = 0; var a = {}; sub a.six { return 6; } T = a.six();", MPDM_I(6));
    do_test("T = 0; var a = {b: {}}; sub a.b.six { return 6; } T = a.b.six();", MPDM_I(6));

    do_test("T = '123'.size();", MPDM_I(3));
    do_test("T = [1, 2, 3, 4].size();", MPDM_I(4));
    do_test("T = {a: 1, b: 2, c: 3}.size();", MPDM_I(3));

    do_test("var class = { key: 0, init: sub (key) { this.key = key; }}; var key = 10; class.init(key); T = class.key;", MPDM_I(10));

    /* this test should fail with "undefined symbol try" */
//    do_test("var class = { key: 0, init: sub (key) { this.key = key; }, try: sub { return 0; }}; class.init(123); try(); T = 10;", MPDM_I(10));

    do_test("#!/usr/bin/env mpsl3\nT = 10;", MPDM_I(10));

    do_test("sub sqr(c) { var v = c.read(); c.write(v * v); } var c = &sqr; c.write(1234); T = c.read();", MPDM_I(1234 * 1234));

    test_summary();

    mpsl_disasm(mpdm_aget(mpsl_asm(MPDM_LS(L"LIT 2\nLIT 3\nADD\nRET\n")), 1));

    return 0;
}
