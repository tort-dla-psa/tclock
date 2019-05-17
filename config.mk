# version
VERSION = 0.0.2

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

PKG_CONFIG = pkg-config

# includes and libs
INCS = `$(PKG_CONFIG) --cflags ncurses`
LIBS = -lpthread\
       `$(PKG_CONFIG) --libs ncurses`

# flags
EXTRA =
STCPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -O2 -s
STCFLAGS = $(INCS) $(STCPPFLAGS) $(CPPFLAGS) $(EXTRA)
STLDFLAGS = $(LIBS) $(LDFLAGS)

# OpenBSD:
#CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
# compiler and linker
# CXX = g++
