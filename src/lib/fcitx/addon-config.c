#ifndef _FCITX_ADDON_CONFIG_H_
#define _FCITX_ADDON_CONFIG_H_

typedef struct _FcitxAddonConfig
{
    char *name; /**< addon name, used as a identifier */
    char *generalname; /**< addon name, translatable user visible string */
    char *comment; /**< longer desc translatable user visible string */
    boolean bEnabled; /**< enabled or not*/
    FcitxAddonCategory category; /**< addon category */
    char *type; /**< addon type */
    char *path;
    char *depend; /**< dependency string */
    int priority; /**< priority */
    IMRegisterMethod registerMethod; /**< the input method register method */
    char* registerArgument; /**< extra argument for register, unused for now */
    bool advanced; /**< hide from ui */
} FcitxAddonConfig;

#endif


CONFIG_BINDING_BEGIN(FcitxAddon)
CONFIG_BINDING_REGISTER("Addon", "Name", name)
CONFIG_BINDING_REGISTER("Addon", "GeneralName", generalname)
CONFIG_BINDING_REGISTER("Addon", "Comment", comment)
CONFIG_BINDING_REGISTER("Addon", "Category", category)
CONFIG_BINDING_REGISTER("Addon", "Enabled", bEnabled)
CONFIG_BINDING_REGISTER("Addon", "Library", library)
CONFIG_BINDING_REGISTER("Addon", "Type", type)
CONFIG_BINDING_REGISTER("Addon", "Dependency", depend)
CONFIG_BINDING_REGISTER("Addon", "Priority", priority)
CONFIG_BINDING_REGISTER("Addon", "SubConfig", subconfig)
CONFIG_BINDING_REGISTER("Addon", "IMRegisterMethod", registerMethod)
CONFIG_BINDING_REGISTER("Addon", "IMRegisterArgument", registerArgument)
CONFIG_BINDING_REGISTER("Addon", "UIFallback", uifallback)
CONFIG_BINDING_REGISTER("Addon", "Advance", advance)
CONFIG_BINDING_REGISTER("Addon", "LoadLocal", loadLocal)
CONFIG_BINDING_END()
