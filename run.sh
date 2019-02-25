#!/bin/sh
shopt -s dotglob

if test ! -d build | test ! -f build/Makefile \
       | test ! -f build/aclocal.m4 | test ! -f build/libtool \
       | test ! -f build/config.h | test ! -f build/Makefile.in \
       | test ! -f build/libspider.la | test ! -f build/stamp-h1 ; then
    mkdir build
    ./autogen.sh
    ./configure
else
    mv -f build/* . &> /dev/null
fi

make CXXFLAGS+='-O0 -g' -j9
mv -f .libs/ libspider.la Makefile aclocal.m4 autom4te.cache build-aux config.h config.h.in config.log config.status configure libtool m4 Makefile.in stamp-h1 build/ &> /dev/null
