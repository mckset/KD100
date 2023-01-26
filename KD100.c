/*
	V1.1
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

int keycodes[] = {1,2,4,8,16,32,64,128, 129, 130, 132, 136, 144, 160, 192, 256, 257, 258, 260, 641, 642};
char file[4096] = "default.cfg";


static void GetDevice(int, int, int);

void GetDevice(int v, int p, int debug){
	libusb_device *dev; //USB device
	libusb_device_handle *handle; // USB handle
	struct libusb_config_descriptor *desc; // USB descrition (For claiming interfaces)
	int i = 0, err = 0, l = 0, subL = 0, wheelFunction = 0;
	char data[512];
	int type[19];
	char events[19][256];
	char wheelEvents[6][256];
	
	system("clear");

	if (debug > 0){
		if (debug > 2)
			debug=2;
		printf("Debug level: %d\n", debug);
	}

	// Load config file
	if (debug == 1){
		printf("Loading config...\n");
	}
	FILE *f = fopen(file, "r");
	if (f == NULL){
		printf("FILE NOT FOUND\n");
		return;
	}

	while (fscanf(f, "%[^\n] ", data) == 1){
		if (l > 19 && l <= 76){ // Button functions
			if (subL == 0){
				type[i] = atoi(data);
				subL = 1;
			}else if (subL == 1){
				strcpy(events[i], data);
				subL = -1;
				i++;
			}else{
				subL+=1;
			}
		}else if (l > 76){ // Wheel functions
			if (subL < 3){
				strcpy(wheelEvents[wheelFunction], data);
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
		handle = libusb_open_device_with_vid_pid(NULL, v, p);		
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
			char prevEvent[521] = "";
			printf("Driver is running!\n");
			while (err >=0){
				unsigned char data[40]; // Stores device input
				int keycode = 0; // Keycode read from the device
				char event[521] = "./output "; // Key command to be sent to the PC
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
				if (keycode == 0 && strlen(prevEvent) > 0){ // Reset key held
					strcat(prevEvent, " 1");
					system(prevEvent);
					strcpy(prevEvent, "");
				}
				if (keycode == 641){ // Wheel Clockwise
					strcat(event, wheelEvents[wheelFunction]);
					system(event);
				}else if (keycode == 642){
					strcat(event, wheelEvents[wheelFunction + 3]);
					system(event);
				}else{
					for (int k = 0; k < 19; k++){
						if (keycodes[k] == keycode){
							if (type[k] == 0){
								strcat(event, events[k]);
								if (strlen(prevEvent) > 0 && strcmp(event, prevEvent) != 0){
									strcat(prevEvent, " 1");
									system(prevEvent);
								}
								strcpy(prevEvent, event);
								strcat(event, " 0");
								system(event);
							}else if (strcmp(events[k], "swap") == 0){
								if (wheelFunction != 2){
									wheelFunction++;
									if (strcmp(wheelEvents[wheelFunction],"NULL") == 0){
										wheelFunction = 0;
									}
									if (debug == 1){
										printf("Function: %s\n", wheelEvents[wheelFunction]);
									}
								}else
									wheelFunction=0;
							}else if (strcmp(events[k], "mouse1") == 0 || strcmp(events[k], "mouse2") == 0 || strcmp(events[k], "mouse3") == 0 || strcmp(events[k], "mouse4") == 0 || strcmp(events[k], "mouse5") == 0){
								strcat(event, events[k]);
								if (strlen(prevEvent) > 0 && strcmp(event, prevEvent) != 0){
									strcat(prevEvent, " 1");
									system(prevEvent);
								}
								strcpy(prevEvent, event);
								strcat(event, " 0");
								system(event);	
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


int main(int args, char *in[])
{
	int d=0;
	for (int arg = 1; arg < args; arg++){
		if (strcmp(in[arg],"-d") == 0){
			d++;
		}
		if (strcmp(in[arg], "-c") == 0){
			if (strlen(in[arg+1]) > 0 && strlen(in[arg+1]) < 4096){
			strcpy(file, in[arg+1]);
			arg++;
			}
		}
	}


	libusb_context **ctx;
	int err;
	
	err = libusb_init(ctx);
	if (err < 0){
		printf("Error: %d\n", err);
		return err;
	}
	libusb_set_option(*ctx, LIBUSB_OPTION_LOG_LEVEL, 0);
	GetDevice(0x256c, 0x006d, d);
	libusb_exit(*ctx);
	return 0;
}
