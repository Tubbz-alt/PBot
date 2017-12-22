#include "main.h"

// Lots of stuff here is from rtl_fm
// Am I excused if I understood what's going on ? :D

// Complex multiply (ar,aj)*(br,bj) into (cr,cj)
void multiply(int ar, int aj, int br, int bj, int *cr, int *cj) {
	*cr = ar*br - aj*bj;
	*cj = aj*br + ar*bj;
}

// Return rotation direction
int rot_direction(int ar, int aj, int br, int bj) {
	int cr, cj;
	double angle;
	
	multiply(ar, aj, br, -bj, &cr, &cj);
	angle = atan2((double)cj, (double)cr);
	
	return (angle < 0.0 ? -1 : 1);
}

void process_dir(struct fm_state *fm, int dir) {
	// We need to sample dir right in the middle of each bit
	// We're using bit_phase (u16) to time this
	// Sampling must be done at 0x10000
	// Bit transitions must be at 0x8000
	// The phase accumulator is inc'd by the phase delta each sample so that the phase acc wraps at 1200bps
	// When the bit changes, see how far from 0 we are to adjust the phase accumulator
	
	// Since we're running at 20000sps, an fsk bit lasts 16.7 samples
	// So the phase delta is 0x10000 / 16.7 = 3932
	
	if (dir != fm->dir_current) {
		// State change
		fm->dir_current = dir;
		
		if (fm->phase_acc < 0x8000U-(3932/2))
			fm->phase_acc += (3932 / 8);		// Lags
		else if (fm->phase_acc > 0x8000U+(3932/2))
			fm->phase_acc -= (3932 / 8);		// Leads
		else
			fm->bit_counter++;	// DEBUG
	}
	
	fm->phase_acc += 3932;		// TODO: Change sampling rate to get integer here
	
	if (fm->phase_acc >= 0x10000U) {
		fm->phase_acc &= 0xFFFFU;

		fm->sr_data <<= 1;
		fm->sr_data |= (dir > 0 ? 0 : 1);
		
		if (fm->sr_data == 0x7CD215D8) {
			printf("Got sync !\n");
			fm->sync_status = 2;
			fm->timeout = 2000;
		//} else if (fm->sr_data == 0x7A89C197) {
			//printf("Got idle !\n");
			//fm->sync_status = 3;
		} else if (fm->sr_data == 0xAAAAAAAA) {
			//printf("Got preamble !\n");
			if (fm->sync_status == 0) printf("Got preamble on channel %u!\n", scan_index);
			fm->sync_status = 1;
			fm->timeout = 2000;
		}
	}
	
	if (!fm->timeout)
		fm->sync_status = 0;
	else
		fm->timeout--;
}

void full_demod(struct fm_state *fm) {
 	// Here, we get a buffer full of unsigned 8bit I/Q samples
	// The POCSAG signal is at -samplerate/4 (= -75000) Hz
	// Rotating by +90° makes the signal appear at 0Hz
	// It is downsampled by averaging to 20ksps before being demodulated
	
	uint32_t i, i2 = 0;
	uint8_t tmp;
	int dir;
	
	uint8_t* buf = fm->buf;
	uint32_t len = fm->buf_len;
	
	// Rotate +90°:
	
	for (i = 0; i < len; i += 8) {		// 4 samples
		tmp = 255 - buf[i + 3];
		buf[i + 3] = buf[i + 2];
		buf[i + 2] = tmp;

		buf[i + 4] = 255 - buf[i + 4];
		buf[i + 5] = 255 - buf[i + 5];

		tmp = 255 - buf[i + 6];
		buf[i + 6] = buf[i + 7];
		buf[i + 7] = tmp;
	}
	
	// Downsample:
	
	i = 0;
	while (i < len) {
		// Unsigned to signed
		fm->now_r += ((int)buf[i] - 128);	// Accumulate
		fm->now_j += ((int)buf[i + 1] - 128);
		i += 2;
		
		fm->prev_index++;
		if (fm->prev_index < fm->downsample)
			continue;						// Skip for downsampling
		
		fm->signal[i2] = fm->now_r * fm->output_scale;
		fm->signal[i2+1] = fm->now_j * fm->output_scale;
		
		fm->prev_index = 0;
		fm->now_r = 0;
		fm->now_j = 0;
		i2 += 2;
	}
	fm->signal_len = i2;
	
	//pthread_mutex_unlock(&data_write);		// Won't be using fm->buf anymore
	
	// Demodulate FSK by just figuring out the direction of rotation
	
	dir = rot_direction(fm->signal[0], fm->signal[1], fm->pre_r, fm->pre_j);
	//fm->signal2[0] = (int16_t)pcm;
	process_dir(fm, dir);
	
	// We're running at 300000 / 15 = 20ksps here
	
	for (i = 2; i < (fm->signal_len); i += 2) {
		dir = rot_direction(fm->signal[i], fm->signal[i+1], fm->signal[i-2], fm->signal[i-1]);
		//fm->signal2[i/2] = (int16_t)pcm;
		process_dir(fm, dir);
	}
	fm->pre_r = fm->signal[fm->signal_len - 2];
	fm->pre_j = fm->signal[fm->signal_len - 1];
	//fm->signal2_len = fm->signal_len/2;
}
