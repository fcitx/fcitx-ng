#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>
#include "config.h"
#include "utils.h"
#include "macro-internal.h"

struct _FcitxStandardPath
{
    char* configHome;
    FcitxStringList* configDirs;
    char* dataHome;
    FcitxStringList* dataDirs;
    char* cacheHome;
    char* runtimeDir;
    int32_t refcount;
    FcitxStringList* addonDirs;
};

// http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
char* fcitx_standard_default_path_construct(const char* env, const char* defaultPath)
{
    char* dir = getenv(env);
    if (dir && dir[0]) {
        dir = strdup(dir);
    } else {
        // caller need to ensure HOME is not empty;
        if (defaultPath[0] != '/') {
            const char* home = getenv("HOME");
            asprintf(&dir, "%s/%s", home, defaultPath);
        } else {
            if (strcmp(env, "XDG_RUNTIME_DIR") == 0) {
                asprintf(&dir, "%s/fcitx-runtime-%u", defaultPath, geteuid());
                if (!fcitx_utils_isdir(dir)) {
                    if (mkdir(dir, 0700) != 0) {
                        free(dir);
                        return NULL;
                    }
                }

            } else {
                dir = strdup(defaultPath);
            }
        }
    }

    if (dir && strcmp(env, "XDG_RUNTIME_DIR") == 0) {
        struct stat buf;
        if (stat(dir, &buf) != 0
            || buf.st_uid != geteuid()
            || buf.st_mode != 0700) {
            free(dir);
            return NULL;
        }
    }
    return dir;
}

FcitxStringList* fcitx_standard_default_paths_construct(const char* env, const char* defaultPath, const char* fcitxPath)
{
    FcitxStringList* dirs = fcitx_utils_string_list_new();

    if (fcitxPath) {
        char* path = fcitx_utils_get_fcitx_path(fcitxPath);
        fcitx_utils_string_list_append_no_copy(dirs, path);
    }

    const char* dir = getenv(env);
    if (!dir || !dir[0]) {
        dir = defaultPath;
    }

    dirs = fcitx_utils_string_list_append_split_full(dirs, dir, ":", false);

    return dirs;
}

FCITX_EXPORT_API
FcitxStandardPath* fcitx_standard_path_new()
{
    FcitxStandardPath* path = fcitx_utils_new(FcitxStandardPath);
    // initialize user directory
    path->configHome = fcitx_standard_default_path_construct("XDG_CONFIG_HOME", ".config");
    path->configDirs = fcitx_standard_default_paths_construct("XDG_CONFIG_DIRS", "/etc/xdg", NULL);
    path->dataHome = fcitx_standard_default_path_construct("XDG_DATA_HOME", ".local/share");
    path->dataDirs = fcitx_standard_default_paths_construct("XDG_DATA_DIRS", "/usr/local/share:/usr/share", "datadir");
    path->cacheHome = fcitx_standard_default_path_construct("XDG_CACHE_HOME", ".cache");
    const char* tmpdir = getenv("TMPDIR");
    path->runtimeDir = fcitx_standard_default_path_construct("XDG_RUNTIME_DIR", !tmpdir || !tmpdir[0] ? "/tmp" : tmpdir);
    path->addonDirs = fcitx_standard_default_paths_construct("FCITX_ADDON_DIRS", FCITX_INSTALL_ADDONDIR, NULL);

    return fcitx_standard_path_ref(path);
}

void fcitx_standard_path_get(FcitxStandardPath* sp, FcitxStandardPathType type, char** pFirst, FcitxStringList** pList)
{
    char* firstDir = NULL;
    FcitxStringList* list = NULL;
    switch (type) {
        case FSPT_Config:
            firstDir = sp->configHome;
            list = sp->dataDirs;
            break;
        case FSPT_Data:
            firstDir = sp->dataHome;
            list = sp->dataDirs;
            break;
        case FSPT_Cache:
            firstDir = sp->cacheHome;
            break;
        case FSPT_Runtime:
            firstDir = sp->runtimeDir;
            break;
        case FSPT_Addon:
            firstDir = NULL;
            list = sp->addonDirs;
            break;
    }

    *pFirst = firstDir;
    *pList = list;
}

