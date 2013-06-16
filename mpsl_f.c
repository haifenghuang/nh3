/*

    MPSL - Minimum Profit Scripting Language 3.x
    Copyright (C) 2003/2012 Angel Ortega <angel@triptico.com>

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

    http://triptico.com

*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <time.h>

#include "mpsl.h"

/** code **/

#define F_ARGS mpdm_t a, mpdm_t l

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

#define mpsl_boolean(b) MPDM_I(b)


/** library **/

/** any type **/

/**
 * v.size - Returns the size of a value.
 * @v: the value
 *
 * Returns the size of a value. For scalars, the size is the
 * string length; for arrays, the number of elements, and
 * for hashes, the number of buckets in the hash (which is
 * probably not useful, see hsize() instead).
 * [Value Management]
 */
/** integer = v.size(); */
/* ; */
static mpdm_t F_size(F_ARGS)
{
    return MPDM_I(mpdm_size(l));
}


wchar_t *mpsl_type(mpdm_t);

static mpdm_t F_type(F_ARGS)
{
    return MPDM_LS(mpsl_type(l));
}


/**
 * v.clone - Creates a clone of a value.
 * @v: the value
 *
 * Creates a clone of a value. If the value is multiple, a new value will
 * be created containing clones of all its elements; otherwise,
 * the same unchanged value is returned.
 * [Value Management]
 */
/** v2 = v.clone(); */
static mpdm_t F_clone(F_ARGS)
{
    return mpdm_clone(l);
}

/**
 * v.dump - Dumps a value to stdin.
 * @v: The value
 *
 * Dumps a value to stdin. The value can be complex. This function
 * is for debugging purposes only.
 * [Debugging]
 * [Input-Output]
 */
/** v.dump(); */
static mpdm_t F_dump(F_ARGS)
{
    mpdm_dump(l);
    return l;
}

/**
 * v.dumper - Returns a visual representation of a complex value.
 * @v: The value
 *
 * Returns a visual representation of a complex value.
 * [Debugging]
 * [Strings]
 */
/** string = v.dumper(); */
static mpdm_t F_dumper(F_ARGS)
{
    return mpdm_dumper(l);
}

/**
 * v.cmp - Compares two values.
 * @v: the first value
 * @v2: the second value
 *
 * Compares two values. If both are strings, a standard string
 * comparison (using wcscmp()) is returned; if both are arrays,
 * the size is compared first and, if they have the same number
 * elements, each one is compared; otherwise, a simple pointer
 * comparison is done.
 *
 * In either case, an integer is returned, which is < 0 if @v
 * is lesser than @v2, > 0 on the contrary or 0 if both are
 * equal.
 * [Strings]
 * [Arrays]
 */
/** integer = v.cmp(v2); */
static mpdm_t F_cmp(F_ARGS)
{
    return MPDM_I(mpdm_cmp(l, A0));
}


/** SCALAR type **/

/**
 * str.splice - Creates a new string value from another.
 * @str: the original value
 * @i: the value to be inserted
 * @offset: offset where the substring is to be inserted
 * @del: number of characters to delete
 *
 * Creates a new string value from @str, deleting @del chars at @offset
 * and substituting them by @i. If @del is 0, no deletion is done.
 * both @offset and @del can be negative; if this is the case, it's
 * assumed as counting from the end of @str. If @str is NULL, @i will become
 * the new string, and both @offset and @del will be ignored. If @str is
 * not NULL and @i is, no insertion process is done (only deletion, if
 * applicable).
 *
 * Returns a two element array, with the new string in the first
 * element and the deleted string in the second (with a NULL value
 * if @del is 0).
 * [Strings]
 */
/** array = str.splice(i, offset, del); */
static mpdm_t F_splice(F_ARGS)
{
    return mpdm_splice(l, A0, IA1, IA2);
}


/**
 * str.split - Separates a string into an array of pieces.
 * @str: the value to be separated
 * @s: the separator
 *
 * Separates the @str string value into an array of pieces, using @s
 * as a separator.
 *
 * If the separator is NULL, the string is splitted by characters.
 *
 * If the string does not contain the separator, an array holding 
 * the complete string as its unique argument is returned.
 * [Arrays]
 * [Strings]
 */
/** array = str.split(s); */
static mpdm_t F_split(F_ARGS)
{
    return mpdm_split(l, A0);
}


