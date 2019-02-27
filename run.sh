#!/bin/sh

trap move_back SIGINT

function set_up()
{
    mkdir build
    ./autogen.sh
    ./configure
}

function move_back()
{
    mv -f config.h.in~ .libs/ libspider.la Makefile aclocal.m4 autom4te.cache build-aux config.h config.h.in config.log config.status configure libtool m4 Makefile.in stamp-h1 build/ &> /dev/null
    exit
}

shopt -s dotglob

if test ! -d build | test ! -f build/Makefile \
       | test ! -f build/aclocal.m4 | test ! -f build/libtool \
       | test ! -f build/config.h | test ! -f build/Makefile.in \
       | test ! -f build/libspider.la | test ! -f build/stamp-h1 ; then
    set_up
else
    mv -f build/* . &> /dev/null
fi

if [ "$1" = "clean" ]; then
    make clean
else
    if [ "$1" = "check" ]; then
        make check
    else
        make CXXFLAGS+='-O0 -g' -j9
    fi
fi

move_back
