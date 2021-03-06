/** 
 * Test code for process command handling
 * @author 
 *
 */

#include <polysat/polysat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include "ode-cmds.h"

#define DFL_BALL_TIME_MS (5*1000)  		// 5 seconds
#define DFL_DOOR_TIME_MS (10*1000)  		// 15 seconds
#define DFL_IR_PERIOD_MS (15*60*1000)		// 15 minutes
#define DFL_FB_DUR_MS (10)  			// 10 ms
#define WAIT_MS (4 * 1000)  			// 4 seconds
#define DFL_BLINK_DELAY_MS (0)			// 0 seconds
#define DFL_BLINK_PERIOD_MS 1000		// 1 second
#define DFL_BLINK_DUR_MS (15*60*1000)		// 15 minutes

struct MulticallInfo;

static int ode_status(int, char**, struct MulticallInfo *);
static int ode_telemetry(int, char**, struct MulticallInfo *);
static int ode_cree(int, char**, struct MulticallInfo *);
static int ode_led_505L(int, char**, struct MulticallInfo *);
static int ode_led_645L(int, char**, struct MulticallInfo *);
static int ode_led_851L(int, char**, struct MulticallInfo *);
static int ode_deploy_small_ball(int, char**, struct MulticallInfo *);
static int ode_deploy_large_ball(int, char**, struct MulticallInfo *);
static int ode_deploy_door(int, char**, struct MulticallInfo *);
static int ode_delayed_deploy_small_ball(int, char**, struct MulticallInfo *);
static int ode_delayed_deploy_large_ball(int, char**, struct MulticallInfo *);
static int ode_delayed_deploy_door(int, char**, struct MulticallInfo *);

// struct holding all possible function calls
// running the executable with the - flags will call that function
// running without flags will print out this struct
struct MulticallInfo {
   int (*func)(int argc, char **argv, struct MulticallInfo *);
   const char *name;
   const char *opt;
   const char *help;
} multicall[] = {
   { &ode_status, "ode-status", "-S", "Display the current status of the ode-payload process" }, 
   { &ode_telemetry, "ode-telemetry", "-T", "Display the current status of the ode-payload process in KVP form" }, 
   { &ode_cree, "ode-cree", "-L1", "Blink Cree LED" }, 
   { &ode_led_505L, "ode-led_505L", "-L2", "Blink 505L LED" }, 
   { &ode_led_645L, "ode-led_645L", "-L3", "Blink 645L LED" }, 
   { &ode_led_851L, "ode-led_851L", "-L4", "Blink 851L LED" }, 
   { &ode_deploy_small_ball, "ode-deploy_small_ball", "-B1", "Deploy small ball" }, 
   { &ode_deploy_large_ball, "ode-deploy_large_ball", "-B2", "Deploy large ball" }, 
   { &ode_deploy_door, "ode-deploy_door", "-B3", "Open door" }, 
   { &ode_delayed_deploy_small_ball, "ode-delayed-deploy-small-ball", "-D1", "Deploy small ball" }, 
   { &ode_delayed_deploy_large_ball, "ode-delayed-deploy-large-ball", "-D2", "Deploy large ball" }, 
   { &ode_delayed_deploy_door, "ode-delayed-deploy-door", "-D3", "Open door" }, 
  { NULL, NULL, NULL, NULL }
};

