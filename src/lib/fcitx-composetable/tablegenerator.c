
#include <locale.h>
#include <stdio.h>
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include "tablegenerator-internal.h"

char* _get_locale() {
    char *name = setlocale(LC_CTYPE, (char *)0);
    return name;
}


bool _fcitx_compose_table_process_file(FcitxComposeTable* table, const char* filename);
    
void _fcitx_compose_table_find_system_compose_dir(FcitxComposeTable* table, int nPossibleLocation, const char* possibleLocation[]) {
    bool found = false;
    for (int i = 0; i < nPossibleLocation; ++i) {
        char* path;
        fcitx_utils_alloc_cat_str(path, possibleLocation[i], "/compose.dir");
        if (fcitx_utils_isreg(path)) {
            table->systemComposeDir = strdup(possibleLocation[i]);
            found = true;
        }
        
        free(path);
        
        if (found) {
            break;
        }
    }
    if (!found) {
        // should we ask to report this in the qt bug tracker?
        table->state = FCTS_UnknownSystemComposeDir;
    }
}

void _fcitx_compose_table_parse_sequence(FcitxComposeTable* table, const char* line)
{
    // we are interested in the lines with the following format:
    // <Multi_key> <numbersign> <S> : "♬" U266c # BEAMED SIXTEENTH NOTE
    const char* colon;
    if (!(colon = strchr(line, ':'))) {
        return;
    }
    
    uint32_t unicode;
    const char* quote;
    if (!(quote = strchr(colon, '"'))) {
        return;
    }
    if (quote[1] == '\\' && fcitx_utils_isdigit(quote[2])) {
        int base = 8;
        const char* start = quote + 2;
        if (quote[3] == 'x') {
            base = 16;
            start = quote + 3;
        }
        const char* quote2;
        if (!(quote2 = strrchr(quote + 3, '"'))) {
            return;
        }
        if (quote2 <= start) {
            return;
        }
        
        char* strDigit = strndup(start, quote2 - start);
        long unsigned int num = strtoul(strDigit, NULL, base);
        free(strDigit);
        
        unicode = FcitxKeySymToUnicode((FcitxKeySym) num);
    } else {
        unicode = fcitx_utf8_get_char_validated(quote + 1, FCITX_UTF8_MAX_LENGTH);
    }
    
    FcitxStringList* keys = fcitx_utils_string_list_new();
    const char* key = line;
    while (true) {
        while (key < colon && *key != '<') {
            key ++;
        }
        
        if (*key != '<') {
            break;
        }
        
        const char *start = key + 1;
        key = start;
        
        while (key < colon && *key != '>') {
            key ++;
        }
        
        if (*key != '>') {
            break;
        }
        
        const char *end = key;
        
        if (end == start) {
            continue;
        }
        
        fcitx_utils_string_list_append_len(keys, start, end - start);
    }
    
    FcitxComposeTableElement element;
    element.value = unicode;
    
    // Convert to X11 keysym
    for (uint i = 0; i < FCITX_KEYSEQUENCE_MAX_LEN; i++) {
        if (i < utarray_len(keys)) {
            char** key = utarray_eltptr(keys, i);
            char* keystr = *key;
            if (strcmp(keystr, "dead_inverted_breve") == 0) {
                keystr = "dead_invertedbreve";
            } else if (strcmp(keystr, "dead_double_grave") == 0) {
                keystr = "dead_doublegrave";
            }
            
            element.keys[i] = FcitxKeySymFromString(keystr);
        } else {
            element.keys[i] = FcitxKey_None;
        }
    }
    fcitx_utils_string_list_free(keys);
    
    utarray_push_back(table->composeTable, &element);
}

void _fcitx_compose_table_parse_include(FcitxComposeTable* table, const char* line)
{
    // Parse something that looks like:
    // include "/usr/share/X11/locale/en_US.UTF-8/Compose"
    char* p = strchr(line + strlen("include"), '"');
    
    if (!p) {
        return;
    }
    char* trimLine = fcitx_utils_trim(p);
    size_t l = strlen(trimLine);
    if (l > 0 && trimLine[l - 1] == '\"' ) {
        trimLine[l - 1] = '\0';
        
        char* result;
        result = fcitx_utils_string_replace(trimLine, "%H", getenv("HOME"), true);
        if (result) {
            free(trimLine);
            trimLine = result;
        }
        result = fcitx_utils_string_replace(trimLine, "%L", table->locale, true);
        if (result) {
            free(trimLine);
            trimLine = result;
        }
        result = fcitx_utils_string_replace(trimLine, "%S", table->systemComposeDir, true);
        if (result) {
            free(trimLine);
            trimLine = result;
        }
        _fcitx_compose_table_process_file(table, trimLine);
    }
    
    free(trimLine);
}

