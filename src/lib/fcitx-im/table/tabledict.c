#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "tabledict.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/atomic.h"
#include "fcitx-utils/memory-pool.h"
#include "fcitx-utils/utf8.h"
#include "fcitx-utils/dict.h"
#include "fcitx-utils/list.h"
#include "fcitx-utils/multiradix.h"
#include "fcitx-utils/stringutils.h"

#define PHRASE_MAX_LENGTH 10
#define FH_MAX_LENGTH  10
#define TABLE_AUTO_SAVE_AFTER 1024
#define AUTO_PHRASE_COUNT 10000

typedef enum _FcitxTableRecordType
{
    RECORDTYPE_NORMAL,
    RECORDTYPE_PINYIN,
    RECORDTYPE_CONSTRUCT,
    RECORDTYPE_PROMPT
} FcitxTableRecordType;

typedef enum _FcitxTableRuleItemFlag {
    FTRIF_CharIndexFromFront,
    FTRIF_CharIndexFromBack,
} FcitxTableRuleItemFlag;

typedef struct _FcitxTableRuleItem {
    uint8_t   flag;  // FcitxTableRuleItemFlag
    uint8_t   which; //第几个字
    uint8_t   index; //第几个编码
} FcitxTableRuleItem;

typedef enum _FcitxTableRuleFlag {
    FTRF_CharNumberAbove,
    FTRF_CharNumberEqual,
} FcitxTableRuleFlag;

typedef struct {
    uint8_t                  nWords; //多少个字
    uint8_t                  flag;  // FcitxTableRuleFlag
    FcitxTableRuleItem      *items;
} FcitxTableRule;

typedef struct _RECORD {
    char* word;
    uint32_t index;
    uint32_t freq;
    int8_t   type; // FcitxTableRecordType
    FcitxListHead list;
} RECORD;

struct _FcitxTableDict
{
    FcitxTableDictParseError error;

    int refcount;
    char* validChars; // allocated, never be NULL
    char* ignoredChars; // allocated, might be NULL
    uint8_t codeLength;
    uint8_t pyCodeLength;
    boolean hasPinyin; // use pinyin feature

    FcitxTableRule* rule; // allocated / NULL

    FcitxDict* singleChar;
    FcitxDict* singleCharCons;

    RECORD* promptCode[UCHAR_MAX + 1];
    uint32_t tableIndex;
    FcitxMultiRadixTree* radix;
    FcitxMemoryPool* pool;

    char cPinyinKey;
    char cPromptKey;
    char cPhraseKey;
};

FCITX_EXPORT_API
FcitxTableDict* fcitx_table_dict_new()
{
    FcitxTableDict* tableDict = fcitx_utils_new(FcitxTableDict);

    fcitx_table_dict_ref(tableDict);

    tableDict->singleChar = fcitx_dict_new(free);
    tableDict->singleCharCons = fcitx_dict_new(free);
    tableDict->pool = fcitx_memory_pool_new();
    tableDict->radix = fcitx_multi_radix_tree_new(offsetof(RECORD, list), NULL, NULL);

    return tableDict;
}

void fcitx_table_dict_free(FcitxTableDict* tableDict)
{
    if (tableDict->rule) {
        for (size_t i = 0; i < (tableDict->codeLength - 1u); i++) {
            free(tableDict->rule[i].items);
        }
        free(tableDict->rule);
    }
    fcitx_dict_free(tableDict->singleChar);
    fcitx_dict_free(tableDict->singleCharCons);
    fcitx_multi_radix_tree_free(tableDict->radix);
    // must be free at last
    fcitx_memory_pool_free(tableDict->pool);
    free(tableDict->ignoredChars);
    free(tableDict->validChars);
    free(tableDict);
}

FCITX_EXPORT_API
FcitxTableDict* fcitx_table_dict_ref(FcitxTableDict* tableDict)
{
    fcitx_utils_atomic_add (&tableDict->refcount, 1);
    return tableDict;
}

