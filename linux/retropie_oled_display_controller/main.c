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

typedef struct args
{
  int help;
  int multi;
} args;

int loadGameConfig();

int retVal;
json_object *bcfg = NULL;
json_object* tmp = NULL;
struct {
		int flag;
		const char *flag_str;
	} json_flags[] = {
		{ JSON_C_TO_STRING_PLAIN, "JSON_C_TO_STRING_PLAIN" },
		{ JSON_C_TO_STRING_SPACED, "JSON_C_TO_STRING_SPACED" },
		{ JSON_C_TO_STRING_PRETTY, "JSON_C_TO_STRING_PRETTY" },
		{ JSON_C_TO_STRING_NOZERO, "JSON_C_TO_STRING_NOZERO" },
		{ JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY, "JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY" },
		{ -1, NULL }
  }

int
main (int argc, char **argv)
{
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

  loadGameConfig(argv[1]);

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
  printf("%s\n---\n", json_object_to_json_string_ext(bcfg, json_flags[i].flag));

  return ulValidateConfig (bcfg);
}

int
loadGameConfig(const char* gameconfig)
{
  printf ("Loading %s...\n", gameconfig);
  retVal = ulValidateConfigFileStr (gameconfig);

  printf ("retVal = %d\n", retVal);
  if (retVal == 0)
  {
    if (json_object_object_get_ex(bcfg, "game", &tmp)) {
      printf ("Game Found - %s\n", json_object_to_json_string(tmp));
    } else {
      printf ("Game Not Found\n");
    }
    // retVal = ulWriteToBoardFileStr(argv[idx], &board);
  }

  return 0;
}