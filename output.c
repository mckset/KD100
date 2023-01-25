#include <xdo.h>
#include <stdio.h>

int main(int args, char *in[]){
	xdo_t *x = xdo_new(NULL);
	xdo_send_keysequence_window(x, CURRENTWINDOW, in[1], 0);
	return 0;
}


