#!/bin/sh

if [ -d .git ] ; then
    echo "char *git_rev_str = \"$(git rev-parse HEAD)\";" > git-rev-str.c
fi
