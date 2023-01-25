#include <xdo.h>
#include <stdio.h>
#include <string.h>

int main(int args, char *in[]){
	xdo_t *x = xdo_new(NULL);
	//charcodemap_t *active_mods = NULL;
	//int mods = 0;
	//xdo_get_active_modifiers(x, &active_mods, &mods);
	xdo_send_keysequence_window(x, CURRENTWINDOW, in[1], 0);
	//xdo_clear_active_modifiers(x, CURRENTWINDOW, active_mods, mods);

	return 0;
}


