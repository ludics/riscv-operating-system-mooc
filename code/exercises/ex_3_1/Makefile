all:
	gcc -c hello.c -o hello.o
	readelf -h hello.o > hello.o.header
	readelf -SW hello.o > hello.o.section
	gcc -c hello.c -o hello.o -g
	objdump -S hello.o > hello.o.disassembly

clean:
	rm -rf *.o *.header *.section *.disassembly