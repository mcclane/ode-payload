include Make.rules.arm

override LDFLAGS+=-rdynamic -lproc -lsatpkt -lpolydrivers -lm -lrt -pthread
override CFLAGS+=-Wall -pedantic -std=gnu99 -g -pthread

SRC=ode-payload.c
OBJS=$(SRC:.c=.o)
EXECUTABLE=ode-payload
CMDS=ode-util
INSTALL_DEST=$(BIN_PATH)
CMD_FILE=payload.cmd.cfg

all: $(EXECUTABLE) $(CMDS)

ode-payload: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@ -lz

ode-util: ode-util.c
	$(CC) $< -rdynamic -g -lproc -lsatpkt -lpolydrivers -ldl -lrt -lz -lm -o $@

install: $(EXECUTABLE) $(CMDS)
	cp $(EXECUTABLE) $(INSTALL_DEST)
	cp $(CMDS) $(INSTALL_DEST)
	ln -sf ode-util $(INSTALL_DEST)/ode-status
	ln -sf ode-util $(INSTALL_DEST)/ode-telemetry
	ln -sf ode-util $(INSTALL_DEST)/ode-cree
	ln -sf ode-util $(INSTALL_DEST)/ode-led_505L
	ln -sf ode-util $(INSTALL_DEST)/ode-led_645L 
	ln -sf ode-util $(INSTALL_DEST)/ode-led_851L
	ln -sf ode-util $(INSTALL_DEST)/ode-deploy_small_ball
	ln -sf ode-util $(INSTALL_DEST)/ode-deploy_large_ball
	ln -sf ode-util $(INSTALL_DEST)/ode-deploy_door
	ln -sf ode-util $(INSTALL_DEST)/ode-delayed-deploy-small-ball
	ln -sf ode-util $(INSTALL_DEST)/ode-delayed-deploy-large-ball
	ln -sf ode-util $(INSTALL_DEST)/ode-delayed-deploy-door
	$(STRIP) $(INSTALL_DEST)/$(EXECUTABLE)
	cp $(CMD_FILE) $(ETC_PATH)

.PHONY: clean install

clean:
	rm -rf *.o $(EXECUTABLE) $(CMDS)
