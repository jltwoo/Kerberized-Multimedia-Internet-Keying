all: test1 test2 test3 test4

test1: test1.o test_util.o
	gcc -o test1 test1.o test_util.o -lm

test2: test2.o test_util.o
	gcc -o test2 test2.o test_util.o -lm

test3: test3.o test_util.o
	gcc -o test3 test3.o test_util.o -lm

test4: test4.o test_util.o
	gcc -o test4 test4.o test_util.o -lm
test_util.o: test_util.c test_util.h
	gcc -c -o test_util.o test_util.c

test1.o: test1.c test.h
	gcc -c -o test1.o test1.c

test2.o: test2.c test.h
	gcc -c -o test2.o test2.c

test3.o: test3.c test.h
	gcc -c -o test3.o test3.c

test4.o: test4.c test.h
	gcc -c -o test4.o test4.c
clean:
	rm -rf test1 test2 test3 test4 *.o a.out core
