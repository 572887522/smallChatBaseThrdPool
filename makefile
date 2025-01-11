CC = g++
CFLAG = -Wall -g -lpthread

OBJ = server.o client.o

TARGET = server client

all : $(TARGET)

server : server.o
	$(CC) $< -o $@ $(CFLAG)

client : client.o
	$(CC) $< -o $@ $(CFLAG)

%.o : %.cc
	$(CC) -c $< -o $@ $(CFLAG)

clean:
	rm -f $(OBJ) $(TARGET)