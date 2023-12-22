/*
	V1.4.2
	https://github.com/mckset/KD100.git
	KD100 Linux driver for X11 desktops
	Other devices can be supported by modifying the code to read data received by the device
	At the moment, only the KD100 mini keydial is supported by this code officially
*/

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

int keycodes[] = {1, 2, 4, 8, 16, 32, 64, 128, 129, 130, 132, 136, 144, 160, 192, 256, 257, 258, 260, 641, 642};
char* file = "default.cfg";

typedef struct event event;
typedef struct wheel wheel;

struct event{
	int type;
	char* function;
};

struct wheel {
	char* right;
	char* left;
};

void GetDevice(int, int, int);
void Handler(char*, int);
char* Substring(char*, int, int);

const int vid = 0x256c;
const int pid = 0x006d;

void GetDevice(int debug, int accept, int dry){
	int err=0, wheelFunction=0, button=-1, totalButtons=0, wheelType=0, leftWheels=0, rightWheels=0, totalWheels=0;
	char* data = malloc(512*sizeof(char)); // Data received from the config file and the USB
	event* events = malloc(1*sizeof(*events)); // Stores key events and functions
	wheel* wheelEvents = malloc(1*sizeof(wheel)); // Stores wheel functions
	event prevEvent;
	uid_t uid=getuid(); // Used to check if the driver was ran as root

	// Not important
	int c=0; // Index of the loading character to display when waiting for a device

	system("clear");

	if (debug > 0){
		if (debug > 2)
			debug=2;
		printf("Version 1.4.1\nDebug level: %d\n", debug);
	}		

	// Load config file
	if (debug == 1){
		printf("Loading config...\n");
	}
	
	FILE *f;
	if (strcmp(file, "default.cfg")){
		f = fopen(file, "r");
		if (f == NULL){
                        char* home = getpwuid(getuid())->pw_dir;
                        char* config = "/.config/KD100/";
                        char temp[strlen(home)+strlen(config)+strlen(file)+1];
                        for (int i = 0; i < strlen(home); i++)
                                temp[i] = home[i];
                        for (int i = 0; i < strlen(config); i++)
                                temp[i+strlen(home)] = config[i];
                        for (int i = 0; i < strlen(file); i++)
                                temp[i+strlen(home)+strlen(config)] = file[i];
                        temp[strlen(home)+strlen(config)+strlen(file)] = '\0';
                        f = fopen(temp, "r");
                        if (f == NULL){
                                printf("CONFIG FILE NOT FOUND\n");
                                return;
                        }
                }
	}else{
		f = fopen(file, "r");
		if (f == NULL){
			char* home = getpwuid(getuid())->pw_dir;
			file = "/.config/KD100/default.cfg";
			char temp[strlen(file)+strlen(home)+1];
			for (int i = 0; i < strlen(home); i++)
				temp[i] = home[i];
			for (int i = 0; i < strlen(file); i++)
				temp[i+strlen(home)] = file[i];
			temp[strlen(home) + strlen(file)] = '\0';

			f = fopen(temp, "r");
			if (f == NULL){
				printf("DEFAULT CONFIGS ARE MISSING!\n");
				printf("Please add default.cfg to %s/.config/KD100/ or specify a file to use with -c\n", home);
				return;
			}
		}
	}
	while (fscanf(f, "%[^\n] ", data) == 1){
		for (int i = 0; i < strlen(data) && strlen(data)-6 > 0; i++){
			if (strcmp(Substring(data, i, 5), "type:") == 0 && button != -1){
				events[button].type = atoi(Substring(data, i+6, strlen(data)-(i+6)));
				break;
			}else if (strcmp(Substring(data, i, 6), "Button") == 0){
				button = atoi(Substring(data, i+7, strlen(data)-(i+7)));
				if (button >= totalButtons){
					event* temp = realloc(events, (button+1)*sizeof(*events));
					events = temp;
					totalButtons = button+1;
				}
				break;
			}else if (strcmp(Substring(data, i, 9), "function:") == 0 && button != -1){
				if (!wheelType)
					events[button].function = Substring(data, i+10, strlen(data)-(i+10));
				else if (wheelType == 1){
					if (rightWheels != 0){
						wheel* temp = realloc(wheelEvents, (rightWheels+1)*sizeof(*wheelEvents));
						wheelEvents = temp;
						wheelEvents[rightWheels].right = Substring(data, i+10, strlen(data)-(i+10));
						wheelEvents[rightWheels].left = "NULL";
					}else{
						wheelEvents[0].right = Substring(data, i+10, strlen(data)-(i+10));
						wheelEvents[0].left = "NULL";
					}
					rightWheels++;
				}else{
					if (leftWheels < rightWheels)
						wheelEvents[leftWheels].left = Substring(data, i+10, strlen(data)-(i+10));
					else{
						wheel* temp = realloc(wheelEvents, (leftWheels+1)*sizeof(*wheelEvents));
						wheelEvents = temp;
						wheelEvents[leftWheels].left = Substring(data, i+10, strlen(data)-(i+10));
						wheelEvents[leftWheels].right = "NULL";
					}
					leftWheels++;
				}	
				break;
			}else if (strcmp(Substring(data, i, 6), "Wheel ") == 0){
				wheelType++;
			}
		}
	}
	wheelFunction=0;
	if (rightWheels > leftWheels)
		totalWheels = rightWheels;
	else
		totalWheels = leftWheels;

	if (debug > 0){
		for (int i = 0; i < totalButtons; i++)
			printf("Button: %d | Type: %d | Function: %s\n", i, events[i].type, events[i].function);
		printf("\n");
		for (int i = 0; i < totalWheels; i++)
			printf("Wheel Right: %s | Wheel Left: %s\n", wheelEvents[i].right, wheelEvents[i].left);
		printf("\n");
	}
	free(data);
	int devI = 0;
	char indi[] = "|/-\\";
	while (err == 0 || err == LIBUSB_ERROR_NO_DEVICE){
		libusb_device **devs; // List of USB devices
		libusb_device *dev; // Selected USB device
		struct libusb_config_descriptor *desc; // USB description (For claiming interfaces)
		libusb_device_handle *handle = NULL; // USB handle

		err = libusb_get_device_list(NULL, &devs);
		if (err < 0){
			printf("Unable to retrieve USB devices. Exitting...\n");
			return;
		}

		// Gets a list of devices and looks for ones that have the same vid and pid
		int d=0, found=0;
		devI=0;
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
					if (uid != 0){
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
					}else{ // If the driver is ran as root, it can safely execute the following
						err = libusb_open(dev, &handle);
						if (err < 0){
							printf("\nUnable to open device. Error: %d\n", err);
							handle=NULL;
						}
						err = libusb_get_string_descriptor_ascii(handle, devDesc.iProduct, info, 200);
						if (debug > 0){
							printf("\n#%d | %04x:%04x : %s\n", d, vid, pid, info);
						}
						if (strlen(info) == 0 || strcmp("Huion Tablet_KD100", info) == 0){
							break;
						}else{
							libusb_close(handle);
							handle = NULL;
							found++;
						}
					}
				}else{
					savedDevs[devI] = dev;
					devI++;
				}
			}
		}
		if (accept == 0){
			int in=-1;
			while(in == -1){
				char buf[64];
				printf("\n");
				system("lsusb");
				printf("\n");
				for(d=0; d < devI; d++){
					printf("%d) %04x:%04x (Bus: %03d Device: %03d)\n", d, vid, pid, libusb_get_bus_number(savedDevs[d]), libusb_get_device_address(savedDevs[d]));
				}
				printf("Select a device to use: ");
				fflush(stdout);
				fgets(buf, 10, stdin);
				in = atoi(buf);
				if (in >= devI || in < 0){
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
		}else if (found > 0){
			printf("Error: Found device does not appear to be the keydial\n");
			printf("Try running without the -a flag\n");
			return;
		}

		int interfaces=0;
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

			interfaces = 0;
			printf("Starting driver...\n");

			// Read device and claim interfaces
			dev = libusb_get_device(handle);
			libusb_get_config_descriptor(dev, 0, &desc);
			interfaces = desc->bNumInterfaces;
			libusb_free_config_descriptor(desc);
			libusb_set_auto_detach_kernel_driver(handle, 1);
			if (debug == 1)
				printf("Claiming interfaces... \n");

			for (int x = 0; x < interfaces; x++){
				libusb_kernel_driver_active(handle, x);
				int err = libusb_claim_interface(handle, x);
				if (err != LIBUSB_SUCCESS && debug == 1)
					printf("Failed to claim interface %d\n", x);
			}

			printf("Driver is running!\n");

			err = 0;
			prevEvent.function = "";
			prevEvent.type = 0;
			while (err >=0){ // Listen for events
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
				if (dry)
					keycode = 0;

				// Compare keycodes to data and trigger events
				if (debug == 1 && keycode != 0){
					printf("Keycode: %d\n", keycode);
				}
				if (keycode == 0 && prevEvent.type != 0){ // Reset key held
					Handler(prevEvent.function, prevEvent.type);
					prevEvent.function = "";
					prevEvent.type = 0;
				}
				if (keycode == 641){ // Wheel clockwise
					Handler(wheelEvents[wheelFunction].right, -1);
				}else if (keycode == 642){ // Counter clockwise
					Handler(wheelEvents[wheelFunction].left, -1);
				}else{
					for (int k = 0; k < 19; k++){
						if (keycodes[k] == keycode){
							if (events[k].function){
								if (strcmp(events[k].function, "NULL") == 0){
									if (prevEvent.type != 0){
										Handler(prevEvent.function, prevEvent.type);
										prevEvent.type = 0;
										prevEvent.function = "";
									}
									break;
								}
								if (events[k].type == 0){
									if (strcmp(events[k].function, prevEvent.function)){
										if (prevEvent.type != 0){
											Handler(prevEvent.function, prevEvent.type);
										}
										prevEvent.function = events[k].function;
										prevEvent.type=1;
									}
									Handler(events[k].function, 0);
								}else if (strcmp(events[k].function, "swap") == 0){
									if (wheelFunction != totalWheels-1){
										wheelFunction++;
									}else
										wheelFunction=0;
									if (debug == 1){
										printf("Function: %s | %s\n", wheelEvents[wheelFunction].left, wheelEvents[wheelFunction].right);
									}
								}else if (strcmp(events[k].function, "mouse1") == 0 || strcmp(events[k].function, "mouse2") == 0 || strcmp(events[k].function, "mouse3") == 0 || strcmp(events[k].function, "mouse4") == 0 || strcmp(events[k].function, "mouse5") == 0){
									if (strcmp(events[k].function, prevEvent.function)){
										if (prevEvent.type != 0){
											Handler(prevEvent.function, prevEvent.type);
										}
										prevEvent.function = events[k].function;
										prevEvent.type=3;
									}
									Handler(events[k].function, 2);
								}else{
									system(events[k].function);
								}
								break;
							}
						}
					}
				}

				if(debug == 2 || dry){
					printf("DATA: [%d", data[0]);
					for (int i = 1; i < sizeof(data); i++){
						printf(", %d", data[i]);
					}
					printf("]\n");
				}
			}

			// Cleanup
			for (int x = 0; x<interfaces; x++) {
				if (debug == 1){
					printf("Releasing interface %d...\n", x);
				}
				libusb_release_interface(handle, x);
			}
			printf("Closing device...\n");
			libusb_close(handle);
			interfaces=0;
			sleep(1); // Buffer to wait in case the device was disconnected
		}
	}
}


