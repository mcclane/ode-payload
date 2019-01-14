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
	
	struct GPIOSensor *enable_5V;
	int enable_5V_active;
	void *enable_5V_evt;
	void *enable_5V_finish_evt;
	
	struct GPIOSensor *cree;
	int cree_active;
	void *cree_blink_evt;
	void *cree_finish_evt;

	struct GPIOSensor *led_505L;
	int led_505L_active;
	void *led_505L_blink_evt;
	void *led_505L_finish_evt;
	
	struct GPIOSensor *led_645L;
	int led_645L_active;
	void *led_645L_blink_evt;
	void *led_645L_finish_evt;
	
	struct GPIOSensor *led_851L;
	int led_851L_active;
	void *led_851L_blink_evt;
	void *led_851L_finish_evt;
	
	struct GPIOSensor *led_IR;
	int led_IR_active;
	void *led_IR_blink_evt;
	void *led_IR_finish_evt;
	
	void *small_ball_evt;
	struct GPIOSensor *deploy_small_ball;
	
	void *large_ball_evt;
	struct GPIOSensor *deploy_large_ball;
	
	void *door_evt;
	struct GPIOSensor *deploy_door;
	
	struct GPIOSensor *Door_Feedback;
	int Door_Feedback_value;
	void *Door_Feedback_evt;
	void *Door_Feedback_finish;
	
	struct GPIOSensor *Small_Ball_Feedback;
	int Small_Ball_Feedback_value;
	void *Small_Ball_Feedback_evt;
	void *Small_Ball_Feedback_finish;
	
	struct GPIOSensor *Large_Ball_Feedback;
	int Large_Ball_Feedback_value;
	void *Large_Ball_Feedback_evt;
	void *Large_Ball_Feedback_finish;
};

static struct ODEPayloadState *state = NULL;
//static struct ODEStatus *sc_status = NULL;
static char codes_for_status[12]={0};

// Function called when a status command is sent
void payload_status(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEStatus status;
   
	status.small_ball_sw=codes_for_status[0];
	status.large_ball_sw=codes_for_status[1];
	status.MW_sw=codes_for_status[2];
	status.small_ball_fb=codes_for_status[3];
	status.large_ball_fb=codes_for_status[4];
	status.MW_fb=codes_for_status[5];
	status.cree_led=codes_for_status[6];
	status.led_505L=codes_for_status[7];
	status.led_645L=codes_for_status[8];
	status.led_851L=codes_for_status[9];
	status.led_IR=codes_for_status[10];
	status.enable_5V=codes_for_status[11];

   // Send the response
   PROC_cmd_sockaddr(state->proc, CMD_STATUS_RESPONSE, &status,
        sizeof(status), src);
}

//__________________________________________________________________
//Blink LED call back functions
static int enable_5V(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn on the 5V regualtor
   state->enable_5V_active = 1;

   // Change the GPIO
   if (state->enable_5V && state->enable_5V->set)
      state->enable_5V->set(state->enable_5V, state->enable_5V_active);
  
   codes_for_status[11]=1;
	 
   // Reschedule the event
   return EVENT_KEEP;
}

static int blink_cree_cb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;
//   struct ODEStatus *sc_status = (struct ODEStatus*)arg;

   // Invert our LED state
   state->cree_active = !state->cree_active;

   // Change the GPIO
   if (state->cree && state->cree->set)
      state->cree->set(state->cree, state->cree_active);
  
   codes_for_status[6]=1;
	 
   // Reschedule the event
   return EVENT_KEEP;
}

static int blink_led_505L_cb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Invert our LED state
   state->led_505L_active = !state->led_505L_active;

   // Change the GPIO
   if (state->led_505L && state->led_505L->set)
      state->led_505L->set(state->led_505L, state->led_505L_active);
  
   codes_for_status[7]=1;

   // Reschedule the event
   return EVENT_KEEP;
}

static int blink_led_645L_cb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Invert our LED state
   state->led_645L_active = !state->led_645L_active;

   // Change the GPIO
   if (state->led_645L && state->led_645L->set)
      state->led_645L->set(state->led_645L, state->led_645L_active);
  
   codes_for_status[8]=1;

   // Reschedule the event
   return EVENT_KEEP;
}

