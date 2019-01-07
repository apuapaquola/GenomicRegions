CFLAGS=-O2 -fPIC
CC=gcc

all: libprb.a intersectbed bedcoverage merge_intervals_judy

libprb.a: prb.o
	ar rc $@ $^
	ranlib $@

bedcoverage: bedcoverage.o  prb.o
	${CC} -o $@ $^ -lJudy

intersectbed: intersectbed.o  prb.o
	${CC} -o $@ $^ -lJudy

merge_intervals_judy: merge_intervals_judy.o prb.o
	${CC} -o $@ $^ -lJudy

merge_and_print_intervals.o: merge_intervals.c prb_types.h prb.h
prb.o: prb.c prb_types.h prb.h

clean:
	\rm -f *~ *.o merge_intervals intersectbed libprb.a
