CC = gcc
FLAGS = -Werror -Wextra -Wall

All: build

build: s21_grep

s21_grep: main.o grep.o
	$(CC) $(FLAGS) main.o grep.o -o s21_grep

main.o:
	$(CC) $(FLAGS) -c main.c -o main.o

grep.o:
	$(CC) $(FLAGS) -c s21_grep.c -o grep.o

clean:
	rm -vf *.o 
	rm -vf s21_grep
	rm -vf s21_grep_test

rebuild: clean build

lint:
	clang-format -i *.c *.h

leaks:
	sh tests/test_leak_grep.sh

valgrind:
	touch tests/valgrind_s21_grep.log
	bash tests/test_valgrind_grep.sh


test: rebuild
	sh tests/test_func_grep.sh
	cp ../../materials/linters/.clang-format .
	clang-format -n *.c *.h