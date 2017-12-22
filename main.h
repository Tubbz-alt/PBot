#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "rtl-sdr.h"

#define BUF_LENGTH 32768
#define SAMPLE_RATE 300000		// 300ksps

struct fm_state {
	int now_r, now_j;
	int pre_r, pre_j;
	int prev_index;
	int downsample;
	int output_scale;
	uint8_t buf[BUF_LENGTH];
	uint32_t buf_len;
	int signal[BUF_LENGTH];		// 16bit I/Q pairs
	uint32_t signal_len;
	uint32_t phase_acc;
	int32_t dir_current;
	uint32_t counter;
	uint32_t sr_data;
	uint32_t sync_status;
	uint32_t bit_counter;
	uint32_t timeout;
};

void tweet_test(void);

void multiply(int ar, int aj, int br, int bj, int *cr, int *cj);
int rot_direction(int ar, int aj, int br, int bj);
void full_demod(struct fm_state *fm);

int init_LEDs(FILE ** f_green_led, FILE ** f_red_led);
void set_LED(FILE * f_led, int state);

extern pthread_mutex_t data_ready;	// locked when no fresh data available
extern pthread_mutex_t data_write;	// locked when r/w buffer
extern uint32_t scan_index;
