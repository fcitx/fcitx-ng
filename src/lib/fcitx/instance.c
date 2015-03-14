/*
 * Copyright (C) 2010~2015 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include "config.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/mainloop.h"
#include "fcitx-utils/stringlist.h"
#include "instance.h"
#include "instance-internal.h"
#include "ime-internal.h"
#include "addon.h"
#include "ime.h"
#include "inputcontext-internal.h"
#include "addon-internal.h"
#include "frontend.h"

typedef struct _FcitxInstanceArguments
{
    bool tryReplace;
    bool quietQuit;
    char* enableList;
    char* disableList;
    char* uiname;
    int overrideDelay;
    bool runasdaemon;
} FcitxInstanceArguments;

typedef struct _FcitxInputContextState
{
} FcitxInputContextState;

static void Usage();
static void Version();

static bool fcitx_instance_input_context_event_dispatch(void* self, FcitxEvent* event);
static FcitxInputMethod*
fcitx_instance_get_input_method_for_input_context(FcitxInstance* instance, FcitxInputContext* ic);

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

void fcitx_instance_arugment_parse(FcitxInstanceArguments* arg, int argc, char* argv[])
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
                    fcitx_utils_string_swap(&arg->enableList, optarg);
                }
                break;
            case 3:
                {
                    fcitx_utils_string_swap(&arg->disableList, optarg);
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

void fcitx_instance_arguments_free(FcitxInstanceArguments* arg)
{
    free(arg->enableList);
    free(arg->disableList);
    free(arg->uiname);
}

FcitxInputContextState* fcitx_input_context_state_new()
{
    FcitxInputContextState* state = fcitx_utils_new(FcitxInputContextState);

    return state;
}

FCITX_EXPORT_API
FcitxInstance* fcitx_instance_create(int argc, char* argv[])
{
    FcitxInstanceArguments arguments;
    memset(&arguments, 0, sizeof(arguments));
    fcitx_instance_arugment_parse(&arguments, argc, argv);
    if (arguments.quietQuit) {
        fcitx_instance_arguments_free(&arguments);
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
    instance->tryReplace = arguments.tryReplace;
    instance->signalPipe = -1;

    FcitxMainLoop* mainloop = fcitx_mainloop_new();
    instance->mainloop = mainloop;

    instance->standardPath = fcitx_standard_path_new();
    instance->addonManager = fcitx_addon_manager_new(instance->standardPath);
    instance->icManager = fcitx_input_context_manager_new();
    fcitx_input_context_manager_set_event_dispatcher(instance->icManager, fcitx_instance_input_context_event_dispatch, NULL, instance);
    instance->imManager = fcitx_input_method_manager_new();
    instance->icStates = fcitx_dict_new(free);

    return instance;
}

void fcitx_instance_handle_signal(FcitxIOEvent* _event, int fd, unsigned int flag, void* data)
{
    FCITX_UNUSED(_event);
    FCITX_UNUSED(flag);
    FcitxInstance* instance = data;
    uint8_t signo = 0;
    while (read(fd, &signo, sizeof(signo)) > 0) {
        if (signo == SIGINT || signo == SIGTERM || signo == SIGQUIT || signo == SIGXCPU) {
            fcitx_instance_shutdown(instance);
        }
    }
}

FCITX_EXPORT_API
int fcitx_instance_run(FcitxInstance* instance)
{
    fcitx_addon_manager_set_property(instance->addonManager, "instance", instance);
    fcitx_addon_manager_set_property(instance->addonManager, "icmanager", instance->icManager);
    fcitx_addon_manager_set_property(instance->addonManager, "immanager", instance->imManager);
    fcitx_addon_manager_register_default_resolver(instance->addonManager, NULL);
    fcitx_addon_manager_set_override(instance->addonManager, instance->enableList, instance->disableList);
    fcitx_addon_manager_load(instance->addonManager);
    instance->running = true;

    if (instance->shutdown) {
        return 1;
    }

    if (instance->signalPipe != -1) {
        fcitx_mainloop_register_io_event(instance->mainloop, instance->signalPipe, FIOEF_IN, fcitx_instance_handle_signal, NULL, instance);
    }

    return fcitx_mainloop_run(instance->mainloop);
}

FCITX_EXPORT_API
void fcitx_instance_set_signal_pipe(FcitxInstance* instance, int fd)
{
    instance->signalPipe = fd;
}

FCITX_EXPORT_API
FcitxMainLoop* fcitx_instance_get_mainloop(FcitxInstance* instance)
{
    return instance->mainloop;
}

FCITX_EXPORT_API
void fcitx_instance_shutdown(FcitxInstance* instance)
{
    instance->shutdown = true;
    if (instance->running) {
        fcitx_mainloop_quit(instance->mainloop);
    }
}

FCITX_EXPORT_API
bool fcitx_instance_get_try_replace(FcitxInstance* instance)
{
    return instance->tryReplace;
}


FCITX_EXPORT_API
void fcitx_instance_destroy(FcitxInstance* instance)
{
    fcitx_dict_free(instance->icStates);
    fcitx_addon_manager_unref(instance->addonManager);
    fcitx_standard_path_unref(instance->standardPath);
    fcitx_mainloop_free(instance->mainloop);
    fcitx_input_method_manager_unref(instance->imManager);
    fcitx_input_context_manager_unref(instance->icManager);
    free(instance->enableList);
    free(instance->disableList);
    free(instance->uiname);
    free(instance);
}

bool fcitx_instance_input_context_event_dispatch(void* self, FcitxEvent* event)
{
    FcitxInstance* instance = self;
    FcitxInputContextEvent* icEvent = (FcitxInputContextEvent*) event;
    if (event->type == ET_InputContextCreated) {
        fcitx_dict_insert_by_data(instance->icStates, icEvent->id, fcitx_input_context_state_new(), false);
    } else if (event->type == ET_InputContextDestroyed) {
        fcitx_dict_remove_by_data(instance->icStates, icEvent->id, NULL);
    }

    // TODO, handle trigger key

    if ((event->type & ET_EventTypeFlag) == ET_InputContextEventFlag) {
        FcitxInputMethod* im = fcitx_instance_get_input_method_for_input_context(instance, icEvent->inputContext);
        if (im) {
            return im->handleEvent(im->imclass, event);
        }
    } else if ((event->type & ET_EventTypeFlag) == ET_InputMethodEventFlag) {
        uint32_t frontend = icEvent->inputContext->frontend;
        if (fcitx_ptr_array_size(instance->addonManager->frontends) > frontend) {
            FcitxAddon* addon = fcitx_ptr_array_index(instance->addonManager->frontends, frontend, FcitxAddon*);

            FcitxAddonAPIFrontend* apiFrontend = (FcitxAddonAPIFrontend*) (addon->inst.api);
            return apiFrontend->handleEvent(addon->inst.data, event);
        }
    }

    return false;
}

FCITX_EXPORT_API
void fcitx_instance_post_event(FcitxInstance* instance, FcitxEvent* event)
{
    if (!event ||
        (event->type & ET_EventTypeFlag) == ET_InputContextEventFlag ||
        (event->type & ET_EventTypeFlag) == ET_InputMethodEventFlag) {
        return;
    }

    // TODO
}

FcitxInputMethod* fcitx_instance_get_input_method_for_input_context(FcitxInstance* instance, FcitxInputContext* ic)
{
    // TODO
    return NULL;
}

void fcitx_instance_set_input_method_group(FcitxInstance* instance, int group)
{
    instance->group = group;
    // TODO notify
}

int fcitx_instance_get_input_method_group(FcitxInstance* instance)
{
    return instance->group;
}

void fcitx_instance_set_input_method_for_input_context(FcitxInstance* instance, FcitxInputContext* ic, const char* name, bool local)
{
}
