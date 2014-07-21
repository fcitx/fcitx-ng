#ifndef _FCITX_UTILS_INI_PARSER_H_
#define _FCITX_UTILS_INI_PARSER_H_

#include "configuration.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

FcitxConfiguration* fcitx_ini_parse(FILE* fp);

void fcitx_ini_print(FcitxConfiguration* config, FILE* fp);

#endif