bool _fcitx_compose_table_process_file(FcitxComposeTable* table, const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp) {

        char* line = NULL;
        size_t bufSize = 0;
        while (getline(&line, &bufSize, fp) != -1) {
            if (line[0] == '<') {
                _fcitx_compose_table_parse_sequence(table, line);
            } else if (fcitx_utils_string_starts_with(line, "include")) {
                _fcitx_compose_table_parse_include(table, line);
            }
        }
        
        free(line);
        fclose(fp);
        
        return true;
    }
    
    return false;
}

void _fcitx_compose_table_read_locale_mappings(FcitxComposeTable* table)
{
    char* mappingsDir;
    fcitx_utils_alloc_cat_str(mappingsDir, table->systemComposeDir, "/compose.dir");
    FILE* mappings = fopen(mappingsDir, "r");
    
    if (mappings) {
        char* line = NULL;
        size_t bufSize = 0;
        while (getline(&line, &bufSize, mappings) != -1) {
            char* trimLine = fcitx_utils_trim(line);
            
            if (trimLine[0] != '#' && trimLine[0] != '\0' && fcitx_utils_islower(trimLine[0])) {
                FcitxStringList* list = fcitx_utils_string_split_full(trimLine, FCITX_WHITESPACE, false);
                if (utarray_len(list) >= 2) {
                    char **plocale = utarray_eltptr(list, 1);
                    char **pitem = utarray_eltptr(list, 0);
                    char *locale = *plocale, *item = *pitem;
                    // we steal this string from list
                    *pitem = NULL;
                    
                    size_t itemLen = strlen(item);
                    if (itemLen > 0 && item[itemLen - 1] == ':') {
                        item[itemLen - 1] = '\0';
                    }
                    
                    char* p = locale;
                    while(*p) {
                        *p = fcitx_utils_toupper(*p);
                        p++;
                    }
                    
                    fcitx_dict_insert_by_str(table->localeToTable, locale, item, true);
                }
                fcitx_utils_string_list_free(list);
            }
            
            free(trimLine);
        }
        free(line);
        fclose(mappings);
    }
    
    free(mappingsDir);
}

void _fcitx_compose_table_find_compose_file(FcitxComposeTable* table)
{
    bool found = false;
    char* env;
    // check if XCOMPOSEFILE points to a Compose file
    if ((env = getenv("XCOMPOSEFILE")) != NULL) {
        // TODO: check end with
        found = _fcitx_compose_table_process_file(table, env);
    }
    // check if user’s home directory has a file named .XCompose
    if (!found && table->state == FCTS_NoErrors) {
        do {
            char* home = getenv("HOME");
            if (!home) {
                break;
            }
            char* composeFile;
            fcitx_utils_alloc_cat_str(composeFile, home, "/.XCompose");
            found = _fcitx_compose_table_process_file(table, composeFile);
            free(composeFile);
        } while(0);
    }
    // check for the system provided compose files
    if (!found && table->state == FCTS_NoErrors) {
        _fcitx_compose_table_read_locale_mappings(table);
        if (table->state == FCTS_NoErrors) {
            char* item;
            if (!fcitx_dict_lookup_by_str(table->localeToTable, table->locale, (void**) &item)) {
                table->state = FCTS_UnsupportedLocale;
            } else {
                char* composeFile;
                fcitx_utils_alloc_cat_str(composeFile, table->systemComposeDir, "/", item);
                found = _fcitx_compose_table_process_file(table, composeFile);
                free(composeFile);
            }
        }
    }
    if (found && utarray_len(table->composeTable) == 0) {
        table->state = FCTS_EmptyTable;
    }
    if (!found) {
        table->state = FCTS_MissingComposeFile;
    }
}

