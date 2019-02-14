#!/bin/sh
shopt -s dotglob

if test ! -d build; then
    mkdir build
    ./autogen.sh
    ./configure
else
    mv -f build/* . &> /dev/null
fi

make CXXFLAGS+='-O0' -j9
mv -f .libs/ libspider.la Makefile aclocal.m4 autom4te.cache build-aux config.h config.h.in config.log config.status configure libtool m4 Makefile.in stamp-h1 build/ &> /dev/null
