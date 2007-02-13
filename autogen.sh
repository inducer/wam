#!/bin/sh

set -e

echo -n "wait: "
echo -n "."
aclocal
echo -n "."
autoheader
echo -n "."
autoconf
echo -n "."
automake -a
echo "done"
rm -f config.cache

echo
echo "ready to run configure."
