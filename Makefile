buildlib:
	gcc -c parser.c -o parser.o
	ar rcs libjsonparse.a parser.o

program: parser.o
	gcc main.c -L. -ljsonparse -o main
run:
	./main