/**
 * str.regex - Matches a regular expression.
 * @str: the value to be matched
 * @r: the regular expression
 * @ra: an array of regular expressions
 * @offset: offset from the start of the value
 *
 * Matches a regular expression against a value. Valid flags are `i',
 * for case-insensitive matching, `m', to treat the string as a
 * multiline string (i.e., one containing newline characters), so
 * that ^ and $ match the boundaries of each line instead of the
 * whole string, `l', to return the last matching instead of the
 * first one, or `g', to match globally; in that last case, an array
 * containing all matches is returned instead of a string scalar.
 *
 * If @r is a string, an ordinary regular expression matching is tried
 * over the @v string. If the matching is possible, the match result
 * is returned, or NULL otherwise.
 *
 * If @r is an array (of strings), each element is tried sequentially
 * as an individual regular expression over the @v string, each one using
 * the offset returned by the previous match. All regular expressions
 * must match to be successful. If this is the case, an array (with
 * the same number of arguments) is returned containing the matched
 * strings, or NULL otherwise.
 *
 * If @r is NULL, the result of the previous regex matching
 * is returned as a two element array. The first element will contain
 * the character offset of the matching and the second the number of
 * characters matched. If the previous regex was unsuccessful, NULL
 * is returned.
 * [Regular Expressions]
 */
/** string = str.regex(r); */
/** string = str.regex(r, offset); */
/** array = str.regex(ra); */
/** array = str.regex(); */
static mpdm_t F_regex(F_ARGS)
{
    return mpdm_regex(l, A0, IA1);
}

/**
 * str.sregex - Matches and substitutes a regular expression.
 * @str: the value to be matched
 * @r: the regular expression
 * @s: the substitution string, hash or code
 * @offset: offset from the start of v
 *
 * Matches a regular expression against a value, and substitutes the
 * found substring with @s. Valid flags are `i', for case-insensitive
 * matching, and `g', for global replacements (all ocurrences in @v
 * will be replaced, instead of just the first found one).
 *
 * If @s is executable, it's executed with the matched part as
 * the only argument and its return value is used as the
 * substitution string.
 *
 * If @s is a hash, the matched string is used as a key to it and
 * its value used as the substitution. If this value itself is
 * executable, it's executed with the matched string as its only
 * argument and its return value used as the substitution.
 *
 * If @r is NULL, returns the number of substitutions made in the
 * previous call to sregex() (can be zero if none was done).
 *
 * Returns the modified string, or the original one if no substitutions
 * were done.
 * [Regular Expressions]
 */
/** string = str.sregex(r, s); */
/** string = str.sregex(r, s, offset); */
/** integer = str.sregex(); */
static mpdm_t F_sregex(F_ARGS)
{
    return mpdm_sregex(l, A0, A1, IA2);
}


/**
 * str.uc - Converts a string to uppercase.
 * @str: the string to be converted
 *
 * Returns @str converted to uppercase.
 * [Strings]
 */
/** string = str.uc(); */
static mpdm_t F_uc(F_ARGS)
{
    return mpdm_ulc(l, 1);
}

/**
 * str.lc - Converts a string to lowercase.
 * @str: the string to be converted
 *
 * Returns @str converted to lowercase.
 * [Strings]
 */
/** string = str.uc(); */
static mpdm_t F_lc(F_ARGS)
{
    return mpdm_ulc(l, 0);
}


/**
 * str.sscanf - Extracts data like sscanf().
 * @str: the string to be parsed
 * @fmt: the string format
 * @offset: the character offset to start scanning
 *
 * Extracts data from a string using a special format pattern, very
 * much like the scanf() series of functions in the C library. Apart
 * from the standard percent-sign-commands (s, u, d, i, f, x,
 * n, [, with optional size and * to ignore), it implements S,
 * to match a string of characters upto what follows in the format
 * string. Also, the [ set of characters can include other % formats.
 *
 * Returns an array with the extracted values. If %n is used, the
 * position in the scanned string is returned as the value.
 * [Strings]
 */
/** array = str.sscanf(fmt); */
/** array = str.sscanf(fmt, offset); */
static mpdm_t F_sscanf(F_ARGS)
{
    return mpdm_sscanf(l, A0, IA1);
}

/**
 * int.chr - Returns the Unicode character represented by the codepoint.
 * @int: the codepoint as an integer value
 *
 * Returns a 1 character string containing the character which
 * Unicode codepoint is @int.
 * [Strings]
 */
/** string = int.chr(); */
static mpdm_t F_chr(F_ARGS)
{
    wchar_t tmp[2];

    tmp[0] = (wchar_t) mpdm_ival(l);
    tmp[1] = L'\0';

    return MPDM_S(tmp);
}


/**
 * str.ord - Returns the Unicode codepoint of a character.
 * @str: the string
 *
 * Returns the Unicode codepoint for the first character in
 * the string.
 * [Strings]
 */
/** integer = str.ord(); */
static mpdm_t F_ord(F_ARGS)
{
    int ret = 0;
    mpdm_t v = l;

    if (v != NULL) {
        wchar_t *ptr = mpdm_string(v);
        ret = (int) *ptr;
    }

    return MPDM_I(ret);
}


