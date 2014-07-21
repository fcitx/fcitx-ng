#include "fcitx-utils/utils.h"

bool foreach_func(const char* path, size_t keyLen, void** data, void* userData)
{
    printf("%s\n", path);
    FcitxStandardPathFile* files = *data;
    while (files->fp) {
        printf("    %s\n", files->path);
        files++;
    }
    return false;
}

int main(int argc, char* argv[])
{
    FcitxStandardPath* sp = fcitx_standard_path_new();
    FcitxStandardPathFile* files = fcitx_standard_path_locate(sp, FSPT_Config, "fcitx/config", 0);
    fcitx_standard_path_file_close(files);
    FcitxStandardPathFilter filter;
    filter.flag = FSPFT_Suffix;
    filter.suffix = ".conf";
    FcitxDict* fileDict = fcitx_standard_path_match(sp, FSPT_Data, "fcitx/addon", &filter);
    fcitx_dict_foreach(fileDict, foreach_func, NULL);
    fcitx_dict_free(fileDict);
    fcitx_standard_path_free(sp);

    return 0;
}
