/*

    mpdm - Minimum Profit Data Manager
    Copyright (C) 2003/2004 Angel Ortega <angel@triptico.com>

    stress.c - Stress tests for mpdm.

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

#include "mpdm.h"

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

void test_basic(void)
{
	int i;
	mpdm_v v;
	mpdm_v w;

	v=MPDM_S(L"65536");
	mpdm_dump(v);
	i=mpdm_ival(v);

	_test("i == 65536", (i == 65536));
	_test("v has MPDM_IVAL", (v->flags & MPDM_IVAL));

	printf("mpdm_string: %ls\n", mpdm_string(MPDM_H(0)));
	printf("mpdm_string: %ls\n", mpdm_string(MPDM_H(0)));

	/* partial copies of strings */
	v=MPDM_LS(L"this is not America");
	v=MPDM_NS((wchar_t *)v->data + 4, 4);

	_test("Partial string values", mpdm_cmp(v, MPDM_LS(L" is ")) == 0);

	v=MPDM_S(L"MUAHAHAHA!");
	_test("Testing mpdm_clone semantics 1", mpdm_clone(v) == v);

	v=MPDM_A(2);
	mpdm_aset(v, MPDM_S(L"evil"), 0);
	mpdm_aset(v, MPDM_S(L"dead"), 1);
	w=mpdm_clone(v);

	_test("Testing mpdm_clone semantics 2.1", w != v);

	v=mpdm_aget(v, 0);
	_test("Testing mpdm_clone semantics 2.2", v->ref > 1);
	_test("Testing mpdm_clone semantics 2.3", mpdm_aget(w, 0) == v);

	/* mbs / wcs tests */
	v=MPDM_MBS("This is (was) a multibyte string");
	mpdm_dump(v);

	/* greek omega is 03a9 */

	v=MPDM_LS(L"�Espa�a! (non-ASCII string, as wchar_t *)");
	mpdm_dump(v);

	v=MPDM_MBS("�Espa�a! (non-ASCII string, as ISO-8859-1 char *)");
	mpdm_dump(v);
	printf("(Previous value will be NULL if locale doesn't match stress.c encoding)\n");

	v=MPDM_2MBS(L"�Espa�a! (should be correcly seen in any locale)");

	if(v == NULL)
		printf("Warning: can't convert to current locale. Broken locales?\n");
	else
		printf("%s\n", (char *)v->data);

	v=MPDM_LS(L"A capital greek omega between brackets [\x03a9]");
	mpdm_dump(v);
	printf("(Previous value will only show on an Unicode terminal)\n");
}

mpdm_v _asort_cb(mpdm_v args)
{
	int d;

	/* sorts reversely */
	d=mpdm_cmp(mpdm_aget(args, 1), mpdm_aget(args, 0));
	return(MPDM_I(d));
}


