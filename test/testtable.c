#include <fcitx-im/table/tabledict.h>
#include <assert.h>
#include <stdio.h>
int main(int argc, char* argv[])
{
    if (argc < 2) {
        return 1;
    }
    
    FcitxTableDict* tableDict = fcitx_tabledict_new();
    FILE* fp = fopen(argv[1], "r");
    assert(fcitx_tabledict_load(tableDict, fp));
    
    printf("%s\n", fcitx_tabledict_get_valid_chars(tableDict));
    printf("%s\n", fcitx_tabledict_get_ignored_chars(tableDict));
    
    fcitx_tabledict_unref(tableDict);
    return 0;
}