// Status call
//_____________________________________________________________________________________
static int ode_status(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   time_t now, depl;
   struct {
      uint8_t cmd;
      struct ODEStatus status;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
   } __attribute__((packed)) send;

   send.cmd = 1;
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != CMD_STATUS_RESPONSE) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, CMD_STATUS_RESPONSE);
      return 5;
   }   
   
   now = time(NULL);
   printf("Spacecraft time: %u\n", ntohl(resp.status.curr_time));
   printf("Small Ball deployment enabled: %d\n", resp.status.small_ball_sw);
   printf("Large Ball deployment enabled: %d\n", resp.status.large_ball_sw);
   printf("Meltwire deployment enabled: %d\n", resp.status.MW_sw);
   printf("Ball 1 fb sw: %d\n", resp.status.small_ball_fb);
   printf("Small Ball deployment time: %u\n", ntohl(resp.status.small_ball_fb_time));
   printf("Ball 2 fb sw: %d\n", resp.status.large_ball_fb);
   printf("Large Ball deployment time: %u\n", ntohl(resp.status.large_ball_fb_time));
   printf("Meltwire fb sw: %d\n", resp.status.MW_fb);
   printf("MW deployment time: %u\n", ntohl(resp.status.MW_fb_time));
   printf("Cree LED status: %d\n", resp.status.cree_led);
   printf("LED 505L status: %d\n", resp.status.led_505L);
   printf("LED 645L status: %d\n", resp.status.led_645L);
   printf("LED 851L status: %d\n", resp.status.led_851L);   
   printf("LED IR status: %d\n", resp.status.led_IR);   
   printf("5V regulator status: %d\n", resp.status.enable_5V);   

   printf("Small Ball auto-deployment delta: %u s\n", ntohl(resp.status.time_until_small));
   depl = now + ntohl(resp.status.time_until_small);
   printf("Small Ball auto-deployment time: %s\n", asctime(gmtime(&depl)));

   printf("Large Ball auto-deployment delta: %u s\n",
      ntohl(resp.status.time_until_large));
   depl = now + ntohl(resp.status.time_until_large);
   printf("Large Ball auto-deployment time: %s\n", asctime(gmtime(&depl)));

   printf("Door auto-deployment delta: %u s\n",
      ntohl(resp.status.time_until_door));
   depl = now + ntohl(resp.status.time_until_door);
   printf("Door auto-deployment time: %s\n", asctime(gmtime(&depl)));

   return 0;
}

static int ode_telemetry(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      struct ODEStatus status;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
   } __attribute__((packed)) send;

   send.cmd = 1;
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != CMD_STATUS_RESPONSE) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, CMD_STATUS_RESPONSE);
      return 5;
   }   
   
   printf("sm_ball_ena=%d\n", resp.status.small_ball_sw);
   printf("lr_ball_ena=%d\n", resp.status.large_ball_sw);
   printf("mw_ena=%d\n", resp.status.MW_sw);
   printf("sm_ball_feedback=%d\n", resp.status.small_ball_fb);
   printf("sm_ball_time=%u\n", ntohl(resp.status.small_ball_fb_time));
   printf("lr_ball_feedback=%d\n", resp.status.large_ball_fb);
   printf("lr_ball_time=%u\n", ntohl(resp.status.large_ball_fb_time));
   printf("mw_feedback=%d\n", resp.status.MW_fb);
   printf("mw_time=%u\n", ntohl(resp.status.MW_fb_time));
   printf("cree_status=%d\n", resp.status.cree_led);
   printf("505l_status=%d\n", resp.status.led_505L);
   printf("645l_status=%d\n", resp.status.led_645L);
   printf("851l_status=%d\n", resp.status.led_851L);   
   printf("ir_status=%d\n", resp.status.led_IR);   
   printf("5v_status=%d\n", resp.status.enable_5V);   
   printf("sm_ball_auto_delay=%u\n", ntohl(resp.status.time_until_small));
   printf("lr_ball_auto_delay=%u\n", ntohl(resp.status.time_until_large));
   printf("mw_auto_delay=%u\n", ntohl(resp.status.time_until_door));

   return 0;
}

//Led Commands
//_____________________________________________________________________________________
static int ode_cree(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEBlinkData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_BLINK_CREE_CMD;
   send.param.delay = htonl(DFL_BLINK_DELAY_MS);
   send.param.period = htonl(DFL_BLINK_PERIOD_MS);
   send.param.duration = htonl(DFL_BLINK_DUR_MS);
	
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:p:D:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
         case 'D':
            send.param.delay = htonl(atol(optarg));
            break;
         case 'p':
            send.param.period = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_BLINK_CREE_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_BLINK_CREE_RESP);
      return 5;
   }

   return 0;
}

