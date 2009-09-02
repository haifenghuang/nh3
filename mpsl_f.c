/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2009 Angel Ortega <angel@triptico.com>

    mpsl_f.c - Minimum Profit Scripting Language Function Library

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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <time.h>

#include "mpdm.h"
#include "mpsl.h"

/** code **/

#define A(n) mpdm_aget(a, n)
#define A0 A(0)
#define A1 A(1)
#define A2 A(2)
#define IA(n) mpdm_ival(A(n))
#define IA0 IA(0)
#define IA1 IA(1)
#define IA2 IA(2)
#define IA3 IA(3)
#define RA(n) mpdm_rval(A(n))
#define RA0 RA(0)
#define RA1 RA(1)
#define RA2 RA(2)
#define RA3 RA(3)

/**
 * size - Returns the size of a value.
 * @v: the value
 *
 * Returns the size of a value. For scalars, the size is the
 * string length; for arrays, the number of elements, and
 * for hashes, the number of buckets in the hash (which is
 * probably not useful, see hsize() instead).
 * [Value Management]
 */
/** integer = size(v); */
/* ; */
static mpdm_t F_size(mpdm_t a) {
	return MPDM_I(mpdm_size(A0));
}

/**
 * clone - Creates a clone of a value.
 * @v: the value
 *
 * Creates a clone of a value. If the value is multiple, a new value will
 * be created containing clones of all its elements; otherwise,
 * the same unchanged value is returned.
 * [Value Management]
 */
/** v2 = clone(v); */
static mpdm_t F_clone(mpdm_t a) {
	return mpdm_clone(A0);
}

/**
 * dump - Dumps a value to stdin.
 * @v: The value
 *
 * Dumps a value to stdin. The value can be complex. This function
 * is for debugging purposes only.
 * [Debugging]
 * [Input / Output]
 */
/** dump(v); */
static mpdm_t F_dump(mpdm_t a) {
	mpdm_dump(A0); return NULL;
}

/**
 * dumper - Returns a visual representation of a complex value.
 * @v: The value
 *
 * Returns a visual representation of a complex value.
 * [Debugging]
 * [Strings]
 */
/** string = dumper(v); */
static mpdm_t F_dumper(mpdm_t a) {
	return mpdm_dumper(A0);
}

/**
 * cmp - Compares two values.
 * @v1: the first value
 * @v2: the second value
 *
 * Compares two values. If both has the MPDM_STRING flag set,
 * a comparison using wcscmp() is returned; if both are arrays,
 * the size is compared first and, if they have the same number
 * elements, each one is compared; otherwise, a simple pointer
 * comparison is done.
 *
 * In either case, an integer is returned, which is < 0 if @v1
 * is lesser than @v2, > 0 on the contrary or 0 if both are
 * equal.
 * [Strings]
 * [Arrays]
 */
/** integer = cmp(v); */
static mpdm_t F_cmp(mpdm_t a) {
	return MPDM_I(mpdm_cmp(A0, A1));
}

/**
 * is_array - Tests if a value is an array.
 * @v: the value
 *
 * Returns non-zero if @v is an array.
 * [Value Management]
 * [Arrays]
 */
/** bool = is_array(v); */
static mpdm_t F_is_array(mpdm_t a) {
	return mpsl_boolean(MPDM_IS_ARRAY(A0));
}

/**
 * is_hash - Tests if a value is a hash.
 * @v: the value
 *
 * Returns non-zero if @v is a hash.
 * [Value Management]
 * [Hashes]
 */
/** bool = is_hash(v); */
static mpdm_t F_is_hash(mpdm_t a) {
	return mpsl_boolean(MPDM_IS_HASH(A0));
}

/**
 * is_exec - Tests if a value is executable.
 * @v: the value
 *
 * Returns non-zero if @v is a executable.
 * [Value Management]
 */
/** bool = is_exec(v); */
static mpdm_t F_is_exec(mpdm_t a) {
	return mpsl_boolean(MPDM_IS_EXEC(A0));
}