char* fcitx_standard_path_construct_path(const char* basepath, const char* path) {
    if (!basepath || !basepath[0]) {
        return NULL;
    }
    char* result = NULL;
    asprintf(&result, "%s/%s", basepath, path);
    if (result) {
        fcitx_utils_clean_path(result, result);
    }
    return result;
}

FcitxStandardPathFile fcitx_standard_path_try_open(const char* basepath, const char* path, uint32_t flag)
{
    char* fullPath = fcitx_standard_path_construct_path(basepath, path);
    FILE* fp = NULL;
    const char* flags = (flag & FSPFT_Append) ? "a" : (flag & FSPFT_Writable ? "w" : "r");
    if (!fullPath) {
        return (FcitxStandardPathFile) { NULL, NULL };
    }
    fp = fopen(fullPath, flags);
    if (!fp && errno == ENOENT) {
        if (flag & FSPFT_Write) {
            char* copyFullPath = strdup(fullPath);
            char* dirName = dirname(copyFullPath);
            if (fcitx_utils_make_path(dirName)) {
                fp = fopen(fullPath, flags);
            }
            free(copyFullPath);
        }
    }

    if (!fp) {
        free(fullPath);
        fullPath = NULL;
    }

    return (FcitxStandardPathFile) { fp, fullPath };
}

FCITX_EXPORT_API
FcitxStandardPathFile* fcitx_standard_path_locate(FcitxStandardPath* sp, FcitxStandardPathType type, const char* path, uint32_t flag)
{
    if (path[0] == '/') {
        const char* flags = (flag & FSPFT_Append) ? "a" : (flag & FSPFT_Writable ? "w" : "r");
        FILE* fp = fopen(path, flags);
        if (!fp) {
            return NULL;
        }
        FcitxStandardPathFile* result = fcitx_utils_newv(FcitxStandardPathFile, 2);
        result[0].fp = fp;
        result[0].path = strdup(path);
        return result;
    }

    char* firstDir = NULL;
    FcitxStringList* list = NULL;
    fcitx_standard_path_get(sp, type, &firstDir, &list);
    if (!firstDir && !list) {
        return NULL;
    }

    // if we don't write and there is a list
    bool checkList = (!(flag & FSPFT_Write) && list);

    FcitxStandardPathFile fileFirst = { NULL, NULL };

    if (firstDir) {
        fileFirst = fcitx_standard_path_try_open(firstDir, path, flag);
    }
    FcitxStandardPathFile* result = NULL;
    if (!checkList) {
        if (!checkList && !fileFirst.fp) {
            return NULL;
        }
        result = fcitx_utils_newv(FcitxStandardPathFile, 2);
        result[0] = fileFirst;
    } else {
        size_t idx = 0;
        size_t resultSize = (flag & FSPFT_LocateAll) ? (1 + (fileFirst.fp ? 1 : 0) + utarray_len(list)) : 2;
        if (fileFirst.fp) {
            // alloc on required
            result = fcitx_utils_newv(FcitxStandardPathFile, resultSize);
            result[idx] = fileFirst;
            idx++;
        }

        if (!(flag & FSPFT_Write) && list) {
            utarray_foreach(dir, list, char*) {
                if (idx == 1 && !(flag & FSPFT_LocateAll)) {
                    break;
                }

                FcitxStandardPathFile file = fcitx_standard_path_try_open(*dir, path, flag);

                if (file.fp) {
                    // alloc on required
                    if (!result) {
                        result = fcitx_utils_newv(FcitxStandardPathFile, resultSize);
                    }
                    result[idx] = file;
                    idx++;
                }
            }
        }
    }

    return result;
}

typedef struct {
    FcitxStandardPath* sp;
    const char* path;
    uint32_t flag;
    FcitxStandardPathType type;
} fcitx_standard_path_foreach_context;

