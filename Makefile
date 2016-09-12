LIB = -lpthread
PORT = 5000

all: gestioneSala.o server.o client.o
	$(CC) server.o gestioneSala.o -o server $(LIB)
	$(CC) client.o gestioneSala.o -o client $(LIB)

clean:
	rm -fr *.o *~

delete: clean
	rm -f  client server

runServer: all
	./server $(PORT)

runClient: all
	./client localhost $(PORT)
