CC = gcc
CPP = g++
LD = g++
FLAGS = -ggdb -O2
CFLAGS = $(FLAGS)
CPPFLAGS = $(FLAGS)
LDFLAGS =
BIN_NAME = pasm
RM = rm

# -fprofile-arcs -ftest-coverage

########################################################

SRC = $(wildcard *.cpp)

headerfiles = $(wildcard *.h)

OBJS = ${SRC:.cpp=.o}

########################################################

# implicit rules

%.o : %.c $(headerfiles)
	$(CC) -c $(CFLAGS) $< -o $@

%.o : %.cpp $(headerfiles)
	$(CPP) -c $(CPPFLAGS) $< -o $@

########################################################

all: $(OBJS)
	$(LD) -o $(BIN_NAME) $(OBJS) $(LDFLAGS)
	@echo program compiled!
	@echo yes!

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BIN_NAME)

