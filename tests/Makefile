override LDFLAGS+=-rdynamic -lproc -lsatpkt -lpolydrivers -lm -lrt -pthread
override CFLAGS+=-Wall -Werror -pedantic -std=gnu99 -g -pthread

CXXFLAGS += -std=c++11 -g -ldl -pthread -lproc -lz -lrt


SRC=verify_scheduled_events.cpp
OBJS=$(SRC:.c=.o)
EXECUTABLE=verify-scheduled-events

all: $(EXECUTABLE) $(CMDS)

verify-scheduled-events: $(OBJS)
	$(CXX) $< -o $@ $(CXXFLAGS) -lzmq

clean:
	rm -rf *.o $(EXECUTABLE)
