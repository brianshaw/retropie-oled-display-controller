/*
 ============================================================================
 Name        : main.c
 Author      : Robert Abram, Katie Snow
 Version     :
 Copyright   : Copyright 2014 Robert Abram, Katie Snow
 Description : Ultimarc command line utility
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json.h> // https://github.com/katie-snow/Ultimarc-linux/blob/master/src/libs/pacdrive.c
#include <stdint.h>
#include <unistd.h>
#include <sys/inotify.h>
#include "ss_oled.h"
#include <signal.h>

SSOLED ssoled[2]; // data structure for 2 OLED objects
unsigned char ucBackBuf[1024];

typedef struct args
{
  int help;
  int multi;
} args;

int loadGameConfig();
int initDisplays();
int turnOffDisplays();
void watchDisplayUpdate();
void bye();

int retVal;
json_object *bcfg = NULL;
json_object* tmp = NULL;
char* pathToPacDriveJsonGameConfig;

#define EVENT_SIZE (sizeof(struct inotify_event)) // Add this line to declare the missing variable
#define BUF_LEN (1024 * (EVENT_SIZE + 16)) // Add this line to declare the missing variable

char* folderToWatch = "/home/pi/retropie-controller-display-and-lights";
// char* folderToWatch = "displayTexts";
int length, ifile = 0;
int fd;
int wd;
char buffer[BUF_LEN];


int
main (int argc, char **argv)
{
  signal(SIGTERM, signalHandler);
  int idx;
  

  // ulboard board;

  args args;
  args.help = 0;
  args.multi = 0;

  if (argc == 1)
  {
    args.help = 1;
  }

  for (idx = 1; idx < argc; ++idx)
  {
    if (strcmp (argv[idx], "-h") == 0 || strcmp (argv[idx], "--help") == 0)
      args.help = 1;
    if (strcmp (argv[idx], "-m") == 0 || strcmp (argv[idx], "--multi") == 0)
      args.multi = 1;
  }

  if (args.help)
  {
    printf ("umtool [-h] [--help] [-m config file] [--multi config file] [config files...]\n");
    printf ("-h | --help\t Prints this information\n");
    printf ("-m | --multi\t File provided has multiple configuration\n");
    printf ("config files\t JSON Configuration files to be processed\n");
    retVal = EXIT_SUCCESS;
    goto exit;
  }

  // if (args.multi)
  // {
  //   printf ("Loading multiple configurations from file %s...\n", argv[2]);
  //   ulMultiConfigurationsFileStr(argv[2]);
  //   retVal = EXIT_SUCCESS;
  //   goto exit;
  // }

  // for (idx = 1; idx < argc; ++idx)
  // {
  //   printf ("Loading %s...\n", argv[idx]);
  //   retVal = ulValidateConfigFileStr (argv[idx], &board);

  //   if (retVal == 0)
  //   {
  //     // retVal = ulWriteToBoardFileStr(argv[idx], &board);
  //   }
  // }

  atexit(bye);

  pathToPacDriveJsonGameConfig = argv[1];
  loadGameConfig();
  watchDisplayUpdate();
  

  exit: return retVal;
}

int
ulValidateConfig (json_object* bcfg)
{
  int retCode = 0;

  if (bcfg)
  {
    // retCode = ulGetProdAndVersion (bcfg, ulcfg);
    
    // if (retCode == 0)
    // {
    //   if (isIPACConfig (bcfg, ulcfg)
    //       || isIPACUltimateConfig (bcfg, ulcfg)
    //       || isPACDriveConfig (bcfg, ulcfg)
    //       || isPACLED64Config (bcfg, ulcfg)
    //       || isUltraStikConfig (bcfg, ulcfg)
    //       || isUSBButtonConfig(bcfg, ulcfg)
    //       || isServoStikConfig(bcfg, ulcfg)
    //       || isUHidConfig (bcfg, ulcfg))
    //   {
    //     log_info("Configuration is %s. [Validated]", ulBoardTypeToString(ulcfg->type));
    //   }
    //   else
    //   {
    //     retCode = -1;
    //     log_err("Configuration is '%s'. [Not validated].", ulBoardTypeToString(ulcfg->type));
    //   }
    // }
    // else
    // {
    //   retCode = -1;
    //   log_err("Configuration is '%s'. [Not validated].", ulBoardTypeToString(ulcfg->type));
    // }
  }
  else
  {
    retCode = -1;
    // log_err("JSON format invalid");
  }

  return retCode;
}

int
ulValidateConfigFileStr (const char* file)
{
  bcfg = json_object_from_file (file);
  printf("%s\n---\n", json_object_to_json_string_ext(bcfg, JSON_C_TO_STRING_PRETTY));

  return ulValidateConfig (bcfg);
}

// loadGameConfig(const char* gameconfig)
int
loadGameConfig()
{
  printf ("Loading %s...\n", pathToPacDriveJsonGameConfig);
  retVal = ulValidateConfigFileStr (pathToPacDriveJsonGameConfig);

  printf ("retVal = %d\n", retVal);
  if (retVal == 0)
  {
    if (json_object_object_get_ex(bcfg, "game", &tmp)) {
      printf ("Game Found - %s\n", json_object_to_json_string(tmp));
      initDisplays();
      oledWriteString(&ssoled[0], 0, 0, 6, (char*)json_object_to_json_string(tmp), FONT_SMALL,0,1);
      // printf("Press ENTER to quit\n");
      // getchar();
      // turnOffDisplays();
    } else {
      printf ("Game Not Found\n");
    }
    // retVal = ulWriteToBoardFileStr(argv[idx], &board);
  }

  return 0;
}

int
initDisplays()
{
  int i;
  int iOLEDAddr = -1; // typical address; it can also be 0x3d
  int iOLEDType0 = OLED_128x64; // Change this for your specific display
  // int iOLEDType1 = OLED_64x32;
  int bFlip = 0, bInvert = 0, bWire = 1;

  // For hardware I2C on the RPI, the clock rate is fixed and set in the
  // /boot/config.txt file, so we pass 0 for the bus speed
	i=oledInit(&ssoled[0], iOLEDType0, iOLEDAddr, bFlip, bInvert, bWire, 4, 9, 8, 0); // initialize 128x64 oled on I2C channel 1
	if (i != OLED_NOT_FOUND)
	{
    printf("Successfully opened I2C bus 1\n");
    oledSetBackBuffer(&ssoled[0], ucBackBuf);
    oledFill(&ssoled[0], 0,1); // fill with black
    // oledWriteString(&ssoled[0], 0,0,0, msgs[i], FONT_NORMAL,0,1);
    oledWriteString(&ssoled[0], 0,0,1,"SS_OLED Library!",FONT_NORMAL,0,1);
    oledWriteString(&ssoled[0], 0,3,2,"BIG!",FONT_LARGE,0,1);
    oledWriteString(&ssoled[0], 0,0,5,"Small", FONT_SMALL,0,1);
    // oledWriteString(&ssoled[0], 0,0,6,argv[1], FONT_SMALL,0,1);
    for (i=0; i<64; i++)
    {
      oledSetPixel(&ssoled[0], i, 16+i, 1, 1);
      oledSetPixel(&ssoled[0], 127-i, 16+i, 1, 1);
    }

    
	}
	else
	{
		printf("Unable to initialize I2C bus 0-2, please check your connections and verify the device address by typing 'i2cdetect -y <channel>\n");
	}
   return 0;
}

int
turnOffDisplays()
{
  oledPower(&ssoled[0], 0); // turn off both displays
  // oledPower(&ssoled[1], 0);
  return 0;
}

void watchDisplayUpdate() {
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
            initDisplays();
            loadGameConfig();
            watchDisplayUpdate();
        }
        ifile += EVENT_SIZE + event->len;
    }

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);
}

void
bye ()
{
  printf ("Goodbye!\n");
  turnOffDisplays();
}

void signalHandler(int sig) {
  printf("Caught signal %d\n", sig);
  // turnOffDisplays();
  exit(0);
}