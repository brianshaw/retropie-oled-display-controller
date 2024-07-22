
CFLAGS=-c -Wall -O2 -D_LINUX_
LIBS = -lss_oled -lpigpio -lBitBang_I2C -lm -lpthread

AM_CFLAGS = $(JSON_CFLAGS) $(LIBUSB_CFLAGS)
AM_LDFLAGS = $(JSON_LIBS) $(LIBUSB_LIBS)

all: main

main: retropie_oled_display_controller.o libss_oled.a
	$(CC) retropie_oled_display_controller.o $(LIBS) -o retropie_oled_display_controller