static int ode_led_505L(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEBlinkData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_BLINK_LED_505L_CMD;
   send.param.delay = htonl(DFL_BLINK_DELAY_MS);
   send.param.period = htonl(DFL_BLINK_PERIOD_MS);
   send.param.duration = htonl(DFL_BLINK_DUR_MS);
	
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:p:D:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
         case 'p':
            send.param.period = htonl(atol(optarg));
            break;
         case 'D':
            send.param.delay = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_BLINK_LED_505L_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_BLINK_LED_505L_RESP);
      return 5;
   }

   return 0;
}

static int ode_led_645L(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEBlinkData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_BLINK_LED_645L_CMD;
   send.param.delay = htonl(DFL_BLINK_DELAY_MS);
   send.param.period = htonl(DFL_BLINK_PERIOD_MS);
   send.param.duration = htonl(DFL_BLINK_DUR_MS);
	
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:p:D:")) != -1) {
      switch(opt) {
         case 'D':
            send.param.delay = htonl(atol(optarg));
            break;
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
         case 'p':
            send.param.period = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_BLINK_LED_645L_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_BLINK_LED_645L_RESP);
      return 5;
   }

   return 0;
}

static int ode_led_851L(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEBlinkData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_BLINK_LED_851L_CMD;
   send.param.delay = htonl(DFL_BLINK_DELAY_MS);
   send.param.period = htonl(DFL_BLINK_PERIOD_MS);
   send.param.duration = htonl(DFL_BLINK_DUR_MS);
	
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:p:D:")) != -1) {
      switch(opt) {
         case 'D':
            send.param.delay = htonl(atol(optarg));
            break;
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
         case 'p':
            send.param.period = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_BLINK_LED_851L_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_BLINK_LED_851L_RESP);
      return 5;
   }

   return 0;
}

//Deployments
//_____________________________________________________________________________________
static int ode_deploy_small_ball(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEDeployData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_DEPLOY_SMALL_BALL_CMD;
   send.param.duration = htonl(DFL_BALL_TIME_MS);
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_DEPLOY_SMALL_BALL_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_DEPLOY_SMALL_BALL_RESP);
      return 5;
   }

   return 0;
}

static int ode_delayed_deploy_small_ball(int argc, char **argv,
      struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEDeployDelayData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_DEPLOY_SMALL_BALL_DELAY_CMD;
   send.param.duration = htonl(DFL_BALL_TIME_MS);
   send.param.mode = 0;
   send.param.delay = 0;
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:u:o:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
         case 'u':
            send.param.delay = htonl(atol(optarg));
            send.param.mode = htonl(1);
            break;
         case 'o':
            send.param.delay = htonl(atol(optarg));
            send.param.mode = htonl(2);
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_DEPLOY_SMALL_BALL_DELAY_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_DEPLOY_SMALL_BALL_DELAY_RESP);
      return 5;
   }

   return 0;
}

static int ode_delayed_deploy_large_ball(int argc, char **argv,
      struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEDeployDelayData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_DEPLOY_LARGE_BALL_DELAY_CMD;
   send.param.duration = htonl(DFL_BALL_TIME_MS);
   send.param.mode = 0;
   send.param.delay = 0;
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:u:o:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
         case 'u':
            send.param.delay = htonl(atol(optarg));
            send.param.mode = htonl(1);
            break;
         case 'o':
            send.param.delay = htonl(atol(optarg));
            send.param.mode = htonl(2);
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_DEPLOY_LARGE_BALL_DELAY_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_DEPLOY_LARGE_BALL_DELAY_RESP);
      return 5;
   }

   return 0;
}

static int ode_deploy_large_ball(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEDeployData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_DEPLOY_LARGE_BALL_CMD;
   send.param.duration = htonl(DFL_BALL_TIME_MS);
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_DEPLOY_LARGE_BALL_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_DEPLOY_LARGE_BALL_RESP);
      return 5;
   }

   return 0;
}

