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
#include <stdbool.h>
#include <json.h> // https://github.com/katie-snow/Ultimarc-linux/blob/master/src/libs/pacdrive.c
#include <stdint.h>
#include <unistd.h>
#include <sys/inotify.h>
#include "ss_oled.h"
#include <signal.h>
#include <time.h>
time_t rawtime;

SSOLED ssoled[2]; // data structure for 2 OLED objects
// SSOLED buttonAoled;
int buttonAinitated = -1;
// SSOLED buttonBoled;
int buttonBinitated = -1;
unsigned char ucBackBuf[1024];

typedef struct args
{
  int help;
  int multi;
} args;

int loadGameConfig();
int loadGameConfigCalledTimeStamp = 0;
int initDisplay(int *buttonInitated, int iOLEDidx, int iOLEDAddr, int iOLEDType, int iOLEDChannel, int SLCpin, int SDApin);
int initDisplays();
int turnOffDisplays();
void watchDisplayUpdate();
void bye();
void signalHandler(int sig);
void delay(int number_of_seconds);
int ulValidateConfigFileStr (const char* file);
void resetDisplays();

int retVal;
int gameJsonFound;
bool watching = true;
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
  printf("Retropie Oled Display Controller - Main called\n");
  // struct sigaction new_action, old_action;

  // /* Set up the structure to specify the new action. */
  // new_action.sa_handler = signalHandler;
  // sigemptyset (&new_action.sa_mask);
  // new_action.sa_flags = 0;

  // sigaction (SIGINT, NULL, &old_action);
  // if (old_action.sa_handler != SIG_IGN)
  //   sigaction (SIGINT, &new_action, NULL);
  // sigaction (SIGHUP, NULL, &old_action);
  // if (old_action.sa_handler != SIG_IGN)
  //   sigaction (SIGHUP, &new_action, NULL);
  // sigaction (SIGTERM, NULL, &old_action);
  // if (old_action.sa_handler != SIG_IGN)
  //   sigaction (SIGTERM, &new_action, NULL);
  // if (old_action.sa_handler != SIG_IGN)
  //   sigaction (SIGCONT, &new_action, NULL);

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

  // atexit(bye);
  // initDisplays(); // used to work
  pathToPacDriveJsonGameConfig = argv[1];
  
  loadGameConfig();
  watchDisplayUpdate();
  
  // while (watching) {
  //   // Wait indefinitely until watching equals false
  // }

  printf("Exiting\n");
  exit: return retVal;
}

