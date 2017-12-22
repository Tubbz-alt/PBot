// POCSAGBot [:)] for OpenWrt/Vocore
// CC-BY furrtek 2017
// With bits from rtl_fm

#include "main.h"
#include <unistd.h>

#include "rtl-sdr.h"
#include <rtl-sdr_export.h>

static rtlsdr_dev_t * dev = NULL;
int term = 0;
pthread_t scan_thread;
uint32_t len_acc;
uint32_t scan_index;

pthread_mutex_t data_ready;		// locked when no fresh data available
pthread_mutex_t data_write;		// locked when r/w buffer

const uint32_t scan_list[4] = {
	466050000,
	466075000,
	466175000,
	466206250
};

void terminate(int sig) {
	term = 1;
	rtlsdr_cancel_async(dev);
}

void fm_init(struct fm_state *fm) {
	fm->prev_index = 0;
	fm->pre_j = fm->pre_r = fm->now_r = fm->now_j = 0;
	fm->counter = 0;
	fm->phase_acc = 0;
	fm->sync_status = 0;
	fm->bit_counter = 0;
	fm->timeout = 0;
}

int tune_to(uint32_t frequency) {
	uint32_t capture_freq = frequency + (SAMPLE_RATE / 4);	// Tune so that the useful signal is below the DC spike
	
	//printf("Setting center frequency to %uMHz...\n", capture_freq);
	if (rtlsdr_set_center_freq(dev, capture_freq)) {
		puts("SDR: Error setting frequency.");
		return 1;
	}
	rtlsdr_reset_buffer(dev);
	
	usleep(5000);
	rtlsdr_read_sync(dev, NULL, 16384, NULL);
	//puts("SDR: Re-tune OK.");
	
	return 0;
}

static void rtlsdr_cb(unsigned char * buf, uint32_t len, void * ctx) {
	struct fm_state *fm2 = ctx;

	if (term)
		return;
		
	if (!ctx)
		return;
	
	pthread_mutex_lock(&data_write);
	memcpy(fm2->buf, buf, len);
	fm2->buf_len = len;
	pthread_mutex_unlock(&data_ready);
	
	len_acc += (len / 2);		// 1 I/Q sample = 2 uchars
}

static void *scan_thread_fn(void * arg) {
	struct fm_state *fm2 = arg;
	
	puts("Processing thread is now running.");
	
	while (!term) {
		pthread_mutex_lock(&data_ready);
		full_demod(fm2);
		
		if (len_acc >= (SAMPLE_RATE * 2)) {
			len_acc = 0;
			
			//printf("%u: %u\n", fm2->counter, fm2->bit_counter);
			printf("%u: Alive\n", fm2->counter);
			
			fm2->bit_counter = 0;
			
			// Hop !
			/*if (!fm2->sync_status) {
				if (scan_index == 3)
					scan_index = 0;
				else
					scan_index++;
				if (tune_to(scan_list[scan_index]))
					return 0;
			}*/
				
			fm2->counter++;
		}
	}
	
	return 0;
}

int main(void) {
	FILE * f_green_led;
	FILE * f_red_led;
	struct fm_state fm;
	int dev_count;
	int r;
	
	tweet_test();
	
	return 1;
	
	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);
	signal(SIGQUIT, terminate);
	
	fm_init(&fm);
	pthread_mutex_init(&data_ready, NULL);
	pthread_mutex_init(&data_write, NULL);
	
    printf("Hello [:)] I'm POCSAGBot.\n");
    
	printf("Setting up LEDs...\n");
	if (init_LEDs(&f_green_led, &f_red_led))
		return 1;
	
	printf("Looking for RTLSDR device...\n");
	dev_count = rtlsdr_get_device_count();
	if (!dev_count) {
		printf("None found.\n");
		return 1;
	}
	
	printf("Opening RTLSDR device... ");
	r = rtlsdr_open(&dev, 0);
	if (r < 0) {
		printf("Failed.\n");
		set_LED(f_green_led, 0);
		set_LED(f_red_led, 1);
		return 1;
	} else {
		printf("Ok :)\n");
		set_LED(f_green_led, 1);
		set_LED(f_red_led, 0);
	}
	
	scan_index = 0;
	fm.downsample = 15;		// TESTING
	fm.output_scale = (1<<15) / (128 * fm.downsample);
	
	printf("Setting gain mode to auto...\n");
	rtlsdr_set_tuner_gain_mode(dev, 0);				// Auto
	
	if (tune_to(scan_list[0]))
		return 1;
		
	printf("Setting sampling rate to %u...\n", SAMPLE_RATE);
	if (rtlsdr_set_sample_rate(dev, SAMPLE_RATE)) {
		puts("SDR: Error setting sampling rate.");
		return 1;
	} else {
		printf("Set to %u OK.\n", rtlsdr_get_sample_rate(dev));
	}
	
	printf("Enabling AGC...\n");
	rtlsdr_set_agc_mode(dev, 1);					// AGC on
	
	tune_to(scan_list[0]);
	
	rtlsdr_reset_buffer(dev);
	
	//pthread_create(&scan_thread, NULL, scan_thread_fn, (void *)(&fm));
	
	/*rtlsdr_read_async(dev, rtlsdr_cb, (void *)(&fm),
					32,		//DEFAULT_ASYNC_BUF_NUMBER,
					BUF_LENGTH);	//lcm_post[fm.post_downsample] * DEFAULT_BUF_LENGTH);
	*/
	
	int n_read;
	
	while (!term) {
		if (rtlsdr_read_sync(dev, &(fm.buf), BUF_LENGTH, &n_read) < 0) break;
		len_acc += (n_read);		// 1 I/Q sample = 2 uchars
		fm.buf_len = n_read;
		
		full_demod(&fm);
		
		if (len_acc >= (SAMPLE_RATE / 5)) {
			len_acc = 0;
			
			//printf("%u: %u\n", fm.counter, fm.bit_counter);
			//printf("%u: Alive\n", fm.counter);
			
			fm.bit_counter = 0;
			
			// Hop !
			if (!fm.sync_status) {
				if (scan_index == 3)
					scan_index = 0;
				else
					scan_index++;
				if (tune_to(scan_list[scan_index]))
					return 0;
			}
				
			fm.counter++;
		}
		
	};
	
	// Only rtlsdr_cancel_async() allows to jump out of rtlsdr_read_async
	
	rtlsdr_cancel_async(dev);
	pthread_mutex_destroy(&data_ready);
	pthread_mutex_destroy(&data_write);
	
	printf("POCSAGBot says goodbye :(\n");
	set_LED(f_red_led, 0);
	set_LED(f_green_led, 0);
	
	//set_LED(f_red_led, 1);
	
	fclose(f_green_led);
	fclose(f_red_led);
	
	rtlsdr_close(dev);
}
