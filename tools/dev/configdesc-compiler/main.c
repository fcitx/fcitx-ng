#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"
#include <fcitx-config/description.h>
#include "common.h"
#include "main.h"

FILE* fout;

typedef enum {
    Operation_PotFile,
    Operation_C_Header,
    Operation_C_Source,
} FcitxConfigDescCompileOperation;

void usage()
{
    fprintf(stderr, "\n");
}


int main(int argc, char* argv[])
{
    int c;
    FcitxConfigDescCompileOperation op = Operation_C_Header;
    char* name = NULL;
    char* prefix = NULL;
    char* output = NULL;
    char* includes = NULL;
    while ((c = getopt(argc, argv, "i:o:n:p:cth")) != EOF) {
        switch (c) {
            case 'i':
                fcitx_utils_string_swap(&includes, optarg);
                break;
            case 'n':
                fcitx_utils_string_swap(&name, optarg);
                break;
            case 'p':
                fcitx_utils_string_swap(&prefix, optarg);
                break;
            case 't':
                op = Operation_PotFile;
                break;
            case 'o':
                fcitx_utils_string_swap(&output, optarg);
                break;
            case 'c':
                op = Operation_C_Source;
                break;
            case 'h':
            default:
                usage();
                exit(0);
                break;
        }
    }

    if (optind >= argc) {
        usage();
        exit(0);
    }

    FcitxConfiguration* config = NULL;
    FcitxDescription* desc = NULL;
    int result = 0;
    FILE* fin = NULL;

    do {

        fin = fopen(argv[optind], "r");
        if (!fin) {
            perror(NULL);
            result = 1;
            break;
        }

        if (output) {
            fout = fopen(output, "w");
        } else {
            fout = stdout;
        }

        if (!fout) {
            perror(NULL);
            result = 1;
            break;
        }

        config = fcitx_ini_parse(fin, NULL);
        desc = fcitx_description_parse(config);

        if (desc->error) {
            fprintf(stderr, "Invalid desc file: %s\n", desc->errorMessage);
            fcitx_description_free(desc);
            result = 1;
            break;
        }

        switch(op) {
            case Operation_C_Header:
                if (name) {
                    compile_to_c_header(config, desc, name, prefix ? prefix : "", includes);
                }
                break;
            case Operation_C_Source:
                if (name) {
                    compile_to_c_source(config, desc, name, prefix ? prefix : "", includes);
                }
                break;
            case Operation_PotFile:
                compile_to_pot(config, desc);
                break;
        }
    } while(0);

    if (fin) {
        fclose(fin);
    }
    if (fout) {
        fclose(fout);
    }
    free(output);
    free(prefix);
    free(includes);
    free(name);

    fcitx_description_free(desc);
    fcitx_configuration_unref(config);

    return result;
}