enum {
    STR_KEYCODE,
    STR_CODELEN,
    STR_IGNORECHAR,
    STR_PINYIN,
    STR_PINYINLEN,
    STR_DATA,
    STR_RULE,
    STR_PROMPT,
    STR_CONSTRUCTPHRASE,
    CONST_STR_SIZE
};

#define CHECK_OPTION(str, x) ((strstr((str), strConst[x]) == (str)) || (strstr((str), strConstNew[x]) == (str)))
#define ADD_LENGTH(str, x) ((strstr((str), strConst[x]) == (str)) ? (strlen(strConst[x])) : (strlen(strConstNew[x])))

char* strConst[CONST_STR_SIZE] = { "键码=", "码长=", "规避字符=", "拼音=", "拼音长度=" , "[数据]", "[组词规则]", "提示=", "构词="};
char* strConstNew[CONST_STR_SIZE] = { "KeyCode=", "Length=", "InvalidChar=", "Pinyin=", "PinyinLength=" , "[Data]", "[Rule]", "Prompt=", "ConstructPhrase="};

#define _RETURN_ERROR_IF(CONDITION, ERRORCODE) \
    do { \
        if (CONDITION) { \
            tableDict->error = ERRORCODE; \
            return false; \
        } \
    } while(0)

#define _RETURN_ERROR(ERRORCODE) \
    do { \
        tableDict->error = ERRORCODE; \
        return false; \
    } while(0)

static boolean fcitx_table_dict_load_metadata_text(FcitxTableDict* tableDict, FILE* fp, char** pBuffer, size_t* pBufferLen)
{
    while (getline(pBuffer, pBufferLen, fp) != -1) {
        char* trimmedBuffer = fcitx_utils_inplace_trim(*pBuffer);
        // skip comments
        if (trimmedBuffer[0] == '#') {
            continue;
        }

        if (CHECK_OPTION(trimmedBuffer, STR_KEYCODE)) {
            trimmedBuffer += ADD_LENGTH(trimmedBuffer, STR_KEYCODE);
            _RETURN_ERROR_IF(strlen(trimmedBuffer) == 0, FTDPE_ValidCharsInvalid);
            tableDict->validChars = strdup(trimmedBuffer);
        } else if (CHECK_OPTION(trimmedBuffer, STR_CODELEN)) {
            char* end;
            trimmedBuffer += ADD_LENGTH(trimmedBuffer, STR_CODELEN);
            ulong len = strtoul(trimmedBuffer, &end, 10);

            _RETURN_ERROR_IF(end && *end != '\0', FTDPE_CodeLengthInvalid);
            _RETURN_ERROR_IF(len > UINT8_MAX || len == 0, FTDPE_CodeLengthInvalid);

            tableDict->codeLength = (uint8_t) len;
        } else if (CHECK_OPTION(trimmedBuffer, STR_IGNORECHAR)) {
            trimmedBuffer += ADD_LENGTH(trimmedBuffer, STR_IGNORECHAR);
            tableDict->ignoredChars = strdup(trimmedBuffer);
        } else if (CHECK_OPTION(trimmedBuffer, STR_PINYIN)) {
            trimmedBuffer += ADD_LENGTH(trimmedBuffer, STR_PINYIN);

            while (*trimmedBuffer == ' ' && *trimmedBuffer != '\0')
                trimmedBuffer++;

            tableDict->cPinyinKey = *trimmedBuffer;
        } else if (CHECK_OPTION(trimmedBuffer, STR_PROMPT)) {
            trimmedBuffer += ADD_LENGTH(trimmedBuffer, STR_PROMPT);

            while (*trimmedBuffer == ' ' && *trimmedBuffer != '\0')
                trimmedBuffer++;

            tableDict->cPromptKey = *trimmedBuffer;
        } else if (CHECK_OPTION(trimmedBuffer, STR_CONSTRUCTPHRASE)) {
            trimmedBuffer += ADD_LENGTH(trimmedBuffer, STR_CONSTRUCTPHRASE);

            while (*trimmedBuffer == ' ' && *trimmedBuffer != '\0') {
                trimmedBuffer++;
            }

            tableDict->cPhraseKey = *trimmedBuffer;
        } else if (CHECK_OPTION(trimmedBuffer, STR_PINYINLEN)) {
            trimmedBuffer += ADD_LENGTH(trimmedBuffer, STR_PINYINLEN);
            tableDict->pyCodeLength = atoi(trimmedBuffer);
        } else if (trimmedBuffer[0] == '[') {
            // leave this line for next function
            _RETURN_ERROR_IF(tableDict->codeLength == 0, FTDPE_CodeLengthMissing);
            _RETURN_ERROR_IF(tableDict->validChars == NULL, FTDPE_ValidCharsMissing);
            return true;
        }
    }

    _RETURN_ERROR(FTDPE_NoDataSection);
}