void test_array(void)
{
	int n;
	mpdm_v a;
	mpdm_v v;

	a=MPDM_A(0);
	_test("a->size == 0", (a->size == 0));

	mpdm_apush(a, MPDM_LS(L"sunday"));
	mpdm_apush(a, MPDM_LS(L"monday"));
	mpdm_apush(a, MPDM_LS(L"tuesday"));
	mpdm_apush(a, MPDM_LS(L"wednesday"));
	mpdm_apush(a, MPDM_LS(L"thursday"));
	mpdm_apush(a, MPDM_LS(L"friday"));
	mpdm_apush(a, MPDM_LS(L"saturday"));
	mpdm_dump(a);
	_test("a->size == 7", (a->size == 7));

	v=mpdm_aset(a, NULL, 3);
	_test("v->ref == 0", (v->ref == 0));
	mpdm_dump(a);

	mpdm_asort(a, 1);
	_test("NULLs are sorted on top", (mpdm_aget(a, 0) == NULL));

	mpdm_aset(a, v, 0);
	v=mpdm_aget(a, 3);
	_test("v is referenced again", (v->ref > 0));

	mpdm_asort(a, 1);
	_test("mpdm_asort() works (1)",
		mpdm_cmp(mpdm_aget(a,0), MPDM_LS(L"friday")) == 0);
	_test("mpdm_asort() works (2)",
		mpdm_cmp(mpdm_aget(a,6), MPDM_LS(L"wednesday")) == 0);

	/* _asort_cb sorts reversely */
	mpdm_asort_cb(a, 1, MPDM_X(_asort_cb));

	_test("mpdm_asort_cb() works (1)",
		mpdm_cmp(mpdm_aget(a,6), MPDM_LS(L"friday")) == 0);
	_test("mpdm_asort_cb() works (2)",
		mpdm_cmp(mpdm_aget(a,0), MPDM_LS(L"wednesday")) == 0);

	n=v->ref;
	v=mpdm_aget(a, 3);
	mpdm_acollapse(a, 3, 1);
	_test("acollapse unrefs values", (v->ref < n));

	/* test queues */
	a=MPDM_A(0);

	/* add several values */
	for(n=0;n < 10;n++)
		v=mpdm_aqueue(a, MPDM_I(n), 10);

	_test("aqueue should still output NULL", (v == NULL));

	v=mpdm_aqueue(a, MPDM_I(11), 10);
	_test("aqueue should no longer output NULL", (v != NULL));

	v=mpdm_aqueue(a, MPDM_I(12), 10);
	_test("aqueue should return 1", mpdm_ival(v) == 1);
	v=mpdm_aqueue(a, MPDM_I(13), 10);
	_test("aqueue should return 2", mpdm_ival(v) == 2);
	_test("queue size should be 10", a->size == 10);

	mpdm_dump(a);
	v=mpdm_aqueue(a, MPDM_I(14), 5);
	mpdm_dump(a);

	_test("queue size should be 5", a->size == 5);
	_test("last taken value should be 8", mpdm_ival(v) == 8);

	a=MPDM_A(4);
	mpdm_aset(a, MPDM_I(666), 6000);

	_test("array should have been automatically expanded",
		mpdm_size(a) == 6001);

	v=mpdm_aget(a, -1);
	_test("negative offsets in arrays 1", mpdm_ival(v) == 666);

	mpdm_aset(a, MPDM_I(777), -2);
	v=mpdm_aget(a, 5999);
	_test("negative offsets in arrays 2", mpdm_ival(v) == 777);

	mpdm_apush(a, MPDM_I(888));
	v=mpdm_aget(a, -1);
	_test("negative offsets in arrays 3", mpdm_ival(v) == 888);
}


void test_hash(void)
{
	mpdm_v h;
	mpdm_v v;
	int i, n;

	h=MPDM_H(0);

	mpdm_hset(h, MPDM_S(L"mp"), MPDM_I(6));
	v=mpdm_hget(h, MPDM_S(L"mp"));

	_test("hash: v != NULL", (v != NULL));
	i=mpdm_ival(v);
	_test("hash: v == 6", (i == 6));

	mpdm_hset(h, MPDM_S(L"mp2"), MPDM_I(66));
	v=mpdm_hget(h, MPDM_S(L"mp2"));

	_test("hash: v != NULL", (v != NULL));
	i=mpdm_ival(v);
	_test("hash: v == 66", (i == 66));

	/* fills 100 values */
	for(n=0;n < 50;n++)
		mpdm_hset(h, MPDM_I(n), MPDM_I(n * 10));
	for(n=100;n >= 50;n--)
		mpdm_hset(h, MPDM_I(n), MPDM_I(n * 10));

	/* tests 100 values */
	for(n=0;n < 100;n++)
	{
		v=mpdm_hget(h, MPDM_I(n));
		_test("hash: hget", (v != NULL));

		if(v != NULL)
		{
			i=mpdm_ival(v);
			_test("hash: ival", (i == n * 10));
		}
	}

	printf("h's size: %d\n", mpdm_hsize(h));

	mpdm_dump(h);

	v=mpdm_hkeys(h);
	mpdm_dump(v);

	/* use of non-strings as hashes */
	h=MPDM_H(0);

	v=MPDM_A(0);
	mpdm_hset(h, v, MPDM_I(1234));
	v=MPDM_H(0);
	mpdm_hset(h, v, MPDM_I(12345));
	v=MPDM_H(0);
	mpdm_hset(h, v, MPDM_I(9876));
	v=MPDM_A(0);
	mpdm_hset(h, v, MPDM_I(6543));
	i=mpdm_ival(mpdm_hget(h, v));

	mpdm_dump(h);
	_test("hash: using non-strings as hash keys", (i == 6543));

	mpdm_hset(h, MPDM_LS(L"ok"), MPDM_I(666));

	_test("hexists 1", mpdm_hexists(h, MPDM_LS(L"ok")));
	_test("hexists 2", ! mpdm_hexists(h, MPDM_LS(L"notok")));
}


