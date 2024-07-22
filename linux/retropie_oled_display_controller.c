//
// multi oled test program
// Written by Larry Bank
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ss_oled.h"
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event)) // Add this line to declare the missing variable
#define BUF_LEN (1024 * (EVENT_SIZE + 16)) // Add this line to declare the missing variable

SSOLED ssoled[2]; // data structure for 2 OLED objects
unsigned char ucBackBuf[1024];
char* folderToWatch = "displayTexts";

void updateDisplay(); // Add this line to provide a function prototype
void watchDisplayUpdate(); // Add this line to provide a function prototype

int main(int argc, char *argv[])
{
  fprintf(stdout, "main");
int i;
int iOLEDAddr = -1; // typical address; it can also be 0x3d
int iOLEDType0 = OLED_128x64; // Change this for your specific display
int iOLEDType1 = OLED_64x32;
int bFlip = 0, bInvert = 0, bWire = 1;



    watchDisplayUpdate();



// For hardware I2C on the RPI, the clock rate is fixed and set in the
// /boot/config.txt file, so we pass 0 for the bus speed
    // i=oledInit(&ssoled[0], iOLEDType0, iOLEDAddr, bFlip, bInvert, bWire, 1, -1, -1, 0); // initialize 128x64 oled on I2C channel 1
    i=oledInit(&ssoled[0], iOLEDType0, iOLEDAddr, bFlip, bInvert, bWire, 4, 9, 8, 0); // initialize 128x64 oled on I2C channel 1
  if (i != OLED_NOT_FOUND)
  {
                char *msgs[] = {(char *)"SSD1306 @ 0x3C", (char *)"SSD1306 @ 0x3D",(char *)"SH1106 @ 0x3C",(char *)"SH1106 @ 0x3D"};
    printf("Successfully opened I2C bus 1\n");
                oledSetBackBuffer(&ssoled[0], ucBackBuf);
    oledFill(&ssoled[0], 0,1); // fill with black
    oledWriteString(&ssoled[0], 0,0,0, msgs[i], FONT_NORMAL,0,1);
    oledWriteString(&ssoled[0], 0,0,1,"SS_OLED Library!",FONT_NORMAL,0,1);
    oledWriteString(&ssoled[0], 0,3,2,"BIG!",FONT_LARGE,0,1);
    oledWriteString(&ssoled[0], 0,0,5,"Small", FONT_SMALL,0,1);
    // oledWriteString(&ssoled[0], 0,0,6,argv[1], FONT_SMALL,0,1);
    for (i=0; i<64; i++)
    {
      oledSetPixel(&ssoled[0], i, 16+i, 1, 1);
      oledSetPixel(&ssoled[0], 127-i, 16+i, 1, 1);
    }
    
    oledInit(&ssoled[1], iOLEDType1, iOLEDAddr, bFlip, bInvert, bWire, 6, -1, -1, 0); // initialize the 64x32 oled on I2C channel 6
    oledFill(&ssoled[1], 0, 1);
    oledWriteString(&ssoled[1], 0,0,0,"OLED on", FONT_NORMAL, 0, 1);
    oledWriteString(&ssoled[1], 0,0,1,"I2C ch 6", FONT_NORMAL, 0, 1);
    
    printf("Press ENTER to quit\n");
    
    getchar();
    oledPower(&ssoled[0], 0); // turn off both displays
    oledPower(&ssoled[1], 0);

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);
  }
  else
  {
    printf("Unable to initialize I2C bus 0-2, please check your connections and verify the device address by typing 'i2cdetect -y <channel>\n");
  }

  
   return 0;
} /* main() */

// void updateDisplay() {
//   FILE *file = fopen(fileToWatch, "r");
//   if (file == NULL) {
//     perror("fopen");
//     exit(EXIT_FAILURE);
//   }

//   char line[256];
//   while (fgets(line, sizeof(line), file)) {
//     // Process each line of the file
//     // ...
//     // oledWriteString(&ssoled[0], 0,0,6,line, FONT_SMALL,0,1);
//     printf("%s", line);
//   }

//   fclose(file);
// }

void watchDisplayUpdate() {
  int length, ifile = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];

    printf("Watching \n");
    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
    }

    // wd = inotify_add_watch(fd, ".",
    wd = inotify_add_watch(fd, folderToWatch,
        IN_MODIFY | IN_CREATE | IN_DELETE);
    length = read(fd, buffer, BUF_LEN);

    if (length < 0) {
        perror("read");
    }

    while (ifile < length) {
        struct inotify_event *event =
            (struct inotify_event *) &buffer[ifile];
        if (event->len) {
            if (event->mask & IN_CREATE) {
                printf("The file %s was created.\n", event->name);
            } else if (event->mask & IN_DELETE) {
                printf("The file %s was deleted.\n", event->name);
            } else if (event->mask & IN_MODIFY) {
                printf("The file %s was modified.\n", event->name);
            }
        }
        ifile += EVENT_SIZE + event->len;
    }

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);
}