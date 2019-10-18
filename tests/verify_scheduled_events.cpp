#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fstream>
#include <polysat3/proclib.h>

#include "zhelpers.hpp"
#include "json.hpp"

#define MAX_SCHEDULED_TIME_DIFF 5 // seconds

using json = nlohmann::json;
class ZMQEvent;

Process *proc = NULL;
ZMQEvent *gEvt = NULL;

json state = {
    {"events_to_monitor", NULL}, 
    {"last_status", NULL}
};

void save_state()
{
   std::ofstream out("test_state.json");
   out << std::setw(4) << state;
}

void load_state()
{
   if(access("test_state.json", F_OK) != -1)
   {
      std::ifstream in("test_state.json");
      in >> state;
   }
}

class ZMQEvent {
public:
   typedef int (*ZMQEVT_cb)(ZMQEvent *event, char type, void *arg);

   ZMQEvent(EventManager *e, zmq::socket_t &s)
      : evt(e), sock(s), read_cb(NULL), write_cb(NULL)
   {
      size_t fd_len = sizeof(fd);
      sock.getsockopt(ZMQ_FD, &fd, &fd_len);
   }

   zmq::socket_t &socket() { return sock; }
   EventManager *events() { return evt; }

   void AddReadEvent(ZMQEVT_cb cb, void *arg = NULL) {
      if (!read_cb && !write_cb)
         evt->AddReadEvent(fd, &fd_callback_static, this);
      read_cb = cb;
      read_arg = arg;
      fd_callback();
   }

   void RemoveReadEvent() {
      if (read_cb && !write_cb)
         evt->RemoveReadEvent(fd);
      read_cb = NULL;
   }

   void AddWriteEvent(ZMQEVT_cb cb, void *arg = NULL) {
      if (!write_cb && !read_cb)
         evt->AddReadEvent(fd, &fd_callback_static, this);
      write_cb = cb;
      write_arg = arg;
      fd_callback();
   }

   void RemoveWriteEvent() {
      if (write_cb && !read_cb)
         evt->RemoveReadEvent(fd);
      write_cb = NULL;
   }

private:
   zmq::socket_t &sock;
   EventManager *evt;
   ZMQEVT_cb read_cb, write_cb;
   void *read_arg, *write_arg;
   int fd;

   static int fd_callback_static(int fd, char type, void *arg)
      { return ((ZMQEvent*)arg)->fd_callback(); }
   int fd_callback(void);
};

int ZMQEvent::fd_callback()
{
   int zevents;
   bool remove;
   size_t zevents_len = sizeof(zevents);
   int res = EVENT_KEEP;

   sock.getsockopt(ZMQ_EVENTS, &zevents, &zevents_len);

   while (read_cb && (zevents & ZMQ_POLLIN) ) {
      remove = read_cb(this, EVENT_FD_READ, read_arg) == EVENT_REMOVE;
      if (remove) {
         read_cb = NULL;
         if (!write_cb)
            res = EVENT_REMOVE;
      }

      sock.getsockopt(ZMQ_EVENTS, &zevents, &zevents_len);
   }

   while (write_cb && (zevents & ZMQ_POLLOUT) ) {
      remove = write_cb(this, EVENT_FD_WRITE, write_arg) == EVENT_REMOVE;
      if (remove) {
         write_cb = NULL;
         if (!read_cb)
            res = EVENT_REMOVE;
      }

      sock.getsockopt(ZMQ_EVENTS, &zevents, &zevents_len);
   }

   return res;
}

static int zmq_write_cb(ZMQEvent *evt, char type, void *arg)
{
   static int cnt = 0;
   s_send (evt->socket(), "{\"command\":\"next\"}");

   return EVENT_REMOVE;
}

