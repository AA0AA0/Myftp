CC = gcc
CFLAG = -lpthread

all: server client MTserver MTclient

server: server.c
	$(CC) -o $@ $<

client: client.c
	$(CC) -o $@ $<

MTserver: MTserver.c
	$(CC) -o $@ $< $(CFLAG)

MTclient: MTclient.c
	$(CC) -o $@ $< $(CFLAG)

clean:
	rm server client MTserver MTclient
