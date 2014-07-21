/***************************************************************************
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <getopt.h>
#include <unistd.h>
#include "config.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/mainloop.h"
#include "fcitx-utils/stringlist.h"
#include "instance.h"
#include "instance-internal.h"

typedef struct _FcitxInstanceArguments
{
    bool tryReplace;
    bool quietQuit;
    FcitxStringList* enableList;
    FcitxStringList* disableList;
    char* uiname;
    int overrideDelay;
    bool runasdaemon;
} FcitxInstanceArguments;

static void Usage();
static void Version();

/**
 * print usage
 */
void Usage()
{
    printf("Usage: fcitx [OPTION]\n"
           "\t-r, --replace\t\ttry replace existing fcitx, need module support.\n"
           "\t-d\t\t\trun as daemon(default)\n"
           "\t-D\t\t\tdon't run as daemon\n"
           "\t-s[sleep time]\t\toverride delay start time in config file, 0 for immediate start\n"
           "\t-v, --version\t\tdisplay the version information and exit\n"
           "\t-u, --ui\t\tspecify the user interface to use\n"
           "\t--enable\t\tspecify a comma separated list for addon that will override the enable option\n"
           "\t--disable\t\tspecify a comma separated list for addon that will explicitly disabled,\n"
           "\t\t\t\t\tpriority is lower than --enable, can use all for disable all module\n"
           "\t-h, --help\t\tdisplay this help and exit\n");
}

/**
 * print version
 */
void Version()
{
    printf("fcitx version: %s\n", FCITX_VERSION);
}

void FcitxInstanceArgumentsParse(FcitxInstanceArguments* arg, int argc, char* argv[])
{
    struct option longOptions[] = {
        {"ui", 1, 0, 0},
        {"replace", 0, 0, 0},
        {"enable", 1, 0, 0},
        {"disable", 1, 0, 0},
        {"version", 0, 0, 0},
        {"help", 0, 0, 0},
        {NULL, 0, 0, 0}
    };

    int optionIndex = 0;
    int c;
    arg->overrideDelay = -1;
    while ((c = getopt_long(argc, argv, "ru:dDs:hv", longOptions, &optionIndex)) != EOF) {
        switch (c) {
        case 0: {
            switch (optionIndex) {
            case 0:
                fcitx_utils_string_swap(&arg->uiname, optarg);
                break;
            case 1:
                arg->tryReplace = true;
                break;
            case 2:
                {
                    if (arg->enableList)
                        fcitx_utils_string_list_free(arg->enableList);
                    arg->enableList = fcitx_utils_string_split(optarg, ",");
                }
                break;
            case 3:
                {
                    if (arg->disableList)
                        fcitx_utils_string_list_free(arg->disableList);
                    arg->disableList = fcitx_utils_string_split(optarg, ",");
                }
                break;
            case 4:
                arg->quietQuit = true;
                Version();
                break;
            default:
                arg->quietQuit = true;
                Usage();
            }
        }
        break;
        case 'r':
            arg->tryReplace = true;
            break;
        case 'u':
            fcitx_utils_string_swap(&arg->uiname, optarg);
            break;
        case 'd':
            arg->runasdaemon = true;
            break;
        case 'D':
            arg->runasdaemon = false;
            break;
        case 's':
            arg->overrideDelay = atoi(optarg);
            break;
        case 'h':
            arg->quietQuit = true;
            Usage();
        case 'v':
            arg->quietQuit = true;
            Version();
            break;
        default:
            arg->quietQuit = true;
            Usage();
        }
    }
}

void FcitxInstanceArgumentsFree(FcitxInstanceArguments* arg)
{
    fcitx_utils_string_list_free(arg->enableList);
    fcitx_utils_string_list_free(arg->disableList);
    free(arg->uiname);
}

FcitxInstance* FcitxInstanceCreate(int argc, char* argv[])
{
    FcitxInstanceArguments arguments;
    FcitxInstanceArgumentsParse(&arguments, argc, argv);
    if (arguments.quietQuit) {
        FcitxInstanceArgumentsFree(&arguments);
        return NULL;
    }

    if (arguments.runasdaemon) {
        fcitx_utils_init_as_daemon();
    }

    if (arguments.overrideDelay > 0) {
        sleep(arguments.overrideDelay);
    }

    FcitxInstance* instance = fcitx_utils_new(FcitxInstance);
    instance->enableList = arguments.enableList;
    instance->disableList = arguments.disableList;
    instance->uiname = arguments.uiname;

    FcitxMainLoop* mainloop = fcitx_mainloop_new();
    instance->mainloop = mainloop;

    return instance;
}

int FcitxInstanceRun(FcitxInstance* instance)
{
    return fcitx_mainloop_run(instance->mainloop);
}

void FcitxInstanceDestroy(FcitxInstance* instance)
{
    fcitx_mainloop_free(instance->mainloop);
    free(instance);
}
