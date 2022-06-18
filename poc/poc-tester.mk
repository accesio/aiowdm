all: poc-tester

GCC ?= gcc

poc-tester: poc-tester.c
	$(GCC) -o poc-tester poc-tester.c