/**
 * str.tr - Transliterates a string.
 * @str: the string
 * @from: set of characters to be replaced
 * @to: set of characters to replace
 *
 * Transliterates @str to a new string with all characters from @from
 * replaced by those in @to.
 * [Threading]
 */
/** string = str.tr(from, to); */
static mpdm_t F_tr(F_ARGS)
{
    return mpdm_tr(l, A0, A1);
}


/** ARRAY type **/

/**
 * array.expand - Expands an array.
 * @array: the array
 * @offset: insertion offset
 * @num: number of elements to insert
 *
 * Expands an array value, inserting @num elements (initialized
 * to NULL) at the specified @offset.
 * [Arrays]
 */
/** array.expand(offset, num); */
static mpdm_t F_expand(F_ARGS)
{
    return mpdm_expand(l, IA0, IA1);
}

/**
 * array.collapse - Collapses an array.
 * @array: the array
 * @offset: deletion offset
 * @num: number of elements to collapse
 *
 * Collapses an array value, deleting @num elements at
 * the specified @offset.
 * [Arrays]
 */
/** array.collapse(offset, num); */
static mpdm_t F_collapse(F_ARGS)
{
    return mpdm_collapse(l, IA0, IA1);
}

/**
 * array.ins - Insert an element in an array.
 * @array: the array
 * @e: the element to be inserted
 * @offset: subscript where the element is going to be inserted
 *
 * Inserts the @e value in the array at @offset.
 * Further elements are pushed up, so the array increases its size
 * by one. Returns the inserted element.
 * [Arrays]
 */
/** e = array.ins(e, offset); */
static mpdm_t F_ins(F_ARGS)
{
    return mpdm_ins(l, A0, IA1);
}

/**
 * array.delete - Deletes an element of an array.
 * @array: the array
 * @offset: subscript of the element to be deleted
 *
 * Deletes the element at @offset of the array. The array
 * is shrinked by one. If @offset is negative, is counted from
 * the end of the array (so a value of -1 means delete the
 * last element of the array).
 *
 * Returns NULL (previous versions returned the deleted element).
 * [Arrays]
 */
/** array.delete(offset); */
static mpdm_t F_adel(F_ARGS)
{
    return mpdm_adel(l, IA0);
}

/**
 * array.shift - Extracts the first element of an array.
 * @array: the array
 *
 * Extracts the first element of the array. The array
 * is shrinked by one.
 *
 * Returns the deleted element.
 * [Arrays]
 */
/** v = array.shift(); */
static mpdm_t F_shift(F_ARGS)
{
    return mpdm_shift(l);
}

/**
 * array.push - Pushes a value into an array.
 * @array: the array
 * @arg1: first value
 * @arg2: second value
 * @argn: nth value
 *
 * Pushes values into an array (i.e. inserts at the end).
 * Returns the last element pushed.
 * [Arrays]
 */
/** argn = array.push(arg1 [, arg2, ... argn]); */
static mpdm_t F_push(F_ARGS)
{
    int n;
    mpdm_t r = NULL;

    for (n = 0; n < mpdm_size(a); n++) {
        mpdm_unref(r);
        r = mpdm_push(l, A(n));
        mpdm_ref(r);
    }

    return mpdm_unrefnd(r);
}

/**
 * array.pop - Pops a value from an array.
 * @array: the array
 *
 * Pops a value from the array (i.e. deletes from the end
 * and returns it).
 * [Arrays]
 */
/** v = array.pop(); */
static mpdm_t F_pop(F_ARGS)
{
    return mpdm_pop(l);
}

/**
 * array.queue - Implements a queue in an array.
 * @array: the array
 * @e: the element to be pushed
 * @size: maximum size of array
 *
 * Pushes the @e element into the array. If the array already has
 * @size elements, the first (oldest) element is deleted from the
 * queue and returned.
 *
 * Returns the deleted element, or NULL if the array doesn't have
 * @size elements yet.
 * [Arrays]
 */
/** v = array.queue(e, size); */
static mpdm_t F_queue(F_ARGS)
{
    return mpdm_queue(l, A0, IA1);
}


/**
 * array.seek - Seeks a value in an array (sequential).
 * @array: the array
 * @k: the key
 * @step: number of elements to step
 *
 * Seeks sequentially the value @k in the array in
 * increments of @step. A complete search should use a step of 1.
 * Returns the offset of the element if found, or -1 otherwise.
 * [Arrays]
 */
/** integer = array.seek(k, step); */
static mpdm_t F_seek(F_ARGS)
{
    return MPDM_I(mpdm_seek(l, A0, IA1));
}

