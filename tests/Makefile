override LDFLAGS+=-rdynamic -lproc -lsatpkt -lpolydrivers -lm -lrt -pthread
override CFLAGS+=-Wall -Werror -pedantic -std=gnu99 -g -pthread

CXXFLAGS += -std=c++11 -g -ldl -pthread -lproc -lz -lrt

SRC=falcon_test.cpp
OBJS=$(SRC:.c=.o)
EXECUTABLE=payload-test

all: $(EXECUTABLE) $(CMDS)

payload-test: $(OBJS)
	$(CXX) $< -o $@ $(CXXFLAGS) -lzmq

dbg-target: dbg_target.c
	$(CC) $< -rdynamic -g -lproc -lsatpkt -lpolydrivers -ldl -lrt -lz -lm -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
