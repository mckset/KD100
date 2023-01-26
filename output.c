#include <xdo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int args, char *in[]){
	xdo_t *x = xdo_new(NULL);
	if (strcmp(in[2], "0") == 0){
		if (strcmp(in[1], "mouse1") != 0 &&	strcmp(in[1], "mouse2") != 0 && strcmp(in[1], "mouse3") != 0 && strcmp(in[1], "mouse4") != 0 && strcmp(in[1], "mouse5") != 0){
			xdo_send_keysequence_window_down(x, CURRENTWINDOW, in[1], 0);
		}else{
			xdo_mouse_down(x, CURRENTWINDOW, atoi(&in[1][5]));
		}
	}else{
		if (strcmp(in[1], "mouse1") != 0 &&	strcmp(in[1], "mouse2") != 0 && strcmp(in[1], "mouse3") != 0 && strcmp(in[1], "mouse4") != 0 && strcmp(in[1], "mouse5") != 0){
			xdo_send_keysequence_window_up(x, CURRENTWINDOW, in[1], 0);
		}else{
			xdo_mouse_up(x, CURRENTWINDOW, atoi(&in[1][5]));
		}
	}
	return 0;
}
