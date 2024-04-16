SRC:=$(wildcard *.c)
#OUT:=$(patsubst %.c,%,$(SRC))
all:Client
Server:$(SRC)
        gcc $^ -o $@ -pthread 
.PHONY:rebuild clean
clean:
        rm -f Server
rebuild:clean all