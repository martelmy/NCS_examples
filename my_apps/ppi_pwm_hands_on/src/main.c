/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

// ------------------ PWM training exercise ----------------------
//
// The purpose of this training exercise is to demonstrate the use of the TIMER, PPI and GPIOTE in the nRF52 series,
// and how to combine them to generate a PWM signal.

#include <zephyr.h>
#include <sys/printk.h>
#include <math.h>

#define LED_1   13  // From the description on the back of the DK
#define LED_2   14  // From the description on the back of the DK
#define LED_3   15  // From the description on the back of the DK
#define LED_4   16  // From the description on the back of the DK

// Peripheral channel assignments
#define PWM0_GPIOTE_CH      0
#define PWM1_GPIOTE_CH      1
#define PWM2_GPIOTE_CH      2
#define PWM3_GPIOTE_CH      3

#define PWM0_PPI_CH_A       0
#define PWM1_PPI_CH_A       1
#define PWM2_PPI_CH_A       2
#define PWM3_PPI_CH_A       3
#define PWM0_PPI_CH_B       4
#define PWM2_PPI_CH_B       5

#define PWM0_TIMER_CC_NUM   0
#define PWM1_TIMER_CC_NUM   1
#define PWM2_TIMER_CC_NUM   2
#define PWM3_TIMER_CC_NUM   3

#define PWMN_GPIOTE_CH      {PWM0_GPIOTE_CH, PWM1_GPIOTE_CH, PWM2_GPIOTE_CH, PWM3_GPIOTE_CH}
#define PWMN_PPI_CH_A       {PWM0_PPI_CH_A, PWM1_PPI_CH_A, PWM2_PPI_CH_A, PWM3_PPI_CH_A}
#define PWMN_PPI_CH_B       {PWM0_PPI_CH_B, PWM0_PPI_CH_B, PWM2_PPI_CH_B, PWM2_PPI_CH_B}
#define PWMN_TIMER_CC_NUM   {PWM0_TIMER_CC_NUM, PWM1_TIMER_CC_NUM, PWM2_TIMER_CC_NUM, PWM3_TIMER_CC_NUM}

static uint32_t pwmN_gpiote_ch[]    = PWMN_GPIOTE_CH;
static uint32_t pwmN_ppi_ch_a[]     = PWMN_PPI_CH_A;
static uint32_t pwmN_ppi_ch_b[]     = PWMN_PPI_CH_B;
static uint32_t pwmN_timer_cc_num[] = PWMN_TIMER_CC_NUM;


// TIMER3 reload value. The PWM frequency equals '16000000 / TIMER_RELOAD'
#define TIMER_RELOAD        1024
// The timer CC register used to reset the timer. Be aware that not all timers in the nRF52 have 6 CC registers.
#define TIMER_RELOAD_CC_NUM 5

// This function initializes timer 3 with the following configuration:
// 24-bit, base frequency 16 MHz, auto clear on COMPARE5 match (CC5 = TIMER_RELOAD)
void timer_init()
{
    NRF_TIMER3->BITMODE                 = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER3->PRESCALER               = 0;
    NRF_TIMER3->SHORTS                  = TIMER_SHORTS_COMPARE0_CLEAR_Msk << TIMER_RELOAD_CC_NUM;
    NRF_TIMER3->MODE                    = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
    NRF_TIMER3->CC[TIMER_RELOAD_CC_NUM] = TIMER_RELOAD;    
}

// Starts TIMER3
void timer_start()
{
    NRF_TIMER3->TASKS_START = 1;
}

