/*
	V1.2
	https://github.com/mckset/KD100.git
	KD 100 Linux driver for X11 desktops
	Other devices can be supported by modifying the code to read data received by the device
	At the moment, only the KD100 mini keypad is supported by this code officially
*/

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <pwd.h>

int keycodes[] = {1,2,4,8,16,32,64,128, 129, 130, 132, 136, 144, 160, 192, 256, 257, 258, 260, 641, 642};
char file[4096] = "default.cfg";


void GetDevice(int);
void Handler(char*, int);

const int vid = 0x256c;
const int pid = 0x006d;


void GetDevice(int debug){
	libusb_device *dev; //USB device
	libusb_device_handle *handle; // USB handle
	struct libusb_config_descriptor *desc; // USB descrition (For claiming interfaces)
	int i = 0, err = 0, l = 0, subL = 0, wheelFunction = 0;
	char data[512];
	int type[19];
	char events[19][256];
	char wheelEvents[6][256];
	int prevType = 0;
	char prevKey[256];
	
	system("clear");

	if (debug > 0){
		if (debug > 2)
			debug=2;
		printf("Version 1.2\nDebug level: %d\n", debug);
	}

	// Load config file
	if (debug == 1){
		printf("Loading config...\n");
	}
	FILE *f;
	if (strcmp(file, "default.cfg")){
		f = fopen(file, "r");
		if (f == NULL){
			printf("CONFIG FILE NOT FOUND\n");
			return;
		}
	}else{
		f = fopen(file, "r");
		if (f == NULL){
			strcpy(file, getpwuid(getuid())->pw_dir);
			strcat(file, "/.config/KD100/default.cfg");
			f = fopen(file, "r");
			if (f == NULL){
				printf("DEFAULT CONFIGS ARE MISSING!\n");
				printf("Please add default.cfg to %s/.config/KD100/ or specify a file to use with -c\n", getpwuid(getuid())->pw_dir);
				return;
			}
		}
	}
	while (fscanf(f, "%[^\n] ", data) == 1){
		if (l > 19 && l <= 76){ // Button functions
			if (subL == 0){
				type[i] = atoi(&data[5]);
				subL = 1;
			}else if (subL == 1){
				char func[256];
				if (strlen(data) < 256){
					for (int d = 10; d < sizeof(data); d++){
						func[d-10] = data[d];
					}
				}else{
					strcpy(func, "NULL");
					printf("Line: %d - Function length exceeded 256 characters. Ignoring %s\n", l, data);
				}
				strcpy(events[i], func);
				subL = -1;
				i++;
			}else{
				subL+=1;
			}
		}else if (l > 76 && l < 85){ // Wheel functions
			if (subL < 3){
				char func[256];
				for (int d = 12; d < sizeof(data); d++){
					func[d-12] = data[d];
				}
				strcpy(wheelEvents[wheelFunction], func);
				wheelFunction++;
				subL++;
			}else{
				subL=0;
			}
		}
		l++;
	}
	wheelFunction=0;

	i = 0;
	char indi[] = "|/-\\";
	while (err == 0 || err == LIBUSB_ERROR_NO_DEVICE){
		err=0;
		// Open device and check if it is there
		handle = libusb_open_device_with_vid_pid(NULL, vid, pid);		
		if (handle == NULL){
			printf("\rWaiting for a device %c", indi[i]);
			fflush(stdout);
			usleep(250000);
			i++;
			if (i == 4){
				i=0;
			}
		}else{
			if (debug == 0){
				system("clear");
			}

			i = 0;
			if (debug == 1)
				printf("Device found...\n");
			printf("Starting driver...\n");

			// Read device and claim interfaces
			dev = libusb_get_device(handle);
			libusb_get_config_descriptor(dev, 0, &desc);
			i = desc->bNumInterfaces;
			libusb_free_config_descriptor(desc);
			libusb_set_auto_detach_kernel_driver(handle, 1);
			if (debug == 1)
				printf("Claiming interfaces... \n");

			for (int x = 0; x < i; x++){
				libusb_kernel_driver_active(handle, x);
				int err = libusb_claim_interface(handle, x);
				if (err != LIBUSB_SUCCESS && debug == 1) 
					printf("Failed to claim interface %d\n", x);
			}

			// Listen for events
			printf("Driver is running!\n");
			while (err >=0){
				unsigned char data[40]; // Stores device input
				int keycode = 0; // Keycode read from the device
				err = libusb_interrupt_transfer(handle, 0x81, data, sizeof(data), NULL, 0); // Get data
				
				// Potential errors			
				if (err == LIBUSB_ERROR_TIMEOUT)
					printf("\nTIMEDOUT\n");
				if (err == LIBUSB_ERROR_PIPE)
					printf("\nPIPE ERROR\n");
				if (err == LIBUSB_ERROR_NO_DEVICE)
					printf("\nDEVICE DISCONNECTED\n");
				if (err == LIBUSB_ERROR_OVERFLOW)
					printf("\nOVERFLOW ERROR\n");
				if (err == LIBUSB_ERROR_INVALID_PARAM)
					printf("\nINVALID PARAMETERS\n");
				if (err == -1)
					printf("\nDRIVER IS ALREADY RUNNING\n");
				if (err < 0){
					if (debug == 1){
						printf("Unable to retrieve data: %d\n", err);
					}			
					break;		
				}

				// Convert data to keycodes
				if (data[4] != 0)
					keycode = data[4];
				else if (data[5] != 0)
					keycode = data[5] + 128;
				else if (data[6] != 0)
					keycode = data[6] + 256;
				if (data[1] == 241)
					keycode+=512;

				// Compare keycodes to data and trigger events
				if (debug == 1 && keycode != 0){
					printf("Keycode: %d\n", keycode);
				}
				if (keycode == 0 && prevType != 0){ // Reset key held
					Handler(prevKey, prevType);
					strcpy(prevKey, "");
					prevType=0;
				}
				if (keycode == 641){ // Wheel Clockwise
					Handler(wheelEvents[wheelFunction], -1);
				}else if (keycode == 642){
					Handler(wheelEvents[wheelFunction + 3], -1);
				}else{
					for (int k = 0; k < 19; k++){
						if (keycodes[k] == keycode){
							if (strcmp(events[k], "NULL") == 0){
								if (prevType != 0){
									Handler(prevKey, prevType);
									prevType = 0;
									strcpy(prevKey, "");
								}
								break;	
							}
							if (type[k] == 0){
								if (strcmp(events[k], prevKey)){
									if (prevType != 0){
										Handler(prevKey, prevType);
									}
									strcpy(prevKey, events[k]);
									prevType=1;
								}
								Handler(events[k], 0);
							}else if (strcmp(events[k], "swap") == 0){
								if (wheelFunction != 2){
									wheelFunction++;
									if (strcmp(wheelEvents[wheelFunction], "NULL") == 0){
										wheelFunction = 0;
									}
									if (debug == 1){
										printf("Function: %s\n", wheelEvents[wheelFunction]);
									}
								}else
									wheelFunction=0;
							}else if (strcmp(events[k], "mouse1") == 0 || strcmp(events[k], "mouse2") == 0 || strcmp(events[k], "mouse3") == 0 || strcmp(events[k], "mouse4") == 0 || strcmp(events[k], "mouse5") == 0){
								if (strcmp(events[k], prevKey)){
									if (prevType != 0){
										Handler(prevKey, prevType);
									}
									strcpy(prevKey, events[k]);
									prevType=3;
								}
								Handler(events[k], 2);
							}else{
								system(events[k]);
							}
							break;
						}
					}
				}

				if(debug == 2){
					printf("DATA: [%d", data[0]);
					for (int i = 1; i < sizeof(data); i++){
						printf(", %d", data[i]);
					}
					printf("]\n");
				}
			}	

			
			// Cleanup
			for (int x = 0; x<i; x++) {
				if (debug == 1){
					printf("Releasing interface %d...\n", x);
				}				
				libusb_release_interface(handle, x);
			}
			printf("Closing device...\n");
			libusb_close(handle);
			i=0;
		}	
	}
}

