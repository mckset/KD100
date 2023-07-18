/*
	V1.31
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

int keycodes[] = {1, 2, 4, 8, 16, 32, 64, 128, 129, 130, 132, 136, 144, 160, 192, 256, 257, 258, 260, 641, 642};
char file[4096] = "default.cfg";


void GetDevice(int, int);
void Handler(char*, int);

const int vid = 0x256c;
const int pid = 0x006d;


void GetDevice(int debug, int accept){
	int i=0, err=0, l=0, subL=0, wheelFunction=0, c=0;
	char data[512]; // Data received from the config file and the USB
	int type[19]; // Stores the button type
	char events[19][256]; // Stores the button event key/function
	char wheelEvents[6][256]; // Stores the wheel keys
	int prevType = 0; // prevKey function type
	char prevKey[256]; // Stores the previous event to release held keys
	uid_t uid=getuid(); // Used to check if the driver was ran as root

	system("clear");

	if (debug > 0){
		if (debug > 2)
			debug=2;
		printf("Version 1.31\nDebug level: %d\n", debug);
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
		libusb_device **devs; // List of USB devices
		libusb_device *dev; // Selected USB device
		struct libusb_config_descriptor *desc; // USB descrition (For claiming interfaces)
		libusb_device_handle *handle = NULL; // USB handle

		err = libusb_get_device_list(NULL, &devs);
		if (err < 0){
			printf("Unable to retrieve USB devices. Exitting...\n");
			return;
		}

		// Gets a list of devices and looks for ones that have the same vid and pid
		int d=0;
		i=0;
		libusb_device *savedDevs[sizeof(devs)];
		while ((dev = devs[d++]) != NULL){
			struct libusb_device_descriptor devDesc;
			unsigned char info[200] = "";
			err = libusb_get_device_descriptor(dev, &devDesc);
			if (err < 0){
				if (debug > 0){
					printf("Unable to retrieve info from device #%d. Ignoring...\n", d);
				}
			}else if (devDesc.idVendor == vid && devDesc.idProduct == pid){
				if (accept == 1){
					err=libusb_open(dev, &handle);
					if (err < 0){
						printf("\nUnable to open device. Error: %d\n", err);
						handle=NULL;
						if (err == LIBUSB_ERROR_ACCESS){
							printf("Error: Permission denied\n");
							return;
						}
					}
					if (debug > 0){
						printf("\nUsing: %04x:%04x (Bus: %03d Device: %03d)\n", vid, pid, libusb_get_bus_number(dev), libusb_get_device_address(dev));
					}
					break;
				}else{
					if (uid == 0){ // If the driver is ran as root, it can safely execute the following
						err = libusb_open(dev, &handle);
						if (err < 0){
							printf("\nUnable to open device. Error: %d\n", err);
							handle=NULL;
						}
						err = libusb_get_string_descriptor_ascii(handle, devDesc.iProduct, info, 200);
						if (debug > 0){
							printf("\n#%d | %04x:%04x : %s\n", d, vid, pid, info);
						}
						if (strlen(info) == 0){
							break;
						}else{
							libusb_close(handle);
							handle = NULL;
						}
					}else{
						savedDevs[i] = dev;
						i++;
					}
				}
			}
		}

		if (i > 0){
			int in=-1;
			while(in == -1){
				char buf[64];
				printf("\n");
				system("lsusb");
				printf("\n");
				for(d=0; d < i; d++){
					printf("%d) %04x:%04x (Bus: %03d Device: %03d)\n", d, vid, pid, libusb_get_bus_number(savedDevs[d]), libusb_get_device_address(savedDevs[d]));
				}
				printf("Select a device to use: ");
				fflush(stdout);
				fgets(buf, 10, stdin);
				in = atoi(buf);
				if (in >= i || in < 0){
					in=-1;
				}
				system("clear");
			}
			err=libusb_open(savedDevs[in], &handle);
			if (err < 0){
				printf("Unable to open device. Error: %d\n", err);
				handle=NULL;
				if (err == LIBUSB_ERROR_ACCESS){
					printf("Error: Permission denied\n");
					return;
				}
			}
		}

		

		i=0;
		if (handle == NULL){
			printf("\rWaiting for a device %c", indi[c]);
			fflush(stdout);
			usleep(250000); // Buffer
			c++;
			if (c == 4){
				c=0;
			}
			err = LIBUSB_ERROR_NO_DEVICE;
		}else{ // Claims the device and starts the driver
			if (debug == 0){
				system("clear");
			}

			i = 0;
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


			err = 0;
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
				printf("\nDEVICE IS ALREADY IN USE\n");
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
			sleep(1); // Buffer to wait in case the device was disconnected
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
	int d=0, a=0, err;

	err = system("xdotool sleep 0.01");
	if (err != 0){
		printf("Exitting...\n");
		return -1;
	}

	for (int arg = 1; arg < args; arg++){
		if (strcmp(in[arg],"-h") == 0 || strcmp(in[arg],"--help") == 0){
			printf("Usage: KD100 [option]...\n");
			printf("\t-a\t\tAssume the first device that matches %04x:%04x is the Keydial\n", vid, pid);
			printf("\t-c [path]\tSpecifies a config file to use\n");
			printf("\t-d [-d]\t\tEnable debug outputs (use twice to view data sent by the device)\n");
			printf("\t-h\t\tDisplays this message\n\n");
			return 0;
		}
		if (strcmp(in[arg],"-d") == 0){
			d++;
		}
		if (strcmp(in[arg],"-a") == 0){
			a=1;
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
	// Uncomment for more detailed debugging (might crash when using older version of libusb)
	//libusb_set_option(*ctx, LIBUSB_OPTION_LOG_LEVEL, 0);
	GetDevice(d, a);
	libusb_exit(*ctx);
	return 0;
}