static boolean fcitx_table_dict_load_rule_text(FcitxTableDict* tableDict, FILE* fp, char** pBuffer, size_t* pBufferLen)
{
    char* trimmedBuffer = fcitx_utils_inplace_trim(*pBuffer);
    if (!CHECK_OPTION(trimmedBuffer, STR_RULE)) {
        return true;
    }

    tableDict->rule = fcitx_utils_newv(FcitxTableRule, (tableDict->codeLength - 1));
    for (size_t i = 0; i < (tableDict->codeLength - 1u); i++) {
        tableDict->rule[i].items = fcitx_utils_newv(FcitxTableRuleItem, tableDict->codeLength);
    }
    // rule index
    uint8_t index = 0;
    while (getline(pBuffer, pBufferLen, fp) != -1) {
        trimmedBuffer = fcitx_utils_inplace_trim(*pBuffer);
        if (trimmedBuffer[0] == '[') {
            _RETURN_ERROR_IF(index != tableDict->codeLength - 1, FTDPT_RuleInvalid);
            return true;
        }

        if (trimmedBuffer[0] == '#') {
            continue;
        }

        if (index == tableDict->codeLength - 1) {
            continue;
        }

        switch (trimmedBuffer[0]) {
            case 'e':
            case 'E':
                tableDict->rule[index].flag = FTRF_CharNumberEqual;
                break;

            case 'a':
            case 'A':
                tableDict->rule[index].flag = FTRF_CharNumberAbove;
                break;

            default:
                _RETURN_ERROR(FTDPT_RuleInvalid);
        }

        trimmedBuffer++;
        char* p = strchr(trimmedBuffer, '=');

        if (!p) {
            _RETURN_ERROR(FTDPT_RuleInvalid);
        }

        *p = 0;
        char* end = NULL;
        ulong nWords = strtoul(trimmedBuffer, &end, 10);
        _RETURN_ERROR_IF(end && *end != '\0', FTDPT_RuleInvalid);
        _RETURN_ERROR_IF(nWords > UINT8_MAX || nWords == 0, FTDPE_CodeLengthInvalid);
        tableDict->rule[index].nWords = nWords;

        p++;
        for (int j = 0; j < tableDict->codeLength; j++) {
            while (fcitx_utils_isspace(*p)) {
                p++;
            }

            switch (*p) {
            case 'p':
            case 'P':
                tableDict->rule[index].items[j].flag = FTRIF_CharIndexFromFront;
                break;

            case 'n':
            case 'N':
                tableDict->rule[index].items[j].flag = FTRIF_CharIndexFromBack;
                break;

            default:
                _RETURN_ERROR(FTDPT_RuleInvalid);
            }

            p++;

            _RETURN_ERROR_IF(!fcitx_utils_isdigit(p[0]) || !fcitx_utils_isdigit(p[1]), FTDPT_RuleInvalid);
            tableDict->rule[index].items[j].which = p[0] - '0';
            tableDict->rule[index].items[j].index = p[1] - '0';
            p += 2;


            if (j != (tableDict->codeLength - 1)) {
                while (fcitx_utils_isspace(*p)) {
                    p++;
                }
                _RETURN_ERROR_IF(*p != '+', FTDPT_RuleInvalid);

                p++;
            }
        }

        index ++;
    }
    _RETURN_ERROR(FTDPE_NoDataSection);
}

