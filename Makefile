all: build/lib build/program

build:
	mkdir -p ./build

build/lib:
	gcc -c parser.c -o parser.o
	ar rcs libjsonparse.a parser.o

build/program: parser.o
	gcc json_parse.c -L. -ljsonparse -o json_parse

build/test: parser.o
	gcc -o ./tests/runtests ./tests/tests.c -L. -ljsonparse -lcunit

run/test: build/test
	(cd tests && ./runtests > output.txt)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f json_parse $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/json_parse

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/json_parse

.PHONY: all build build/lib build/program install uninstall