void test_splice(void)
{
	mpdm_v w;
	mpdm_v v;

	w=mpdm_splice(MPDM_LS(L"I'm agent Johnson"),
		MPDM_LS(L"special "), 4, 0);
	_test("splice insertion", 
		mpdm_cmp(mpdm_aget(w, 0),
		MPDM_LS(L"I'm special agent Johnson")) == 0);
	mpdm_dump(w);

	w=mpdm_splice(MPDM_LS(L"Life is a shit"), MPDM_LS(L"cheat"), 10, 4);
	_test("splice insertion and deletion (1)", 
		mpdm_cmp(mpdm_aget(w, 0), MPDM_LS(L"Life is a cheat")) == 0);
	_test("splice insertion and deletion (2)", 
		mpdm_cmp(mpdm_aget(w, 1), MPDM_LS(L"shit")) == 0);
	mpdm_dump(w);

	w=mpdm_splice(MPDM_LS(L"I'm with dumb"), NULL, 4, 4);
	_test("splice deletion (1)", 
		mpdm_cmp(mpdm_aget(w, 0), MPDM_LS(L"I'm  dumb")) == 0);
	_test("splice deletion (2)", 
		mpdm_cmp(mpdm_aget(w, 1), MPDM_LS(L"with")) == 0);
	mpdm_dump(w);

	v=MPDM_LS(L"It doesn't matter");
	w=mpdm_splice(v, MPDM_LS(L" two"), v->size, 0);
	_test("splice insertion at the end", 
		mpdm_cmp(mpdm_aget(w, 0),
		MPDM_LS(L"It doesn't matter two")) == 0);
	mpdm_dump(w);

	w=mpdm_splice(NULL, NULL, 0, 0);
	_test("splice with two NULLS", (mpdm_aget(w, 0) == NULL));

	w=mpdm_splice(NULL, MPDM_LS(L"foo"), 0, 0);
	_test("splice with first value NULL",
		(mpdm_cmp(mpdm_aget(w, 0), MPDM_LS(L"foo")) == 0));

	w=mpdm_splice(MPDM_LS(L"foo"), NULL, 0, 0);
	_test("splice with second value NULL",
		(mpdm_cmp(mpdm_aget(w, 0), MPDM_LS(L"foo")) == 0));
}


void test_asplit(void)
{
	mpdm_v w;

	printf("mpdm_asplit test\n\n");

	w=mpdm_asplit(MPDM_S(L"."), MPDM_S(L"four.elems.in.string"));
	mpdm_dump(w);
	_test("4 elems: ", (w->size == 4));

	w=mpdm_asplit(MPDM_S(L"."), MPDM_S(L"unseparated string"));
	mpdm_dump(w);
	_test("1 elem: ", (w->size == 1));

	w=mpdm_asplit(MPDM_S(L"."), MPDM_S(L".dot.at start"));
	mpdm_dump(w);
	_test("3 elems: ", (w->size == 3));

	w=mpdm_asplit(MPDM_S(L"."), MPDM_S(L"dot.at end."));
	mpdm_dump(w);
	_test("3 elems: ", (w->size == 3));

	w=mpdm_asplit(MPDM_S(L"."), MPDM_S(L"three...dots (two empty elements)"));
	mpdm_dump(w);
	_test("4 elems: ", (w->size == 4));

	w=mpdm_asplit(MPDM_S(L"."), MPDM_S(L"."));
	mpdm_dump(w);
	_test("2 elems: ", (w->size == 2));
}


