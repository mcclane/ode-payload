#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fstream>
#include <polysat3/proclib.h>

#include "zhelpers.hpp"
#include "json.hpp"

using json = nlohmann::json;
class ZMQEvent;

Process *proc = NULL;
ZMQEvent *gEvt = NULL;

json event_states = {
   {"deploy_small_ball_evt", 0},
   {"deploy_large_ball_evt", 0},
   {"door_delay_evt", 0},
   {"cree_blink_evt", 0},
   {"led_505L_blink_evt", 0},
   {"led_645L_blink_evt", 0},
   {"led_851L_blink_evt", 0},
   {"rebooted", false}
};

void save_sim_state()
{
   std::ofstream out("sim_state.json");
   out << std::setw(4) << event_states;
}

void load_sim_state()
{
   if(access("event_states.json", F_OK) != -1)
   {
      std::ifstream in("event_states.json");
      in >> event_states;
      std::cout << event_states << '\n';
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

static int zmq_read_cb(ZMQEvent *evt, char type, void *arg)
{
   zmq::socket_t *client = (zmq::socket_t*)arg;

   std::string msg = s_recv (*client);
   json j = json::parse(msg);
   float current_time = time(NULL);
   std::cout << "Received: " << j << std::endl;

   std::string name;
   float time_remaining;
   time_t deploy_time;
   for(json::iterator it = j["timed_events"].begin(); it != j["timed_events"].end();++it) {
      if(event_states.contains((*it)["name"])) {
         time_remaining = (*it)["time_remaining"];
         name = (*it)["name"];
         deploy_time = current_time + time_remaining;
         std::cout << name << ": " << asctime(gmtime(&deploy_time)) << '\n';
         if(event_states[name] != deploy_time)
         {
            std::cout << "deploy time mismatch " << name << " saved: " << event_states[name] << " actual: " << deploy_time << '\n';
         }
         event_states[name] = deploy_time;
      }
   }
   if (j["dbg_state"] == "stopped") {
      gEvt->AddWriteEvent(&zmq_write_cb, NULL);
   }
   std::cout << event_states << '\n';
   save_sim_state();
   return EVENT_KEEP;
}
static int proc_exit(int sig, void *arg) {
   EventManager *evt = (EventManager*)arg;
   evt->Exit();

   return EVENT_KEEP;
}

void reboot_test()
{
   if(event_states["rebooted"] == false)
   {
      
   }
   else
   {

   }
}

int main(int argc, char **argv)
{
   char buff[1024];

   if (argc < 2) {
      printf("Usage: %s <port number>\n", argv[0]);
      return 0;
   }

   proc = new Process("test2", WD_DISABLED);
   if (!proc)
      return 0;
   proc->AddSignalEvent(SIGINT, &proc_exit, proc->event_manager());

   //  Create context
   zmq::context_t context(1);

   // Create client socket
   zmq::socket_t client (context, ZMQ_PAIR);
   sprintf(buff, "tcp://localhost:%s", argv[1]);

   client.connect(buff);
   ZMQEvent evt(proc->event_manager(), client);
   gEvt = &evt;
   evt.AddReadEvent(zmq_read_cb, &client);

   load_sim_state();

   proc->event_manager()->EventLoop();

   s_send (client, "{\"command\":\"quit\"}");

   client.close();
   context.close();

   delete proc;

   return 0;
}
