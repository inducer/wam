#!/bin/sh

set -e

libtoolize --force
echo -n "wait: "
echo -n "."
aclocal -I $HOME/pool/share/aclocal
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
