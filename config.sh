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
	fi
fi

# set archiver
if [ "$AR" = "" ] ; then
	AR=ar
fi

# set parser
if [ "$YACC" = "" ] ; then
	YACC=yacc
fi

echo "CC=$CC" >> makefile.opts
echo "AR=$AR" >> makefile.opts
echo "YACC=$YACC" >> makefile.opts

# add version
cat VERSION >> config.h

# add installation prefix
echo "#define CONFOPT_PREFIX \"$PREFIX\"" >> config.h

#########################################################

# configuration directives

# mpdm
echo -n "Testing for mpdm... "
if [ -f ../mpdm/mpdm.h ] ; then
	echo "-I../mpdm" >> config.cflags
	echo "-L../mpdm -lmpdm" >> config.ldflags
	echo "OK"
else
	echo "No"
	exit 1
fi

#########################################################

cat ../mpdm/config.ldflags >> config.ldflags

# final setup

echo "VERSION=$VERSION" >> makefile.opts
echo "PREFIX=$PREFIX" >> makefile.opts
echo >> makefile.opts

cat makefile.opts makefile.in makefile.depend > Makefile

#########################################################

# cleanup

rm -f .tmp.c .tmp.o

exit 0
