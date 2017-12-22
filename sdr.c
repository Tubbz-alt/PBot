#include "main.h"

int sdr_init(void) {
	int r;
	int dev_count;
	
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
		return 1;
	} else {
		printf("Ok :)\n");
	}
	
	printf("Setting gain mode to auto...\n");
	rtlsdr_set_tuner_gain_mode(dev, 0);				// Auto
		
	printf("Setting sampling rate to %u...\n", SAMPLE_RATE);
	if (rtlsdr_set_sample_rate(dev, SAMPLE_RATE)) {
		puts("SDR: Error setting sampling rate.");
		return 1;
	} else {
		printf("Set to %u OK.\n", rtlsdr_get_sample_rate(dev));
	}
	
	printf("Enabling AGC...\n");
	rtlsdr_set_agc_mode(dev, 1);					// AGC on
	
	return 0;
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
	
	return 0;
}