static int blink_led_851L_cb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Invert our LED state
   state->led_851L_active = !state->led_851L_active;

   // Change the GPIO
   if (state->led_851L && state->led_851L->set)
      state->led_851L->set(state->led_851L, state->led_851L_active);
  
   codes_for_status[9]=1;

   // Reschedule the event
   return EVENT_KEEP;
}

static int blink_led_IR_cb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Invert our LED state
   state->led_IR_active = !state->led_IR_active;

   // Change the GPIO
   if (state->led_IR && state->led_IR->set)
      state->led_IR->set(state->led_IR, state->led_IR_active);
  
   codes_for_status[10]=1;

   // Reschedule the event
   return EVENT_KEEP;
}

//__________________________________________________________________
//Stop LED call back functions
static int disable_5V(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off the 5V regualtor
   state->enable_5V_active = 0;

   // Change the GPIO
   if (state->enable_5V && state->enable_5V->set)
      state->enable_5V->set(state->enable_5V, state->enable_5V_active);
  
   codes_for_status[11]=0;
	 
   // Reschedule the event
   return EVENT_KEEP;
}

static int stop_cree(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;
 //  struct ODEStatus *sc_status = (struct ODEStatus*)arg;

   // Turn off the LED
   if (state->cree && state->cree->set)
      state->cree->set(state->cree, 0);

   // Remove the blink callback
   if (state->cree_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->cree_blink_evt);
      state->cree_blink_evt = NULL;
   }

   codes_for_status[6]=0;

   // Do not reschedule this event
   state->cree_finish_evt = NULL;
   return EVENT_REMOVE;
}

static int stop_led_505L(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off the LED
   if (state->led_505L && state->led_505L->set)
      state->led_505L->set(state->led_505L, 0);

   // Remove the blink callback
   if (state->led_505L_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_505L_blink_evt);
      state->led_505L_blink_evt = NULL;
   }
  
   disable_5V(0);
   codes_for_status[7]=0;

   // Do not reschedule this event
   state->led_505L_finish_evt = NULL;
   return EVENT_REMOVE;
}

static int stop_led_645L(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off the LED
   if (state->led_645L && state->led_645L->set)
      state->led_645L->set(state->led_645L, 0);

   // Remove the blink callback
   if (state->led_645L_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_645L_blink_evt);
      state->led_645L_blink_evt = NULL;
   }
  
   disable_5V(0);
   codes_for_status[8]=0;

   // Do not reschedule this event
   state->led_645L_finish_evt = NULL;
   return EVENT_REMOVE;
}

static int stop_led_851L(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off the LED
   if (state->led_851L && state->led_851L->set)
      state->led_851L->set(state->led_851L, 0);

   // Remove the blink callback
   if (state->led_851L_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_851L_blink_evt);
      state->led_851L_blink_evt = NULL;
   }
  
   disable_5V(0);  
   codes_for_status[9]=0;

   // Do not reschedule this event
   state->led_851L_finish_evt = NULL;
   return EVENT_REMOVE;
}

static int stop_led_IR(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off the LED
   if (state->led_IR && state->led_IR->set)
      state->led_IR->set(state->led_IR, 0);

   // Remove the blink callback
   if (state->led_IR_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_IR_blink_evt);
      state->led_IR_blink_evt = NULL;
   }
  
   codes_for_status[10]=0;

   // Do not reschedule this event
   state->led_IR_finish_evt = NULL;
   return EVENT_REMOVE;
}

//__________________________________________________________________
//Delay Blink LED functions
int delay_blink_cree(uint32_t period;)
{
   struct ODEBlinkData *params = (struct ODEBlinkData*)data;

   // Clean up from previous events, if any
   if (state->cree_finish_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->cree_finish_evt);
      state->cree_finish_evt = NULL;
   }
   if (state->cree_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->cree_blink_evt);
      state->cree_blink_evt = NULL;
   }
 
   state->cree_active = 1;
   if (state->cree && state->cree->set)
      state->cree->set(state->cree, state->cree_active);

   // Create the blink event
   state->cree_blink_evt = EVT_sched_add(PROC_evt(state->proc),
      EVT_ms2tv(ntohl(params->period)), &blink_cree_cb, state);	   	   
	
  // Do not reschedule this event
   return EVENT_REMOVE;
}
//__________________________________________________________________
//Blink LED functions

