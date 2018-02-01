# set this to what I want
# OTHER_FLAGS=$(ENV_HPC_OPTS)
OTHER_FLAGS= -O3

# Standard all target
all: hw1

# HW 1
hw1: hw1.o MatrixMultiply.o
	g++ -o hw1 hw1.o MatrixMultiply.o

hw1.o:	hw1.cpp 
	g++ ${OTHER_FLAGS} -Werror -Wall -c hw1.cpp

MatrixMultiply.o : MatrixMultiply.hpp MatrixMultiply.cpp
	g++ ${OTHER_FLAGS} -Werror -Wall -c MatrixMultiply.cpp

clean:
	rm -f *.o hw1