/**
 * array.sort - Sorts an array.
 * @array: the array
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
/** array = array.sort(); */
/** array = array.sort(sorting_func); */
static mpdm_t F_sort(F_ARGS)
{
    mpdm_t r, v;

    v = mpdm_ref(l);
    r = mpdm_sort_cb(mpdm_clone(v), 1, A0);
    mpdm_unref(v);

    return r;
}


/** HASH type **/

static mpdm_t F_hsize(F_ARGS)
{
    return MPDM_I(mpdm_hsize(l));
}

/**
 * hash.exists - Tests if a key exists.
 * @hash: the hash
 * @k: the key
 *
 * Returns 1 if @k is defined in the hash, or 0 othersize.
 * [Hashes]
 */
/** bool = hash.exists(k); */
static mpdm_t F_exists(F_ARGS)
{
    return mpsl_boolean(mpdm_exists(l, A0));
}

/**
 * hash.delete - Deletes a key from a hash.
 * @hash: the hash
 * @k: the key
 *
 * Deletes the key @k from the hash. Returns the previous
 * value, or NULL if the key was not defined.
 * [Hashes]
 */
/** v = array.delete(k); */
static mpdm_t F_hdel(F_ARGS)
{
    return mpdm_hdel(l, A0);
}

/** I/O **/

/**
 * io.close - Closes an I/O stream.
 * @io: the I/O stream
 *
 * Closes the file, pipe or socket.
 * [Input-Output]
 */
/** io.close(); */
static mpdm_t M_close(F_ARGS)
{
    return mpdm_close(l);
}

/**
 * io.read - Reads a line from an I/O stream.
 * @io: the I/O stream
 *
 * Reads a line from and I/O stream, doing character conversion.
 * Returns the line, or NULL on EOF.
 * [Input-Output]
 * [Character Set Conversion]
 */
/** string = io.read(); */
static mpdm_t M_read(F_ARGS)
{
    return mpdm_read(l);
}

/**
 * io.write - Writes to an I/O stream.
 * @io: the I/O stream
 * @arg1: first argument
 * @arg2: second argument
 * @argn: nth argument
 *
 * Writes the variable list of arguments to the I/O stream, doing
 * charset conversion in the process.
 *
 * Returns the total size written to @fd.
 * [Input-Output]
 * [Character Set Conversion]
 */
/** integer = io.write(arg1 [,arg2 ... argn]); */
static mpdm_t M_write(F_ARGS)
{
    int n, r = 0;

    for (n = 0; n < mpdm_size(a); n++)
        r += mpdm_write(l, A(n));

    return MPDM_I(r);
}


/**
 * io.getchar - Reads a character from an I/O stream.
 * @io: the I/O stream
 *
 * Returns a character read, or NULL on EOF. No
 * charset conversion is done.
 * [Input-Output]
 */
/** string = io.getchar(); */
static mpdm_t M_getchar(F_ARGS)
{
    return mpdm_getchar(l);
}

/**
 * io.putchar - Writes a character to an I/O stream.
 * @io: the I/O stream
 * @s: the string
 *
 * Writes the first character in @s to the I/O stram. No charset
 * conversion is done.
 *
 * Returns the number of chars written (0 or 1).
 * [Input-Output]
 */
/** integer = io.putchar(s); */
static mpdm_t M_putchar(F_ARGS)
{
    return MPDM_I(mpdm_putchar(l, A0));
}

/**
 * io.seek - Sets the position of an I/O stream.
 * @io: the I/O stream
 * @offset: the offset
 * @whence: the position
 *
 * Sets the file pointer position to @offset. @whence can
 * be: 0 for SEEK_SET, 1 for SEEK_CUR and 2 for SEEK_END.
 *
 * Returns the value from the fseek() C function call.
 * [Input-Output]
 */
/** integer = io.seek(offset, whence); */
static mpdm_t M_fseek(F_ARGS)
{
    return MPDM_I(mpdm_fseek(l, IA0, IA1));
}

/**
 * io.tell - Returns the current file pointer.
 * @io: the I/O stream
 *
 * Returns the position of the file pointer.
 * [Input-Output]
 */
/** integer = io.tell(); */
static mpdm_t M_ftell(F_ARGS)
{
    return MPDM_I(mpdm_ftell(l));
}


/**
 * sys.open - Opens a file.
 * @filename: the file name
 * @mode: an fopen-like mode string
 *
 * Opens a file. If @filename can be open in the specified @mode, a
 * value will be returned containing an I/O stream object, or NULL
 * otherwise.
 *
 * If the file is open for reading, some charset detection methods are
 * used. If any of them is successful, its name is stored in the
 * `DETECTED_ENCODING' global variable. This value is
 * suitable to be copied over `ENCODING' or `TEMP_ENCODING'.
 *
 * If the file is open for writing, the encoding to be used is read from
 * the `ENCODING' global variable and, if not set, from the
 * `TEMP_ENCODING' one. The latter will always be deleted afterwards.
 * [Input-Output]
 * [Character Set Conversion]
 */