void blink_cree(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEBlinkData *params = (struct ODEBlinkData*)data;
   uint8_t resp = 0;

   if (dataLen != sizeof(*params))
      return;

   // Clean up from previous events, if any
   if (state->cree_finish_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->cree_finish_evt);
      state->cree_finish_evt = NULL;
   }
   if (state->cree_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->cree_blink_evt);
      state->cree_blink_evt = NULL;
   }

   // Only drive the LED if the period and duration are > 0
   if (ntohl(params->period) > 0 && ntohl(params->duration) > 0) {
      // Turn the LED on
      state->cree_active = 1;
      if (state->cree && state->cree->set)
         state->cree->set(state->cree, state->cree_active);

      // Create the blink event
//      state->cree_blink_evt = EVT_sched_add(PROC_evt(state->proc),
//            EVT_ms2tv(ntohl(params->period)), &blink_cree_cb, state);	   
     state->cree_blink_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->delay)), &delay_blink_cree(params->period), state);	   

      // Create the event to stop blinking
      state->cree_finish_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->duration+params->delay)), &stop_cree, state);
   }

   PROC_cmd_sockaddr(state->proc, ODE_BLINK_CREE_RESP, &resp,
        sizeof(resp), src);
}

void blink_led_505L(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEBlinkData *params = (struct ODEBlinkData*)data;
   uint8_t resp = 0;

   if (dataLen != sizeof(*params))
      return;

   // Clean up from previous events, if any
   if (state->led_505L_finish_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_505L_finish_evt);
      state->led_505L_finish_evt = NULL;
   }
   if (state->led_505L_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_505L_blink_evt);
      state->led_505L_blink_evt = NULL;
   }

   // Only drive the LED if the period and duration are > 0
   if (ntohl(params->period) > 0 && ntohl(params->duration) > 0) {
	
      enable_5V(0);   
	   
      // Turn the LED on
      state->led_505L_active = 1;
      if (state->led_505L && state->led_505L->set)
         state->led_505L->set(state->led_505L, state->led_505L_active);

      // Create the blink event
      state->led_505L_blink_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->period)), &blink_led_505L_cb, state);	   
					   
      // Create the event to stop blinking
      state->led_505L_finish_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->duration)), &stop_led_505L, state);
   }

   PROC_cmd_sockaddr(state->proc, ODE_BLINK_LED_505L_RESP, &resp,
        sizeof(resp), src);
}

void blink_led_645L(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEBlinkData *params = (struct ODEBlinkData*)data;
   uint8_t resp = 0;

   if (dataLen != sizeof(*params))
      return;

   // Clean up from previous events, if any
   if (state->led_645L_finish_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_645L_finish_evt);
      state->led_645L_finish_evt = NULL;
   }
   if (state->led_645L_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_645L_blink_evt);
      state->led_645L_blink_evt = NULL;
   }

   // Only drive the LED if the period and duration are > 0
   if (ntohl(params->period) > 0 && ntohl(params->duration) > 0) {
	
      enable_5V(0);   
	   
      // Turn the LED on
      state->led_645L_active = 1;
      if (state->led_645L && state->led_645L->set)
         state->led_645L->set(state->led_645L, state->led_645L_active);

      // Create the blink event
      state->led_645L_blink_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->period)), &blink_led_645L_cb, state);
					       
      // Create the event to stop blinking
      state->led_645L_finish_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->duration)), &stop_led_645L, state);
   }

   PROC_cmd_sockaddr(state->proc, ODE_BLINK_LED_645L_RESP, &resp,
        sizeof(resp), src);
}

