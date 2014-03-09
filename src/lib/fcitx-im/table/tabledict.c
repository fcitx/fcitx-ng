#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "tabledict.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/atomic.h"
#include "fcitx-utils/memory-pool.h"
#include "fcitx-utils/utf8.h"
#include "fcitx-utils/radix.h"
#include "fcitx-utils/dict.h"
#include "fcitx-utils/list.h"


#define MAX_CODE_LENGTH  30
#define PHRASE_MAX_LENGTH 10
#define FH_MAX_LENGTH  10
#define TABLE_AUTO_SAVE_AFTER 1024
#define AUTO_PHRASE_COUNT 10000
#define SINGLE_HZ_COUNT 66000

#define RECORDTYPE_NORMAL 0x0
#define RECORDTYPE_PINYIN 0x1
#define RECORDTYPE_CONSTRUCT 0x2
#define RECORDTYPE_PROMPT 0x3

typedef struct _FcitxTableRuleItem {
    uint8_t   flag;  // 1 --> 正序   0 --> 逆序
    uint8_t   which; //第几个字
    uint8_t   index; //第几个编码
} FcitxTableRuleItem;

typedef struct {
    uint8_t                  nWords; //多少个字
    uint8_t                  flag;  //1 --> 大于等于iWords  0 --> 等于iWords
    FcitxTableRuleItem      *items;
} FcitxTableRule;

typedef struct _RECORD {
    char           *strHZ;
    uint32_t    iHit;
    uint32_t    iIndex;
    int8_t          type;
    FcitxListHead list;
} RECORD;

/* 根据键码生成一个简单的索引，指向该键码起始的第一个记录 */
typedef struct {
    RECORD         *record;
    char            cCode;
} RECORD_INDEX;

struct _FcitxTableDict
{
    int refcount;
    char* validChars;
    char* ignoredChars;
    uint8_t codeLength;
    uint8_t pyCodeLength;
    uint8_t hasRule;
    uint32_t recordCount;
    boolean hasPinyin;

    FcitxTableRule* rule;


    RECORD_INDEX* recordIndex;
    RECORD* recordHead;
    RECORD* currentRecord;
    
    FcitxDict* singleChar;
    FcitxDict* singleCharCons;
    
    RECORD* promptCode[UCHAR_MAX + 1];
    uint32_t tableIndex;
    FcitxRadixTree* radix;
    FcitxMemoryPool* pool;
};

FCITX_EXPORT_API
FcitxTableDict* fcitx_tabledict_new()
{
    FcitxTableDict* tableDict = fcitx_utils_new(FcitxTableDict);
    
    fcitx_tabledict_ref(tableDict);
    
    return tableDict;
}

void fcitx_tabledict_free(FcitxTableDict* tableDict)
{
    free(tableDict);
}

FCITX_EXPORT_API
FcitxTableDict* fcitx_tabledict_ref(FcitxTableDict* tableDict)
{
    fcitx_utils_atomic_add (&tableDict->refcount, 1);
    return tableDict;
}
#define CHECK_LOAD_TABLE_ERROR(SIZE) if (size < (SIZE)) { return false; }