static void fcitx_table_dict_store_record(FcitxTableDict* tableDict, const char* key, RECORD* record)
{
    if (record->index > tableDict->tableIndex) {
        tableDict->tableIndex = record->index;
    }

    if (record->type == RECORDTYPE_NORMAL || record->type == RECORDTYPE_PINYIN) {
        fcitx_multi_radix_tree_add(tableDict->radix, key, record);
    }

    if (record->type == RECORDTYPE_PINYIN) {
        tableDict->hasPinyin = true;
    }

    if (record->type == RECORDTYPE_PROMPT && strlen(key) == 1) {
        tableDict->promptCode[(uint8_t) key[0]] = record;
    }

    /*
     * if we have construct phrase rule and current phrase is a single character
     * then we put it in a table
     */
    if (tableDict->rule && fcitx_utf8_strlen(record->word) == 1 && !fcitx_utils_strchr0(tableDict->ignoredChars, key[0]))
    {
        FcitxDict* tableSingleChar = NULL;
        if (record->type == RECORDTYPE_NORMAL) {
            tableSingleChar = tableDict->singleChar;
        } else if (record->type == RECORDTYPE_CONSTRUCT) {
            tableSingleChar = tableDict->singleCharCons;
        }

        if (tableSingleChar) {
            char* oldCode;
            if (fcitx_dict_lookup_by_str(tableSingleChar, record->word, (void**) &oldCode)) {
                if (fcitx_utf8_strlen(record->word) > fcitx_utf8_strlen(oldCode)) {
                    fcitx_dict_insert_by_str(tableSingleChar, record->word, strdup(key), true);
                }
            } else {
                fcitx_dict_insert_by_str(tableSingleChar, record->word, strdup(key), false);
            }
        }
    }
}

