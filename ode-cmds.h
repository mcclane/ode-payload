#ifndef ODE_CMDS_H
#define ODE_CMDS_H

#define ODE_STATUS_CMD 1
#define ODE_STATUS_RESP (ODE_STATUS_CMD | 0x80)

#define ODE_BLINK_CREE_CMD 2
#define ODE_BLINK_CREE_RESP (ODE_BLINK_CREE_CMD | 0x80)

#define ODE_BLINK_LED_505L_CMD 3
#define ODE_BLINK_LED_505L_RESP (ODE_BLINK_LED_505L_CMD | 0x80)

#define ODE_BLINK_LED_645L_CMD 4
#define ODE_BLINK_LED_645L_RESP (ODE_BLINK_LED_645L_CMD | 0x80)

#define ODE_BLINK_LED_851L_CMD 5
#define ODE_BLINK_LED_851L_RESP (ODE_BLINK_LED_851L_CMD | 0x80)

#define ODE_BLINK_IR_LED_CMD 6
#define ODE_BLINK_IR_LED_RESP (ODE_BLINK_IR_LED_CMD | 0x80)

#define ODE_DEPLOY_SMALL_BALL_CMD 7
#define ODE_DEPLOY_SMALL_BALL_RESP (ODE_DEPLOY_SMALL_BALL_CMD | 0x80)

#define ODE_DEPLOY_LARGE_BALL_CMD 8
#define ODE_DEPLOY_LARGE_BALL_RESP (ODE_DEPLOY_LARGE_BALL_CMD | 0x80)

#define ODE_DEPLOY_DOOR_CMD 9
#define ODE_DEPLOY_DOOR_RESP (ODE_DEPLOY_DOOR_CMD | 0x80)

#define ODE_SMALL_BALL_STATUS_CMD 10
#define ODE_SMALL_BALL_STATUS_RESP (ODE_SMALL_BALL_STATUS_CMD | 0x80)

#define ODE_LARGE_BALL_STATUS_CMD 11
#define ODE_LARGE_BALL_STATUS_RESP (ODE_LARGE_BALL_STATUS_CMD | 0x80)

#define ODE_MW_STATUS_CMD 12
#define ODE_MW_STATUS_RESP (ODE_MW_STATUS_CMD | 0x80)

#define ODE_SET_LED_DELAY_CMD 13
#define ODE_SET_LED_DELAY_RESP (ODE_SET_LED_DELAY_CMD | 0x80)

#define ODE_SET_LED_PERIOD_CMD 14
#define ODE_SET_LED_PERIOD_RESP (ODE_SET_LED_PERIOD_CMD | 0x80)

#define ODE_SET_LED_DURATION_CMD 15
#define ODE_SET_LED_DURATION_RESP (ODE_SET_LED_DURATION_CMD | 0x80)

#define ODE_DEPLOY_SMALL_BALL_DELAY_CMD 16
#define ODE_DEPLOY_SMALL_BALL_DELAY_RESP (ODE_DEPLOY_SMALL_BALL_DELAY_CMD | 0x80)

#define ODE_DEPLOY_LARGE_BALL_DELAY_CMD 17
#define ODE_DEPLOY_LARGE_BALL_DELAY_RESP (ODE_DEPLOY_LARGE_BALL_DELAY_CMD | 0x80)

#define ODE_DEPLOY_DOOR_DELAY_CMD 18
#define ODE_DEPLOY_DOOR_DELAY_RESP (ODE_DEPLOY_DOOR_DELAY_CMD | 0x80)


struct ODEStatus {
   uint8_t small_ball_sw; //0
   uint8_t large_ball_sw; 
   uint8_t MW_sw;    //2
   uint8_t small_ball_fb;
   uint8_t large_ball_fb; //4
   uint8_t MW_fb;
   uint8_t cree_led; //6
   uint8_t led_505L;
   uint8_t led_645L; //8
   uint8_t led_851L; 
   uint8_t led_IR;   //10
   uint8_t enable_5V;   //10
   uint32_t small_ball_fb_time;
   uint32_t large_ball_fb_time; //4
   uint32_t MW_fb_time;
   uint32_t curr_time;
   uint32_t time_until_small;
   uint32_t time_until_large;
   uint32_t time_until_door;
} __attribute__((packed));

struct ODEBlinkData {
   uint32_t period;
   uint32_t fb_led_period;
   int32_t duration;
   int32_t delay;
} __attribute__((packed));

struct ODEDeployData {
   uint32_t duration;
} __attribute__((packed));

struct ODEDeployDelayData {
   uint32_t duration;
   uint32_t mode;
   uint32_t delay;
} __attribute__((packed));

struct ODEFeedBackData {
   uint32_t duration;
} __attribute__((packed));
#endif
