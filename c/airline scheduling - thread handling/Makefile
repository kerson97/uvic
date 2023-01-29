.phony all:
all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c
	gcc -Wall diskinfo.c -o  diskinfo -g

disklist: disklist.c
	gcc -Wall disklist.c -o  disklist -g

diskget: diskget.c
	gcc -Wall diskget.c -o  diskget -g

diskput: diskput.c
	gcc -Wall diskput.c -o  diskput -g

.PHONY clean:
clean:
	-rm -rf *.o *.exe
