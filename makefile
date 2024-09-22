CXX = g++
FLAGS = -g -W -Wall -Wextra -Wpedantic -Werror -std=c++11

.PHONY: default run

default: run

run:
	${CXX} ${FLAGS} *.cpp -o program