void blink_led_851L(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEBlinkData *params = (struct ODEBlinkData*)data;
   uint8_t resp = 0;

   if (dataLen != sizeof(*params))
      return;

   // Clean up from previous events, if any
   if (state->led_851L_finish_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_851L_finish_evt);
      state->led_851L_finish_evt = NULL;
   }
   if (state->led_851L_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_851L_blink_evt);
      state->led_851L_blink_evt = NULL;
   }

   // Only drive the LED if the period and duration are > 0
   if (ntohl(params->period) > 0 && ntohl(params->duration) > 0) {
	
      enable_5V(0);   
	   
      // Turn the LED on
      state->led_851L_active = 1;
      if (state->led_851L && state->led_851L->set)
         state->led_851L->set(state->led_851L, state->led_851L_active);

      // Create the blink event
      state->led_851L_blink_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->period)), &blink_led_851L_cb, state);
					       
      // Create the event to stop blinking
      state->led_851L_finish_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->duration)), &stop_led_851L, state);
   }

   PROC_cmd_sockaddr(state->proc, ODE_BLINK_LED_851L_RESP, &resp,
        sizeof(resp), src);
}

void blink_led_IR(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEBlinkData *params = (struct ODEBlinkData*)data;
   uint8_t resp = 0;

   if (dataLen != sizeof(*params))
      return;

   // Clean up from previous events, if any
   if (state->led_IR_finish_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_IR_finish_evt);
      state->led_IR_finish_evt = NULL;
   }
   if (state->led_IR_blink_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->led_IR_blink_evt);
      state->led_IR_blink_evt = NULL;
   }

   // Only drive the LED if the period and duration are > 0
   if (ntohl(params->fb_led_period) > 0 && ntohl(params->duration) > 0) {
      // Turn the LED on
      state->led_IR_active = 1;
      if (state->led_IR && state->led_IR->set)
         state->led_IR->set(state->led_IR, state->led_IR_active);

      // Create the blink event
      state->led_IR_blink_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->period)), &blink_led_IR_cb, state);

      // Create the event to stop blinking
      state->led_IR_finish_evt = EVT_sched_add(PROC_evt(state->proc),
            EVT_ms2tv(ntohl(params->duration)), &stop_led_IR, state);
   }

   PROC_cmd_sockaddr(state->proc, ODE_BLINK_IR_LED_RESP, &resp,
        sizeof(resp), src);
}

//__________________________________________________________________
//Deployment functions
//__________________________________________________________________
//Stop deployment LEDs

static int stop_small_ball(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off GPIO
   if (state->deploy_small_ball && state->deploy_small_ball->set)
      state->deploy_small_ball->set(state->deploy_small_ball, 0);

   // Zero out our event state
   state->small_ball_evt = NULL;
  
   codes_for_status[0]=0;

   // Tell the event system to not reschedule this event
   return EVENT_REMOVE;
}

static int stop_large_ball(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off GPIO
   if (state->deploy_large_ball && state->deploy_large_ball->set)
      state->deploy_large_ball->set(state->deploy_large_ball, 0);

   // Zero out our event state
   state->large_ball_evt = NULL;
  
   codes_for_status[1]=0;

   // Tell the event system to not reschedule this event
   return EVENT_REMOVE;
}

static int stop_door(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;

   // Turn off GPIO
   if (state->deploy_door && state->deploy_door->set)
      state->deploy_door->set(state->deploy_door, 0);

   // Zero out our event state
   state->door_evt = NULL;
  
   codes_for_status[2]=0;   

   // Tell the event system to not reschedule this event
   return EVENT_REMOVE;
}

//__________________________________________________________________
//deployment functions

void deploy_small_ball(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEDeployData *param = (struct ODEDeployData*)data;
   uint8_t resp = 1;

   if (dataLen != sizeof(*param))
      return;

   // Remove any preexisting small_ball deployment events
   if (state->small_ball_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->small_ball_evt);
      state->small_ball_evt = NULL;
   }

   // Drive the GPIO
   if (state->deploy_small_ball && state->deploy_small_ball->set){
	   
      state->deploy_small_ball->set(state->deploy_small_ball, 1);  
	  codes_for_status[0]=1;
   }

   // Register async callback to disable GPIO
   state->small_ball_evt = EVT_sched_add(PROC_evt(state->proc),
         EVT_ms2tv(ntohl(param->duration)), &stop_small_ball, state);

   PROC_cmd_sockaddr(state->proc, ODE_DEPLOY_SMALL_BALL_RESP, &resp,
        sizeof(resp), src);
}