void test_ajoin(void)
{
	mpdm_v v;
	mpdm_v s;
	mpdm_v w;

	printf("mpdm_ajoin test\n\n");

	/* separator */
	s=MPDM_LS(L"--");

	w=MPDM_A(1);
	mpdm_aset(w, MPDM_S(L"ce"), 0);

	v=mpdm_ajoin(NULL, w);
	_test("1 elem, no separator", (mpdm_cmp(v, MPDM_LS(L"ce")) == 0));

	v=mpdm_ajoin(s, w);
	_test("1 elem, '--' separator", (mpdm_cmp(v, MPDM_LS(L"ce")) == 0));

	mpdm_apush(w, MPDM_LS(L"n'est"));
	v=mpdm_ajoin(s, w);
	_test("2 elems, '--' separator", (mpdm_cmp(v, MPDM_LS(L"ce--n'est")) == 0));

	mpdm_apush(w, MPDM_LS(L"pas"));
	v=mpdm_ajoin(s, w);
	_test("3 elems, '--' separator", (mpdm_cmp(v, MPDM_LS(L"ce--n'est--pas")) == 0));

	v=mpdm_ajoin(NULL, w);
	_test("3 elems, no separator", (mpdm_cmp(v, MPDM_LS(L"cen'estpas")) == 0));
}


void test_sym(void)
{
	mpdm_v v;
	int i;

	printf("mpdm_sset / mpdm_sget tests\n\n");

	mpdm_sset(NULL, MPDM_LS(L"mp"), MPDM_H(7));
	mpdm_sset(NULL, MPDM_LS(L"mp.config"), MPDM_H(7));
	mpdm_sset(NULL, MPDM_LS(L"mp.config.auto_indent"), MPDM_I(16384));
	mpdm_sset(NULL, MPDM_LS(L"mp.config.use_regex"), MPDM_I(1357));
	mpdm_sset(NULL, MPDM_LS(L"mp.config.gtk_font_face"), MPDM_LS(L"profontwindows"));
	mpdm_sset(NULL, MPDM_LS(L"mp.lines"), MPDM_A(2));
	mpdm_sset(NULL, MPDM_LS(L"mp.lines.0"), MPDM_LS(L"First post!"));
	mpdm_sset(NULL, MPDM_LS(L"mp.lines.1"), MPDM_LS(L"Second post!"));
	mpdm_dump(mpdm_root());

	v=mpdm_sget(NULL, MPDM_LS(L"mp.config.auto_indent"));
	i=mpdm_ival(v);

	_test("auto_indent == 16384", (i == 16384));

	mpdm_sweep(-1);
}


void test_file(void)
{
	mpdm_v f;
	mpdm_v v;
	mpdm_v w;

	f=mpdm_open(MPDM_LS(L"test.txt"), MPDM_LS(L"w"));
	_test("Create test.txt", f != NULL);

	mpdm_write(f, MPDM_LS(L"0"));
	mpdm_write(f, MPDM_LS(L"1"));

	/* test an array */
	v=MPDM_A(4);
	mpdm_aset(v, MPDM_LS(L"2.0"), 0);
	mpdm_aset(v, MPDM_LS(L"2.1"), 1);
	mpdm_aset(v, MPDM_LS(L"2.2"), 2);

	w=MPDM_A(2);
	mpdm_aset(w, MPDM_LS(L"3.0.0"), 0);
	mpdm_aset(w, MPDM_LS(L"3.0.1"), 1);
	mpdm_aset(v, w, 3);

	mpdm_write(f, v);

	/* write its own file pointer */
	mpdm_write(f, f);

	mpdm_close(f);

	f=mpdm_open(MPDM_LS(L"test.txt"), MPDM_LS(L"r"));

	_test("test written file 0", mpdm_cmp(mpdm_read(f), MPDM_LS(L"0")) == 0);
	_test("test written file 1", mpdm_cmp(mpdm_read(f), MPDM_LS(L"1")) == 0);
	_test("test written file 2.0", mpdm_cmp(mpdm_read(f), MPDM_LS(L"2.0")) == 0);
	_test("test written file 2.1", mpdm_cmp(mpdm_read(f), MPDM_LS(L"2.1")) == 0);
	_test("test written file 2.2", mpdm_cmp(mpdm_read(f), MPDM_LS(L"2.2")) == 0);
	_test("test written file 3.0.1", mpdm_cmp(mpdm_read(f), MPDM_LS(L"3.0.0")) == 0);
	_test("test written file 3.0.2", mpdm_cmp(mpdm_read(f), MPDM_LS(L"3.0.1")) == 0);
	v=mpdm_read(f);
	mpdm_dump(v);

	mpdm_close(f);

	mpdm_unlink(MPDM_LS(L"test.txt"));
	_test("unlink", mpdm_open(MPDM_LS(L"test.txt"), MPDM_LS(L"r")) == NULL);

/*	v=mpdm_glob(MPDM_LS(L"*"));*/
	printf("Glob:\n");
	v=mpdm_glob(NULL);
	mpdm_dump(v);
}


