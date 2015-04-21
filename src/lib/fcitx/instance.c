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
#include "fcitx_version.h"
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

typedef enum {
    LRK_None,
    LRK_Trigger,
    LRK_TriggerAlt,
    LRK_ScrollForward,
    LRK_ScrollBackward,
    LRK_Activate,
    LRK_Deactivate,
} FcitxLastReleasedKey;

typedef enum _FcitxInputMethodStateUpdateType
{
    UpdateType_SetInputMethod,
} FcitxInputMethodStateUpdateType;

typedef struct _FcitxInputMethodStateUpdate
{
    FcitxInputMethodStateUpdateType type;

    union {
        struct {
            bool local;
            FcitxInputMethodItem* imItem;
        } setIM;
    };
} FcitxInputMethodStateUpdate;

typedef struct _FcitxInputMethodPrivateState
{

    uint64_t lastKeyPressedTime;
    FcitxLastReleasedKey releasedKey;
} FcitxInputMethodPrivateState;

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

typedef struct _FcitxInputMethodState
{
    
    FcitxPtrArray* localInputMethod;
} FcitxInputMethodState;

static void Usage();
static void Version();

static bool fcitx_instance_event_dispatch(void* self, FcitxEvent* event);

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
    printf("fcitx version: %s\n", FCITX_VERSION_STRING);
}

