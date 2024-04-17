SRC:=$(wildcard *.c)
#OUT:=$(patsubst %.c,%,$(SRC))
all:Server
Server:$(SRC)
	gcc $^ -o $@ -pthread -lmysqlclient
.PHONY:rebuild clean
clean:
	rm -f Server
rebuild:clean all