# *****************************************************
# Variables to control Makefile operation

CXX = g++
CXXFLAGS = -Wall -g
# the build target executable:
TARGET = main

all: $(TARGET)

# ****************************************************
# Targets needed to bring the executable up to date

main: main.o
	$(CXX) $(CXXFLAGS) -o main main.o

clean: 
	$(RM) count *.o *~