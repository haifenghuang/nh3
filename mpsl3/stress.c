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

/* total number of tests and oks */
int tests = 0;
int oks = 0;


/* failed tests messages */
char *failed_msgs[5000];
int i_failed_msgs = 0;


#define do_test(s, o) _do_test(s, o ,__LINE__)

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

	sprintf(tmp, "Test %d: \"%s\" (line %d): %s\n", tests + 1, prg, line, ok ? "OK!" : "*** Failed ***");
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

    mpdm_startup();

    /* create the T global variable */
    mpdm_hset_s(mpdm_root(), L"T", NULL);

    do_test("1;", NULL);
    do_test("1 + 2;", NULL);
    do_test("T = 1;", MPDM_I(1));
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

    do_test("local tt = 1234; T = tt;", MPDM_I(1234));
    do_test("global TT = { 'name': 'me', 'host': 'localhost', 'one': 1, 'two': 2, 'german': { 'ein': 1, 'zwei': 2 } }; T = 1;", MPDM_I(1));
    do_test("T = TT.name;", MPDM_LS(L"me"));
    do_test("T = TT.one + TT.two;", MPDM_I(3));
    do_test("T = 1 + TT.two;", MPDM_I(3));
    do_test("T = TT.one + 2;", MPDM_I(3));
    do_test("TT.one = 111; T = TT.one;", MPDM_I(111));
    do_test("TT.german.ein = 222; T = TT.german.ein;", MPDM_I(222));
    do_test("global L = [10, 20, 30];", NULL);
    do_test("T = L[1];", MPDM_I(20));
    do_test("L[2] = 1000; T = L[2];", MPDM_I(1000));
    do_test("L = [10, 20, 30, [1, 2, 3]];", NULL);
    do_test("L[3][0] = 123; T = L[3][0];", MPDM_I(123));
    do_test("T = TT['host'];", MPDM_LS(L"localhost"));
    do_test("T = L[1 + 1];", MPDM_I(30));

    test_summary();

    return 0;
}