/**
 * splice - Creates a new string value from another.
 * @v: the original value
 * @i: the value to be inserted
 * @offset: offset where the substring is to be inserted
 * @del: number of characters to delete
 *
 * Creates a new string value from @v, deleting @del chars at @offset
 * and substituting them by @i. If @del is 0, no deletion is done.
 * both @offset and @del can be negative; if this is the case, it's
 * assumed as counting from the end of @v. If @v is NULL, @i will become
 * the new string, and both @offset and @del will be ignored. If @v is
 * not NULL and @i is, no insertion process is done (only deletion, if
 * applicable).
 *
 * Returns a two element array, with the new string in the first
 * element and the deleted string in the second (with a NULL value
 * if @del is 0).
 * [Strings]
 */
/** array = splice(v, i, offset, del); */
static mpdm_t F_splice(mpdm_t a) {
	return mpdm_splice(A0,A1,IA2,IA3);
}

/**
 * expand - Expands an array.
 * @a: the array
 * @offset: insertion offset
 * @num: number of elements to insert
 *
 * Expands an array value, inserting @num elements (initialized
 * to NULL) at the specified @offset.
 * [Arrays]
 */
/** expand(a, offset, num); */
static mpdm_t F_expand(mpdm_t a) {
	return mpdm_expand(A0,IA1,IA2);
}

/**
 * collapse - Collapses an array.
 * @a: the array
 * @offset: deletion offset
 * @num: number of elements to collapse
 *
 * Collapses an array value, deleting @num elements at
 * the specified @offset.
 * [Arrays]
 */
/** collapse(a, offset, num); */
static mpdm_t F_collapse(mpdm_t a) {
	return mpdm_collapse(A0,IA1,IA2);
}

/**
 * ins - Insert an element in an array.
 * @a: the array
 * @e: the element to be inserted
 * @offset: subscript where the element is going to be inserted
 *
 * Inserts the @e value in the @a array at @offset.
 * Further elements are pushed up, so the array increases its size
 * by one. Returns the inserted element.
 * [Arrays]
 */
/** e = ins(a, e, offset); */
static mpdm_t F_ins(mpdm_t a) {
	return mpdm_ins(A0,A1,IA2);
}

/**
 * adel - Deletes an element of an array.
 * @a: the array
 * @offset: subscript of the element to be deleted
 *
 * Deletes the element at @offset of the @a array. The array
 * is shrinked by one. If @offset is negative, is counted from
 * the end of the array (so a value of -1 means delete the
 * last element of the array).
 *
 * Returns the deleted element.
 * [Arrays]
 */
/** v = adel(a, offset); */
static mpdm_t F_adel(mpdm_t a) {
	return mpdm_adel(A0,IA1);
}

/**
 * shift - Extracts the first element of an array.
 * @a: the array
 *
 * Extracts the first element of the array. The array
 * is shrinked by one.
 *
 * Returns the deleted element.
 * [Arrays]
 */
/** v = shift(a); */
static mpdm_t F_shift(mpdm_t a) {
	return mpdm_shift(A0);
}

/**
 * push - Pushes a value into an array.
 * @a: the array
 * @e: the value
 *
 * Pushes a value into an array (i.e. inserts at the end).
 * [Arrays]
 */
/** e = push(a, e); */
static mpdm_t F_push(mpdm_t a) {
	return mpdm_push(A0,A1);
}

/**
 * pop - Pops a value from an array.
 * @a: the array
 *
 * Pops a value from the array (i.e. deletes from the end
 * and returns it).
 * [Arrays]
 */
/** v = pop(a); */
static mpdm_t F_pop(mpdm_t a) {
	return mpdm_pop(A0);
}

/**
 * queue - Implements a queue in an array.
 * @a: the array
 * @e: the element to be pushed
 * @size: maximum size of array
 *
 * Pushes the @e element into the @a array. If the array already has
 * @size elements, the first (oldest) element is deleted from the
 * queue and returned.
 *
 * Returns the deleted element, or NULL if the array doesn't have
 * @size elements yet.
 * [Arrays]
 */
