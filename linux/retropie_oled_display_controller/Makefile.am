
CFLAGS=-c -Wall -O2 -D_LINUX_
LIBS = -lss_oled -lpigpio -lBitBang_I2C -lm -lpthread
CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)

AM_CFLAGS = $(JSON_CFLAGS) $(LIBUSB_CFLAGS)
AM_LDFLAGS = $(JSON_LIBS) $(LIBUSB_LIBS)

all: main

main: main.o
	$(CC) main.o $(LIBS) -o retropie_oled_display_controller


bin_PROGRAMS = main
main_SOURCES = main.c
main_LDADD = -ljson-c