#include <stdio.h>

int init_LEDs(FILE ** f_green_led, FILE ** f_red_led) {
	// GPIO22 = green
	*f_green_led = fopen("/sys/devices/10000000.palmbus/10000660.gpio/gpio/gpio22/direction", "w");
	if (*f_green_led == NULL) {
		puts("LED: Error setting direction.");
		return 1;
	}
	fwrite("out", 1, 3, *f_green_led);
	fflush(*f_green_led);
	fclose(*f_green_led);
	*f_green_led = fopen("/sys/devices/10000000.palmbus/10000660.gpio/gpio/gpio22/value", "w");
	if (*f_green_led == NULL) {
		puts("LED: Error opening output.");
		return 1;
	}
	
	// GPIO23 = red
	*f_red_led = fopen("/sys/devices/10000000.palmbus/10000660.gpio/gpio/gpio23/direction", "w");
	if (*f_red_led == NULL) {
		puts("LED: Error setting direction.");
		return 1;
	}
	fwrite("out", 1, 3, *f_red_led);
	fflush(*f_red_led);
	fclose(*f_red_led);
	*f_red_led = fopen("/sys/devices/10000000.palmbus/10000660.gpio/gpio/gpio23/value", "w");
	if (*f_red_led == NULL) {
		puts("LED: Error opening output.");
		return 1;
	}
	
	return 0;
}

void set_LED(FILE * f_led, int state) {
	if (state)
		fwrite("1", 1, 1, f_led);
	else
		fwrite("0", 1, 1, f_led);
	fflush(f_led);
}