/** v = queue(a, e, size); */
static mpdm_t F_queue(mpdm_t a) {
	return mpdm_queue(A0,A1,IA2);
}

/**
 * seek - Seeks a value in an array (sequential).
 * @a: the array
 * @k: the key
 * @step: number of elements to step
 *
 * Seeks sequentially the value @k in the @a array in
 * increments of @step. A complete search should use a step of 1.
 * Returns the offset of the element if found, or -1 otherwise.
 * [Arrays]
 */
/** integer = seek(a, k, step); */
static mpdm_t F_seek(mpdm_t a) {
	return MPDM_I(mpdm_seek(A0,A1,IA2));
}

/**
 * sort - Sorts an array.
 * @a: the array
 * @sorting_func: sorting function
 *
 * Sorts the array. For each pair of elements being sorted, the
 * @sorting_func is called with the two elements to be sorted as
 * arguments. This function must return a signed integer value indicating
 * the sorting order.
 *
 * If no function is supplied, the sorting is done using cmp().
 *
 * Returns the sorted array (the original one is left untouched).
 * [Arrays]
 */
/** array = sort(a); */
/** array = sort(a, sorting_func); */
static mpdm_t F_sort(mpdm_t a) {
	return mpdm_sort_cb(A0,1,A1);
}

/**
 * split - Separates a string into an array of pieces.
 * @s: the separator
 * @v: the value to be separated
 *
 * Separates the @v string value into an array of pieces, using @s
 * as a separator.
 *
 * If the separator is NULL, the string is splitted by characters.
 *
 * If the string does not contain the separator, an array holding 
 * the complete string as its unique argument is returned.
 * [Arrays]
 * [Strings]
 */
/** array = split(s, v); */
static mpdm_t F_split(mpdm_t a) {
	return mpdm_split(A0,A1);
}

/**
 * join - Joins all elements of an array into one.
 * @s: joiner string
 * @a: array to be joined
 *
 * Joins all elements from @a into one string, using @s as a glue.
 * [Arrays]
 * [Strings]
 */
/** string = join(s, a); */
static mpdm_t F_join(mpdm_t a) {
	return mpdm_join(A0,A1);
}

/**
 * hsize - Returns the number of pairs of a hash.
 * @h: the hash
 *
 * Returns the number of key-value pairs of a hash.
 * [Hashes]
 */
/** integer = hsize(h); */
static mpdm_t F_hsize(mpdm_t a) {
	return MPDM_I(mpdm_hsize(A0));
}

/**
 * exists - Tests if a key exists.
 * @h: the hash
 * @k: the key
 *
 * Returns 1 if @k is defined in @h, or 0 othersize.
 * [Hashes]
 */
/** bool = exists(h, k); */
static mpdm_t F_exists(mpdm_t a) {
	return mpsl_boolean(mpdm_exists(A0, A1));
}

/**
 * hdel - Deletes a key from a hash.
 * @h: the hash
 * @k: the key
 *
 * Deletes the key @k from the hash @h. Returns the previous
 * value, or NULL if the key was not defined.
 * [Hashes]
 */
/** v = hdel(h, k); */
static mpdm_t F_hdel(mpdm_t a) {
	return mpdm_hdel(A0, A1);
}

/**
 * keys - Returns the keys of a hash.
 * @h: the hash
 *
 * Returns an array containing all the keys of the @h hash.
 * [Hashes]
 * [Arrays]
 */
/** array = keys(h); */
static mpdm_t F_keys(mpdm_t a) {
	return mpdm_keys(A0);
}

/**
 * open - Opens a file.
 * @filename: the file name
 * @mode: an fopen-like mode string
 *
 * Opens a file. If @filename can be open in the specified @mode, a
 * value will be returned containing the file descriptor, or NULL
 * otherwise.
 *
 * If the file is open for reading, some charset detection methods are
 * used. If any of them is successful, its name is stored in the
 * DETECTED_ENCODING element of the mpdm_root() hash. This value is
 * suitable to be copied over ENCODING or TEMP_ENCODING.
 *
 * If the file is open for writing, the encoding to be used is read from
 * the ENCODING element of mpdm_root() and, if not set, from the
 * TEMP_ENCODING one. The latter will always be deleted afterwards.
 * [Input / Output]
 * [Character Set Conversion]
 */