void test_regex(void)
{
	mpdm_v v;
	mpdm_v w;

	v=mpdm_regex(MPDM_LS(L"/[0-9]+/"), MPDM_LS(L"123456"), 0);
	_test("regex 0", v != NULL);

	v=mpdm_regex(MPDM_LS(L"/[0-9]+/"), MPDM_I(65536), 0);
	_test("regex 1", v != NULL);

	v=mpdm_regex(MPDM_LS(L"/^[0-9]+$/"), MPDM_LS(L"12345678"), 0);
	_test("regex 2", v != NULL);

	v=mpdm_regex(MPDM_LS(L"/^[0-9]+$/"), MPDM_I(1), 0);
	_test("regex 3", v != NULL);

	v=mpdm_regex(MPDM_LS(L"/^[0-9]+$/"), MPDM_LS(L"A12345-678"), 0);
	_test("regex 4", v == NULL);

	w=MPDM_LS(L"Hell street, 666");
	v=mpdm_regex(MPDM_LS(L"/[0-9]+/"), w, 0);
	_test("regex 5", mpdm_cmp(v, MPDM_I(666)) == 0);

	mpdm_dump(v);

	v=mpdm_regex(MPDM_LS(L"/regex/i"), MPDM_LS(L"CASE-INSENSITIVE REGEX"), 0);
	_test("regex 6", v != NULL);

	v=mpdm_regex(MPDM_LS(L"/^\\s*/"), MPDM_LS(L"123456"), 0);
	_test("regex 7", v != NULL);

	v=mpdm_regex(MPDM_LS(L"/^\\s+/"), MPDM_LS(L"123456"), 0);
	_test("regex 8", v == NULL);

	/* sregex */

	v=mpdm_sregex(MPDM_LS(L"/A/"),MPDM_LS(L"change all A to A"),
		MPDM_LS(L"E"),0);
	_test("sregex 0", mpdm_cmp(v, MPDM_LS(L"change all E to A")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/A/g"),MPDM_LS(L"change all A to A"),
		MPDM_LS(L"E"),0);
	_test("sregex 1", mpdm_cmp(v, MPDM_LS(L"change all E to E")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/A+/g"),MPDM_LS(L"change all AAAAAA to E"),
		MPDM_LS(L"E"),0);
	_test("sregex 2", mpdm_cmp(v, MPDM_LS(L"change all E to E")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/A+/g"),MPDM_LS(L"change all A A A A A A to E"),
		MPDM_LS(L"E"),0);
	_test("sregex 3", mpdm_cmp(v, MPDM_LS(L"change all E E E E E E to E")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/A+/g"),MPDM_LS(L"change all AAA A AA AAAAA A AAA to E"),
		MPDM_LS(L"E"),0);
	_test("sregex 3.2", mpdm_cmp(v, MPDM_LS(L"change all E E E E E E to E")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/[0-9]+/g"),MPDM_LS(L"1, 20, 333, 40 all are numbers"),
		MPDM_LS(L"numbers"),0);
	_test("sregex 4", mpdm_cmp(v, MPDM_LS(L"numbers, numbers, numbers, numbers all are numbers")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/[a-zA-Z_]+/g"),MPDM_LS(L"regex, mpdm_regex, TexMex"),
		MPDM_LS(L"sex"),0);
	_test("sregex 5", mpdm_cmp(v, MPDM_LS(L"sex, sex, sex")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/[a-zA-Z]+/g"),MPDM_LS(L"regex, mpdm_regex, TexMex"),
		NULL,0);
	_test("sregex 6", mpdm_cmp(v, MPDM_LS(L", _, ")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/\\\\/g"),MPDM_LS(L"\\MSDOS\\style\\path"),
		MPDM_LS(L"/"),0);
	_test("sregex 7", mpdm_cmp(v, MPDM_LS(L"/MSDOS/style/path")) == 0);

	v=mpdm_sregex(MPDM_LS(L"/regex/gi"),MPDM_LS(L"regex, Regex, REGEX"),
		MPDM_LS(L"sex"),0);
	_test("sregex 8", mpdm_cmp(v, MPDM_LS(L"sex, sex, sex")) == 0);

	/* multiple regex tests */
	w=MPDM_A(0);

	mpdm_apush(w, MPDM_LS(L"/^[ \t]*/"));
	mpdm_apush(w, MPDM_LS(L"/[^ \t=]+/"));
	mpdm_apush(w, MPDM_LS(L"/[ \t]*=[ \t]*/"));
	mpdm_apush(w, MPDM_LS(L"/[^ \t]+/"));
	mpdm_apush(w, MPDM_LS(L"/[ \t]*$/"));

	v=mpdm_regex(w, MPDM_LS(L"key=value"), 0);
	_test("multi-regex 1.1", mpdm_cmp(mpdm_aget(v, 1),MPDM_LS(L"key")) == 0);
	_test("multi-regex 1.2", mpdm_cmp(mpdm_aget(v, 3),MPDM_LS(L"value")) == 0);

	v=mpdm_regex(w, MPDM_LS(L" key = value"), 0);
	_test("multi-regex 2.1", mpdm_cmp(mpdm_aget(v, 1),MPDM_LS(L"key")) == 0);
	_test("multi-regex 2.2", mpdm_cmp(mpdm_aget(v, 3),MPDM_LS(L"value")) == 0);

	v=mpdm_regex(w, MPDM_LS(L"\t\tkey\t=\tvalue  "), 0);
	_test("multi-regex 3.1", mpdm_cmp(mpdm_aget(v, 1),MPDM_LS(L"key")) == 0);
	_test("multi-regex 3.2", mpdm_cmp(mpdm_aget(v, 3),MPDM_LS(L"value")) == 0);

	v=mpdm_regex(w, MPDM_LS(L"key= "), 0);
	_test("multi-regex 4", v == NULL);
}


static mpdm_v _dumper(mpdm_v args)
/* executable value */
{
	mpdm_dump(args);
	return(NULL);
}


static mpdm_v _sum(mpdm_v args)
/* executable value: sum all args */
{
	int n,t=0;

	if(args != NULL)
	{
		for(n=t=0;n < args->size;n++)
			t+=mpdm_ival(mpdm_aget(args, n));
	}

	return(MPDM_I(t));
}


static mpdm_v _calculator(mpdm_v c, mpdm_v args)
/* 2 argument version: calculator. c contains a 'script' to
   do things with the arguments */
{
	int n,t;
	mpdm_v v;
	mpdm_v a;
	mpdm_v o;

	/* to avoid destroying args */
	a=mpdm_clone(args);

	/* unshift first argument */
	v=mpdm_adel(a, 0);
	t=mpdm_ival(v);

	for(n=0;n < mpdm_size(c);n++)
	{
		/* gets operator */
		o=mpdm_aget(c, n);

		/* gets next value */
		v=mpdm_adel(a, 0);

		switch(*(wchar_t *)o->data)
		{
		case '+': t += mpdm_ival(v); break;
		case '-': t -= mpdm_ival(v); break;
		case '*': t *= mpdm_ival(v); break;
		case '/': t /= mpdm_ival(v); break;
		}
	}

	return(MPDM_I(t));
}


void test_exec(void)
{
	mpdm_v x;
	mpdm_v w;
	mpdm_v p;

	x=MPDM_X(_dumper);

	/* a simple value */
	mpdm_exec(x, NULL);
	mpdm_exec(x, x);

	x=MPDM_X(_sum);
	w=MPDM_A(3);
	mpdm_aset(w, MPDM_I(100), 0);
	mpdm_aset(w, MPDM_I(220), 1);
	mpdm_aset(w, MPDM_I(333), 2);

	_test("exec 0", mpdm_ival(mpdm_exec(x, w)) == 653);

	mpdm_apush(w, MPDM_I(1));

	/* multiple executable value: vm and compiler support */

	/* calculator 'script' */
	p=MPDM_A(0);
	mpdm_apush(p, MPDM_LS(L"+"));
	mpdm_apush(p, MPDM_LS(L"-"));
	mpdm_apush(p, MPDM_LS(L"+"));

	/* the value */
	x=MPDM_A(2);
	x->flags |= MPDM_EXEC;

	mpdm_aset(x, MPDM_X(_calculator), 0);
	mpdm_aset(x, p, 1);

	_test("exec 1", mpdm_ival(mpdm_exec(x, w)) == -12);

	/* another 'script', different operations with the same values */
	p=MPDM_A(0);
	mpdm_apush(p, MPDM_LS(L"*"));
	mpdm_apush(p, MPDM_LS(L"/"));
	mpdm_apush(p, MPDM_LS(L"+"));

	mpdm_aset(x, p, 1);

	_test("exec 2", mpdm_ival(mpdm_exec(x, w)) == 67);
}


void test_dh(void)
{
	mpdm_v h;
	mpdm_v k;
	mpdm_v v;
	mpdm_v v2;

	h=mpdm_gdbm(MPDM_LS(L"test.db"));
	mpdm_dump(h);

	if(h == NULL)
	{
		printf("Can't open test.db; no further gdbm tests possible.\n");
		return;
	}

	k=MPDM_LS(L"lastval");

	v=mpdm_hget(h, k);
	mpdm_dump(v);

	v=MPDM_I(mpdm_ival(v) + 1);
	mpdm_dump(v);
	mpdm_hset(h, k, v);

	v2=mpdm_hget(h, k);
	mpdm_dump(v2);

	_test("Disk hash 1", mpdm_ival(v) == mpdm_ival(v2));

	mpdm_hset(h, MPDM_LS(L"monday"), MPDM_LS(L"lunes"));
	mpdm_hset(h, MPDM_LS(L"tuesday"), MPDM_LS(L"martes"));

	_test("Disk hash size", mpdm_hsize(h) == 3);
	mpdm_dump(h);

	_test("hexists in disk hash 1", mpdm_hexists(h, MPDM_LS(L"lastval")));
	_test("hexists in disk hash 2", ! mpdm_hexists(h, MPDM_LS(L"nextval")));
}


void test_nondyn(void)
{
	mpdm_v v;
	mpdm_v w;
	mpdm_v a;
	mpdm_v av[2] = { NULL, NULL };

	a=MPDM_A(1);

	printf("Non-dynamic values\n");

	v=MPDM_ND_LS(L"This is a non-dynamic value");
	mpdm_dump(v);

	_test("Non-dynamic literal value size", mpdm_size(v) == 27);

	/* test auto-cloning when storing nondyn values to arrays */
	mpdm_aset(a, v, 0);
	w=mpdm_aget(a, 0);

	_test("ND_LS values should have been cloned on mpdm_aset",
		w != v);

	v=MPDM_ND_A(av);
	mpdm_dump(v);

	/* test auto-cloning when storing nondyn values to arrays */
	mpdm_aset(a, v, 0);
	w=mpdm_aget(a, 0);

	_test("ND_A values should have been cloned on mpdm_aset",
		w != v);
}


void _test_mpsl(char * code)
{
	mpdm_v v=mpsl_compile(MPDM_MBS(code));

	_test(code, v != NULL);
	mpdm_exec(v, NULL);
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
	_test_mpsl("a[\"b\"]=1;");
	_test_mpsl("a[\"b\"].c=1;");
	_test_mpsl("a[\"b\"][\"c\"]=1;");
	_test_mpsl("/* empty hash */ days={};");
	_test_mpsl("days.lunes=\"monday\";");
	_test_mpsl("days[\"martes\"]=\"tuesday\";");

	_test_mpsl("a=1 + ((3 - 5) * 8);");

	_test_mpsl("/* hash */ y={ \"enero\" => \"january\", \"febrero\" => \"february\" };");
	_test_mpsl("/* array */ a=[\"this\", \"one\", \"is\", 666, \"cool\"];");
}


int main(void)
{
	mpdm_startup();

	test_basic();
	test_array();
	test_hash();
	test_splice();
	test_asplit();
	test_ajoin();
	test_sym();
	test_file();
	test_regex();
	test_exec();
	test_dh();
	test_nondyn();
	test_mpsl();

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
