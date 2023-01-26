#include <xdo.h>
#include <stdio.h>
#include <string.h>

int main(int args, char *in[]){
	xdo_t *x = xdo_new(NULL);
	if (strcmp(in[2], "0") == 0){		
		xdo_send_keysequence_window_down(x, CURRENTWINDOW, in[1], 0);
	}else{
		xdo_send_keysequence_window_up(x, CURRENTWINDOW, in[1], 0);
	}
	return 0;
}