bool fcitx_standard_path_foreach_func(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(keyLen);
    fcitx_standard_path_foreach_context* context = userData;
    char* fullPath = NULL;
    asprintf(&fullPath, "%s%s%s", context->path, context->path[0] ? "/" : "", key);
    *data = fcitx_standard_path_locate(context->sp, context->type, fullPath, context->flag);
    free(fullPath);
    return false;
}

bool fcitx_standard_path_remove_if_empty_func(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(key);
    FCITX_UNUSED(keyLen);
    FCITX_UNUSED(userData);
    return (*data == NULL);
}

FCITX_EXPORT_API
FcitxDict* fcitx_standard_path_match(FcitxStandardPath* sp, FcitxStandardPathType type, const char* path, FcitxStandardPathFilter* filter)
{
    char* firstDir = NULL;
    FcitxStringList* list = NULL;
    fcitx_standard_path_get(sp, type, &firstDir, &list);
    if (!firstDir && !list) {
        return NULL;
    }

    FcitxDict* result = fcitx_dict_new((FcitxDestroyNotify) fcitx_standard_path_file_close);

    size_t suffixlen = 0;
    size_t prefixlen = 0;
    const char* prefix = NULL, *suffix = NULL;

    if (filter->flag & FSPFT_Prefix) {
        prefixlen = strlen(filter->prefix);
        prefix = filter->prefix;
    }

    if (filter->flag & FSPFT_Suffix) {
        suffixlen = strlen(filter->suffix);
        suffix = filter->suffix;
    }

    size_t len = (firstDir ? 1 : 0) + (list ? utarray_len(list) : 0);

    for (size_t i = 0; i < len; i++) {
        const char* dirBasePath = NULL;
        if (firstDir) {
            dirBasePath = (i == 0) ? firstDir : (*(char**) utarray_eltptr(list, i - 1));
        } else {
            dirBasePath =  (*(char**) utarray_eltptr(list, i));
        }
        char* fullPath = fcitx_standard_path_construct_path(dirBasePath, path);
        do {
            DIR* dir = opendir(fullPath);
            if (dir == NULL)
                break;

            struct dirent* drt;
            /* collect all *.conf files */
            while ((drt = readdir(dir)) != NULL) {
                size_t nameLen = strlen(drt->d_name);
                if (nameLen <= suffixlen + prefixlen)
                    continue;

                // do the filtering
                if (suffix && strcmp(drt->d_name + nameLen - suffixlen, suffix) != 0) {
                    continue;
                }
                if (prefix && strncmp(drt->d_name, prefix, prefixlen) != 0) {
                    continue;
                }
                if ((filter->flag & FSPFT_Callback) && !filter->callback(drt->d_name, filter->userData)) {
                    continue;
                }

                if (!fcitx_dict_lookup_by_str(result, drt->d_name, NULL)) {
                    fcitx_dict_insert_by_str(result, drt->d_name, NULL, false);
                }
            }

            closedir(dir);
        } while(0);
        free(fullPath);
    }

    fcitx_standard_path_foreach_context context;
    context.flag = filter->flag;
    context.path = path;
    context.sp = sp;
    context.type = type;
    fcitx_dict_foreach(result, fcitx_standard_path_foreach_func, &context);
    fcitx_dict_remove_if(result, fcitx_standard_path_remove_if_empty_func, NULL);

    if (filter->flag & FSPFT_Sort) {
        fcitx_dict_sort(result, NULL, NULL);
    }

    return result;
}

FCITX_EXPORT_API
void fcitx_standard_path_file_close(FcitxStandardPathFile* file)
{
    if (!file) {
        return;
    }

    size_t idx = 0;
    while (file[idx].fp) {
        fclose(file[idx].fp);
        free(file[idx].path);
        idx++;
    }
    free(file);
}

void fcitx_standard_path_free(FcitxStandardPath* sp)
{
    free(sp->cacheHome);
    free(sp->configHome);
    free(sp->runtimeDir);
    free(sp->dataHome);
    fcitx_utils_string_list_free(sp->configDirs);
    fcitx_utils_string_list_free(sp->dataDirs);
    fcitx_utils_string_list_free(sp->addonDirs);
    free(sp);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxStandardPath, fcitx_standard_path)
