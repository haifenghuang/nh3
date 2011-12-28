#!/bin/sh

# Configuration shell script

TARGET=mpsl

# gets program version
VERSION=`cut -f2 -d\" VERSION`

# default installation prefix
PREFIX=/usr/local

# installation directory for documents
DOCDIR=""

# store command line args for configuring the libraries
CONF_ARGS="$*"

# parse arguments
while [ $# -gt 0 ] ; do

	case $1 in
	--help)			CONFIG_HELP=1 ;;

	--mingw32)		CC=i586-mingw32msvc-cc
				WINDRES=i586-mingw32msvc-windres
				AR=i586-mingw32msvc-ar
				;;

	--prefix)		PREFIX=$2 ; shift ;;
	--prefix=*)		PREFIX=`echo $1 | sed -e 's/--prefix=//'` ;;

	--docdir)		DOCDIR=$2 ; shift ;;
	--docdir=*)		DOCDIR=`echo $1 | sed -e 's/--docdir=//'` ;;

	esac

	shift
done

if [ "$CONFIG_HELP" = "1" ] ; then

	echo "Available options:"
	echo "--prefix=PREFIX       Installation prefix ($PREFIX)."
	echo "--docdir=DOCDIR       Instalation directory for documentation."
	echo "--without-win32       Disable win32 interface detection."
	echo "--without-unix-glob   Disable glob.h usage (use workaround)."
	echo "--with-included-regex Use included regex code (gnu_regex.c)."
	echo "--with-pcre           Enable PCRE library detection."
	echo "--without-gettext     Disable gettext usage."
	echo "--without-iconv       Disable iconv usage."
	echo "--without-wcwidth     Disable system wcwidth() (use workaround)."
	echo "--mingw32             Build using the mingw32 compiler."

	echo
	echo "Environment variables:"
	echo "CC                    C Compiler."
	echo "AR                    Library Archiver."
	echo "YACC                  Parser."

	exit 1
fi

if [ "$DOCDIR" = "" ] ; then
	DOCDIR=$PREFIX/share/doc/mpsl
fi

echo "Configuring MPSL..."

echo "/* automatically created by config.sh - do not modify */" > config.h
echo "# automatically created by config.sh - do not modify" > makefile.opts
> config.ldflags
> config.cflags
> .config.log

# set compiler
if [ "$CC" = "" ] ; then
	CC=cc
	# if CC is unset, try if gcc is available
	which gcc > /dev/null 2>&1

	if [ $? = 0 ] ; then
		CC=gcc
	fi
fi

echo "CC=$CC" >> makefile.opts

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

# CFLAGS
if [ -z "$CFLAGS" ] ; then
    CFLAGS="-g -Wall"
fi

echo -n "Testing if C compiler supports ${CFLAGS}... "
echo "int main(int argc, char *argv[]) { return 0; }" > .tmp.c

$CC .tmp.c -o .tmp.o 2>> .config.log

if [ $? = 0 ] ; then
    echo "OK"
else
    echo "No; resetting to defaults"
    CFLAGS=""
fi

echo "CFLAGS=$CFLAGS" >> makefile.opts

# Add CFLAGS to CC
CC="$CC $CFLAGS"

# MPDM
echo -n "Looking for MPDM... "

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

# If MPDM is not configured, do it
if [ ! -f $MPDM/Makefile ] ; then
	( echo ; cd $MPDM ; ./config.sh $CONF_ARGS ; echo )
fi

cat $MPDM/config.ldflags >> config.ldflags
echo "-lm" >> config.ldflags

# if win32, the interpreter is called mpsl.exe
grep CONFOPT_WIN32 ${MPDM}/config.h >/dev/null && TARGET=mpsl.exe

#########################################################

# final setup

echo "MPDM=$MPDM" >> makefile.opts
grep DOCS $MPDM/makefile.opts >> makefile.opts
echo "VERSION=$VERSION" >> makefile.opts
echo "PREFIX=\$(DESTDIR)$PREFIX" >> makefile.opts
echo "DOCDIR=\$(DESTDIR)$DOCDIR" >> makefile.opts
echo "TARGET=$TARGET" >> makefile.opts
echo >> makefile.opts

cat makefile.opts makefile.in makefile.depend > Makefile

#########################################################

# cleanup

rm -f .tmp.c .tmp.o

exit 0