void Handler(char* key, int type){
	if (strcmp(key, "NULL") == 0)
		return;

	char* cmd = "";
	char mouse = 'a';

	if (type < 2){
		if (type == 0)
			cmd = "xdotool keydown ";
		else if (type == 1)
			cmd = "xdotool keyup ";	
		else
			cmd = " xdotool key ";		
	}else{
		if (type == 2)
			cmd = "xdotool mousedown ";
		else if (type == 3)
			cmd = "xdotool mouseup ";
		mouse = key[5];
	}
	if (cmd){
		char temp[strlen(cmd)+strlen(key)+1];
		for (int i = 0; i < strlen(cmd); i++)
			temp[i] = cmd[i];
		if (mouse == 'a'){
			for (int i = 0; i < strlen(key); i++)
				temp[i+strlen(cmd)] = key[i];
			temp[strlen(cmd)+strlen(key)] = '\0';
		}else{
			temp[strlen(cmd)] = ' ';
			temp[strlen(cmd)+1] = mouse;
			temp[strlen(cmd)+2] = '\0';
		}
		system(temp);
	}
}

char* Substring(char* in, int start, int end){
	if (start > strlen(in) || end + start > strlen(in)){
		return in;
	}
	char* out = malloc(end+1);
	for (int i = 0; i < end; i++)
		out[i] = in[i+start];
	out[end] = '\0';
	return out;
}