static inline
FcitxInputMethodPrivateState* _fcitx_instance_input_context_get_private_state(FcitxInstance* instance, FcitxInputContext* ic)
{
    return fcitx_input_context_set_property(ic, instance->inputMethodPrivateStateId, NULL);
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
            break;
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

FCITX_EXPORT_API
FcitxInstance* fcitx_instance_new(int argc, char* argv[])
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
    fcitx_input_context_manager_set_event_dispatcher(instance->icManager, fcitx_instance_event_dispatch, NULL, instance);
    instance->imManager = fcitx_input_method_manager_new(instance->addonManager);
    fcitx_input_method_manager_set_event_dispatcher(instance->imManager, fcitx_instance_event_dispatch, NULL, instance);
    instance->globalInputMethod = fcitx_ptr_array_new(NULL);
    instance->globalConfig = fcitx_global_config_new();

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

void* fcitx_input_context_app_name_set(void* data, void* value, void* userData)
{
    FCITX_UNUSED(userData);

    char* str = data;
    fcitx_utils_string_swap(&str, value);

    return str;
}

void* fcitx_input_context_input_method_private_state_set(void* data, void* value, void* userData)
{
    FCITX_UNUSED(userData);
    FCITX_UNUSED(value);
    FcitxInputMethodPrivateState* state = data;
    if (!data) {
        state = fcitx_utils_new(FcitxInputMethodPrivateState);
    }

    return state;
}

void* fcitx_input_context_input_method_state_set(void* data, void* value, void* userData)
{
    FcitxInputMethodState* state = data;
    if (!data) {
        state = fcitx_utils_new(FcitxInputMethodState);
    }

    FcitxInstance* instance = userData;
    FcitxInputMethodStateUpdate* update = (FcitxInputMethodStateUpdate*) value;

    switch (update->type) {
        case UpdateType_SetInputMethod:
            if (update->setIM.local) {
                if (!state->localInputMethod) {
                    state->localInputMethod = fcitx_ptr_array_new(NULL);
                    fcitx_ptr_array_resize(state->localInputMethod, fcitx_ptr_array_size(instance->globalInputMethod), NULL, NULL);
                }
                fcitx_ptr_array_set(state->localInputMethod, instance->group, update->setIM.imItem);
            } else {
                if (fcitx_ptr_array_size(instance->globalInputMethod)) {
                    fcitx_ptr_array_set(instance->globalInputMethod, instance->group, update->setIM.imItem);
                }
            }
            break;
        default:
            break;
    }

    return state;
}

void fcitx_input_context_input_method_state_free(void* data, void* userData)
{
    FCITX_UNUSED(userData);
    if (!data) {
        return;
    }
    FcitxInputMethodState* state = data;
    fcitx_ptr_array_free(state->localInputMethod);
    free(state);
}

void* fcitx_input_context_input_method_state_copy(void* dst, void* src, void* userData)
{
    if (!src) {
        fcitx_input_context_input_method_state_free(dst, userData);
        return NULL;
    }

    FcitxInputMethodState* srcState = src;
    FcitxInputMethodState* state = dst;
    if (!state) {
        state = fcitx_utils_new(FcitxInputMethodState);
    }

    if (srcState->localInputMethod) {
        if (!state->localInputMethod) {
            state->localInputMethod = fcitx_ptr_array_new(NULL);
            fcitx_ptr_array_resize(state->localInputMethod, fcitx_ptr_array_size(srcState->localInputMethod), NULL, NULL);
            for (size_t i = 0; i < fcitx_ptr_array_size(srcState->localInputMethod); i++) {
                fcitx_ptr_array_set(state->localInputMethod, i, fcitx_ptr_array_index(srcState->localInputMethod, i, FcitxInputMethodItem*));
            }
        }
    } else {
        fcitx_ptr_array_free(state->localInputMethod);
        state->localInputMethod = NULL;
    }

    return state;
}

char*
fcitx_app_name_property_key(void* value, size_t* len, void* userData)
{
    FCITX_UNUSED(userData);
    *len = value ? strlen(value) : 0;
    return value;
}

FCITX_EXPORT_API
int fcitx_instance_run(FcitxInstance* instance)
{
    instance->inputMethodStateId = fcitx_input_context_manager_register_property(instance->icManager,
                                                                                 "inputMethodState",
                                                                                 fcitx_input_context_input_method_state_set,
                                                                                 fcitx_input_context_input_method_state_copy,
                                                                                 fcitx_input_context_input_method_state_free,
                                                                                 NULL, instance);
    // this part of state is not shared, only used to instance
    instance->inputMethodPrivateStateId =
        fcitx_input_context_manager_register_property(instance->icManager,
                                                      "inputMethodPrivateState",
                                                      fcitx_input_context_input_method_private_state_set,
                                                      NULL,
                                                      fcitx_utils_closure_free,
                                                      NULL, instance);
    int32_t appNameId = fcitx_input_context_manager_register_property(instance->icManager,
                                                      FCITX_APPLICATION_NAME_PROPERTY,
                                                      fcitx_input_context_app_name_set,
                                                      NULL,
                                                      fcitx_utils_closure_free,
                                                      NULL, instance);
    instance->appNamePolicy = fcitx_input_context_shared_state_policy_new(instance->icManager, appNameId, fcitx_app_name_property_key, NULL, NULL);
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
    // input context may have callback to addon, so destroy all input context first.
    fcitx_input_context_manager_destroy_all_input_context(instance->icManager);
    fcitx_input_method_manager_unref(instance->imManager);
    fcitx_addon_manager_unref(instance->addonManager);
    fcitx_input_context_manager_unref(instance->icManager);
    fcitx_standard_path_unref(instance->standardPath);
    fcitx_mainloop_free(instance->mainloop);
    fcitx_ptr_array_free(instance->globalInputMethod);
    fcitx_global_config_free(instance->globalConfig);
    free(instance->enableList);
    free(instance->disableList);
    free(instance->uiname);
    free(instance);
}

bool fcitx_instance_input_method_group_reset_foreach(FcitxInputContext* ic, void* userData)
{
    FcitxInstance* instance = userData;
    FcitxInputMethodState* state = fcitx_input_context_get_property(ic, instance->inputMethodStateId);
    if (state && state->localInputMethod) {
        fcitx_ptr_array_clear(state->localInputMethod);
    }

    return true;
}


bool fcitx_instance_event_dispatch(void* self, FcitxEvent* event)
{
    FcitxInstance* instance = self;
    FcitxInputContextEvent* icEvent = (FcitxInputContextEvent*) event;

    if ((event->type & ET_EventTypeFlag) == ET_InputContextEventFlag) {
        if (event->type == ET_InputContextKeyEvent) {
            bool hasMoreThanOneIM = fcitx_input_method_manager_get_group_size(instance->imManager, instance->group) > 0;

            struct {
                FcitxLastReleasedKey releasedKey;
                FcitxKeyList* keyList;
                void (*callback)(FcitxInstance* instance, FcitxInputContext* ic);
                bool valid;
            } keyHandle[] = {
                {LRK_Trigger, instance->globalConfig->hotkey.triggerKey, NULL, hasMoreThanOneIM},
                {LRK_TriggerAlt, instance->globalConfig->hotkey.triggerKey, NULL, hasMoreThanOneIM},
                {LRK_ScrollForward, instance->globalConfig->hotkey.triggerKey, NULL, hasMoreThanOneIM},
                {LRK_ScrollBackward, instance->globalConfig->hotkey.triggerKey, NULL, hasMoreThanOneIM},
                {LRK_Activate, instance->globalConfig->hotkey.triggerKey, NULL, hasMoreThanOneIM},
                {LRK_Deactivate, instance->globalConfig->hotkey.triggerKey, NULL, hasMoreThanOneIM},
            };

            FcitxInputContextKeyEvent* keyEvent = (FcitxInputContextKeyEvent*) event;
            bool isModifier = fcitx_key_is_modifier(keyEvent->detail.key);

            FcitxInputMethodPrivateState* privState = _fcitx_instance_input_context_get_private_state(instance, icEvent->inputContext);
            if (!keyEvent->detail.isRelease) {
                privState->lastKeyPressedTime = keyEvent->detail.time;
                bool handled = false;
                for (size_t i = 0; i < FCITX_ARRAY_SIZE(keyHandle); i ++) {
                    if (privState->releasedKey == keyHandle[i].releasedKey
                        && keyHandle[i].valid
                        && fcitx_key_list_check(keyHandle[i].keyList, keyEvent->detail.key)) {
                        handled = true;
                        if (isModifier) {
                            privState->releasedKey = keyHandle[i].releasedKey;
                        } else {
                            keyHandle[i].callback(instance, keyEvent->inputContext);
                        }
                        break;
                    }
                }

                if (handled) {
                    return true;
                } else {
                    privState->releasedKey = LRK_None;
                }
            } else {
                bool handled = false;
                if (isModifier && (keyEvent->detail.time < privState->lastKeyPressedTime + 500)) {
                    for (size_t i = 0; i < FCITX_ARRAY_SIZE(keyHandle); i ++) {
                        if (privState->releasedKey == keyHandle[i].releasedKey
                            && keyHandle[i].valid
                            && fcitx_key_list_check(keyHandle[i].keyList, keyEvent->detail.key)) {
                            handled = true;
                            keyHandle[i].callback(instance, keyEvent->inputContext);
                            break;
                        }
                    }
                }
                privState->releasedKey = LRK_None;
                if (handled) {
                    return true;
                }
            }

            if (fcitx_key_list_check(instance->globalConfig->hotkey.triggerKey, keyEvent->detail.key)) {
                if (fcitx_input_method_manager_get_group_size(instance->imManager, instance->group)) {
                } else {
                    return false;
                }
            }
        }

        FcitxInputMethod* im = NULL;
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
    } else if ((event->type & ET_EventTypeFlag) == ET_InputMethodManagerEventFlag) {
        switch (event->type) {
            case ET_InputMethodGroupAboutToReset:
                // TODO save global configuration here
                break;
            case ET_InputMethodGroupReset:
                fcitx_ptr_array_clear(instance->globalInputMethod);
                fcitx_input_context_manager_foreach(instance->icManager, fcitx_instance_input_method_group_reset_foreach, instance);
                fcitx_instance_set_input_method_group(instance, -1);
                break;
            case ET_NewInputMethodGroup:
                // TODO apply configuration
                fcitx_ptr_array_append(instance->globalInputMethod, NULL);
                if (instance->group < 0) {
                    fcitx_instance_set_input_method_group(instance, 0);
                }
                break;
            default:
                break;
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

void fcitx_instance_set_input_method_group(FcitxInstance* instance, int group)
{
    if (group >= 0 && (size_t) group >= fcitx_ptr_array_size(instance->globalInputMethod)) {
        return;
    }

    if (instance->group != group) {
        instance->group = group;
        // TODO notify
    }
}

int fcitx_instance_get_input_method_group(FcitxInstance* instance)
{
    return instance->group;
}

FCITX_EXPORT_API
void fcitx_instance_set_input_method_for_input_context(FcitxInstance* instance, FcitxInputContext* ic, FcitxInputMethodItem* item, bool local)
{
    FcitxInputMethodStateUpdate update;
    update.type = UpdateType_SetInputMethod;
    update.setIM.imItem = item;
    update.setIM.local = local;
    fcitx_input_context_set_property(ic, instance->inputMethodStateId, &update);
}

FCITX_EXPORT_API
void fcitx_instance_toggle_input_context(FcitxInstance* instance, FcitxInputContext* ic, bool quickSwitch)
{
    // TODO
}

FCITX_EXPORT_API
void fcitx_instance_scroll_input_method(FcitxInstance* instance, FcitxInputContext* ic, bool forward)
{
}

FCITX_EXPORT_API
void fcitx_instance_switch_input_method_group(FcitxInstance* instance, FcitxInputContext* ic, bool forward)
{
}

