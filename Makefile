all: multiThreadSorter_thread.c merge.c strhelper.c iohelper.c
	make clean;
	gcc -Ofast -c merge.c;
	gcc -Ofast -c strhelper.c;
	gcc -Ofast -c iohelper.c;
	gcc -Ofast -c multiThreadSorter_thread.c;
	gcc merge.o strhelper.o iohelper.o multiThreadSorter_thread.o -pthread -Ofast -o multiThreadSorter;
	make clean-obj;

clean-obj:
	rm -f merge.o;
	rm -f strhelper.o;
	rm -f iohelper.o
	rm -f multiThreadSorter_thread.o;
clean:
	rm -f multiThreadSorter;
	rm -f merge.o;
	rm -f strhelper.o;
	rm -f iohelper.o;
	rm -f multiThreadSorter_thread.o;