void deploy_large_ball(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEDeployData *param = (struct ODEDeployData*)data;
   uint8_t resp = 1;

   if (dataLen != sizeof(*param))
      return;

   // Remove any preexisting large_ball deployment events
   if (state->large_ball_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->large_ball_evt);
      state->large_ball_evt = NULL;
   }

   // Drive the GPIO
   if (state->deploy_large_ball && state->deploy_large_ball->set){
      state->deploy_large_ball->set(state->deploy_large_ball, 1);  
      codes_for_status[1]=1;
   }

   // Register async callback to disable GPIO
   state->large_ball_evt = EVT_sched_add(PROC_evt(state->proc),
         EVT_ms2tv(ntohl(param->duration)), &stop_large_ball, state);

   PROC_cmd_sockaddr(state->proc, ODE_DEPLOY_LARGE_BALL_RESP, &resp,
        sizeof(resp), src);
}

void deploy_door(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
   struct ODEDeployData *param = (struct ODEDeployData*)data;
   uint8_t resp = 1;

   if (dataLen != sizeof(*param))
      return;

   // Remove any preexisting door deployment events
   if (state->door_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->door_evt);
      state->door_evt = NULL;
   }

   // Drive the GPIO
   if (state->deploy_door && state->deploy_door->set){
      state->deploy_door->set(state->deploy_door, 1);
	  codes_for_status[2]=1;
   }  

   // Register async callback to disable GPIO
   state->door_evt = EVT_sched_add(PROC_evt(state->proc),
         EVT_ms2tv(ntohl(param->duration)), &stop_door, state);

   PROC_cmd_sockaddr(state->proc, ODE_DEPLOY_DOOR_RESP, &resp,
        sizeof(resp), src);
}

//__________________________________________________________________
//Feedback functions
//__________________________________________________________________
//Feedback start functions

static int start_small_ball_fb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;
   codes_for_status[3] = -1;
	
   // Read the GPIO
   codes_for_status[3] = state->Small_Ball_Feedback->read(state->Small_Ball_Feedback);
   
   // Do not reschedule this event
   state->Small_Ball_Feedback_finish = NULL;
   return EVENT_REMOVE;
}

static int start_large_ball_fb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;
   codes_for_status[4] = -1;
	
   // Read the GPIO
   codes_for_status[4] = state->Large_Ball_Feedback->read(state->Large_Ball_Feedback);
   
   // Do not reschedule this event
   state->Large_Ball_Feedback_finish = NULL;
   return EVENT_REMOVE;
}

static int start_mw_fb(void *arg)
{
   struct ODEPayloadState *state = (struct ODEPayloadState*)arg;
   codes_for_status[5] = -1;
	
   // Read the GPIO
   codes_for_status[5] = state->Door_Feedback->read(state->Door_Feedback);
   
   // Do not reschedule this event
   state->Door_Feedback_finish = NULL;
   return EVENT_REMOVE;
}

//__________________________________________________________________
//Feedback commands

void small_ball_status(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
  struct ODEBlinkData *params = (struct ODEBlinkData*)data;
  uint8_t resp = 0;

   // Clean up from previous events, if any
   if (state->Small_Ball_Feedback_finish) {
      EVT_sched_remove(PROC_evt(state->proc), state->Small_Ball_Feedback_finish);
      state->Small_Ball_Feedback_finish = NULL;
   }
   if (state->Small_Ball_Feedback_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->Small_Ball_Feedback_evt);
      state->Small_Ball_Feedback_evt = NULL;
   }
	
   // Create the event to check the Small_Ball
//   codes_for_status[5] = state->Small_Ball_Feedback->read(state->Small_Ball_Feedback);
   state->Small_Ball_Feedback_evt = EVT_sched_add(PROC_evt(state->proc),
      EVT_ms2tv(ntohl(params->period)), &start_mw_fb, state);
	
   PROC_cmd_sockaddr(state->proc, ODE_SMALL_BALL_STATUS_RESP , &resp,
        sizeof(resp), src);
}

