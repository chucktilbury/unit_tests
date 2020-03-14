#	This is a demonstration of how to configure a makefile for use with the unit
#	tests. The tests are stand-alone programs that a made and run using the
#	makefile. The intermediate files, such as the executables are deleted after
#	the test is run.

TESTDIR	=	./tests/
TARGETS	=	fifo_tests_using_malloc \
			fifo_tests_mocking_malloc

CARGS	=	-Wall -Wextra -I src -I tests -g
CC		=	gcc

all: $(TARGETS)

fifo_tests_using_malloc: $(TESTDIR)fifo_tests_using_malloc.c
	$(CC) $(CARGS) $< -o $@ && echo "running test: $@" && ./$@

fifo_tests_mocking_malloc: $(TESTDIR)fifo_tests_mocking_malloc.c
	$(CC) $(CARGS) $< -o $@ && echo "running test: $@" && ./$@

clean:
	-rm -f $(TARGETS)
