CC = gcc
FLAGS = -Werror -Wall -Wextra

s21_cat: build

build: main.o cat.o
	$(CC) $(FLAGS) main.o cat.o -o s21_cat

main.o:
	$(CC) $(FLAGS) -c main.c -o main.o

cat.o:
	$(CC) $(FLAGS) -c cat.c -o cat.o

clean:
	rm -vf cat.o main.o

rebuild: clean build

test: rebuild
	sh tests/test_func_cat.sh
	cp ../../materials/linters/.clang-format .
	clang-format -n *.c *.h

leaks:
	sh tests/test_leak_cat.sh

valgrind:
	touch tests/valgrind_s21_grep.log
	bash tests/test_valgrind_cat.sh

lint:
	clang-format -i *.c *.h