void large_ball_status(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
  struct ODEBlinkData *params = (struct ODEBlinkData*)data;
  uint8_t resp = 0;

   // Clean up from previous events, if any
   if (state->Large_Ball_Feedback_finish) {
      EVT_sched_remove(PROC_evt(state->proc), state->Large_Ball_Feedback_finish);
      state->Large_Ball_Feedback_finish = NULL;
   }
   if (state->Large_Ball_Feedback_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->Large_Ball_Feedback_evt);
      state->Large_Ball_Feedback_evt = NULL;
   }
	
   // Create the event to check the Large_Ball
//   codes_for_status[5] = state->Large_Ball_Feedback->read(state->Large_Ball_Feedback);
   state->Large_Ball_Feedback_evt = EVT_sched_add(PROC_evt(state->proc),
      EVT_ms2tv(ntohl(params->period)), &start_small_ball_fb, state);
	
   PROC_cmd_sockaddr(state->proc, ODE_LARGE_BALL_STATUS_RESP , &resp,
        sizeof(resp), src);
}

void mw_status(int socket, unsigned char cmd, void * data, size_t dataLen,
                     struct sockaddr_in * src)
{
  struct ODEBlinkData *params = (struct ODEBlinkData*)data;
  uint8_t resp = 0;

   // Clean up from previous events, if any
   if (state->Door_Feedback_finish) {
      EVT_sched_remove(PROC_evt(state->proc), state->Door_Feedback_finish);
      state->Door_Feedback_finish = NULL;
   }
   if (state->Door_Feedback_evt) {
      EVT_sched_remove(PROC_evt(state->proc), state->Door_Feedback_evt);
      state->Door_Feedback_evt = NULL;
   }
	
   // Create the event to check the door
//   codes_for_status[5] = state->Door_Feedback->read(state->Door_Feedback);
   state->Door_Feedback_evt = EVT_sched_add(PROC_evt(state->proc),
      EVT_ms2tv(ntohl(params->period)), &start_large_ball_fb, state);
	