/** io = sys.open(filename, mode); */
static mpdm_t F_open(F_ARGS)
{
    mpdm_t f = mpdm_open(A0, A1);

    return f;
}


/**
 * sys.popen - Opens a pipe.
 * @prg: the program to pipe
 * @mode: an fopen-like mode string
 *
 * Opens a pipe to a program. If @prg can be open in the specified @mode,
 * returns an I/O stream, or NULL otherwise.
 *
 * The @mode can be `r' (for reading), `w' (for writing), or `r+' or `w+'
 * for a special double pipe reading-writing mode.
 * [Input-Output]
 */
/** io = sys.popen(prg, mode); */
static mpdm_t F_popen(F_ARGS)
{
    return mpdm_popen(A0, A1);
}


/**
 * sys.connect - Opens a client TCP/IP socket.
 * @h: host name or ip
 * @s: service or port number
 *
 * Opens a client TCP/IP socket to the @h host at @s service (or port).
 * Returns NULL if the connection cannot be done or an I/O stream,
 * that can be used with all file operation functions, including close().
 * [Sockets]
 * [Input-Output]
 */
/** io = sys.connect(h, s); */
static mpdm_t F_connect(F_ARGS)
{
    return mpdm_connect(A0, A1);
}


/**
 * sys.unlink - Deletes a file.
 * @filename: file name to be deleted
 *
 * Deletes a file.
 * [Input-Output]
 */
/** bool = sys.unlink(filename); */
static mpdm_t F_unlink(F_ARGS)
{
    return mpsl_boolean(mpdm_unlink(A0));
}

/**
 * sys.rename - Renames a file.
 * @oldpath: old path
 * @newpath: new path
 *
 * Renames a file.
 * [Input-Output]
 */
/** bool = sys.rename(oldpath, newpath); */
static mpdm_t F_rename(F_ARGS)
{
    return mpsl_boolean(mpdm_rename(A0, A1));
}

/**
 * sys.stat - Gives status from a file.
 * @filename: file name to get the status from
 *
 * Returns a 14 element array of the status (permissions, onwer, etc.)
 * from the desired @filename, or NULL if the file cannot be accessed.
 * (man 2 stat).
 *
 * The values are: 0, device number of filesystem; 1, inode number;
 * 2, file mode; 3, number of hard links to the file; 4, uid; 5, gid;
 * 6, device identifier; 7, total size of file in bytes; 8, atime;
 * 9, mtime; 10, ctime; 11, preferred block size for system I/O;
 * 12, number of blocks allocated and 13, canonicalized file name.
 * Not all elements have necesarily meaningful values, as most are
 * system-dependent.
 * [Input-Output]
 */
/** array = sys.stat(filename); */
static mpdm_t F_stat(F_ARGS)
{
    return mpdm_stat(A0);
}

/**
 * sys.chmod - Changes a file's permissions.
 * @filename: the file name
 * @perms: permissions (element 2 from stat())
 *
 * Changes the permissions for a file.
 * [Input-Output]
 */
/** integer = sys.chmod(filename, perms); */
static mpdm_t F_chmod(F_ARGS)
{
    return MPDM_I(mpdm_chmod(A0, A1));
}

/**
 * sys.chown - Changes a file's owner.
 * @filename: the file name
 * @uid: user id (element 4 from stat())
 * @gid: group id (element 5 from stat())
 *
 * Changes the owner and group id's for a file.
 * [Input-Output]
 */
/** integer = sys.chown(filename, uid, gid); */
static mpdm_t F_chown(F_ARGS)
{
    return MPDM_I(mpdm_chown(A0, A1, A2));
}

/**
 * sys.glob - Executes a file globbing.
 * @spec: Globbing spec
 * @base: Optional base directory
 *
 * Executes a file globbing. @spec is system-dependent, but usually
 * the * and ? metacharacters work everywhere. @base can contain a
 * directory; if that's the case, the output strings will include it.
 * In any case, each returned value will be suitable for a call to
 * open().
 *
 * Returns an array of files that match the globbing (can be an empty
 * array if no file matches), or NULL if globbing is unsupported.
 * Directories are returned first and their names end with a slash.
 * [Input-Output]
 */
/** array = sys.glob(spec, base); */
static mpdm_t F_glob(F_ARGS)
{
    return mpdm_glob(A0, A1);
}

