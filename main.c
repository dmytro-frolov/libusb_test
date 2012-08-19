/* demo usb device-controlling program.
   (c) Andrei Borovsky
   You can find more information on http://symmetrica.net/ */ 

#define LINUX

#ifdef WINDOWS
#include "libusb.h"
#endif
#ifdef LINUX
#include <libusb-1.0/libusb.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEV_VID 0x1D34
#define DEV_PID 0x0004
#define DEV_CONFIG 1
#define DEV_INTF 0
#define EP_IN 0x81
#define EP_OUT 0x01

unsigned char COMMAND_1[8] = {0x1F,0x1E,0x92,0x7C,0xB8,0x1,0x14,0x03};
unsigned char COMMAND_2[8] = {0x00,0x1E,0x92,0x7C,0xB8,0x1,0x14,0x04};
unsigned char COMMAND_ON[8] = {0x00,0x00,0xaa,0x00,0x0,0x1,0x14,0x05};
int main(int argc, char * argv[])
{
	libusb_device_handle * handle;
	int ret;
	char r, g, b;
	unsigned char buf[8];
	if (argc != 4) {
		printf("Использование: %s r g b\n", argv[0]);
		return -1;
	}
	r = atoi(argv[1]);
	g = atoi(argv[2]);
	b = atoi(argv[3]);
	libusb_init(NULL);
	libusb_set_debug(NULL, 3);
	handle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);
	if (handle == NULL) {
	    printf("Не удалось найти устройство\n");
	    libusb_exit(NULL);
	    return 0;
	}
	if (libusb_kernel_driver_active(handle,DEV_INTF))
		libusb_detach_kernel_driver(handle, DEV_INTF);
	if ((ret = libusb_set_configuration(handle, DEV_CONFIG)) < 0)
	{
		printf("Ошибка конфигурации\n");
		libusb_close(handle);
		libusb_exit(NULL);
		if (ret == LIBUSB_ERROR_BUSY)
		    printf("B\n");
		printf("ret:%i\n", ret);    
		return 0;
	}
	if (libusb_claim_interface(handle,  DEV_INTF) < 0)
	{
		printf("Ошибка интерфейса\n");
		libusb_close(handle);
		libusb_exit(NULL);
		return 0;
	}
	ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT, 0x9, 0x200, 0, COMMAND_1, 8, 100);
	libusb_interrupt_transfer(handle, EP_IN, buf, 8, &ret, 100);
	ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT, 0x9, 0x200, 0, COMMAND_2, 8, 100);
	libusb_interrupt_transfer(handle, EP_IN, buf, 8, &ret, 100);
	COMMAND_ON[0] = r;
	COMMAND_ON[1] = g;
	COMMAND_ON[2] = b;
	ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT, 0x9, 0x200, 0, COMMAND_ON, 8, 100);
	buf[7] = 0;
	libusb_interrupt_transfer(handle, EP_IN, buf, 8, &ret, 100);
	if (buf[7] != 1) {
		printf("Сбой в управлении устройством\n");
		libusb_close(handle);
		libusb_exit(NULL);
		return 0;
	}
	libusb_attach_kernel_driver(handle, DEV_INTF);
	libusb_close(handle);
	libusb_exit(NULL);
	return 0;
}
