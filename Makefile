build:
	mkdir -p ./build

build/lib:
	gcc -c parser.c -o parser.o
	ar rcs libjsonparse.a parser.o

build/program: parser.o
	gcc main.c -L. -ljsonparse -o main

build/test: parser.o
	gcc -o ./tests/runtests ./tests/tests.c -L. -ljsonparse -lcunit

run/test: build/test
	(cd tests && ./runtests > output.txt)
run:
	./main
