override LDFLAGS+=-rdynamic -lproc3 -lsatpkt -lpolydrivers -lm -lrt -pthread
override CFLAGS+=-Wall -Werror -pedantic -std=gnu99 -g -DLIBPROC3 -pthread

CXXFLAGS += -std=c++11 -g -DLIBPROC3 -ldl -pthread -lproc3 -lz -lrt

dbg-target: dbg_target.c
	$(CC) $< -rdynamic -g -DLIBPROC3 -lproc3 -lsatpkt -lpolydrivers -ldl -lrt -lz -lm -o $@

payload-test: falcon_test.cpp
	$(CXX) $< -o payload-test $(CXXFLAGS) -lzmq

clean:
	rm -f payload-test dbg_target