boolean fcitx_tabledict_load_metadata(FcitxTableDict* tableDict, FILE* fpDict)
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
    tableDict->validChars = (char*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * (validCharsLen + 1));
    size = fread(tableDict->validChars, sizeof(char), validCharsLen + 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(validCharsLen + 1);
    size = fread(&(tableDict->codeLength), sizeof(uint8_t), 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fread(&(tableDict->pyCodeLength), sizeof(uint8_t), 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(1);
    size = fcitx_utils_read_uint32(fpDict, &ignoredCharsLen);
    CHECK_LOAD_TABLE_ERROR(1);
    tableDict->ignoredChars = (char*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * (ignoredCharsLen + 1));
    size = fread(tableDict->ignoredChars, sizeof(char), ignoredCharsLen + 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(ignoredCharsLen + 1);
    
    return true;
}

boolean fcitx_tabledict_load_rule(FcitxTableDict* tableDict, FILE* fpDict) {
    size_t size = fread(&(tableDict->hasRule), sizeof(unsigned char), 1, fpDict);
    CHECK_LOAD_TABLE_ERROR(1);
    
    if (tableDict->hasRule) { //表示有组词规则
        tableDict->rule = (FcitxTableRule*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(FcitxTableRule) * (tableDict->codeLength - 1));
        for (int i = 0; i < tableDict->codeLength - 1; i++) {
            size = fread(&(tableDict->rule[i].flag), sizeof(unsigned char), 1, fpDict);
            CHECK_LOAD_TABLE_ERROR(1);
            size = fread(&(tableDict->rule[i].nWords), sizeof(unsigned char), 1, fpDict);
            CHECK_LOAD_TABLE_ERROR(1);
            tableDict->rule[i].items = (FcitxTableRuleItem*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(FcitxTableRuleItem) * tableDict->codeLength);
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

boolean fcitx_tabledict_load_record(FcitxTableDict* tableDict, FILE* fpDict)
{
    size_t size = fcitx_utils_read_uint32(fpDict, &tableDict->recordCount);
    CHECK_LOAD_TABLE_ERROR(1);
    
    size_t bufSize = 0;
    char            strCode[MAX_CODE_LENGTH + 1];
    char* strHZ = NULL;
    for (uint32_t i = 0; i < tableDict->recordCount; i++) {
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
        fcitx_radix_tree_add(tableDict->radix, strCode, recTemp);
        recTemp->strHZ = (char*)fcitx_memory_pool_alloc(tableDict->pool, sizeof(char) * hzLen);
        strcpy(recTemp->strHZ, strHZ);

        int8_t type;
        size = fread(&type, sizeof(int8_t), 1, fpDict);
        CHECK_LOAD_TABLE_ERROR(1);
        recTemp->type = type;

        size = fcitx_utils_read_uint32(fpDict, &recTemp->iHit);
        CHECK_LOAD_TABLE_ERROR(1);
        size = fcitx_utils_read_uint32(fpDict, &recTemp->iIndex);
        CHECK_LOAD_TABLE_ERROR(1);
        if (recTemp->iIndex > tableDict->tableIndex)
            tableDict->tableIndex = recTemp->iIndex;
        
        /*
         * if we have construct phrase rule and current phrase is a single character
         * then we put it in a table
         */
        if (tableDict->hasRule && fcitx_utf8_strlen(recTemp->strHZ) == 1 && !strchr(tableDict->ignoredChars, strCode[0]))
        {
            FcitxDict* tableSingleChar = NULL;
            if (recTemp->type == RECORDTYPE_NORMAL)
                tableSingleChar = tableDict->singleChar;
            else if (recTemp->type == RECORDTYPE_CONSTRUCT)
                tableSingleChar = tableDict->singleCharCons;

            if (tableSingleChar) {
                char* oldCode;
                if (fcitx_dict_lookup(tableSingleChar, strHZ, (void**) &oldCode)) {
                    if (strlen(strCode) > strlen(oldCode)) {
                        fcitx_dict_insert(tableSingleChar, strCode, strdup(strHZ), true);
                    }
                } else {
                    fcitx_dict_insert(tableSingleChar, strCode, strdup(strHZ), false);
                }
            }
        }

        if (recTemp->type == RECORDTYPE_PINYIN) {
            tableDict->hasPinyin = true;
        }

        if (recTemp->type == RECORDTYPE_PROMPT && strlen(strCode) == 1) {
            tableDict->promptCode[(uint8_t) strCode[0]] = recTemp;
        }
    }
    if (strHZ) {
        free(strHZ);
        strHZ = NULL;
    }
}

#undef CHECK_LOAD_TABLE_ERROR

FCITX_EXPORT_API
boolean fcitx_tabledict_load(FcitxTableDict* tableDict, FILE* fpDict)
{
    tableDict->pool = fcitx_memory_pool_new();
    tableDict->radix = fcitx_radix_tree_new(NULL, NULL);
    
    do {
        if (!fcitx_tabledict_load_metadata(tableDict, fpDict)) {
            break;
        }
        if (!fcitx_tabledict_load_rule(tableDict, fpDict)) {
            break;
        }
        if (!fcitx_tabledict_load_record(tableDict, fpDict)) {
            break;
        }
    } while(0);

table_load_error:
    fclose(fpDict);
    fcitx_memory_pool_free(tableDict->pool);
    
    return 0;
}

FCITX_EXPORT_API
boolean fcitx_tabledict_load_symbol(FcitxTableDict* tableDict, const char* file)
{
    return false;
}

FCITX_EXPORT_API
boolean fcitx_tabledict_load_auto_phrase_file(FcitxTableDict* tableDict, const char* filename)
{
    return false;
}

FCITX_EXPORT_API
void fcitx_tabledict_save(FcitxTableDict* tableDict, const char* file)
{

}

FCITX_EXPORT_API
void fcitx_tabledict_unref(FcitxTableDict* tableDict)
{
    int32_t oldvalue = fcitx_utils_atomic_add (&tableDict->refcount, -1);
    if (oldvalue == 1) {
        fcitx_tabledict_free(tableDict);
    }
}

FCITX_EXPORT_API
const char* fcitx_tabledict_get_valid_chars(FcitxTableDict* tableDict)
{
    return tableDict->validChars;
}

FCITX_EXPORT_API
const char* fcitx_tabledict_get_ignored_chars(FcitxTableDict* tableDict)
{
    return tableDict->ignoredChars;
}

