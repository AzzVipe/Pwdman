CC=gcc
SRCSPATH=src
LIBSPATH=lib
OBJSPATH=obj
CFLAGS=-Iinclude
OBJS=$(OBJSPATH)/pwdman.o $(OBJSPATH)/database.o $(OBJSPATH)/command.o \
	$(OBJSPATH)/pwdman_request.o $(OBJSPATH)/pwdman_response.o
LIBS=$(LIBSPATH)/sock_lib.o $(LIBSPATH)/sqlite3.o $(LIBSPATH)/list.o $(LIBSPATH)/iter.o \
	$(LIBSPATH)/request.o $(LIBSPATH)/str.o $(LIBSPATH)/validator.o $(OBJS)
CFLAGS+=-fPIE
CFLAGS+=-g
CLI_LIBS=$(OBJSPATH)/command.o $(OBJSPATH)/pwdman.o $(OBJSPATH)/pwdman_request.o  \
	$(LIBSPATH)/sock_lib.o $(LIBSPATH)/request.o $(LIBSPATH)/str.o $(LIBSPATH)/validator.o \
	$(OBJSPATH)/pwdman_response.o

all: objects server client

objects: pwdman database pwdman_request pwdman_response command

server_old: server_old.c  
	$(CC) -o server_old server_old.c $(CFLAGS) $(LIBS) -lpthread -lm -ldl

server: server.c  
	$(CC) -o server server.c $(CFLAGS) $(LIBS) -lpthread -lm -ldl

client: client.c  
	$(CC) -o client client.c $(CFLAGS) $(LIBS) -lpthread -lm -ldl

pwdman: $(SRCSPATH)/pwdman.c
	$(CC) -c -o $(OBJSPATH)/pwdman.o $(SRCSPATH)/pwdman.c $(CFLAGS)

command: $(SRCSPATH)/command.c
	$(CC) -c -o $(OBJSPATH)/command.o $(SRCSPATH)/command.c $(CFLAGS)

pwdman_request: $(SRCSPATH)/pwdman_request.c
	$(CC) -c -o $(OBJSPATH)/pwdman_request.o $(SRCSPATH)/pwdman_request.c $(CFLAGS)

pwdman_response: $(SRCSPATH)/pwdman_response.c
	$(CC) -c -o $(OBJSPATH)/pwdman_response.o $(SRCSPATH)/pwdman_response.c $(CFLAGS)


database: $(SRCSPATH)/database.c  
	$(CC) -c -g -o $(OBJSPATH)/database.o $(SRCSPATH)/database.c $(CFLAGS)

clean:
	rm $(OBJSPATH)/*.o
	rm client
	rm server

