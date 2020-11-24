INCLUDE=./include
SRC=./src
OBJ=./obj
BIN=./bin

CC=g++
CFLAGS=-I$(INCLUDE)

mqtt-discovery: $(BIN)/mqtt-discovery

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) -c -o $@ $(CFLAGS) $<

$(BIN)/mqtt-discovery: $(OBJ)/mqtt-discovery.o
	$(CC) -o $@ $(CFLAGS) $^

clean:
	rm -f $(OBJ)/*.o $(BIN)/* core