/**
 * sys.encoding - Sets the current charset encoding for files.
 * @charset: the charset name.
 *
 * Sets the current charset encoding for files. Future opened
 * files will be assumed to be encoded with @charset, which can
 * be any of the supported charset names (utf-8, iso-8859-1, etc.),
 * and converted on each read / write. If charset is NULL, it
 * is reverted to default charset conversion (i.e. the one defined
 * in the locale).
 *
 * This function stores the @charset value into the `ENCODING' global
 * variable.
 *
 * Returns a negative number if @charset is unsupported, or zero
 * if no errors were found.
 * [Input-Output]
 * [Character Set Conversion]
 */
/** integer = sys.encoding(charset); */
static mpdm_t F_encoding(F_ARGS)
{
    return MPDM_I(mpdm_encoding(A0));
}

/**
 * sys.gettext - Translates a string to the current language.
 * @str: the string
 *
 * Translates the @str string to the current language.
 *
 * This function can still be used even if there is no real gettext
 * support by manually filling the __I18N__ hash.
 *
 * If the string is found in the current table, the translation is
 * returned; otherwise, the same @str value is returned.
 * [Strings]
 * [Localization]
 */
/** string = sys.gettext(str); */
static mpdm_t F_gettext(F_ARGS)
{
    return mpdm_gettext(A0);
}

/**
 * sys.gettext_domain - Sets domain and data directory for translations.
 * @dom: the domain (application name)
 * @data: directory contaning the .mo files
 *
 * Sets the domain (application name) and translation data for translating
 * strings that will be returned by gettext(). @data must point to a
 * directory containing the .mo (compiled .po) files.
 *
 * If there is no gettext support, returns 0, or 1 otherwise.
 * [Strings]
 * [Localization]
 */
/** bool = sys.gettext_domain(dom, data); */
static mpdm_t F_gettext_domain(F_ARGS)
{
    return MPDM_I(mpdm_gettext_domain(A0, A1));
}


/**
 * sys.time - Returns the current time.
 *
 * Returns the current time from the epoch (C library time()).
 * [Time]
 */
/** integer = sys.time(); */
static mpdm_t F_time(F_ARGS)
{
    return MPDM_I(time(NULL));
}

/**
 * sys.chdir - Changes the working directory
 * @dir: the new path
 *
 * Changes the working directory
 * [Input-Output]
 */
/** integer = sys.chdir(dir); */
static mpdm_t F_chdir(F_ARGS)
{
    return MPDM_I(mpdm_chdir(A0));
}


/**
 * sys.p - Writes values to stdout.
 * @arg1: first argument
 * @arg2: second argument
 * @argn: nth argument
 *
 * Writes the variable arguments to stdout.
 * [Input-Output]
 */
/** sys.p(arg1 [,arg2 ... argn]); */
static mpdm_t F_print(F_ARGS)
{
    int n;

    for (n = 0; n < mpdm_size(a); n++)
        mpdm_write_wcs(stdout, mpdm_string(A(n)));
    return NULL;
}


static mpdm_t F_bincall(F_ARGS)
{
    void *func;
    char *ptr = mpdm_wcstombs(mpdm_string(mpdm_aget(a, 0)), NULL);

    sscanf(ptr, "%p", &func);
    free(ptr);

    return MPDM_X(func);
}

/* FIXME: for randomizing */
int getpid(void);

static unsigned int _rnd_seed = 0;
static unsigned int _rnd_init = 0;

static unsigned int _srand(unsigned int seed)
{
    unsigned int p_seed = _rnd_seed;

    _rnd_init = 1;
    _rnd_seed = seed;

    return p_seed;
}


static unsigned int _rnd(unsigned int range)
{
    unsigned int r = 0;

    if (_rnd_init == 0) {
        FILE *f;
        unsigned int s;

        if ((f = fopen("/dev/urandom", "rb")) != NULL) {
            fread(&s, sizeof(s), 1, f);
            fclose(f);
        }
        else
            s = time(NULL) ^ getpid();

        _srand(s);
    }

    /* Linear congruential generator by Numerical Recipes */
    r = _rnd_seed = (_rnd_seed * 1664525) + 1013904223;

    if (range)
        r %= range;

    return r;
}


/**
 * sys.randomize - Sets the random seed.
 *
 * Sets the random seed.
 * [Miscellaneous]
 */
/** previous_seed = sys.randomize(new_seed); */
static mpdm_t F_randomize(F_ARGS)
{
    return MPDM_I(_srand(mpdm_ival(mpdm_aget(a, 0))));
};


/**
 * v.rnd - Returns a random value.
 *
 * Returns a random number, array element or key/value pair
 * from the value. For numbers, the result is in the 0 .. number - 1
 * range (if the number is a numeral constant, remember to enclose it
 * in parenthesis, like any other method applied to numerals).
 * [Arrays]
 * [Hashes]
 */
