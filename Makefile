# tclock - clock for terminal/console
# See LICENSE file for copyright and license details.
.POSIX:

include config.mk

SRC = tclock.cpp
OBJ = $(SRC:.cpp=.o)

all: options tclock

options:
	@echo termclock build options:
	@echo "CXXFLAGS  = $(STCFLAGS)"
	@echo "LDFLAGS = $(STLDFLAGS)"
	@echo "CXX      = $(CXX)"

tclock: config.h tclock.h config.mk $(SRC)
	$(CXX) $(SRC) -o $@ $(STCFLAGS) $(STLDFLAGS)

clean:
	rm -f tclock $(OBJ) tclock-$(VERSION).tar.gz

dist: clean
	mkdir -p tclock-$(VERSION)
	cp -Rr * tclock-$(VERSION)
	tar -cf - tclock-$(VERSION) | gzip > tclock-$(VERSION).tar.gz
	rm -rf tclock-$(VERSION)

install: tclock
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f tclock $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/tclock
#	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
#	sed "s/VERSION/$(VERSION)/g" < tclock.1 > $(DESTDIR)$(MANPREFIX)/man1/tclock.1
#	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/tclock.1
#	tic -sx st.info
#	@echo Please see the README file regarding the terminfo entry of tclock.

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/tclock
#	rm -f $(DESTDIR)$(MANPREFIX)/man1/tclock.1

.PHONY: all options clean dist install uninstall
