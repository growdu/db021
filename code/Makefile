
ifeq ($(OS),Windows_NT)
 PLATFORM="Windows"
else
 ifeq ($(shell uname),Darwin)
  PLATFORM="MacOS"
 else
  PLATFORM="Unix-Like"
 endif
endif
SRC=$(wildcard ./src/*.c)
all:
	gcc -g -O0 $(SRC) -o db
clean:
ifeq ($(OS),Windows_NT)
	rm db.exe
else
	rm db
endif