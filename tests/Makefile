override LDFLAGS+=-rdynamic -lproc -lsatpkt -lz -pthread
override CFLAGS+=-Wall -pedantic -std=gnu99 -g
override CPPFLAGS:=-Wall -std=c++11 -g $(CPPFLAGS)
MORE_FLAGS=-lzmq -lpolydrivers


SRC=verify_scheduled_events.cpp
OBJS=$(SRC:.c=.o)
EXECUTABLE=verify-scheduled-events

all: $(EXECUTABLE) $(CMDS)

verify-scheduled-events: $(OBJS)
	$(CXX) -o $@ $(CPPFLAGS) $(OBJS) $(LDFLAGS) $(MORE_FLAGS)
	#$(CXX) $< -o $@ $(CXXFLAGS) -lzmq

clean:
	rm -rf *.o $(EXECUTABLE)