/** fd = open(filename, mode); */
static mpdm_t F_open(mpdm_t a) {
	return mpdm_open(A0, A1);
}

/**
 * close - Closes a file descriptor.
 * @fd: the file descriptor
 *
 * Closes the file descriptor.
 * [Input / Output]
 */
/** close(fd); */
static mpdm_t F_close(mpdm_t a) {
	return mpdm_close(A0);
}

/**
 * read - Reads a line from a file descriptor.
 * @fd: the file descriptor
 *
 * Reads a line from @fd. Returns the line, or NULL on EOF.
 * [Input / Output]
 * [Character Set Conversion]
 */
/** string = read(fd); */
static mpdm_t F_read(mpdm_t a) {
	return mpdm_read(A0);
}

/**
 * getchar - Reads a character from a file descriptor.
 * @fd: the file descriptor
 *
 * Returns a character read from @fd, or NULL on EOF. No
 * charset conversion is done.
 * [Input / Output]
 */
/** string = getchar(fd); */
static mpdm_t F_getchar(mpdm_t a) {
	return mpdm_getchar(A0);
}

/**
 * putchar - Writes a character to a file descriptor.
 * @fd: the file descriptor
 * @s: the string
 *
 * Writes the first character in @s into @fd. No charset
 * conversion is done.
 *
 * Returns @s or NULL if the writing failed.
 * [Input / Output]
 */
/** s = putchar(fd, s); */
static mpdm_t F_putchar(mpdm_t a) {
	return mpdm_putchar(A0, A1);
}

/**
 * fseek - Sets a file pointer.
 * @fd: the file descriptor
 * @offset: the offset
 * @whence: the position
 *
 * Sets the file pointer position of @fd to @offset. @whence can
 * be: 0 for SEEK_SET, 1 for SEEK_CUR and 2 for SEEK_END.
 *
 * Returns the value from the fseek() C function call.
 * [Input / Output]
 */
/** integer = fseek(fd, offset, whence); */
static mpdm_t F_fseek(mpdm_t a) {
	return MPDM_I(mpdm_fseek(A0, IA1, IA2));
}

/**
 * fseek - Returns the current file pointer.
 * @fd: the file descriptor
 *
 * Returns the position of the file pointer in @fd.
 * [Input / Output]
 */
/** integer = ftell(fd); */
static mpdm_t F_ftell(mpdm_t a) {
	return MPDM_I(mpdm_ftell(A0));
}

/**
 * unlink - Deletes a file.
 * @filename: file name to be deleted
 *
 * Deletes a file.
 * [Input / Output]
 */
/** bool = unlink(filename); */
static mpdm_t F_unlink(mpdm_t a) {
	return mpsl_boolean(mpdm_unlink(A0));
}

static mpdm_t F_stat(mpdm_t a) {
	return mpdm_stat(A0);
}

static mpdm_t F_chmod(mpdm_t a) {
	return MPDM_I(mpdm_chmod(A0,A1));
}

static mpdm_t F_chown(mpdm_t a) {
	return MPDM_I(mpdm_chown(A0,A1,A2));
}

static mpdm_t F_glob(mpdm_t a) {
	return mpdm_glob(A0, A1);
}

static mpdm_t F_encoding(mpdm_t a) {
	return MPDM_I(mpdm_encoding(A0));
}

static mpdm_t F_popen(mpdm_t a) {
	return mpdm_popen(A0, A1);
}

static mpdm_t F_pclose(mpdm_t a) {
	return mpdm_pclose(A0);
}

static mpdm_t F_regex(mpdm_t a) {
	return mpdm_regex(A0,A1,IA2);
}

static mpdm_t F_sregex(mpdm_t a) {
	return mpdm_sregex(A0,A1,A2,IA3);
}

static mpdm_t F_gettext(mpdm_t a) {
	return mpdm_gettext(A0);
}

