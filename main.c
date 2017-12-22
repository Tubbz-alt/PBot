// POCSAGBot [:)] for OpenWrt/Vocore
// CC-BY furrtek 2017
// With bits from rtl_fm

#include "main.h"
#include <unistd.h>

#include <rtl-sdr_export.h>

rtlsdr_dev_t * dev = NULL;
int term = 0;
pthread_t scan_thread;
uint32_t len_acc;
uint32_t scan_index;
char txt_buffer[128] = { 0 };

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

int main(void) {
	FILE * f_green_led;
	FILE * f_red_led;
	struct fm_state fm;
	uint32_t reset_count;
	int n_read;
	
	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);
	signal(SIGQUIT, terminate);

	printf("Hello [:)] I'm POCSAGBot.\n");

	snprintf(txt_buffer, 128, "Started at %u", (uint32_t)time(NULL));
	send_dm(txt_buffer);

	printf("Setting up LEDs...\n");
	if (init_LEDs(&f_green_led, &f_red_led))
		return 1;
	
	do {
		fm_init(&fm);
		
		scan_index = 0;
		fm.downsample = 15;
		fm.output_scale = (1<<15) / (128 * fm.downsample);
		
		if (!sdr_init())
			set_LED(f_green_led, 1);
		
		if (tune_to(scan_list[0]))
			return 1;
		
		rtlsdr_reset_buffer(dev);
		
		//pthread_create(&scan_thread, NULL, scan_thread_fn, (void *)(&fm));
		
		while (!term) {
			if (rtlsdr_read_sync(dev, &(fm.buf), BUF_LENGTH, &n_read) < 0) {
				puts("rtlsdr_read_sync() failed.");
				break;
			}
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
					if (tune_to(scan_list[scan_index])) {
						puts("tune_to() failed.");
						break;
					}
				}
					
				fm.counter++;
			}
		};
		
		set_LED(f_green_led, 0);
	
		if (!term) {
			if (reset_count == 10) {
				puts("SDR fucked up too many times, giving up.");
				//snprintf(txt_buffer, 128, "%u: 10 resets, going offline.", (uint32_t)time(NULL));
				//tweet(txt_buffer);
				snprintf(txt_buffer, 128, "Given up at %u", (uint32_t)time(NULL));
				send_dm(txt_buffer);
				term = 1;
			} else {
				printf("SDR fuck up #%u, resetting...\n", ++reset_count);
				rtlsdr_close(dev);
				system("usbreset 0bda:2838");
				usleep(1000000);
			}
		}
	
	} while (!term);
	
	//rtlsdr_cancel_async(dev);
	
	set_LED(f_red_led, 1);
	
	printf("POCSAGBot says goodbye :(\n");
	
	fclose(f_green_led);
	fclose(f_red_led);
	
	if (dev)
		rtlsdr_close(dev);
}
