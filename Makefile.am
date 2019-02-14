NULL =

EXTRA_DIST = autogen.sh
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
              -include $(top_builddir)/config.h \
              -I${top_srcdir}/src \
              -I${top_srcdir}/lib

AM_CXXFLAGS = \
              $(WARN_CXXFLAGS) \
              $(BOOST_CPPFLAGS) \
              -Werror=unused-parameter \
              -Werror=missing-field-initializers \
              -Werror=overloaded-virtual \
              -Wno-inline \
              $(NULL)

AM_LDFLAGS = \
             $(WARN_LDFLAGS) \
             $(BOOST_LDFLAGS) \
             -lev \
             -lssl \
             -lcrypto \
             -Wno-unused-command-line-argument \
             $(NULL)

bin_PROGRAMS = spider
lib_LTLIBRARIES = libspider.la

libspider_la_SOURCES = \
                       src/config/config.cc \
                       src/config/config.hh \
                       src/error/not-implemented.hh \
                       src/events/event-loop.hh \
                       src/events/events.cc \
                       src/events/events.hh \
                       src/events/listener.hh \
                       src/events/server.hh \
                       src/events/register.hh \
                       src/events/register.hxx \
                       src/misc/addrinfo/addrinfo-error.cc \
                       src/misc/addrinfo/addrinfo-error.hh \
                       src/misc/addrinfo/addrinfo-iterator.hh \
                       src/misc/addrinfo/addrinfo.cc \
                       src/misc/addrinfo/addrinfo.hh \
                       src/misc/fd.cc \
                       src/misc/fd.hh \
                       src/misc/socket.hh \
                       src/misc/sys-wrapper.hh \
                       src/misc/unistd.hh \
                       src/request/error.cc \
                       src/request/error.hh \
                       src/request/request.hh \
                       src/request/request.cc \
                       src/request/response.hh \
                       src/request/response.cc \
                       src/request/types.hh \
                       src/socket/default-socket.cc \
                       src/socket/default-socket.hh \
                       src/socket/socket.cc \
                       src/socket/socket.hh \
                       src/vhost/connection.hh \
                       src/vhost/dispatcher.hh \
                       src/vhost/dispatcher.cc \
                       src/vhost/vhost-factory.hh \
                       src/vhost/vhost-static-file.hh \
                       src/vhost/vhost-static-file.cc \
                       src/vhost/vhost.hh \
                       $(NULL)

spider_SOURCES = \
                 src/main.cc \
                 $(NULL)

spider_LDADD = libspider.la
