#ifndef FCITX_UTILS_INPUTDEVICE_H
#define FCITX_UTILS_INPUTDEVICE_H

typedef enum _FcitxInputDeviceCapability
{
    FIDT_Keyboard = 0x1,
    FIDT_OnScreenKeyboard = 0x2,
    FIDT_Tablet = 0x4,
    FIDT_None = 0
} FcitxInputDeviceCapabilty;

typedef struct _FcitxInputDevice FcitxInputDevice;

FcitxInputDevice* FcitxInputDevice();

#endif // FCITX_UTILS_INPUTDEVICE_H