// This function sets up TIMER3, the PPI and the GPIOTE modules to configure a single PWM channel.
// Timer CC num, PPI channel nums and GPIOTE channel num is defined at the top of this file
void pwm0_init(uint32_t pinselect)
{  
    NRF_GPIOTE->CONFIG[PWM0_GPIOTE_CH] = GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos | 
                                         GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos | 
                                         pinselect << GPIOTE_CONFIG_PSEL_Pos | 
                                         GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos;

    NRF_PPI->CH[PWM0_PPI_CH_A].EEP = (uint32_t)&NRF_TIMER3->EVENTS_COMPARE[PWM0_TIMER_CC_NUM];
    NRF_PPI->CH[PWM0_PPI_CH_A].TEP = (uint32_t)&NRF_GPIOTE->TASKS_CLR[PWM0_GPIOTE_CH];
    NRF_PPI->CH[PWM0_PPI_CH_B].EEP = (uint32_t)&NRF_TIMER3->EVENTS_COMPARE[TIMER_RELOAD_CC_NUM];
    NRF_PPI->CH[PWM0_PPI_CH_B].TEP = (uint32_t)&NRF_GPIOTE->TASKS_SET[PWM0_GPIOTE_CH];
    
    NRF_PPI->CHENSET               = (1 << PWM0_PPI_CH_A) | (1 << PWM0_PPI_CH_B);
}

//  STEP 1: Implement a method for initializing PWM channel 1, for a total of 2 individual PWM channels
//                      Hint 1: Reusing code from pwm0_init(..) is recommended.
//                      Hint 2: You need to create definitions similar to PWM0_GPIOTE_CH, 
//                              PWM0_PPI_CH_A, PWM0_PPI_CH_B and PWM0_TIMER_CC_NUM
void pwm1_init(uint32_t pinselect)
{
    NRF_GPIOTE->CONFIG[PWM1_GPIOTE_CH] = GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos |
										 GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos |
										 pinselect << GPIOTE_CONFIG_PSEL_Pos |
										 GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos;

	NRF_PPI->CH[PWM1_PPI_CH_A].EEP = (uint32_t)&NRF_TIMER3->EVENTS_COMPARE[PWM1_TIMER_CC_NUM];
    NRF_PPI->CH[PWM1_PPI_CH_A].TEP = (uint32_t)&NRF_GPIOTE->TASKS_CLR[PWM1_GPIOTE_CH];

    NRF_PPI->FORK[PWM0_PPI_CH_B].TEP = (uint32_t)&NRF_GPIOTE->TASKS_SET[PWM1_GPIOTE_CH];
    
    NRF_PPI->CHENSET               = (1 << PWM1_PPI_CH_A);
}


//  STEP 5: Using the FORK feature of the PPI try to modify the function implemented in step 1
//          to get away with a single additional PPI channel for the second PWM output.
//          (in total it should be sufficient to use 3 PPI channels for 2 PWM outputs)
//          Hint: The FORK feature is useful when the same event needs to trigger several tasks

//  (STEP 6 - OPTIONAL): Make a generic init function that takes both the PWM channel number and the pinselect as arguments, 
//                       to avoid having to implement one function for each channel. The function should support up to 4 PWM channels total. 
//                       Hint1 : Don't start on the optional steps until all the required steps are complete
//                       Hint 2: You should create arrays that holds e.g. all PWMN_GPIOTE_CH:
//
//                       #define PWMN_GPIOTE_CH      {PWM0_GPIOTE_CH, PWM1_GPIOTE_CH, 2, 3}
//                       static uint32_t pwmN_gpiote_ch[]    = PWMN_GPIOTE_CH;

void pwmN_init(uint32_t N, uint32_t pinselect)
{
	if(N > 3) {
		return;
	}
    NRF_GPIOTE->CONFIG[pwmN_gpiote_ch[N]] = GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos |
										    GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos |
										    pinselect << GPIOTE_CONFIG_PSEL_Pos |
										    GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos;

	NRF_PPI->CH[pwmN_ppi_ch_a[N]].EEP = (uint32_t)&NRF_TIMER3->EVENTS_COMPARE[pwmN_timer_cc_num[N]];
    NRF_PPI->CH[pwmN_ppi_ch_a[N]].TEP = (uint32_t)&NRF_GPIOTE->TASKS_CLR[pwmN_gpiote_ch[N]];

	if (N%2 == 0) {
		NRF_PPI->CH[pwmN_ppi_ch_b[N]].EEP = (uint32_t)&NRF_TIMER3->EVENTS_COMPARE[TIMER_RELOAD_CC_NUM];
    	NRF_PPI->CH[pwmN_ppi_ch_b[N]].TEP = (uint32_t)&NRF_GPIOTE->TASKS_SET[pwmN_gpiote_ch[N]];
	}
	else {
		NRF_PPI->FORK[pwmN_ppi_ch_b[N-1]].TEP = (uint32_t)&NRF_GPIOTE->TASKS_SET[pwmN_gpiote_ch[N]];
	}

	NRF_PPI->CHENSET               = (1 << pwmN_ppi_ch_a[N]) | (1 << pwmN_ppi_ch_b[N]);
}