static mpdm_t F_gettext_domain(mpdm_t a) {
	return MPDM_I(mpdm_gettext_domain(A0, A1));
}

static mpdm_t F_load(mpdm_t a) {
	return mpdm_exec(mpsl_compile_file(A0), NULL);
}

static mpdm_t F_compile(mpdm_t a) {
	return mpsl_compile(A0);
}

static mpdm_t F_error(mpdm_t a) {
	return mpsl_error(A0);
}

static mpdm_t F_sweep(mpdm_t a) {
	mpdm_sweep(IA0); return NULL;
}

static mpdm_t F_uc(mpdm_t a) {
	return mpdm_ulc(A0, 1);
}

static mpdm_t F_lc(mpdm_t a) {
	return mpdm_ulc(A0, 0);
}

static mpdm_t F_time(mpdm_t a) {
	return MPDM_I(time(NULL));
}

static mpdm_t F_chdir(mpdm_t a) {
	return MPDM_I(mpdm_chdir(A0));
}

static mpdm_t F_sscanf(mpdm_t a) {
	return mpdm_sscanf(A0, A1, IA2);
}

static mpdm_t F_eval(mpdm_t a)
{
	mpdm_t c;

	a = mpdm_clone(a);
	c = mpdm_adel(a, 0);

	return mpsl_eval(c, a);
}

static mpdm_t F_sprintf(mpdm_t a)
{
	mpdm_t f;
	mpdm_t v;

	a = mpdm_clone(a);
	f = mpdm_adel(a, 0);

	/* if the first argument is an array, take it as the arguments */
	if ((v = mpdm_aget(a, 0)) != NULL && MPDM_IS_ARRAY(v))
		a = v;

	return mpdm_sprintf(f, a);
}


static mpdm_t F_print(mpdm_t a)
{
	int n;

	for (n = 0; n < mpdm_size(a); n++)
		mpdm_write_wcs(stdout, mpdm_string(A(n)));
	return NULL;
}


static mpdm_t F_write(mpdm_t a)
{
	int n, r = 0;

	for (n = 1; n < mpdm_size(a); n++)
		r += mpdm_write(A0, A(n));

	return MPDM_I(r);
}


static mpdm_t F_chr(mpdm_t a)
{
	wchar_t tmp[2];

	tmp[0] = (wchar_t) mpdm_ival(mpdm_aget(a, 0));
	tmp[1] = L'\0';

	return MPDM_S(tmp);
}


static mpdm_t F_ord(mpdm_t a)
{
	int ret = 0;
	mpdm_t v = mpdm_aget(a, 0);

	if (v != NULL) {
		wchar_t * ptr = mpdm_string(v);
		ret = (int) *ptr;
	}

	return MPDM_I(ret);
}


static mpdm_t F_map(mpdm_t a)
{
	mpdm_t key = mpdm_aget(a, 0);
	mpdm_t set = mpdm_aget(a, 1);
	mpdm_t out;

	/* map NULL to NULL */
	if (set == NULL)
		return NULL;

	out = mpdm_ref(MPDM_A(mpdm_size(set)));

	if (MPDM_IS_EXEC(key)) {
		int n;

		/* executes the code using the element as argument
		   and stores the result in the output array */
		for (n = 0; n < mpdm_size(set); n++)
			mpdm_aset(out, mpdm_exec_1(key, mpdm_aget(set, n)), n);
	}
	else
	if(MPDM_IS_HASH(key)) {
		int n;

		/* maps each value using the element as key */
		for (n = 0; n < mpdm_size(set); n++)
			mpdm_aset(out, mpdm_hget(key, mpdm_aget(set, n)), n);
	}

	return mpdm_unref(out);
}


