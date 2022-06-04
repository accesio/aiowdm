all: poc-tester

GCC ?= gcc

poc-tester:
	$(GCC) -o poc-tester poc-tester.c