// Function for changing the duty cycle on PWM channel 0
void pwm0_set_duty_cycle(uint32_t value)
{
    if(value == 0) {
        value = 1;
    }
    else if(value >= TIMER_RELOAD) {
        value = TIMER_RELOAD - 1;
    }
    NRF_TIMER3->CC[PWM0_TIMER_CC_NUM] = value;
}


//  STEP 2: Implement a method for setting the duty cycle of PWM channel 1
void pwm1_set_duty_cycle(uint32_t value)
{
    if (value == 0) {
		value = 1;
	}
	else if (value >= TIMER_RELOAD) {
		value = TIMER_RELOAD -1;
	}
	NRF_TIMER3->CC[PWM1_TIMER_CC_NUM] = value;
}

//  (STEP 7 - OPTIONAL):   Make a generic set duty cycle function to support a total of 4 PWM channels

void pwmN_set_duty_cycle(uint32_t N, uint32_t value)
{
	if (N>3) {
		return;
	}
	if (value == 0) {
		value = 1;
	}
	else if (value >= TIMER_RELOAD) {
		value = TIMER_RELOAD -1;
	}
	NRF_TIMER3->CC[pwmN_timer_cc_num[N]] = value;
     
}

// Utility function for providing sin values, and converting them to integers
// input values in the range [0 - input_max] will be converted to 0-360 degrees (0-2*PI)
// output values will be scaled to the range [output_min - output_max]
uint32_t sin_scaled(uint32_t input, uint32_t input_max, uint32_t output_min, uint32_t output_max)
{
    float sin_val = sin((float)input * 2.0f * 3.141592f / (float)input_max);
    return (uint32_t)(((sin_val + 1.0f) / 2.0f) * (float)(output_max - output_min)) + output_min; 
}

int main(void)
{
    printk("Hello World! %s\n", CONFIG_BOARD);
    uint32_t counter = 0;
    
    // Initialize the timer
    timer_init();
    
    // Initialize PWM channel 0
    pwm0_init(LED_1);
    
    // STEP 3: Call the init function implemented in STEP 1, and configure the additional PWM channel on LED_2
	pwm1_init(LED_2);
    
    // (STEP 8: OPTIONAL) : Call the generic init function implemented in STEP 6, and configure 2 more PWM channels on LED_3 and LED_4
	pwmN_init(2, LED_3);
	pwmN_init(3, LED_4);
    
    // Start the timer
    timer_start();

    for (;;) {

        k_sleep(K_MSEC(4));
        
        // Update the duty cycle of PWM channel 0 and increment the counter
        pwm0_set_duty_cycle(sin_scaled(counter++, 200, 0, TIMER_RELOAD));
        
        // STEP 4:  Update the duty cycle of PWM channel 1, and add an offset to the counter to make the LED's blink out of phase
		pwm1_set_duty_cycle(sin_scaled(counter+50, 200, 0, TIMER_RELOAD));
        
        // (STEP 9 - OPTIONAL): Update the duty cycle of PWM channel 2 and 3, using the generic functions implemented earlier
		pwmN_set_duty_cycle(2,sin_scaled(counter+100, 200, 0, TIMER_RELOAD));
		pwmN_set_duty_cycle(3,sin_scaled(counter+150, 200, 0, TIMER_RELOAD));   
    }
}