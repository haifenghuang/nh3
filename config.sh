#!/bin/sh

# Configuration shell script

# gets program version
VERSION=`cut -f2 -d\" VERSION`

# default installation prefix
PREFIX=/usr/local

# parse arguments
while [ $# -gt 0 ] ; do

	case $1 in
	--without-win32)	WITHOUT_WIN32=1 ;;
	--without-unix-glob)	WITHOUT_UNIX_GLOB=1 ;;
	--without-regex)	WITHOUT_REGEX=1 ;;
	--with-included-regex)	WITH_INCLUDED_REGEX=1 ;;
	--with-pcre)		WITH_PCRE=1 ;;
	--help)			CONFIG_HELP=1 ;;

	--prefix)		PREFIX=$2 ; shift ;;
	--prefix=*)		PREFIX=`echo $1 | sed -e 's/--prefix=//'` ;;
	esac

	shift
done

if [ "$CONFIG_HELP" = "1" ] ; then

	echo "Available options:"
	echo "--prefix=PREFIX       Installation prefix ($PREFIX)."
	echo "--without-win32       Disable win32 interface detection."
	echo "--without-unix-glob   Disable glob.h usage (use workaround)."
	echo "--with-included-regex Use included regex code (gnu_regex.c)."
	echo "--with-pcre           Enable PCRE library detection."

	echo
	echo "Environment variables:"
	echo "CC                    C Compiler."
	echo "AR                    Library Archiver."
	echo "LEX                   Lexer."
	echo "YACC                  Parser."

	exit 1
fi

echo "Configuring..."

echo "/* automatically created by config.sh - do not modify */" > config.h
echo "# automatically created by config.sh - do not modify" > makefile.opts
> config.ldflags
> config.cflags
> .config.log

# set compiler
if [ "$CC" = "" ] ; then
	CC=cc
	# if CC is unset, try if gcc is available
	which gcc > /dev/null

	if [ $? = 0 ] ; then
		CC=gcc
		echo "#define _GNU_SOURCE 1" >> config.h
	fi
fi

# set archiver
if [ "$AR" = "" ] ; then
	AR=ar
fi

# set lexer
if [ "$LEX" = "" ] ; then
	LEX=lex
fi

# set parser
if [ "$YACC" = "" ] ; then
	YACC=yacc
fi

echo "CC=$CC" >> makefile.opts
echo "AR=$AR" >> makefile.opts
echo "LEX=$LEX" >> makefile.opts
echo "YACC=$YACC" >> makefile.opts

# add version
cat VERSION >> config.h

# add installation prefix
echo "#define CONFOPT_PREFIX \"$PREFIX\"" >> config.h

#########################################################

# configuration directives

# Win32
echo -n "Testing for win32... "
if [ "$WITHOUT_WIN32" = "1" ] ; then
	echo "Disabled by user"
else
	echo "#include <windows.h>" > .tmp.c
	echo "#include <commctrl.h>" >> .tmp.c
	echo "int STDCALL WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int m)" >> .tmp.c
	echo "{ return 0; }" >> .tmp.c

	TMP_LDFLAGS="-mwindows -lcomctl32"
	$CC .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "#define CONFOPT_WIN32 1" >> config.h
		echo "$TMP_LDFLAGS " >> config.ldflags
		echo "OK"
		WITHOUT_UNIX_GLOB=1
	else
		echo "No"
	fi
fi

# glob.h support
if [ "$WITHOUT_UNIX_GLOB" != 1 ] ; then
	echo -n "Testing for unix-like glob.h... "
	echo "#include <stdio.h>" > .tmp.c
	echo "#include <glob.h>" >> .tmp.c
	echo "int main(void) { glob_t g; g.gl_offs=1; glob(\"*\",GLOB_MARK,NULL,&g); return 0; }" >> .tmp.c

	$CC .tmp.c -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "#define CONFOPT_GLOB_H 1" >> config.h
		echo "OK"
	else
		echo "No; activated workaround"
	fi
fi

# regex
echo -n "Testing for regular expressions... "

if [ "$WITH_PCRE" = 1 ] ; then
	# try first the pcre library
	TMP_CFLAGS="-I/usr/local/include"
	TMP_LDFLAGS="-L/usr/local/lib -lpcre -lpcreposix"
	echo "#include <pcreposix.h>" > .tmp.c
	echo "int main(void) { regex_t r; regmatch_t m; regcomp(&r,\".*\",REG_EXTENDED|REG_ICASE); return 0; }" >> .tmp.c

	$CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "OK (using pcre library)"
		echo "#define CONFOPT_PCRE 1" >> config.h
		echo "$TMP_CFLAGS " >> config.cflags
		echo "$TMP_LDFLAGS " >> config.ldflags
		REGEX_YET=1
	fi
fi

if [ "$REGEX_YET" != 1 -a "$WITH_INCLUDED_REGEX" != 1 ] ; then
	echo "#include <sys/types.h>" > .tmp.c
	echo "#include <regex.h>" >> .tmp.c
	echo "int main(void) { regex_t r; regmatch_t m; regcomp(&r,\".*\",REG_EXTENDED|REG_ICASE); return 0; }" >> .tmp.c

	$CC .tmp.c -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "OK (using system one)"
		echo "#define CONFOPT_SYSTEM_REGEX 1" >> config.h
		REGEX_YET=1
	fi
fi

if [ "$REGEX_YET" != 1 ] ; then
	# if system libraries lack regex, try compiling the
	# included gnu_regex.c

	$CC -c -DSTD_HEADERS -DREGEX gnu_regex.c -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "OK (using included gnu_regex.c)"
		echo "#define HAVE_STRING_H 1" >> config.h
		echo "#define REGEX 1" >> config.h
		echo "#define CONFOPT_INCLUDED_REGEX 1" >> config.h
	else
		echo "#define CONFOPT_NO_REGEX 1" >> config.h
		echo "No (No usable regex library)"
	fi
fi

# unistd.h detection
echo -n "Testing for unistd.h... "
echo "#include <unistd.h>" > .tmp.c
echo "int main(void) { return(0); }" >> .tmp.c

$CC .tmp.c -o .tmp.o 2>> .config.log

if [ $? = 0 ] ; then
	echo "#define CONFOPT_UNISTD_H 1" >> config.h
	echo "OK"
else
	echo "No"
fi

# gdbm detection
echo -n "Testing for GDBM library... "
echo "#include <gdbm.h>" > .tmp.c
echo "int main(void) {" >> .tmp.c
echo "gdbm_close(gdbm_open(\".t\", 512, GDBM_WRCREAT, 0666, 0));" >> .tmp.c
echo "return 0; }" >> .tmp.c
TMP_LDFLAGS="-lgdbm"

$CC .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

if [ $? = 0 ] ; then
	echo "#define CONFOPT_GDBM 1" >> config.h
	echo "$TMP_LDFLAGS" >> config.ldflags
	echo "OK"
else
	echo "No"
fi

echo "#ifdef CONFOPT_WIN32" >> config.h
echo "#define SWPRINTF _snwprintf" >> config.h
echo "#else" >> config.h
echo "#define SWPRINTF swprintf" >> config.h
echo "#endif" >> config.h

#########################################################

# final setup

echo "VERSION=$VERSION" >> makefile.opts
echo "PREFIX=$PREFIX" >> makefile.opts
echo >> makefile.opts

cat makefile.opts makefile.in makefile.depend > Makefile

#########################################################

# cleanup

rm -f .tmp.c .tmp.o

exit 0