/** number = number.rnd(); */
/** elem = array.rnd(); */
/** key_pair = hash.rnd(); */
static mpdm_t F_rnd(F_ARGS)
{
    mpdm_t r;

    if (MPDM_IS_HASH(l)) {
        mpdm_t t = mpdm_ref(mpdm_keys(l));
        mpdm_t k = mpdm_aget(t, _rnd(mpdm_size(t)));
        r = mpdm_ref(MPDM_A(2));
        mpdm_aset(r, k, 0);
        mpdm_aset(r, mpdm_hget(l, k), 1);
        mpdm_unref(t);
        mpdm_unrefnd(r);
    }
    else
    if (MPDM_IS_ARRAY(l))
        r = mpdm_aget(l, _rnd(mpdm_size(l)));
    else
        r = MPDM_R((double)_rnd(mpdm_ival(l)));

    return r;
}


/**
 * sys.sleep - Sleeps a number of milliseconds.
 *
 * Sleeps a number of milliseconds.
 * [Time]
 */
/** sys.sleep(msecs); */
static mpdm_t F_sleep(F_ARGS)
{
    mpdm_sleep(mpdm_ival(mpdm_aget(a, 0)));

    return NULL;
}


/**
 * new - Creates a new object using another as its base.
 * @c1: class / base object
 * @c2: class / base object
 * @cn: class / base object
 *
 * Creates a new object using as classes or base objects all the ones
 * sent as arguments (assumed to be hashes).
 * 
 * [Object-oriented programming]
 */
/** o = new(c1 [, c2, ...cn]); */
static mpdm_t F_new(F_ARGS)
{
    int n;
    mpdm_t r = mpdm_ref(MPDM_H(0));

    for (n = 0; n < mpdm_size(a); n++) {
        mpdm_t w, k, v;
        int m = 0;

        w = mpdm_ref(A(n));

        if (MPDM_IS_EXEC(w))
            w = mpdm_ref(mpdm_exec(mpdm_unrefnd(w), NULL, NULL));

        if (MPDM_IS_HASH(w)) {
            while (mpdm_iterator(w, &m, &k, &v))
                mpdm_hset(r, k, mpdm_clone(v));
        }

        mpdm_unref(w);
    }

    return mpdm_unrefnd(r);
}


/** init **/
wchar_t *mpsl_dump_1(const mpdm_t v, int l, wchar_t * ptr, int *size);