int fcitx_compose_table_element_cmp(const void* a, const void* b, void* data)
{
    const FcitxComposeTableElement* ea = a;
    const FcitxComposeTableElement* eb = b;
    for (size_t i = 0; i < FCITX_KEYSEQUENCE_MAX_LEN; i++) {
        if (ea->keys[i] != eb->keys[i]) {
            return (ea->keys[i] < eb->keys[i]) ? -1 : 1;
        }
    }
    return 0;
}

void _fcitx_compose_table_order_compose_table(FcitxComposeTable* table)
{
    // Stable-sorting to ensure that the item that appeared before the other in the
    // original container will still appear first after the sort. This property is
    // needed to handle the cases when user re-defines already defined key sequence
    utarray_msort_r(table->composeTable, fcitx_compose_table_element_cmp, NULL);
}

static const UT_icd composeElementIcd = {sizeof(FcitxComposeTableElement), 0, 0, 0};

FcitxComposeTable* _fcitx_compose_table_alloc(const char* locale)
{
    FcitxComposeTable* table = fcitx_utils_new(FcitxComposeTable);
    table->localeToTable = fcitx_dict_new(free);
    table->composeTable = utarray_new(&composeElementIcd);
    table->locale = locale ? strdup(locale) : strdup(_get_locale());
    char* p = table->locale;
    while(*p) {
        *p = fcitx_utils_toupper(*p);
        p++;
    }

    return fcitx_compose_table_ref(table);
}

FCITX_EXPORT_API
FcitxComposeTable* fcitx_compose_table_new(const char* locale)
{
    FcitxComposeTable* table = _fcitx_compose_table_alloc(locale);
    int nPossibleLocation = 0;
    const char* possibleLocation[5];
    char* needFree[2];

    char* env;
    if ((env = getenv("FCITX_COMPOSE_DIR")) != NULL) {
        possibleLocation[nPossibleLocation++] = env;
    }

    possibleLocation[nPossibleLocation++] = needFree[0] = fcitx_utils_get_fcitx_path_with_filename("datadir", "X11/locale");
    possibleLocation[nPossibleLocation++] = needFree[1] = fcitx_utils_get_fcitx_path_with_filename("libdir", "X11/locale");
    possibleLocation[nPossibleLocation++] = "/usr/share/X11/locale";
    possibleLocation[nPossibleLocation++] = "/usr/lib/X11/locale";
    
    _fcitx_compose_table_find_system_compose_dir(table, nPossibleLocation, possibleLocation);
    _fcitx_compose_table_find_compose_file(table);
    _fcitx_compose_table_order_compose_table(table);
    
    free(needFree[0]);
    free(needFree[1]);
    
    return table;
}

FCITX_EXPORT_API
FcitxComposeTable* fcitx_compose_table_new_from_file(const char* systemComposeDir, const char* composeFile, const char* locale)
{
    FcitxComposeTable* table = _fcitx_compose_table_alloc(locale);
    int nPossibleLocation = 1;
    const char* possibleLocation[1] = {systemComposeDir};
    _fcitx_compose_table_find_system_compose_dir(table, nPossibleLocation, possibleLocation);

    bool found = _fcitx_compose_table_process_file(table, composeFile);
    if (found && utarray_len(table->composeTable) == 0) {
        table->state = FCTS_EmptyTable;
    }
    if (!found) {
        table->state = FCTS_MissingComposeFile;
    }
    _fcitx_compose_table_order_compose_table(table);

    return table;
}

void fcitx_compose_table_free(FcitxComposeTable* table)
{
    free(table->locale);
    free(table->systemComposeDir);
    utarray_free(table->composeTable);
    fcitx_dict_free(table->localeToTable);
    free(table);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxComposeTable, fcitx_compose_table)

FCITX_EXPORT_API
void
fcitx_compose_table_print(FcitxComposeTable* table)
{
    utarray_foreach(element, table->composeTable, FcitxComposeTableElement) {
        int i = 0;
        while(element->keys[i] != FcitxKey_None) {
            const char* key = FcitxKeySymToString(element->keys[i]);
            printf("<%s> ", key);
            i ++;
        }

        char utf8[FCITX_UTF8_MAX_LENGTH + 1];
        fcitx_ucs4_to_utf8(element->value, utf8);
        printf(": \"%s\"\n", utf8);
    }
}

FcitxComposeTableState fcitx_compose_table_state(FcitxComposeTable* table)
{
    return table->state;
}