static mpdm_t F_grep(mpdm_t a)
{
	mpdm_t key = mpdm_aget(a, 0);
	mpdm_t set = mpdm_aget(a, 1);
	mpdm_t out = mpdm_ref(MPDM_A(0));

	if (MPDM_IS_EXEC(key)) {
		int n;

		/* it's executable */
		for (n = 0; n < mpdm_size(set); n++) {
			mpdm_t v = mpdm_aget(set, n);

			if (mpsl_is_true(mpdm_exec_1(key, v)))
				mpdm_push(out, v);
		}
	}
	else
	if(key->flags & MPDM_STRING) {
		int n;

		/* it's a string; use it as a regular expression */
		for (n = 0; n < mpdm_size(set); n++) {
			mpdm_t v = mpdm_aget(set, n);

			if (mpdm_regex(key, v, 0))
				mpdm_push(out, v);
		}
	}

	return mpdm_size(mpdm_unref(out)) == 0 ? NULL : out;
}

static mpdm_t F_getenv(mpdm_t a)
{
	mpdm_t e = mpdm_hget_s(mpdm_root(), L"ENV");

	return mpdm_hget(e, mpdm_aget(a, 0));
}

static mpdm_t F_bincall(mpdm_t a) { return MPDM_X(mpdm_ival(mpdm_aget(a, 0))); }


static struct {
	wchar_t * name;
	mpdm_t (* func)(mpdm_t);
} mpsl_funcs[] = {
	{ L"size",	F_size },
	{ L"clone",	F_clone },
	{ L"dump",	F_dump },
	{ L"dumper",	F_dumper },
	{ L"cmp",	F_cmp },
	{ L"is_array",	F_is_array },
	{ L"is_hash",	F_is_hash },
	{ L"is_exec",	F_is_exec },
	{ L"splice",	F_splice },
	{ L"expand",	F_expand },
	{ L"collapse",	F_collapse },
	{ L"ins",	F_ins },
	{ L"adel",	F_adel },
	{ L"shift",	F_shift },
	{ L"push",	F_push },
	{ L"pop",	F_pop },
	{ L"queue",	F_queue },
	{ L"seek",	F_seek },
	{ L"sort",	F_sort },
	{ L"split",	F_split },
	{ L"join",	F_join },
	{ L"hsize",	F_hsize },
	{ L"exists",	F_exists },
	{ L"hdel",	F_hdel },
	{ L"keys",	F_keys },
	{ L"open",	F_open },
	{ L"close",	F_close },
	{ L"read",	F_read },
	{ L"write",	F_write },
	{ L"getchar",	F_getchar },
	{ L"putchar",	F_putchar },
	{ L"fseek",	F_fseek },
	{ L"ftell",	F_ftell },
	{ L"unlink",	F_unlink },
	{ L"stat",	F_stat },
	{ L"chmod",	F_chmod },
	{ L"chown",	F_chown },
	{ L"glob",	F_glob },
	{ L"encoding",	F_encoding },
	{ L"popen",	F_popen },
	{ L"pclose",	F_pclose },
	{ L"regex",	F_regex },
	{ L"sregex",	F_sregex },
	{ L"load",	F_load },
	{ L"compile",	F_compile },
	{ L"error",	F_error },
	{ L"eval",	F_eval },
	{ L"print",	F_print },
	{ L"gettext",	F_gettext },
	{ L"gettext_domain", F_gettext_domain },
	{ L"sprintf",	F_sprintf },
	{ L"sweep",	F_sweep },
	{ L"chr",	F_chr },
	{ L"ord",	F_ord },
	{ L"map",	F_map },
	{ L"grep",	F_grep },
	{ L"getenv",	F_getenv },
	{ L"uc",	F_uc },
	{ L"lc",	F_lc },
	{ L"time",	F_time },
	{ L"chdir",	F_chdir },
	{ L"sscanf",	F_sscanf },
	{ L"bincall",	F_bincall },
	{ NULL,		NULL }
};


mpdm_t mpsl_build_funcs(void)
/* build all functions */
{
	mpdm_t c;
	int n;

	/* creates all the symbols in the CORE library */
	c = MPDM_H(0);
	for (n = 0; mpsl_funcs[n].name != NULL; n++) {
		mpdm_t f = MPDM_S(mpsl_funcs[n].name);
		mpdm_t x = MPDM_X(mpsl_funcs[n].func);

		mpdm_hset(mpdm_root(), f, x);
		mpdm_hset(c, f, x);
	}

	return c;
}