void make_sure_expected_events_are_present(json j) {
    // Just make sure monitored events are scheduled if we don't have anything to compare
    json timed_events = j["timed_events"];
    json etm = state["events_to_monitor"];
    bool found;
    time_t now = time(NULL);
    time_t scheduled_time;
    for(json::iterator name = etm.begin();name != etm.end();++name) {
        found = false;
//        std::cout << "Looking for: " << *name << '\n';
        for(json::iterator timed_event = timed_events.begin();timed_event != timed_events.end();++timed_event) {
            if((*name).get<std::string>().compare((*timed_event)["name"]) == 0) {
                found = true;
                /*
                scheduled_time = now + (time_t)(*timed_event)["time_remaining"];
                std::cout << (*timed_event)["name"]
                          << ": Scheduled for: " 
                          << strtok(asctime(gmtime(&scheduled_time)), "\n") 
                          << '\n';
                */
                break;

            }
            
        }
        if(!found) {
            std::cout << *name << ": Missing" << '\n';
        }
    }

}
void compare_statuses(json old_status, json current_status) {
    /* Make sure all of the old events are in the current status (or the time has passed) */

    json monitored_events = state["events_to_monitor"];
    json ote = old_status["timed_events"];
    json cte = current_status["timed_events"];

    bool skip;
    bool found;

    time_t now = time(NULL);
    time_t old_current_time = old_status["current_time"];
    time_t new_current_time = current_status["current_time"];
    time_t old_scheduled_time;
    time_t new_scheduled_time;

    // iterate through the old status timed events
    for(json::iterator old = ote.begin();old != ote.end();++old) {
        // Only look at events in the events to monitor arguments
        skip = true;
        for(json::iterator me_name = monitored_events.begin();me_name != monitored_events.end();++me_name) {
            if ((*old)["name"].get<std::string>().compare((*me_name).get<std::string>()) == 0) {
                skip = false;
                break;
            }
        }
        if(skip)
            continue;
        // iterate through the current status timed events
        found = false;
//        std::cout << "Looking for: " << (*old)["name"] << '\n';
        for(json::iterator curr = cte.begin();curr != cte.end();++curr) {
            if((*old)["name"].get<std::string>().compare((*curr)["name"]) == 0) {
                found = true;
                // Make sure the old and new scheduled times are relatively close
                old_scheduled_time = now - (new_current_time - old_current_time) + (time_t)(*old)["time_remaining"];
                new_scheduled_time = now + (time_t)(*curr)["time_remaining"];

                if(abs(old_scheduled_time - new_scheduled_time) >= MAX_SCHEDULED_TIME_DIFF) {
                    std::cout << (*curr)["name"] 
                              << ": Old: "
                              << strtok(asctime(gmtime(&old_scheduled_time)), "\n") 
                              << " New: "
                              << strtok(asctime(gmtime(&new_scheduled_time)), "\n") 
                              << '\n';
                }
                else {
                    /*
                    std::cout << (*curr)["name"] 
                              << ": Scheduled around the same time"
                              << '\n';
                              */
                }
                break;
            }
        }
        if (!found) {
            // TODO: Has the event already elapsed?
            old_scheduled_time = now - (new_current_time - old_current_time) + (time_t)(*old)["time_remaining"];
            if(old_scheduled_time < now) {
                std::cout << (*old)["name"] 
                          << ": already happened"
                          << '\n';
            }
            else {
                // Otherwise, it's missing
                std::cout << (*old)["name"]
                          << ": Missing"
                          << '\n';
            }
        }
    }
}

static int zmq_read_cb(ZMQEvent *evt, char type, void *arg)
{
    zmq::socket_t *client = (zmq::socket_t*)arg;

    std::string msg = s_recv (*client);
    json j = json::parse(msg);
    /*
    std::cout << "Received: " << j << std::endl;
    */

    make_sure_expected_events_are_present(j);
    /*
    if(state["last_status"] == NULL) {
        make_sure_expected_events_are_present(j);
    }
    */
    if(state["last_status"] != NULL) {
        compare_statuses(state["last_status"], j);
    }

    state["last_status"] = j;
    save_state();
    proc->event_manager()->Exit();
    return 0;

    /*
    if (j["dbg_state"] == "stopped") {
        gEvt->AddWriteEvent(&zmq_write_cb, NULL);
    }
    return EVENT_KEEP;
    */
}
static int proc_exit(int sig, void *arg) {
   EventManager *evt = (EventManager*)arg;
   evt->Exit();

   return EVENT_KEEP;
}

int main(int argc, char **argv)
{
    int i;
    char buff[1024];

    if (argc < 3) {
        printf("Usage: %s <port number> <[event names...]>\n", argv[0]);
        return 0;
    }

    std::string s;
    s += "{\"names\": [";
    for(i = 2;i < argc;++i) {
        s += "\"";
        s += argv[i];
        s += "\"";
        if(i < argc - 1) {
            s += ",";
        }
    }
    s += "]}";

   load_state();

   state["events_to_monitor"] = json::parse(s)["names"];
   /*
   std::cout << state << '\n';
   */

   proc = new Process("test2", WD_DISABLED);
   if (!proc)
      return 0;
   proc->AddSignalEvent(SIGINT, &proc_exit, proc->event_manager());

   // Create context
   zmq::context_t context(1);

   // Create client socket
   zmq::socket_t client (context, ZMQ_PAIR);
   sprintf(buff, "tcp://localhost:%s", argv[1]);

   client.connect(buff);
   ZMQEvent evt(proc->event_manager(), client);
   gEvt = &evt;
   evt.AddReadEvent(zmq_read_cb, &client);


   std::cout << '\n';
   std::cout << "Checking for events: " << state["events_to_monitor"] << '\n';

   proc->event_manager()->EventLoop();
   std::cout << "Done." << "\n\n";

   s_send (client, "{\"command\":\"quit\"}");

   client.close();
   context.close();

   delete proc;

   return 0;
}
