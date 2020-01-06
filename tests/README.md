# Falcon Payload Tests

# What does it do/test?
On the first run, the script checks the libproc status packet for the event names scheduled on the command line. It saves the names of these events and the last received libproc status packet to a file. Then, you can do whatever you want to the running process. Kill it, restart the computer, do something weird. Run the script again, and if a saved JSON file is available, and the actual scheduled events (from the last libproc status packet) are compared against the saved scheduled time.

# How do I run it?

## Start the payload process

`LIBPROC_DEBUGGER=ENABLED ode-payload`
Note the port the debugger is running on.

## Run the tests
Usage: `./verify-scheduled-events <ip> <port number> <[event names...]>`

ip is the ip address of the computer running the tests.

Port number is the port the libproc debugger is running on.

Event names are the space separated names of the timed events to check for. These event names can set with the libproc `EVT_sched_set_name` function in the payload process.

See the `falcon_test.sh` script for a complete example of a command