static boolean fcitx_table_dict_load_data_text(FcitxTableDict* tableDict, FILE* fp, char** pBuffer, size_t* pBufferLen)
{
    char* trimmedBuffer = fcitx_utils_inplace_trim(*pBuffer);
    if (!CHECK_OPTION(trimmedBuffer, STR_DATA)) {
        _RETURN_ERROR(FTDPE_NoDataSection);
    }

    while (getline(pBuffer, pBufferLen, fp) != -1) {
        char* trimmedBuffer = fcitx_utils_inplace_trim(*pBuffer);
        char *saved = NULL;

        char* code = strtok_r(trimmedBuffer, FCITX_WHITESPACE, &saved);
        if (!code) {
            continue;
        }
        char *word = strtok_r(NULL, FCITX_WHITESPACE, &saved);
        if (!word) {
            continue;
        }
        char *strFreq = strtok_r(NULL, FCITX_WHITESPACE, &saved);
        char *strIndex = NULL;
        if (strFreq) {
            strIndex = strtok_r(NULL, FCITX_WHITESPACE, &saved);
        }

        uint32_t freq = 0;
        uint32_t index = 0;
        if (strFreq) {
            char* end = strFreq + strlen(strFreq);
            ulong result = strtoul(strFreq, &end, 10);
            if ((!end || *end == '\0') && result <= UINT32_MAX) {
                freq = (uint32_t) result;
            }
        }
        if (strIndex) {
            char* end = strIndex + strlen(strIndex);
            ulong result = strtoul(strIndex, &end, 10);
            if ((!end || *end == '\0') && result <= UINT32_MAX) {
                index = (uint32_t) result;
            }
        }

        size_t keyLen = strlen(trimmedBuffer);

        /* do some validation */
        if (((trimmedBuffer[0] != tableDict->cPinyinKey) && (keyLen > tableDict->codeLength))
            || ((trimmedBuffer[0] == tableDict->cPinyinKey) && (keyLen - 1 > tableDict->pyCodeLength))
            || ((trimmedBuffer[0] == tableDict->cPhraseKey) && (keyLen - 1 > tableDict->codeLength))
            || ((trimmedBuffer[0] == tableDict->cPromptKey) && (keyLen - 1 > tableDict->codeLength))
        ) {
            continue;
        }

        if (!fcitx_utf8_check_string(word)) {
            continue;
        }

        size_t wordLen = fcitx_utf8_strlen(word);
        if (trimmedBuffer[0] == tableDict->cPhraseKey && wordLen != 1) {
            continue;
        }
        FcitxTableRecordType type = RECORDTYPE_NORMAL;
        if (trimmedBuffer[0] == tableDict->cPinyinKey) {
            type = RECORDTYPE_PINYIN;
        } else if (trimmedBuffer[0] == tableDict->cPhraseKey) {
            type = RECORDTYPE_CONSTRUCT;
        } else if (trimmedBuffer[0] == tableDict->cPromptKey) {
            type = RECORDTYPE_PROMPT;
        }

        if (type != RECORDTYPE_NORMAL) {
            trimmedBuffer ++;
        }

        RECORD* recTemp = (RECORD*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
        recTemp->word = (char*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * (strlen(word) + 1));
        strcpy(recTemp->word, word);
        recTemp->type = type;
        recTemp->freq = freq;
        recTemp->index = index;

        fcitx_table_dict_store_record(tableDict, trimmedBuffer, recTemp);
    }

    return true;
}

FCITX_EXPORT_API
boolean fcitx_table_dict_load_text(FcitxTableDict* tableDict, FILE* fp)
{
    tableDict->cPinyinKey = '\0';
    tableDict->cPromptKey = '&';
    tableDict->cPhraseKey = '^';

    char* buffer = NULL;
    size_t bufferLen = 0;

    do {
        if (!fcitx_table_dict_load_metadata_text(tableDict, fp, &buffer, &bufferLen)) {
            break;
        }

        if (tableDict->pyCodeLength < tableDict->codeLength) {
            tableDict->pyCodeLength = tableDict->codeLength;
        }

        if (!fcitx_table_dict_load_rule_text(tableDict, fp, &buffer, &bufferLen)) {
            break;
        }
        if (!fcitx_table_dict_load_data_text(tableDict, fp, &buffer, &bufferLen)) {
            break;
        }
    } while(0);

    free(buffer);

    return (tableDict->error == FTDPE_NoError);
}

boolean fcitx_table_dict_save_text(FcitxTableDict* tableDict, FILE* fp)
{
    return true;
}

#define CHECK_LOAD_TABLE_ERROR(SIZE) if (size < (SIZE)) { tableDict->error = FTDPE_Corrupted; return false; }

static boolean fcitx_table_dict_load_metadata_binary(FcitxTableDict* tableDict, FILE* fpDict)
{
    size_t size;
    uint32_t dummy;
    uint32_t validCharsLen, ignoredCharsLen;
    int8_t          iVersion = 1;
    size = fcitx_utils_read_uint32(fpDict, &dummy);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(&iVersion, sizeof(int8_t), 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fcitx_utils_read_uint32(fpDict, &validCharsLen);
    CHECK_LOAD_TABLE_ERROR(1);
    tableDict->validChars = fcitx_utils_newv(char, validCharsLen + 1);
    size = fread(tableDict->validChars, sizeof(char), validCharsLen + 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(validCharsLen + 1);
    size = fread(&(tableDict->codeLength), sizeof(uint8_t), 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(&(tableDict->pyCodeLength), sizeof(uint8_t), 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fcitx_utils_read_uint32(fpDict, &ignoredCharsLen);
    CHECK_LOAD_TABLE_ERROR(1);
    tableDict->ignoredChars = fcitx_utils_newv(char, ignoredCharsLen + 1);
    size = fread(tableDict->ignoredChars, sizeof(char), ignoredCharsLen + 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(ignoredCharsLen + 1);

    if (tableDict->pyCodeLength < tableDict->codeLength) {
        tableDict->pyCodeLength = tableDict->codeLength;
    }
    return true;
}

static boolean fcitx_table_dict_load_rule_binary(FcitxTableDict* tableDict, FILE* fpDict)
{
    uint8_t hasRule = 0;
    size_t size = fread(&hasRule, sizeof(unsigned char), 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(1);

    if (hasRule) { //表示有组词规则
        tableDict->rule = fcitx_utils_newv(FcitxTableRule, (tableDict->codeLength - 1));
        for (int i = 0; i < tableDict->codeLength - 1; i++) {
            size = fread(&(tableDict->rule[i].flag), sizeof(unsigned char), 1, fpDict);
            CHECK_LOAD_TABLE_ERROR(1);
            size = fread(&(tableDict->rule[i].nWords), sizeof(unsigned char), 1, fpDict);
            CHECK_LOAD_TABLE_ERROR(1);
            tableDict->rule[i].items = fcitx_utils_newv(FcitxTableRuleItem, tableDict->codeLength);
            for (int j = 0; j < tableDict->codeLength; j++) {
                size = fread(&(tableDict->rule[i].items[j].flag), sizeof(unsigned char), 1, fpDict);
                CHECK_LOAD_TABLE_ERROR(1);
                size = fread(&(tableDict->rule[i].items[j].which), sizeof(unsigned char), 1, fpDict);
                CHECK_LOAD_TABLE_ERROR(1);
                size = fread(&(tableDict->rule[i].items[j].index), sizeof(unsigned char), 1, fpDict);
                CHECK_LOAD_TABLE_ERROR(1);
            }
        }
    }
    return true;
}

static boolean fcitx_table_dict_load_record_binary(FcitxTableDict* tableDict, FILE* fpDict)
{
    uint32_t recordCount;
    size_t size = fcitx_utils_read_uint32(fpDict, &recordCount);
    CHECK_LOAD_TABLE_ERROR(1);

    size_t bufSize = 0;
    char *strCode = fcitx_utils_newv(char, tableDict->pyCodeLength + 1);
    char* strHZ = NULL;
    for (uint32_t i = 0; i < recordCount; i++) {
        uint32_t hzLen;
        size = fread(strCode, sizeof(int8_t), tableDict->pyCodeLength + 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(tableDict->pyCodeLength + 1u);
        size = fcitx_utils_read_uint32(fpDict, &hzLen);
        CHECK_LOAD_TABLE_ERROR(1);
        /* we don't actually have such limit, but sometimes, corrupted table
         * may break this, so we need to give a limitation.
         */
        if (hzLen > FCITX_UTF8_MAX_LENGTH * 30) {
            return false;
        }
        if (hzLen > bufSize) {
            bufSize = hzLen;
            strHZ = realloc(strHZ, bufSize);
        }
        size = fread(strHZ, sizeof(int8_t), hzLen, fpDict);
        CHECK_LOAD_TABLE_ERROR(hzLen);

        RECORD* recTemp = (RECORD*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(RECORD));
        fcitx_multi_radix_tree_add(tableDict->radix, strCode, recTemp);
        recTemp->word = (char*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * hzLen);
        strcpy(recTemp->word, strHZ);

        int8_t type;
        size = fread(&type, sizeof(int8_t), 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(1);
        recTemp->type = type;

        size = fcitx_utils_read_uint32(fpDict, &recTemp->freq);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fcitx_utils_read_uint32(fpDict, &recTemp->index);
        CHECK_LOAD_TABLE_ERROR(1);
        if (recTemp->index > tableDict->tableIndex)
            tableDict->tableIndex = recTemp->index;

        fcitx_table_dict_store_record(tableDict, strCode, recTemp);
    }
    if (strHZ) {
        free(strHZ);
        strHZ = NULL;
    }

    return true;
}

#undef CHECK_LOAD_TABLE_ERROR

FCITX_EXPORT_API
boolean fcitx_table_dict_load_binary(FcitxTableDict* tableDict, FILE* fpDict)
{
    do {
        if (!fcitx_table_dict_load_metadata_binary(tableDict, fpDict)) {
            break;
        }
        if (!fcitx_table_dict_load_rule_binary(tableDict, fpDict)) {
            break;
        }
        if (!fcitx_table_dict_load_record_binary(tableDict, fpDict)) {
            break;
        }
    } while(0);

    return (tableDict->error == FTDPE_NoError);
}

FCITX_EXPORT_API
boolean fcitx_table_dict_load_symbol(FcitxTableDict* tableDict, const char* file)
{
    return false;
}

FCITX_EXPORT_API
boolean fcitx_table_dict_load_auto_phrase_file(FcitxTableDict* tableDict, const char* filename)
{
    return false;
}

FCITX_EXPORT_API
boolean fcitx_table_dict_save_binary(FcitxTableDict* tableDict, FILE* fp)
{
    return false;
}

FCITX_EXPORT_API
void fcitx_table_dict_unref(FcitxTableDict* tableDict)
{
    int32_t oldvalue = fcitx_utils_atomic_add (&tableDict->refcount, -1);
    if (oldvalue == 1) {
        fcitx_table_dict_free(tableDict);
    }
}

FCITX_EXPORT_API
uint8_t fcitx_table_dict_get_code_length(FcitxTableDict* tableDict)
{
    return tableDict->codeLength;
}

FCITX_EXPORT_API
boolean fcitx_table_dict_construct_word(FcitxTableDict* tableDict, const char *word, char* newWordCode)
{
    if (!tableDict->rule) {
        return false;
    }

    if (!fcitx_utf8_check_string(word)) {
        return false;
    }

    char chr[FCITX_UTF8_MAX_LENGTH + 1] = {'\0', };
    size_t wordLen = fcitx_utf8_strlen(word);

    int i;
    for (i = 0; i < tableDict->codeLength - 1; i++) {
        if (tableDict->rule[i].flag == FTRF_CharNumberEqual && tableDict->rule[i].nWords == wordLen) {
            break;
        } else if (tableDict->rule[i].flag == FTRF_CharNumberAbove && tableDict->rule[i].nWords <= wordLen) {
            break;
        }
    }

    if (i == tableDict->codeLength - 1)
        return false;

    int codeIndex = 0;
    for (int j = 0; j < tableDict->codeLength; j++) {
        int clen;
        char* ps;
        if (tableDict->rule[i].items[j].flag == FTRIF_CharIndexFromFront) {
            ps = fcitx_utf8_get_nth_char(word, tableDict->rule[i].items[j].which - 1);
        } else { // FTRIF_CharIndexFromBack
            ps = fcitx_utf8_get_nth_char(word, wordLen - tableDict->rule[i].items[j].which);
        }
        clen = fcitx_utf8_char_len(ps);
        strncpy(chr, ps, clen);
        chr[clen] = '\0';

        char* code;
        if (!fcitx_dict_lookup_by_str(tableDict->singleCharCons, chr, (void**) &code)) {
            if (!fcitx_dict_lookup_by_str(tableDict->singleChar, chr, (void**) &code)) {
                return false;
            }
        }

        if (strlen(code) >= tableDict->rule[i].items[j].index) {
            newWordCode[codeIndex] = code[tableDict->rule[i].items[j].index - 1];
            codeIndex++;
        }

    }

    return true;
}

FCITX_EXPORT_API
boolean fcitx_table_dict_add_word(FcitxTableDict* tableDict, const char* word)
{
    return false;
}


FCITX_EXPORT_API
const char* fcitx_table_dict_get_valid_chars(FcitxTableDict* tableDict)
{
    return tableDict->validChars;
}

FCITX_EXPORT_API
const char* fcitx_table_dict_get_ignored_chars(FcitxTableDict* tableDict)
{
    return tableDict->ignoredChars;
}