static int ode_delayed_deploy_door(int argc, char **argv,
      struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEDeployDelayData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_DEPLOY_DOOR_DELAY_CMD;
   send.param.duration = htonl(DFL_BALL_TIME_MS);
   send.param.mode = 0;
   send.param.delay = 0;
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:u:o:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
         case 'u':
            send.param.delay = htonl(atol(optarg));
            send.param.mode = htonl(1);
            break;
         case 'o':
            send.param.delay = htonl(atol(optarg));
            send.param.mode = htonl(2);
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_DEPLOY_DOOR_DELAY_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_DEPLOY_DOOR_DELAY_RESP);
      return 5;
   }

   return 0;
}

static int ode_deploy_door(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEDeployData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_DEPLOY_DOOR_CMD;
   send.param.duration = htonl(DFL_DOOR_TIME_MS);
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_DEPLOY_DOOR_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_DEPLOY_DOOR_RESP);
      return 5;
   }

   return 0;
}

//Read functions
//_____________________________________________________________________________________
static int small_ball_status(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEFeedBackData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_SMALL_BALL_STATUS_CMD;
   send.param.duration = htonl(DFL_FB_DUR_MS);
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_SMALL_BALL_STATUS_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_SMALL_BALL_STATUS_RESP);
      return 5;
   }

   return 0;
}

static int large_ball_status(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEFeedBackData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_LARGE_BALL_STATUS_CMD;
   send.param.duration = htonl(DFL_FB_DUR_MS);
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_LARGE_BALL_STATUS_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_LARGE_BALL_STATUS_RESP);
      return 5;
   }

   return 0;
}

static int mw_status(int argc, char **argv, struct MulticallInfo * self) 
{
   // struct to hold response from payload process
   struct {
      uint8_t cmd;
      uint8_t resp;
   } __attribute__((packed)) resp;

   struct {
      uint8_t cmd;
      struct ODEFeedBackData param;
   } __attribute__((packed)) send;

   send.cmd = ODE_MW_STATUS_CMD;
   send.param.duration = htonl(DFL_FB_DUR_MS);
   const char *ip = "127.0.0.1";
   int len, opt;
   
   while ((opt = getopt(argc, argv, "h:d:")) != -1) {
      switch(opt) {
         case 'h':
            ip = optarg;
            break;
         case 'd':
            send.param.duration = htonl(atol(optarg));
            break;
      }
   }
   
   // send packet and wait for response
   if ((len = socket_send_packet_and_read_response(ip, "payload", &send, 
    sizeof(send), &resp, sizeof(resp), WAIT_MS)) <= 0) {
      return len;
   }
 
   if (resp.cmd != ODE_MW_STATUS_RESP) {
      printf("response code incorrect, Got 0x%02X expected 0x%02X\n", 
       resp.cmd, ODE_MW_STATUS_RESP);
      return 5;
   }

   return 0;
}

static int ode_test(int argc, char **argv, struct MulticallInfo * self) {return 0;}

// prints out available commands for this util
static int print_usage(const char *name)
{
   struct MulticallInfo *curr;

   printf("lsb-util multicall binary, use the following names instead:\n");

   for (curr = multicall; curr->func; curr++) {
      printf("   %-16s %s\n", curr->name, curr->help);
   }

   return 0;
}

int main(int argc, char **argv) 
{   
   struct MulticallInfo *curr;
   char *exec_name;

   exec_name = rindex(argv[0], '/');
   if (!exec_name) {
      exec_name = argv[0];
   }
   else {
      exec_name++;
   }

   for (curr = multicall; curr->func; curr++) {
      if (!strcmp(curr->name, exec_name)) {
         return curr->func(argc, argv, curr);
      }
   }

   if (argc > 1) {
      for (curr = multicall; curr->func; curr++) {
         if (!strcmp(curr->opt, argv[1])) {
            return curr->func(argc - 1, argv + 1, curr);
         }
      }
   }
   else {
      return print_usage(argv[0]);
   }

   return 0;
}
