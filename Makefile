SRC := $(wildcard src/*.cpp src/*/*.cpp src/*/*/*.cpp)

build:
	@g++ -o bin/a.exe $(SRC) -I include -std=c++17 -Wall -Wextra -Werror -pedantic \
	-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-unused-but-set-variable \
	-Wno-unused-value -Wno-unused-local-typedefs -Wno-unused-private-field

run:
	@./bin/a.exe

clear:
	@del bin\a.exe

all: build run
