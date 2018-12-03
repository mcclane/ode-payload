/**
 * @file paylaod.c Example libproc process
 *
 */

#include <polysat/polysat.h>
#include <polysat_drivers/drivers/gpio.h>
#include <stdio.h>
#include <string.h>
#include "ode-cmds.h"

struct ODEPayloadState {
   ProcessData *proc;
   struct GPIOSensor *led1;
   int led1_active;
   void *led1_blink_evt;
   void *led1_finish_evt;

   void *ball1_evt;
   struct GPIOSensor *deploy_ball1;
};

static struct ODEPayloadState *state = NULL;

// Function called when a status command is sent
void payload_status(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEStatus status;

   // Fill in the values we want to return to the requestor
   status.sw_1 = 1;
   status.sw_2 = 2;
   status.sw_3 = 3;

   // Send the response
   PROC_cmd_sockaddr(state->proc, CMD_STATUS_RESPONSE, &status,
        sizeof(status), src);
}

static int blink_led1_cb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Invert our LED state
   state->led1_active = !state->led1_active;

   // Change the GPIO
   if (state->led1 && state->led1->set)
      state->led1->set(state->led1, state->led1_active);

   // Reschedule the event
   return EVENT_KEEP;
}

static int stop_led1(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off the LED
   if (state->led1 && state->led1->set)
      state->led1->set(state->led1, 0);

   // Remove the blink callback
   if (state->led1_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led1_blink_evt);
      state->led1_blink_evt = NULL;
   }

   // Do not reschedule this event
   state->led1_finish_evt = NULL;
   return EVENT_REMOVE;
}

void blink_led1(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEBlinkData *params = (struct ODEBlinkData*)data;
   uint8_t resp = 0;

   if (dataLen != sizeof(*params))
      return;

   // Clean up from previous events, if any
   if (state->led1_finish_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led1_finish_evt);
      state->led1_finish_evt = NULL;
   }
   if (state->led1_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led1_blink_evt);
      state->led1_blink_evt = NULL;
   }

   // Only drive the LED if the period and duration are > 0
   if (ntohl(params->period) > 0 && ntohl(params->duration) > 0) {
      // Turn the LED on
      state->led1_active = 1;
      if (state->led1 && state->led1->set)
         state->led1->set(state->led1, state->led1_active);

      // Create the blink event
      state->led1_blink_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->period)), &blink_led1_cb, state);

      // Create the event to stop blinking
      state->led1_finish_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->duration)), &stop_led1, state);
   }

   PROC_cmd_sockaddr(state->proc, ODE_BLINK_LED1_RESP, &resp,
        sizeof(resp), src);
}

static int stop_ball1(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off GPIO
   if (state->deploy_ball1 && state->deploy_ball1->set)
      state->deploy_ball1->set(state->deploy_ball1, 0);

   // Zero out our event state
   state->ball1_evt = NULL;

   // Tell the event system to not reschedule this event
   return EVENT_REMOVE;
}

void deploy_ball1(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEDeployData *param = (struct ODEDeployData*)data;
   uint8_t resp = 1;

   if (dataLen != sizeof(*param))
      return;

   // Remove any preexisting ball1 deployment events
   if (state->ball1_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->ball1_evt);
      state->ball1_evt = NULL;
   }

   // Drive the GPIO
   if (state->deploy_ball1 && state->deploy_ball1->set)
      state->deploy_ball1->set(state->deploy_ball1, 1);

   // Register async callback to disable GPIO
   state->ball1_evt = EVT_sched_add(PROC_evt(state->proc),
         EVT_ms2tv(ntohl(param->duration)), &stop_ball1, state);

   PROC_cmd_sockaddr(state->proc, ODE_BURN_BALL1_RESP, &resp,
        sizeof(resp), src);
}

// Simple SIGINT handler for cleanup
static int sigint_handler(int signum, void *arg)
{
   EVT_exit_loop(arg);
   return EVENT_KEEP;
}

int usage(const char *name)
{
   printf("Usage: %s\n"
          ""
          , name);

   return 0;
}

// Entry point
int main(int argc, char *argv[])
{
   struct ODEPayloadState payload;

   memset(&payload, 0, sizeof(payload));
   state = &payload;

   // Initialize the process
   state->proc = PROC_init("payload", WD_ENABLED);
   DBG_setLevel(DBG_LEVEL_ALL);

   // Initialize GPIOs
   state->deploy_ball1 = create_named_gpio_device("DEPLOY_BALL1");
   state->led1 = create_named_gpio_device("LED1");

   // Add a signal handler call back for SIGINT signal
   PROC_signal(state->proc, SIGINT, &sigint_handler, PROC_evt(state->proc));

   // Enter the main event loop
   EVT_start_loop(PROC_evt(state->proc));

   // Clean up, whenever we exit event loop
   DBG_print(DBG_LEVEL_INFO, "Cleaning up\n");

   // Clean up the ball1 deployment event
   if (state->ball1_evt)
      EVT_sched_remove(PROC_evt(state->proc), state->ball1_evt);

   if (state->deploy_ball1) {
      // Turn off the ball1 GPIO if able
      if (state->deploy_ball1->set)
         state->deploy_ball1->set(state->deploy_ball1, 0);
      // Delete the ball1 GPIO sensor
      state->deploy_ball1->sensor.close((struct Sensor **)&state->deploy_ball1);
   }

   if (state->led1) {
      // Turn off the led1 GPIO if able
      if (state->led1->set)
         state->led1->set(state->led1, 0);
      // Delete the led1 GPIO sensor
      state->led1->sensor.close((struct Sensor **)&state->led1);
   }

   PROC_cleanup(state->proc);

   return 0;
}