   PROC_cmd_sockaddr(state->proc, ODE_MW_STATUS_RESP , &resp,
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
   state->enable_5V = create_named_gpio_device("ENABLE_5V");
   state->cree = create_named_gpio_device("CREE");
   state->led_505L = create_named_gpio_device("LED_505L");
   state->led_645L = create_named_gpio_device("LED_645L");
   state->led_851L = create_named_gpio_device("LED_851L");
   state->led_IR = create_named_gpio_device("LED_IR");
   state->deploy_small_ball = create_named_gpio_device("DEPLOY_SMALL_BALL");
   state->deploy_large_ball = create_named_gpio_device("DEPLOY_LARGE_BALL");
   state->deploy_door = create_named_gpio_device("DEPLOY_DOOR");
   state->Small_Ball_Feedback = create_named_gpio_device("SM_BALL_FB");
   state->Large_Ball_Feedback = create_named_gpio_device("LG_BALL_FB");
   state->Door_Feedback = create_named_gpio_device("DOOR_FEEDBACK");

   // Add a signal handler call back for SIGINT signal
   PROC_signal(state->proc, SIGINT, &sigint_handler, PROC_evt(state->proc));

   // Enter the main event loop
   EVT_start_loop(PROC_evt(state->proc));

   // Clean up, whenever we exit event loop
   DBG_print(DBG_LEVEL_INFO, "Cleaning up\n");
	
   // Clean up the input pins
   if (state->small_ball_evt)
      EVT_sched_remove(PROC_evt(state->proc), state->small_ball_evt);
  
   if (state->deploy_small_ball) {
      // Turn off the ball1 GPIO if able
      if (state->deploy_small_ball->set)
         state->deploy_small_ball->set(state->deploy_small_ball, 0);
      // Delete the ball1 GPIO sensor
      state->deploy_small_ball->sensor.close((struct Sensor **)&state->deploy_small_ball);
   }   	

   if (state->Small_Ball_Feedback_evt)
      EVT_sched_remove(PROC_evt(state->proc), state->Small_Ball_Feedback_evt);
  
   if (state->Small_Ball_Feedback) {
      // Turn off the ball1 GPIO if able
      if (state->Small_Ball_Feedback->set)
         state->Small_Ball_Feedback->set(state->Small_Ball_Feedback, 0);
      // Delete the ball1 GPIO sensor
      state->Small_Ball_Feedback->sensor.close((struct Sensor **)&state->Small_Ball_Feedback);
   }   
   
   if (state->Door_Feedback_evt)
      EVT_sched_remove(PROC_evt(state->proc), state->Door_Feedback_evt);
  
   if (state->Door_Feedback) {
      // Turn off the ball1 GPIO if able
      if (state->Door_Feedback->set)
         state->Door_Feedback->set(state->Door_Feedback, 0);
      // Delete the ball1 GPIO sensor
      state->Door_Feedback->sensor.close((struct Sensor **)&state->Door_Feedback);
   } 	

   // Clean up the deployment events
   if (state->small_ball_evt)
      EVT_sched_remove(PROC_evt(state->proc), state->small_ball_evt);
  
   if (state->deploy_small_ball) {
      // Turn off the ball1 GPIO if able
      if (state->deploy_small_ball->set)
         state->deploy_small_ball->set(state->deploy_small_ball, 0);
      // Delete the ball1 GPIO sensor
      state->deploy_small_ball->sensor.close((struct Sensor **)&state->deploy_small_ball);
   }   
   
   if (state->large_ball_evt)
      EVT_sched_remove(PROC_evt(state->proc), state->large_ball_evt);
  
  if (state->deploy_large_ball) {
      // Turn off the ball1 GPIO if able
      if (state->deploy_large_ball->set)
         state->deploy_large_ball->set(state->deploy_large_ball, 0);
      // Delete the ball1 GPIO sensor
      state->deploy_large_ball->sensor.close((struct Sensor **)&state->deploy_large_ball);
   }   
     
   if (state->door_evt)
      EVT_sched_remove(PROC_evt(state->proc), state->door_evt);
  
  if (state->deploy_door) {
      // Turn off the ball1 GPIO if able
      if (state->deploy_door->set)
         state->deploy_door->set(state->deploy_door, 0);
      // Delete the ball1 GPIO sensor
      state->deploy_door->sensor.close((struct Sensor **)&state->deploy_door);
   } 

   // Clean up the LED events
   if (state->enable_5V) {
      // Turn off the cree GPIO if able
      if (state->enable_5V->set)
         state->enable_5V->set(state->enable_5V, 0);
	 codes_for_status[11]=0;
      // Delete the cree GPIO sensor
      state->enable_5V->sensor.close((struct Sensor **)&state->enable_5V);
   }	
	
   if (state->cree) {
      // Turn off the cree GPIO if able
      if (state->cree->set)
         state->cree->set(state->cree, 0);
	 codes_for_status[6]=0;
      // Delete the cree GPIO sensor
      state->cree->sensor.close((struct Sensor **)&state->cree);
   }
   
   if (state->led_505L) {
      // Turn off the led_505L GPIO if able
      if (state->led_505L->set)
         state->led_505L->set(state->led_505L, 0);
      // Delete the led_505L GPIO sensor
      state->led_505L->sensor.close((struct Sensor **)&state->led_505L);
   }
   
   if (state->led_645L) {
      // Turn off the led_645L GPIO if able
      if (state->led_645L->set)
         state->led_645L->set(state->led_645L, 0);
      // Delete the led_645L GPIO sensor
      state->led_645L->sensor.close((struct Sensor **)&state->led_645L);
   }
      
   if (state->led_851L) {
      // Turn off the led_851L GPIO if able
      if (state->led_851L->set)
         state->led_851L->set(state->led_851L, 0);
      // Delete the led_851L GPIO sensor
      state->led_851L->sensor.close((struct Sensor **)&state->led_851L);
   }
      
   if (state->led_IR) {
      // Turn off the led_IR GPIO if able
      if (state->led_IR->set)
         state->led_IR->set(state->led_IR, 0);
      // Delete the led_IR GPIO sensor
      state->led_IR->sensor.close((struct Sensor **)&state->led_IR);
   }
   PROC_cleanup(state->proc);

   return 0;
}
