#############################################################################
# Estructura del proyecto:
# root/
# 	src/
# 		server.c			Programa principal del servidor
# 	srclib/
# 		daemonize.c			Funciones para demonizar un proceso
#		liblog.c			Funciones para loggear mensajes de debug y error
# 		libsocket.c			Funciones que encapsulan el uso de sockets
# 		picohttpparser.c	Funciones de parseo de mensajes HTTP
#		server_utils.c		Funciones de utilidad para el servidor
#		httplib.c			Tratamiento de peticiones y respuestas HTTP
# 	include/
# 							Directorio con ficheros de cabeceras
# 	lib/
# 							Directorio con ficheros de librerias
# 	obj/
#							Directorio con ficheros objeto generados
#############################################################################

# Constantes
CC = gcc
CFLAGS = -g -std=gnu99 -Iinclude -I .

# Directorios
DOBJ := obj

LINK = -lconfuse
SERVER = $(DOBJ)/server.o $(DOBJ)/picohttpparser.o $(DOBJ)/liblog.o $(DOBJ)/daemonize.o $(DOBJ)/server_utils.o $(DOBJ)/httplib.o
LIBS = lib/libpico.a lib/libsocket.a
LIBPICO = -Llib/ -lpico
LIBSOCK = -Llib/ -lsocket

# Objetivos basicos de compilacion
all: objs server
server:	$(LIBS) $(SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LINK) $(LIBPICO) $(LIBSOCK)
objs:
	mkdir obj
	mkdir lib

# Objetos
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

$(DOBJ)/picohttpparser.o: srclib/picohttpparser.c include/picohttpparser.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/liblog.o: srclib/liblog.c include/liblog.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ)/server_utils.o: srclib/server_utils.c include/server_utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Librerias
lib/libpico.a: $(DOBJ)/picohttpparser.o
	ar -rv $@ $^

lib/libsocket.a: $(DOBJ)/libsocket.o
	ar -rv $@ $^

# Pasar valgrind
val:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server

# Borrar ficheros y directorios generados
clean:
	rm -f server
	rm -f *.o
	rm -R lib
	rm -R obj
	
# Resetear binarios (clean + all)
reset:
	make clean
	make all
	
	