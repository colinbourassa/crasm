SHELL=/bin/sh
MAKE=make

prefix=/usr
bindir=${prefix}/bin
mandir=${prefix}/share/man

all:
	( cd src; ${MAKE} all )

clean:
	( cd src; ${MAKE} clean )

install:
	cp src/crasm ${bindir}/crasm
	chmod 0755 ${bindir}/crasm
	cp crasm.1 ${mandir}/man1/crasm.1
	chmod 0644 ${mandir}/man1/crasm.1
