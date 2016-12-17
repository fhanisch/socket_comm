SL = libsocketcomm.so

all: $(SL) install

$(SL): socket.c comm.c socket_comm.h
	gcc -shared -Wall -o $@ socket.c comm.c -lmsd
	
install:
	sudo cp socket_comm.h /usr/local/include
	sudo cp $(SL) /usr/local/lib
	
clean:
	rm $(SL)
