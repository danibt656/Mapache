CC = gcc
CFLAGS = -g -std=gnu99 -pthread -Iinclude -I .

SERVER_EXE := mapacheServer
DOBJ := obj

LINK = 
SERVER = $(DOBJ)/server.o $(DOBJ)/map_parser.o $(DOBJ)/liblog.o $(DOBJ)/daemonize.o $(DOBJ)/server_utils.o $(DOBJ)/httplib.o $(DOBJ)/mime.o $(DOBJ)/io.o $(DOBJ)/dir.o $(DOBJ)/cgi.o $(DOBJ)/queue.o $(DOBJ)/cfgparser.o
LIBS = lib/libpico.a lib/libsocket.a
LIBPICO = -Llib/ -lpico
LIBSOCK = -Llib/ -lsocket

# Basic build objectives
all: objs $(SERVER_EXE)
$(SERVER_EXE): $(LIBS) $(SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LINK) $(LIBPICO) $(LIBSOCK)
objs:
	mkdir obj
	mkdir lib

# Object files
$(DOBJ)/server.o: src/server.c include/daemonize.h include/liblog.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LINK)

$(DOBJ)/libsocket.o: srclib/libsocket.c include/libsocket.h include/liblog.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/httplib.o: srclib/httplib.c include/liblog.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/daemonize.o: srclib/daemonize.c include/daemonize.h include/liblog.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LINK)

$(DOBJ)/libsocket.o: srclib/libsocket.c include/libsocket.h include/liblog.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/map_parser.o: srclib/map_parser.c include/map_parser.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/liblog.o: srclib/liblog.c include/liblog.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/server_utils.o: srclib/server_utils.c include/server_utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/mime.o: srclib/mime.c include/mime.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/io.o: srclib/io.c include/io.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/dir.o: srclib/dir.c include/dir.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/cgi.o: srclib/cgi.c include/cgi.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/queue.o: srclib/queue.c include/queue.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/cfgparser.o: srclib/cfgparser.c include/cfgparser.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Libraries
lib/libpico.a: $(DOBJ)/map_parser.o
	ar -rv $@ $^

lib/libsocket.a: $(DOBJ)/libsocket.o
	ar -rv $@ $^

# Valgrind
val:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(SERVER_EXE)

# Clean binaries & build directories
clean:
	rm -f $(SERVER_EXE)
	rm -f *.o
	rm -R lib
	rm -R obj
	
# Reset build
reset:
	make clean
	make all

run:
	make reset
	./$(SERVER_EXE)