int main(int args, char *in[]){
	int debug=0, accept=0, dry=0, err;

	err = system("xdotool sleep 0.01");
	if (err != 0){
		printf("Exitting...\n");
		return -9;
	}

	for (int arg = 1; arg < args; arg++){
		if (strcmp(in[arg],"-h") == 0 || strcmp(in[arg],"--help") == 0){
			printf("Usage: KD100 [option]...\n");
			printf("\t-a\t\tAssume the first device that matches %04x:%04x is the Keydial\n", vid, pid);
			printf("\t-c [path]\tSpecifies a config file to use\n");
			printf("\t-d [-d]\t\tEnable debug outputs (use twice to view data sent by the device)\n");
			printf("\t-dry \t\tDisplay data sent by the device without sending events\n");
			printf("\t-h\t\tDisplays this message\n\n");
			return 0;
		}
		if (strcmp(in[arg],"-d") == 0){
			debug++;
		}
		if (strcmp(in[arg],"-dry") == 0){
			dry=1;
		}
		if (strcmp(in[arg],"-a") == 0){
			accept=1;
		}
		if (strcmp(in[arg], "-c") == 0){
			if (in[arg+1]){
				file = in[arg+1];
				printf("%s\n", file);
				arg++;
			}else{
				printf("No config file specified. Exiting...\n");
				return -8;
			}
		}
	}

	libusb_context *ctx;
	err = libusb_init(&ctx);
	if (err < 0){
		printf("Error: %d\n", err);
		return err;
	}
	// Uncomment to enable libusb debug messages
	// libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 1);
	GetDevice(debug, accept, dry);
	libusb_exit(ctx);
	return 0;
}