void mpsl_library_init(mpdm_t r, int argc, char *argv[])
/* inits the library */
{
    int n;
    mpdm_t v;

    mpdm_ref(r);

    /* hash methods */
    v = mpdm_hset_s(r, L"HASH", MPDM_H(0));
    mpdm_hset_s(v, L"size",     MPDM_X(F_hsize));
    mpdm_hset_s(v, L"exists",   MPDM_X(F_exists));
    mpdm_hset_s(v, L"delete",   MPDM_X(F_hdel));
    mpdm_hset_s(v, L"dump",     MPDM_X(F_dump));
    mpdm_hset_s(v, L"clone",    MPDM_X(F_clone));
    mpdm_hset_s(v, L"dumper",   MPDM_X(F_dumper));
    mpdm_hset_s(v, L"cmp",      MPDM_X(F_cmp));
    mpdm_hset_s(v, L"type",     MPDM_X(F_type));
    mpdm_hset_s(v, L"rnd",      MPDM_X(F_rnd));

    /* array methods */
    v = mpdm_hset_s(r, L"ARRAY",    MPDM_H(0));
    mpdm_hset_s(v, L"size",         MPDM_X(F_size));
    mpdm_hset_s(v, L"expand",       MPDM_X(F_expand));
    mpdm_hset_s(v, L"collapse",     MPDM_X(F_collapse));
    mpdm_hset_s(v, L"ins",          MPDM_X(F_ins));
    mpdm_hset_s(v, L"delete",       MPDM_X(F_adel));
    mpdm_hset_s(v, L"shift",        MPDM_X(F_shift));
    mpdm_hset_s(v, L"push",         MPDM_X(F_push));
    mpdm_hset_s(v, L"pop",          MPDM_X(F_pop));
    mpdm_hset_s(v, L"queue",        MPDM_X(F_queue));
    mpdm_hset_s(v, L"seek",         MPDM_X(F_seek));
    mpdm_hset_s(v, L"sort",         MPDM_X(F_sort));
    mpdm_hset_s(v, L"dump",         MPDM_X(F_dump));
    mpdm_hset_s(v, L"clone",        MPDM_X(F_clone));
    mpdm_hset_s(v, L"dumper",       MPDM_X(F_dumper));
    mpdm_hset_s(v, L"cmp",          MPDM_X(F_cmp));
    mpdm_hset_s(v, L"type",         MPDM_X(F_type));
    mpdm_hset_s(v, L"rnd",          MPDM_X(F_rnd));

    /* scalar methods */
    v = mpdm_hset_s(r, L"SCALAR",   MPDM_H(0));
    mpdm_hset_s(v, L"size",         MPDM_X(F_size));
    mpdm_hset_s(v, L"splice",       MPDM_X(F_splice));
    mpdm_hset_s(v, L"split",        MPDM_X(F_split));
    mpdm_hset_s(v, L"regex",        MPDM_X(F_regex));
    mpdm_hset_s(v, L"sregex",       MPDM_X(F_sregex));
    mpdm_hset_s(v, L"sscanf",       MPDM_X(F_sscanf));
    mpdm_hset_s(v, L"chr",          MPDM_X(F_chr));
    mpdm_hset_s(v, L"ord",          MPDM_X(F_ord));
    mpdm_hset_s(v, L"uc",           MPDM_X(F_uc));
    mpdm_hset_s(v, L"lc",           MPDM_X(F_lc));
    mpdm_hset_s(v, L"tr",           MPDM_X(F_tr));
    mpdm_hset_s(v, L"dump",         MPDM_X(F_dump));
    mpdm_hset_s(v, L"clone",        MPDM_X(F_clone));
    mpdm_hset_s(v, L"dumper",       MPDM_X(F_dumper));
    mpdm_hset_s(v, L"cmp",          MPDM_X(F_cmp));
    mpdm_hset_s(v, L"type",         MPDM_X(F_type));
    mpdm_hset_s(v, L"rnd",          MPDM_X(F_rnd));

    /* I/O methods */
    v = mpdm_hset_s(r, L"IO",           MPDM_H(0));
    mpdm_hset_s(v, L"read",             MPDM_X(M_read));
    mpdm_hset_s(v, L"write",            MPDM_X(M_write));
    mpdm_hset_s(v, L"getchar",          MPDM_X(M_getchar));
    mpdm_hset_s(v, L"putchar",          MPDM_X(M_putchar));
    mpdm_hset_s(v, L"fseek",            MPDM_X(M_fseek));
    mpdm_hset_s(v, L"ftell",            MPDM_X(M_ftell));
    mpdm_hset_s(v, L"close",            MPDM_X(M_close));
    mpdm_hset_s(v, L"type",             MPDM_X(F_type));

    /* "sys" namespace */
    v = mpdm_hset_s(r, L"sys",          MPDM_H(0));
    mpdm_hset_s(v, L"p",                MPDM_X(F_print));
    mpdm_hset_s(v, L"open",             MPDM_X(F_open));
    mpdm_hset_s(v, L"popen",            MPDM_X(F_popen));
    mpdm_hset_s(v, L"connect",          MPDM_X(F_connect));
    mpdm_hset_s(v, L"unlink",           MPDM_X(F_unlink));
    mpdm_hset_s(v, L"rename",           MPDM_X(F_rename));
    mpdm_hset_s(v, L"stat",             MPDM_X(F_stat));
    mpdm_hset_s(v, L"chmod",            MPDM_X(F_chmod));
    mpdm_hset_s(v, L"chown",            MPDM_X(F_chown));
    mpdm_hset_s(v, L"glob",             MPDM_X(F_glob));
    mpdm_hset_s(v, L"chdir",            MPDM_X(F_chdir));
    mpdm_hset_s(v, L"encoding",         MPDM_X(F_encoding));
    mpdm_hset_s(v, L"gettext",          MPDM_X(F_gettext));
    mpdm_hset_s(v, L"gettext_domain",   MPDM_X(F_gettext_domain));
    mpdm_hset_s(v, L"time",             MPDM_X(F_time));
    mpdm_hset_s(v, L"randomize",        MPDM_X(F_randomize));
    mpdm_hset_s(v, L"sleep",            MPDM_X(F_sleep));
    mpdm_hset_s(v, L"STDIN",            MPDM_F(stdin));
    mpdm_hset_s(v, L"STDOUT",           MPDM_F(stdout));
    mpdm_hset_s(v, L"STDERR",           MPDM_F(stderr));

    mpdm_hset_s(r, L"new",      MPDM_X(F_new));

    /* version */
    v = mpdm_hset_s(r, L"MPSL", MPDM_H(0));
    mpdm_hset_s(v, L"VERSION",  MPDM_MBS(VERSION));
    mpdm_hset_s(v, L"HOMEDIR",  mpdm_home_dir());
    mpdm_hset_s(v, L"APPDIR",   mpdm_app_dir());


    /* command line arguments */
    v = mpdm_hset_s(r, L"ARGV", MPDM_A(0));

    for (n = 0; n < argc; n++)
        mpdm_push(v, MPDM_MBS(argv[n]));

    /* special dump plugin */
    mpdm_dump_1 = mpsl_dump_1;

    mpdm_unref(r);
}
