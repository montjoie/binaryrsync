#!/bin/sh

cd $(dirname $0)

autoreconf --install || exit $?
./configure || exit $?
make || exit $?