void Handler(char *key, int type){
	char cmd[529] = "xdotool key";
	if (type < 2){
		if (type == 0){
			strcat(cmd, "down");
		}else if (type == 1){
			strcat(cmd, "up");
		}
		strcat(cmd, " ");
		strcat(cmd, key);
	}else{
		if (type == 2){
			strcpy(cmd,"xdotool mousedown ");
		}else if (type == 3){
			strcpy(cmd,"xdotool mouseup ");
		}
		strcat(cmd, " ");
		strcat(cmd, &key[5]);
	}
	system(cmd);
}

int main(int args, char *in[])
{
	int d=0, err;
	
	err = system("xdotool sleep 0.01");
	if (err != 0){
		printf("Exitting...\n");
		return -1;
	}

	for (int arg = 1; arg < args; arg++){
		if (strcmp(in[arg],"-d") == 0){
			d++;
		}
		if (strcmp(in[arg], "-c") == 0){
			if (strlen(in[arg+1]) > 0){
				strcpy(file, in[arg+1]);
				arg++;
			}
		}
	}

	libusb_context **ctx;

	err = libusb_init(ctx);
	if (err < 0){
		printf("Error: %d\n", err);
		return err;
	}
	libusb_set_option(*ctx, LIBUSB_OPTION_LOG_LEVEL, 0);
	GetDevice(d);
	libusb_exit(*ctx);
	return 0;
}
