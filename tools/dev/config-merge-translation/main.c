/*
 * Copyright (C) 2015~2015 by CSSlayer
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

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <gettext-po.h>
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"

FILE* fout = NULL;
FcitxDict* poDict = NULL;
FcitxConfiguration* newconfig = NULL;

void usage()
{
    fprintf(stderr, "\n");
}


FcitxDict* parse_po(po_file_t pofile)
{
    FcitxDict* result = fcitx_dict_new(free);
    po_message_iterator_t iter = po_message_iterator(pofile, NULL);
    po_message_t msg;

    while ((msg = po_next_message(iter))) {
        const char* msgid = po_message_msgid(msg);
        const char* msgstr = po_message_msgstr(msg);

        fcitx_dict_insert_by_str(result, msgid, fcitx_utils_strdup(msgstr), true);
    }

    po_message_iterator_free(iter);

    return result;
}

FcitxDict* scan_po_files(const char* path)
{
    DIR* dir = opendir(path);
    FcitxDict* poDict = fcitx_dict_new((FcitxDestroyNotify) fcitx_dict_free);

    struct dirent* dp = NULL;
    while ((dp = readdir (dir)) != NULL) {
        if (!fcitx_utils_string_ends_with(dp->d_name, ".po")) {
            continue;
        }

        char* fullpath = NULL;
        asprintf(&fullpath, "%s/%s", path, dp->d_name);

        if (!fullpath) {
            break;
        }

        struct po_xerror_handler gettext_error_handler = {
            NULL,
            NULL
        };
        po_file_t pofile = po_file_read(fullpath, &gettext_error_handler);
        free(fullpath);
        if (!pofile) {
            perror(NULL);
            continue;
        }

        FcitxDict* strDict = parse_po(pofile);
        fcitx_dict_insert(poDict, dp->d_name, strlen(dp->d_name) - strlen(".po"), strDict, true);
        po_file_free(pofile);
    }

    closedir(dir);

    return poDict;
}

void translate_callback (FcitxConfiguration* config,
                         const char* path,
                         void* userData)
{
    FCITX_UNUSED(userData);
    const char* name = fcitx_configuration_get_name(config);
    const char* value = fcitx_configuration_get_value(config);
    if (name[0] == '_') {
        char* fullpath = NULL;
        asprintf(&fullpath ,"%.*s/%s", (int)(strlen(path) - strlen(name) - 1), path, name + 1);
        fcitx_configuration_set_value_by_path(newconfig, fullpath, value);
        free(fullpath);
        for (FcitxDictData* data = fcitx_dict_first(poDict); data; data = fcitx_dict_data_next(data)) {
            FcitxDict* strDict = data->data;
            char* i18nValue = NULL;
            if (!fcitx_dict_lookup_by_str(strDict, value, &i18nValue)) {
                continue;
            }

            if (!i18nValue[0]) {
                continue;
            }

            char* fullpath = NULL;
            asprintf(&fullpath ,"%.*s/%s[%s]", (int)(strlen(path) - strlen(name) - 1), path, name + 1, data->key);
            if (!fullpath) {
                break;
            }

            fcitx_configuration_set_value_by_path(newconfig, fullpath, i18nValue);
        }
    } else {
        fcitx_configuration_set_value_by_path(newconfig, path, value);
    }
}

int main(int argc, char* argv[])
{
    if (2 >= argc) {
        usage();
        exit(0);
    }

    FcitxConfiguration* config = NULL;
    int result = 0;
    FILE* fin = NULL;

    do {
        fin = fopen(argv[1], "r");
        if (!fin) {
            perror(NULL);
            result = 1;
            break;
        }

        if (argc > 3) {
            fout = fopen(argv[3], "w");
        } else {
            fout = stdout;
        }

        if (!fout) {
            perror(NULL);
            result = 1;
            break;
        }

        config = fcitx_ini_parse(fin, NULL);
        poDict = scan_po_files(argv[2]);
        newconfig = fcitx_configuration_new(NULL);

        fcitx_configuration_foreach(config, "", true, NULL, translate_callback, NULL);

        fcitx_ini_print(newconfig, fout);
    } while(0);

    if (fin) {
        fclose(fin);
    }

    if (fout) {
        fclose(fout);
    }

    fcitx_configuration_unref(config);
    fcitx_configuration_unref(newconfig);

    return result;
}