int
ulValidateConfig (json_object* bcfg)
{
  int retCode = 0;

  if (bcfg)
  {
    printf("%s\n---\n", json_object_to_json_string_ext(bcfg, JSON_C_TO_STRING_PRETTY));
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
  return ulValidateConfig (bcfg);
}

// loadGameConfig(const char* gameconfig)
int
loadGameConfig()
{
  if (loadGameConfigCalledTimeStamp == 0) {
    loadGameConfigCalledTimeStamp = time(&rawtime);
  } else {
    time_t currentTime;
    currentTime = time(&rawtime);
    printf("diff time: %f\n", difftime(currentTime, loadGameConfigCalledTimeStamp));
    if (difftime(currentTime, loadGameConfigCalledTimeStamp) < 3) {
      printf("loadGameConfig called too soon\n");
      return 0;
    } else {
      loadGameConfigCalledTimeStamp = time(&rawtime);
    }
  }
  printf ("Loading %s...\n", pathToPacDriveJsonGameConfig);
  gameJsonFound = ulValidateConfigFileStr (pathToPacDriveJsonGameConfig);
  if (gameJsonFound == 0)
  {
    if (json_object_object_get_ex(bcfg, "game", &tmp)) {
      printf ("Game Found - %s\n", json_object_get_string(tmp));
      const char* gameFound = json_object_get_string(tmp);
      // sleep(2);
      initDisplays(); // wasn't here before just testing if i need to turn on displays again
      
      if (strcmp(gameFound, "alloff") == 0) {
        printf ("Game Found - All Off Reset Displays\n");
        resetDisplays();
      } else {
        // initDisplays();
        // oledWriteString(&ssoled[0], 0, 0, 6, (char*)json_object_to_json_string(tmp), FONT_SMALL,0,1);
        // oledWriteString(&ssoled[1], 0, 0, 6, "something here", FONT_SMALL,0,1);
        if (json_object_object_get_ex(bcfg, "P1_BUTTON1", &tmp)) {
          if (json_object_get_string(tmp)) {
            oledWriteString(&ssoled[0], 0,0,5, (char*)json_object_get_string(tmp), FONT_SMALL,0,1);
          } else {
            // oledPower(&ssoled[0], 0);
            printf("P1_BUTTON1 not found 1\n");
            buttonAinitated = -1;
          }
        } else {
          // oledPower(&ssoled[0], 0);
          printf("P1_BUTTON1 not found 2\n");
          buttonAinitated = -1;
        }
        if (json_object_object_get_ex(bcfg, "P1_BUTTON2", &tmp)) {
          if (json_object_get_string(tmp)) {
            oledWriteString(&ssoled[1], 0,0,5, (char*)json_object_get_string(tmp), FONT_SMALL,0,1);
          } else {
            // oledPower(&ssoled[1], 0);
            printf("P1_BUTTON2 not found 1\n");
            buttonBinitated = -1;
          }
        } else {
          // oledPower(&ssoled[1], 0);
          printf("P1_BUTTON2 not found 2\n");
          buttonBinitated = -1;
        }
        // printf("Press ENTER to quit\n");
        // getchar();
        // turnOffDisplays();
      }
    } else {
      printf ("Game Not Found\n");
      resetDisplays();
    }
    // retVal = ulWriteToBoardFileStr(argv[idx], &board);
  } else {
    printf ("Game Not Found\n");
    resetDisplays();
  }

  return 0;
}

// create method to initDisplay params iOLEDAddr, iOLEDType, bFlip, bInvert, bWire, iOLEDChannel, SLCpin, SDApin
int initDisplay(int *buttonInitated, int iOLEDidx, int iOLEDAddr, int iOLEDType, int iOLEDChannel, int SLCpin, int SDApin) {
  int bFlip = 0, bInvert = 0, bWire = 1;
  // int i;
  
  if (*buttonInitated == -1) {
    // oledInit(&ssoled[0], iOLEDType0, iOLEDAddr1, bFlip, bInvert, bWire, 4, 9, 8, 0); // initialize 128x64 oled on I2C channel 1
    *buttonInitated = oledInit(&ssoled[iOLEDidx], iOLEDType, iOLEDAddr, bFlip, bInvert, bWire, iOLEDChannel, SLCpin, SDApin, 0);
    if (*buttonInitated != OLED_NOT_FOUND) {
      printf("Successfully opened I2C bus %d on address %d on SLC pin %d on SDA pin %d\n", iOLEDChannel, iOLEDAddr, SLCpin, SDApin);
      oledSetBackBuffer(&ssoled[iOLEDidx], ucBackBuf);
      resetDisplays();
    } else {
      printf("Unable to initialize I2C bus %d on address %d, please check your connections and verify the device address by typing 'i2cdetect -y %d'\n", iOLEDChannel, iOLEDAddr, iOLEDChannel);
      return -1;
    }
  } else {
    printf("Display already initialized %d\n", *buttonInitated);
  }
  return 0;
}
int
initDisplays()
{
  int iOLEDAddr1 = 0x3c; // typical address; it can also be 0x3d
  int iOLEDAddr2 = 0x3d;
  int iOLEDType = OLED_128x64;
  // Y X L
  // B A R
  // initDisplay(int iOLEDidx, int iOLEDAddr, int iOLEDType, int iOLEDChannel, int SLCpin, int SDApin)
  int iOLED_AB_Channel = 4;
  int iOLED_SLCpin = 9;
  int iOLED_SDApin = 8;
  // A
  int buttonAidx = 0;
  int buttonA_created = initDisplay(&buttonAinitated, buttonAidx, iOLEDAddr1, iOLEDType, iOLED_AB_Channel, iOLED_SLCpin, iOLED_SDApin);
  // B
  int buttonBidx = 1;
  int buttonB_created = initDisplay(&buttonBinitated,  buttonBidx, iOLEDAddr2, iOLEDType, iOLED_AB_Channel, iOLED_SLCpin, iOLED_SDApin);

  if (buttonA_created == 0 && buttonB_created == 0) {
    printf("Displays created successfully\n");
    resetDisplays();
  } else {
    printf("Displays not created successfully\n");
  }
  return 0;
}

int
initDisplaysOldWorking()
{
  int i;
  // int iOLEDAddr = -1; // typical address; it can also be 0x3d
  int iOLEDAddr1 = 0x3c; // typical address; it can also be 0x3d
  int iOLEDAddr2 = 0x3d; // typical address; it can also be 0x3d
  int iOLEDType0 = OLED_128x64; // Change this for your specific display
  // int iOLEDType1 = OLED_64x32;
  int bFlip = 0, bInvert = 0, bWire = 1;

  // For hardware I2C on the RPI, the clock rate is fixed and set in the
  // /boot/config.txt file, so we pass 0 for the bus speed
	i=oledInit(&ssoled[0], iOLEDType0, iOLEDAddr1, bFlip, bInvert, bWire, 4, 9, 8, 0); // initialize 128x64 oled on I2C channel 1
	if (i != OLED_NOT_FOUND)
	{
    printf("Successfully opened I2C bus 4\n");
    oledSetBackBuffer(&ssoled[0], ucBackBuf);
    // oledFill(&ssoled[0], 0,1); // fill with black
    // oledWriteString(&ssoled[0], 0,0,0, msgs[i], FONT_NORMAL,0,1);
    // oledWriteString(&ssoled[0], 0,0,1,"SS_OLED 1",FONT_NORMAL,0,1);
    // oledWriteString(&ssoled[0], 0,3,2,"OLED 1",FONT_LARGE,0,1);
    // oledWriteString(&ssoled[0], 0,0,5,"Small", FONT_SMALL,0,1);
    // oledWriteString(&ssoled[0], 0,0,6,argv[1], FONT_SMALL,0,1);
    // for (i=0; i<64; i++)
    // {
    //   oledSetPixel(&ssoled[0], i, 16+i, 1, 1);
    //   oledSetPixel(&ssoled[0], 127-i, 16+i, 1, 1);
    // }


    oledInit(&ssoled[1], iOLEDType0, iOLEDAddr2, bFlip, bInvert, bWire, 4, 9, 8, 0); // initialize 128x64 oled on I2C channel 1
    oledSetBackBuffer(&ssoled[1], ucBackBuf);
    // oledFill(&ssoled[1], 0, 1); // fill with black
    // oledWriteString(&ssoled[0], 0,0,0, msgs[i], FONT_NORMAL,0,1);
    // oledWriteString(&ssoled[1], 0,0,1,"SS_OLED 2",FONT_NORMAL,0,1);
    // oledWriteString(&ssoled[1], 0,3,2,"OLED 2",FONT_LARGE,0,1);
    // oledWriteString(&ssoled[1], 0,0,5,"Small", FONT_SMALL,0,1);
    // oledWriteString(&ssoled[0], 0,0,6,argv[1], FONT_SMALL,0,1);
    // for (i=0; i<64; i++)
    // {
    //   oledSetPixel(&ssoled[1], i, 16+i, 1, 1);
    //   oledSetPixel(&ssoled[1], 127-i, 16+i, 1, 1);
    // }

    resetDisplays();
	}
	else
	{
		printf("Unable to initialize I2C bus 0-2, please check your connections and verify the device address by typing 'i2cdetect -y <channel>\n");
	}
   return 0;
}

void
resetDisplays()
{
  oledFill(&ssoled[0], 0,1); // fill with black
  oledWriteString(&ssoled[0], 0,0,1,"SS_OLED 1",FONT_NORMAL,0,1);
  buttonAinitated = -1;

  oledFill(&ssoled[1], 0, 1); // fill with black
  oledWriteString(&ssoled[1], 0,0,1,"SS_OLED 2",FONT_NORMAL,0,1);
  buttonBinitated = -1;
}

int
turnOffDisplays()
{
  // if (!ssoled[0]) {
  //   initDisplays();
  // }
  oledPower(&ssoled[0], 0); // turn off both displays
  oledPower(&ssoled[1], 0);
  return 0;
}

void watchDisplayUpdate() {
    printf("watchDisplayUpdate called\n");
    fd = inotify_init();
    
    if (fd < 0) {
        perror("inotify_init");
        printf("inotify_init error starting\n");
    }

    // wd = inotify_add_watch(fd, ".",
    wd = inotify_add_watch(fd, folderToWatch,
        IN_MODIFY | IN_CREATE | IN_DELETE);
    
    
    struct tm *timeinfo;
    char timestamp[20];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %I:%M%p", timeinfo);
    printf("Watching: %s\n", timestamp);

    while (watching) {
      printf("Watching\n");
      length = read(fd, buffer, BUF_LEN);
      if (length < 0) {
          perror("read");
          printf("read error starting\n");
      }
      ifile = 0;
      while (ifile < length) {
        struct inotify_event *event = (struct inotify_event *) &buffer[ifile];
        printf("Event: %s\n", event->name);
        printf("Event len: %d\n", event->len);
        printf("Event type: %d\n", event->mask);
        // printf("Event IN_CLOSE_WRITE: %d\n", IN_CLOSE_WRITE);
        // printf("Event IN_MODIFY: %d\n", IN_MODIFY);
        // if (event->len && event->mask & IN_CLOSE_WRITE && strcmp(event->name, "pacdrive.json") == 0) {
        if (event->len && event->mask & IN_MODIFY && strcmp(event->name, "pacdrive.json") == 0) {
          sleep(1);
          loadGameConfig();
        }
        ifile += EVENT_SIZE + event->len;
      }
    }
}

void stopWatchingFolder() {
  inotify_rm_watch(fd, wd);
  close(fd);
}


// void watchDisplayUpdate() {
//     printf("watchDisplayUpdate called\n");
//     fd = inotify_init();
    
//     if (fd < 0) {
//         perror("inotify_init");
//         printf("inotify_init error starting\n");
//     }

//     // wd = inotify_add_watch(fd, ".",
//     wd = inotify_add_watch(fd, folderToWatch,
//         IN_MODIFY | IN_CREATE | IN_DELETE);
//     length = read(fd, buffer, BUF_LEN);

//     if (length < 0) {
//         perror("read");
//         printf("read error starting\n");
//     }
//     printf("Watching \n");
//     while (ifile < length && watching == true) {
//         struct inotify_event *event =
//             (struct inotify_event *) &buffer[ifile];
//         if (event->len) {
//             if (event->mask & IN_CREATE) {
//                 printf("The file %s was created.\n", event->name);
//             } else if (event->mask & IN_DELETE) {
//                 printf("The file %s was deleted.\n", event->name);
//             } else if (event->mask & IN_MODIFY) {
//                 printf("The file %s was modified.\n", event->name);
//             }
//             // initDisplays();
//             loadGameConfig();
//             watchDisplayUpdate();
//         }
//         ifile += EVENT_SIZE + event->len;
//     }

//     (void) inotify_rm_watch(fd, wd);
//     (void) close(fd);
// }



void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void
bye ()
{
  printf ("Goodbye!\n");
  if (turnOffDisplays() == 0) {
    // delay(1);
    printf ("Displays turned off\n");
  }
  watching = false;
  stopWatchingFolder();
}

void signalHandler(int sig) {
  printf("Caught signal %d\n", sig);
  // turnOffDisplays();
  bye();
  exit(0);
}

