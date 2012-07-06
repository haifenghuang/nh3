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
static mpdm_t F_size(F_ARGS)
{
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
static mpdm_t F_clone(F_ARGS)
{
    return mpdm_clone(A0);
}

/**
 * dump - Dumps a value to stdin.
 * @v: The value
 *
 * Dumps a value to stdin. The value can be complex. This function
 * is for debugging purposes only.
 * [Debugging]
 * [Input-Output]
 */
/** dump(v); */
static mpdm_t F_dump(F_ARGS)
{
    mpdm_dump(A0);
    return NULL;
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
static mpdm_t F_dumper(F_ARGS)
{
    return mpdm_dumper(A0);
}

/**
 * cmp - Compares two values.
 * @v1: the first value
 * @v2: the second value
 *
 * Compares two values. If both are strings, a standard string
 * comparison (using wcscmp()) is returned; if both are arrays,
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
static mpdm_t F_cmp(F_ARGS)
{
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
static mpdm_t F_is_array(F_ARGS)
{
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
static mpdm_t F_is_hash(F_ARGS)
{
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
static mpdm_t F_is_exec(F_ARGS)
{
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
static mpdm_t F_splice(F_ARGS)
{
    return mpdm_splice(A0, A1, IA2, IA3);
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
static mpdm_t F_expand(F_ARGS)
{
    return mpdm_expand(A0, IA1, IA2);
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
static mpdm_t F_collapse(F_ARGS)
{
    return mpdm_collapse(A0, IA1, IA2);
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
static mpdm_t F_ins(F_ARGS)
{
    return mpdm_ins(A0, A1, IA2);
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
 * Returns NULL (previous versions returned the deleted element).
 * [Arrays]
 */
/** v = adel(a, offset); */
static mpdm_t F_adel(F_ARGS)
{
    return mpdm_adel(A0, IA1);
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
static mpdm_t F_shift(F_ARGS)
{
    return mpdm_shift(A0);
}

/**
 * push - Pushes a value into an array.
 * @a: the array
 * @arg1: first value
 * @arg2: second value
 * @argn: nth value
 *
 * Pushes values into an array (i.e. inserts at the end).
 * Returns the last element pushed.
 * [Arrays]
 */
/** argn = push(a, arg1 [, arg2, ... argn]); */
static mpdm_t F_push(F_ARGS)
{
    int n;
    mpdm_t r = NULL;

    for (n = 1; n < mpdm_size(a); n++) {
        mpdm_unref(r);
        r = mpdm_push(A0, A(n));
        mpdm_ref(r);
    }

    return mpdm_unrefnd(r);
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
static mpdm_t F_pop(F_ARGS)
{
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
static mpdm_t F_queue(F_ARGS)
{
    return mpdm_queue(A0, A1, IA2);
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
static mpdm_t F_seek(F_ARGS)
{
    return MPDM_I(mpdm_seek(A0, A1, IA2));
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
static mpdm_t F_sort(F_ARGS)
{
    mpdm_t r, v;

    v = mpdm_ref(A0);
    r = mpdm_sort_cb(mpdm_clone(v), 1, A1);
    mpdm_unref(v);

    return r;
}

/**
 * split - Separates a string into an array of pieces.
 * @v: the value to be separated
 * @s: the separator
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
/** array = split(v, s); */
static mpdm_t F_split(F_ARGS)
{
    return mpdm_split(A0, A1);
}

/**
 * join - Joins an array.
 * @a: array to be joined
 * @s: joiner string or second array
 *
 * If @s is a string or NULL, returns a new string with all elements
 * in @a joined using @s. If @s is an array, it returns a new one
 * containing all elements of @a followed by all elements of @s.
 * [Arrays]
 * [Strings]
 */
/** string = join(a, joiner_str); */
/** array = join(a1, a2); */
static mpdm_t F_join(F_ARGS)
{
    return mpdm_join(A0, A1);
}

/**
 * hsize - Returns the number of pairs of a hash.
 * @h: the hash
 *
 * Returns the number of key-value pairs of a hash.
 * [Hashes]
 */
/** integer = hsize(h); */
static mpdm_t F_hsize(F_ARGS)
{
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
static mpdm_t F_exists(F_ARGS)
{
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
static mpdm_t F_hdel(F_ARGS)
{
    return mpdm_hdel(A0, A1);
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
 * `DETECTED_ENCODING' global variable. This value is
 * suitable to be copied over `ENCODING' or `TEMP_ENCODING'.
 *
 * If the file is open for writing, the encoding to be used is read from
 * the `ENCODING' global variable and, if not set, from the
 * `TEMP_ENCODING' one. The latter will always be deleted afterwards.
 * [Input-Output]
 * [Character Set Conversion]
 */
/** fd = open(filename, mode); */
static mpdm_t F_open(F_ARGS)
{
    return mpdm_open(A0, A1);
}

/**
 * close - Closes a file descriptor.
 * @fd: the file descriptor
 *
 * Closes the file descriptor.
 * [Input-Output]
 */
/** close(fd); */
static mpdm_t F_close(F_ARGS)
{
    return mpdm_close(A0);
}

/**
 * read - Reads a line from a file descriptor.
 * @fd: the file descriptor
 *
 * Reads a line from @fd. Returns the line, or NULL on EOF.
 * [Input-Output]
 * [Character Set Conversion]
 */
/** string = read(fd); */
static mpdm_t F_read(F_ARGS)
{
    return mpdm_read(A0);
}

/**
 * write - Writes values to a file descriptor.
 * @fd: the file descriptor
 * @arg1: first argument
 * @arg2: second argument
 * @argn: nth argument
 *
 * Writes the variable arguments to the file descriptor, doing
 * charset conversion in the process.
 *
 * Returns the total size written to @fd.
 * [Input-Output]
 * [Character Set Conversion]
 */
/** integer = write(fd, arg1 [,arg2 ... argn]); */
static mpdm_t F_write(F_ARGS)
{
    int n, r = 0;

    for (n = 1; n < mpdm_size(a); n++)
        r += mpdm_write(A0, A(n));

    return MPDM_I(r);
}


/**
 * getchar - Reads a character from a file descriptor.
 * @fd: the file descriptor
 *
 * Returns a character read from @fd, or NULL on EOF. No
 * charset conversion is done.
 * [Input-Output]
 */
/** string = getchar(fd); */
static mpdm_t F_getchar(F_ARGS)
{
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
 * Returns the number of chars written (0 or 1).
 * [Input-Output]
 */
/** s = putchar(fd, s); */
static mpdm_t F_putchar(F_ARGS)
{
    return MPDM_I(mpdm_putchar(A0, A1));
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
 * [Input-Output]
 */
/** integer = fseek(fd, offset, whence); */
static mpdm_t F_fseek(F_ARGS)
{
    return MPDM_I(mpdm_fseek(A0, IA1, IA2));
}

/**
 * ftell - Returns the current file pointer.
 * @fd: the file descriptor
 *
 * Returns the position of the file pointer in @fd.
 * [Input-Output]
 */
/** integer = ftell(fd); */
static mpdm_t F_ftell(F_ARGS)
{
    return MPDM_I(mpdm_ftell(A0));
}

/**
 * unlink - Deletes a file.
 * @filename: file name to be deleted
 *
 * Deletes a file.
 * [Input-Output]
 */
/** bool = unlink(filename); */
static mpdm_t F_unlink(F_ARGS)
{
    return mpsl_boolean(mpdm_unlink(A0));
}

/**
 * stat - Gives status from a file.
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
/** array = stat(filename); */
static mpdm_t F_stat(F_ARGS)
{
    return mpdm_stat(A0);
}

/**
 * chmod - Changes a file's permissions.
 * @filename: the file name
 * @perms: permissions (element 2 from stat())
 *
 * Changes the permissions for a file.
 * [Input-Output]
 */
/** integer = chmod(filename, perms); */
static mpdm_t F_chmod(F_ARGS)
{
    return MPDM_I(mpdm_chmod(A0, A1));
}

/**
 * chown - Changes a file's owner.
 * @filename: the file name
 * @uid: user id (element 4 from stat())
 * @gid: group id (element 5 from stat())
 *
 * Changes the owner and group id's for a file.
 * [Input-Output]
 */
/** integer = chown(filename, uid, gid); */
static mpdm_t F_chown(F_ARGS)
{
    return MPDM_I(mpdm_chown(A0, A1, A2));
}

/**
 * glob - Executes a file globbing.
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
/** array = glob(spec, base); */
static mpdm_t F_glob(F_ARGS)
{
    return mpdm_glob(A0, A1);
}

/**
 * encoding - Sets the current charset encoding for files.
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
/** integer = encoding(charset); */
static mpdm_t F_encoding(F_ARGS)
{
    return MPDM_I(mpdm_encoding(A0));
}

/**
 * popen - Opens a pipe.
 * @prg: the program to pipe
 * @mode: an fopen-like mode string
 *
 * Opens a pipe to a program. If @prg can be open in the specified @mode,
 * return file descriptor, or NULL otherwise.
 *
 * The @mode can be `r' (for reading), `w' (for writing), or `r+' or `w+'
 * for a special double pipe reading-writing mode.
 * [Input-Output]
 */
/** fd = popen(prg, mode); */
static mpdm_t F_popen(F_ARGS)
{
    return mpdm_popen(A0, A1);
}

/**
 * popen2 - Opens a pipe and returns an array of two pipes.
 * @prg: the program to pipe
 *
 * Opens a read-write pipe and returns an array of two descriptors,
 * one for reading and one for writing. If @prg could not be piped to,
 * returns NULL.
 * [Input-Output]
 */
/** array = popen2(prg); */
static mpdm_t F_popen2(F_ARGS)
{
    return mpdm_popen2(A0);
}

/**
 * pclose - Closes a pipe.
 * @fd: the value containing the file descriptor
 *
 * Closes a pipe.
 * [Input-Output]
 */
/** pclose(fd); */
static mpdm_t F_pclose(F_ARGS)
{
    return mpdm_pclose(A0);
}

/**
 * regex - Matches a regular expression.
 * @v: the value to be matched
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
/** string = regex(v, r); */
/** string = regex(v, r, offset); */
/** array = regex(v, ra); */
/** array = regex(); */
static mpdm_t F_regex(F_ARGS)
{
    return mpdm_regex(A0, A1, IA2);
}

/**
 * sregex - Matches and substitutes a regular expression.
 * @v: the value to be matched
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
/** string = sregex(v, r, s); */
/** string = sregex(v, r, s, offset); */
/** integer = sregex(); */
static mpdm_t F_sregex(F_ARGS)
{
    return mpdm_sregex(A0, A1, A2, IA3);
}

/**
 * gettext - Translates a string to the current language.
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
/** string = gettext(str); */
static mpdm_t F_gettext(F_ARGS)
{
    return mpdm_gettext(A0);
}

/**
 * gettext_domain - Sets domain and data directory for translations.
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
/** bool = gettext_domain(dom, data); */
static mpdm_t F_gettext_domain(F_ARGS)
{
    return MPDM_I(mpdm_gettext_domain(A0, A1));
}

/**
 * uc - Converts a string to uppercase.
 * @str: the string to be converted
 *
 * Returns @str converted to uppercase.
 * [Strings]
 */
/** string = uc(str); */
static mpdm_t F_uc(F_ARGS)
{
    return mpdm_ulc(A0, 1);
}

/**
 * lc - Converts a string to lowercase.
 * @str: the string to be converted
 *
 * Returns @str converted to lowercase.
 * [Strings]
 */
/** string = uc(str); */
static mpdm_t F_lc(F_ARGS)
{
    return mpdm_ulc(A0, 0);
}

/**
 * time - Returns the current time.
 *
 * Returns the current time from the epoch (C library time()).
 * [Time]
 */
/** integer = time(); */
static mpdm_t F_time(F_ARGS)
{
    return MPDM_I(time(NULL));
}

/**
 * chdir - Changes the working directory
 * @dir: the new path
 *
 * Changes the working directory
 * [Input-Output]
 */
/** integer = chdir(dir); */
static mpdm_t F_chdir(F_ARGS)
{
    return MPDM_I(mpdm_chdir(A0));
}

/**
 * sscanf - Extracts data like sscanf().
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
/** array = sscanf(str, fmt); */
/** array = sscanf(str, fmt, offset); */
static mpdm_t F_sscanf(F_ARGS)
{
    return mpdm_sscanf(A0, A1, IA2);
}

/**
 * sprintf - Formats a sprintf()-like string.
 * @fmt: the string format
 * @arg1: first argument
 * @arg2: second argument
 * @argn: nth argument
 *
 * Formats a string using the sprintf() format taking the values from
 * the variable arguments.
 * [Strings]
 */
/** string = sprintf(fmt, arg1 [,arg2 ... argn]); */
static mpdm_t F_sprintf(F_ARGS)
{
    mpdm_t f, v, r;

    a = mpdm_ref(mpdm_clone(a));
    f = mpdm_shift(a);

    /* if the first argument is an array, take it as the arguments */
    if ((v = mpdm_aget(a, 0)) != NULL && MPDM_IS_ARRAY(v))
        a = v;

    r = mpdm_sprintf(f, a);

    mpdm_unref(a);

    return r;
}


/**
 * print - Writes values to stdout.
 * @arg1: first argument
 * @arg2: second argument
 * @argn: nth argument
 *
 * Writes the variable arguments to stdout.
 * [Input-Output]
 */
/** print(arg1 [,arg2 ... argn]); */
static mpdm_t F_print(F_ARGS)
{
    int n;

    for (n = 0; n < mpdm_size(a); n++)
        mpdm_write_wcs(stdout, mpdm_string(A(n)));
    return NULL;
}


/**
 * chr - Returns the Unicode character represented by the codepoint.
 * @c: the codepoint as an integer value
 *
 * Returns a 1 character string containing the character which
 * Unicode codepoint is @c.
 * [Strings]
 */
/** string = chr(c); */
static mpdm_t F_chr(F_ARGS)
{
    wchar_t tmp[2];

    tmp[0] = (wchar_t) mpdm_ival(mpdm_aget(a, 0));
    tmp[1] = L'\0';

    return MPDM_S(tmp);
}


/**
 * ord - Returns the Unicode codepoint of a character.
 * @str: the string
 *
 * Returns the Unicode codepoint for the first character in
 * the string.
 * [Strings]
 */
/** integer = ord(str); */
static mpdm_t F_ord(F_ARGS)
{
    int ret = 0;
    mpdm_t v = mpdm_aget(a, 0);

    if (v != NULL) {
        wchar_t *ptr = mpdm_string(v);
        ret = (int) *ptr;
    }

    return MPDM_I(ret);
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

/**
 * random - Returns a random value.
 *
 * Returns a random number from 0 to value - 1.
 * [Miscellaneous]
 */
/** integer = random(value); */
static mpdm_t F_random(F_ARGS)
{
    static unsigned int seed = 0;
    int r = 0;
    int range = mpdm_ival(mpdm_aget(a, 0));

    if (range == 0 || seed == 0) {
        seed = time(NULL) ^ getpid();

        if (r)
            r = seed % range;
    }
    else {
        seed = (seed * 58321) + 11113;
        r = (seed >> 16) % range;
    }

    return MPDM_I(r);
};


/**
 * sleep - Sleeps a number of milliseconds.
 *
 * Sleeps a number of milliseconds.
 * [Threading]
 * [Time]
 */
/** sleep(msecs); */
static mpdm_t F_sleep(F_ARGS)
{
    mpdm_sleep(mpdm_ival(mpdm_aget(a, 0)));

    return NULL;
}


/**
 * mutex.lock - Locks a mutex (possibly waiting).
 *
 * Locks a mutex. If the mutex is already locked by
 * another process, it waits until it's unlocked.
 * [Threading]
 */
/** mutex.lock(); */
static mpdm_t F_mutex_lock(F_ARGS)
{
    mpdm_mutex_lock(mpdm_hget_s(l, L"v"));
    return l;
}


/**
 * mutex.unlock - Unlocks a mutex.
 *
 * Unlocks a mutex.
 * [Threading]
 */
/** mutex.unlock(); */
static mpdm_t F_mutex_unlock(F_ARGS)
{
    mpdm_mutex_unlock(mpdm_hget_s(l, L"v"));
    return l;
}


/**
 * mutex - Returns a new mutex object.
 *
 * Returns a new mutex object.
 * [Threading]
 */
/** local m = mutex(); */
static mpdm_t F_mutex(F_ARGS)
{
    mpdm_t o = mpdm_ref(MPDM_H(0));

    mpdm_hset_s(o, L"v",        mpdm_new_mutex());
    mpdm_hset_s(o, L"lock",     MPDM_X(F_mutex_lock));
    mpdm_hset_s(o, L"unlock",   MPDM_X(F_mutex_unlock));

    return mpdm_unrefnd(o);
}


/**
 * semaphore.wait - Waits for a semaphore to be ready.
 *
 * Waits for the value of a semaphore to be > 0. If it's
 * not, the thread waits until it is.
 * [Threading]
 */
/** semaphore.wait(); */
static mpdm_t F_semaphore_wait(F_ARGS)
{
    mpdm_semaphore_wait(mpdm_hget_s(l, L"v"));
    return NULL;
}


/**
 * semaphore.post - Increments the value of a semaphore.
 *
 * Increments by 1 the value of a semaphore.
 * [Threading]
 */
/** semaphore.post(); */
static mpdm_t F_semaphore_post(F_ARGS)
{
    mpdm_semaphore_post(mpdm_hget_s(l, L"v"));
    return NULL;
}


/**
 * semaphore - Returns a new semaphore.
 * cnt: the initial count of the semaphore.
 *
 * Returns a new semaphore.
 * [Threading]
 */
/** local s = semaphore(); */
/** local s = semaphore(cnt); */
static mpdm_t F_semaphore(F_ARGS)
{
    mpdm_t o = mpdm_ref(MPDM_H(0));

    mpdm_hset_s(o, L"v",    mpdm_new_semaphore(IA0));
    mpdm_hset_s(o, L"post", MPDM_X(F_semaphore_post));
    mpdm_hset_s(o, L"wait", MPDM_X(F_semaphore_wait));

    return mpdm_unrefnd(o);
}


/**
 * tr - Transliterates a string.
 * @str: the string
 * @from: set of characters to be replaced
 * @to: set of characters to replace
 *
 * Transliterates @str to a new string with all characters from @from
 * replaced by those in @to.
 * [Threading]
 */
/** tr(str, from, to); */
static mpdm_t F_tr(F_ARGS)
{
    return mpdm_tr(A0, A1, A2);
}


/**
 * strftime - Formats the time as a string.
 * @f: format string
 * @t: seconds (from the Unix epoch)
 *
 * Returns as a string, formatted using the @f format string,
 * the time @t given as argument (or now if @t is not given).
 * See the local clib strftime() implementation for details
 * on the format string.
 * [Strings]
 * [Time]
 */
/** string = strftime(@f); */
/** string = strftime(@f, @t); */
static mpdm_t F_strftime(F_ARGS)
{
    char *ptr;
    time_t t;
    char tmp[2048];
    struct tm *tm;

    ptr = mpdm_wcstombs(mpdm_string(A0), NULL);

    if ((t = IA1) == 0)
        t = time(NULL);

    tm = localtime(&t);

    strftime(tmp, sizeof(tmp) - 1, ptr, tm);

    free(ptr);

    return MPDM_MBS(tmp);
}


/**
 * connect - Opens a client TCP/IP socket.
 * @h: host name or ip
 * @s: service or port number
 *
 * Opens a client TCP/IP socket to the @h host at @s service (or port).
 * Returns NULL if the connection cannot be done or a file type value,
 * that can be used with all file operation functions, including close().
 * [Sockets]
 * [Input-Output]
 */
/** f = connect(@h, @s); */
static mpdm_t F_connect(F_ARGS)
{
    return mpdm_connect(A0, A1);
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

void mpsl_library_init(mpdm_t r, int argc, char *argv[])
/* inits the library */
{
    int n;
    mpdm_t v;

    mpdm_ref(r);

    /* standard file descriptors */
    mpdm_hset_s(r, L"STDIN",    MPDM_F(stdin));
    mpdm_hset_s(r, L"STDOUT",   MPDM_F(stdout));
    mpdm_hset_s(r, L"STDERR",   MPDM_F(stderr));

    /* home and application directories */
    mpdm_hset_s(r, L"HOMEDIR",  mpdm_home_dir());
    mpdm_hset_s(r, L"APPDIR",   mpdm_app_dir());

    /* library functions */
    mpdm_hset_s(r, L"size",     MPDM_X(F_size));
    mpdm_hset_s(r, L"clone",    MPDM_X(F_clone));
    mpdm_hset_s(r, L"dump",     MPDM_X(F_dump));
    mpdm_hset_s(r, L"dumper",   MPDM_X(F_dumper));
    mpdm_hset_s(r, L"cmp",      MPDM_X(F_cmp));
    mpdm_hset_s(r, L"is_array", MPDM_X(F_is_array));
    mpdm_hset_s(r, L"is_hash",  MPDM_X(F_is_hash));
    mpdm_hset_s(r, L"is_exec",  MPDM_X(F_is_exec));
    mpdm_hset_s(r, L"splice",   MPDM_X(F_splice));
    mpdm_hset_s(r, L"expand",   MPDM_X(F_expand));
    mpdm_hset_s(r, L"collapse", MPDM_X(F_collapse));
    mpdm_hset_s(r, L"ins",      MPDM_X(F_ins));
    mpdm_hset_s(r, L"adel",     MPDM_X(F_adel));
    mpdm_hset_s(r, L"shift",    MPDM_X(F_shift));
    mpdm_hset_s(r, L"push",     MPDM_X(F_push));
    mpdm_hset_s(r, L"pop",      MPDM_X(F_pop));
    mpdm_hset_s(r, L"queue",    MPDM_X(F_queue));
    mpdm_hset_s(r, L"hsize",    MPDM_X(F_hsize));
    mpdm_hset_s(r, L"exists",   MPDM_X(F_exists));
    mpdm_hset_s(r, L"hdel",     MPDM_X(F_hdel));

    mpdm_hset_s(r, L"open",     MPDM_X(F_open));
    mpdm_hset_s(r, L"close",    MPDM_X(F_close));
    mpdm_hset_s(r, L"read",     MPDM_X(F_read));
    mpdm_hset_s(r, L"write",    MPDM_X(F_write));
    mpdm_hset_s(r, L"getchar",  MPDM_X(F_getchar));
    mpdm_hset_s(r, L"putchar",  MPDM_X(F_putchar));
    mpdm_hset_s(r, L"fseek",    MPDM_X(F_fseek));
    mpdm_hset_s(r, L"ftell",    MPDM_X(F_ftell));

    mpdm_hset_s(r, L"unlink",   MPDM_X(F_unlink));
    mpdm_hset_s(r, L"stat",     MPDM_X(F_stat));
    mpdm_hset_s(r, L"chmod",    MPDM_X(F_chmod));
    mpdm_hset_s(r, L"chown",    MPDM_X(F_chown));
    mpdm_hset_s(r, L"glob",     MPDM_X(F_glob));
    mpdm_hset_s(r, L"chdir",    MPDM_X(F_chdir));

    mpdm_hset_s(r, L"seek",     MPDM_X(F_seek));
    mpdm_hset_s(r, L"sort",     MPDM_X(F_sort));
    mpdm_hset_s(r, L"split",    MPDM_X(F_split));
    mpdm_hset_s(r, L"join",     MPDM_X(F_join));

    mpdm_hset_s(r, L"encoding", MPDM_X(F_encoding));
    mpdm_hset_s(r, L"popen",    MPDM_X(F_popen));
    mpdm_hset_s(r, L"popen2",   MPDM_X(F_popen2));
    mpdm_hset_s(r, L"pclose",   MPDM_X(F_pclose));

    mpdm_hset_s(r, L"regex",    MPDM_X(F_regex));
    mpdm_hset_s(r, L"sregex",   MPDM_X(F_sregex));
    mpdm_hset_s(r, L"sscanf",   MPDM_X(F_sscanf));
    mpdm_hset_s(r, L"sprintf",  MPDM_X(F_sprintf));
    mpdm_hset_s(r, L"strftime", MPDM_X(F_strftime));

    mpdm_hset_s(r, L"gettext",  MPDM_X(F_gettext));
    mpdm_hset_s(r, L"gettext_domain",  MPDM_X(F_gettext_domain));

    mpdm_hset_s(r, L"print",    MPDM_X(F_print));

    mpdm_hset_s(r, L"chr",      MPDM_X(F_chr));
    mpdm_hset_s(r, L"ord",      MPDM_X(F_ord));
    mpdm_hset_s(r, L"uc",       MPDM_X(F_uc));
    mpdm_hset_s(r, L"lc",       MPDM_X(F_lc));
    mpdm_hset_s(r, L"tr",       MPDM_X(F_tr));

    mpdm_hset_s(r, L"time",     MPDM_X(F_time));
    mpdm_hset_s(r, L"random",   MPDM_X(F_random));
    mpdm_hset_s(r, L"sleep",    MPDM_X(F_sleep));

    mpdm_hset_s(r, L"mutex",     MPDM_X(F_mutex));
    mpdm_hset_s(r, L"semaphore", MPDM_X(F_semaphore));

    mpdm_hset_s(r, L"connect",     MPDM_X(F_connect));

    mpdm_hset_s(r, L"bincall",  MPDM_X(F_bincall));

    mpdm_hset_s(r, L"new",      MPDM_X(F_new));

    /* version */
    v = mpdm_hset_s(r, L"MPSL", MPDM_H(0));
    mpdm_hset_s(v, L"VERSION",  MPDM_MBS(VERSION));

    /* command line arguments */
    v = mpdm_hset_s(r, L"ARGV", MPDM_A(0));

    for (n = 0; n < argc; n++)
        mpdm_push(v, MPDM_MBS(argv[n]));

    mpdm_unref(r);
}
