#!/bin/sh

# Configuration shell script

TARGET=mpsl

# gets program version
VERSION=`cut -f2 -d\" VERSION`

# default installation prefix
PREFIX=/usr/local

# parse arguments
while [ $# -gt 0 ] ; do

	case $1 in
	--without-win32)	WITHOUT_WIN32=1 ;;
	--without-unix-glob)	WITHOUT_UNIX_GLOB=1 ;;
	--with-included-regex)	WITH_INCLUDED_REGEX=1 ;;
	--with-pcre)		WITH_PCRE=1 ;;
	--without-gettext)	WITHOUT_GETTEXT=1 ;;
	--without-iconv)	WITHOUT_ICONV=1 ;;
	--without-wcwidth)	WITHOUT_WCWIDTH=1 ;;
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
	echo "--without-gettext     Disable gettext usage."
	echo "--without-iconv       Disable iconv usage."
	echo "--without-wcwidth     Disable system wcwidth() (use workaround)."

	echo
	echo "Environment variables:"
	echo "CC                    C Compiler."
	echo "AR                    Library Archiver."
	echo "YACC                  Parser."

	exit 1
fi

echo "Configuring mpsl..."

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

echo "CC=$CC" >> makefile.opts

# set cflags
if [ "$CFLAGS" = "" ] ; then
	CFLAGS="-g -Wall"
fi

echo "CFLAGS=$CFLAGS" >> makefile.opts

# Add CFLAGS to CC
CC="$CC $CFLAGS"

# set archiver
if [ "$AR" = "" ] ; then
	AR=ar
fi

echo "AR=$AR" >> makefile.opts

# set parser
if [ "$YACC" = "" ] ; then
	YACC=yacc
fi

echo "YACC=$YACC" >> makefile.opts

# add version
cat VERSION >> config.h

# add installation prefix
echo "#define CONFOPT_PREFIX \"$PREFIX\"" >> config.h

#########################################################

# configuration directives

# mpdm
echo -n "Looking for mpdm... "

for MPDM in ./mpdm ../mpdm NOTFOUND ; do
	if [ -d $MPDM ] && [ -f $MPDM/mpdm.h ] ; then
		break
	fi
done

if [ "$MPDM" != "NOTFOUND" ] ; then
	echo "-I$MPDM" >> config.cflags
	echo "-L$MPDM -lmpdm" >> config.ldflags
	echo "OK ($MPDM)"
else
	echo "No"
	exit 1
fi

# If mpdm is not configured, do it
if [ ! -f $MPDM/Makefile ] ; then
	CONF_ARGS="--prefix=$PREFIX"
	[ "$WITHOUT_WIN32" = 1 ] && CONF_ARGS="$CONF_ARGS --without-win32"
	[ "$WITHOUT_UNIX_GLOB" = 1 ] && CONF_ARGS="$CONF_ARGS --without-unix-glob"
	[ "$WITH_INCLUDED_REGEX" = 1 ] && CONF_ARGS="$CONF_ARGS --with-included-regex"
	[ "$WITH_PCRE" = 1 ] && CONF_ARGS="$CONF_ARGS --with-pcre"
	[ "$WITHOUT_GETTEXT" = 1 ] && CONF_ARGS="$CONF_ARGS --without-gettext"
	[ "$WITHOUT_ICONV" = 1 ] && CONF_ARGS="$CONF_ARGS --without-iconv"
	[ "$WITHOUT_WCWIDTH" = 1 ] && CONF_ARGS="$CONF_ARGS --without-wcwidth"
	( echo ; cd $MPDM ; ./config.sh $CONF_ARGS ; echo )
fi

cat $MPDM/config.ldflags >> config.ldflags

# if win32, the interpreter is called mpsl.exe
grep CONFOPT_WIN32 ${MPDM}/config.h >/dev/null && TARGET=mpsl.exe

#########################################################

# final setup

echo "MPDM=$MPDM" >> makefile.opts
echo "VERSION=$VERSION" >> makefile.opts
echo "PREFIX=$PREFIX" >> makefile.opts
echo "TARGET=$TARGET" >> makefile.opts
echo >> makefile.opts

cat makefile.opts makefile.in makefile.depend > Makefile

#########################################################

# cleanup

rm -f .tmp.c .tmp.o

exit 0
