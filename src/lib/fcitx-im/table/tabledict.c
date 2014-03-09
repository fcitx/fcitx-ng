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
#include "fcitx-utils/multiradix.h"


#define MAX_CODE_LENGTH  30
#define PHRASE_MAX_LENGTH 10
#define FH_MAX_LENGTH  10
#define TABLE_AUTO_SAVE_AFTER 1024
#define AUTO_PHRASE_COUNT 10000

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

    FcitxDict* singleChar;
    FcitxDict* singleCharCons;

    RECORD* promptCode[UCHAR_MAX + 1];
    uint32_t tableIndex;
    FcitxMultiRadixTree* radix;
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

boolean fcitx_tabledict_load_text(FcitxTableDict* tableDict, FILE* fp)
{
    size_t lineNumber = 0;
    int i;
    char* buffer = NULL;
    size_t bufferLen;
    while (getline(&buffer, &bufferLen, fp) == -1) {
        lineNumber++;

        if (getline(&buffer, &bufferLen, fp) == -1)
            break;

        i = strlen(buffer) - 1;

        while ((i >= 0) && (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r'))
            buffer[i--] = '\0';

        char* line = buffer;

        if (*line == ' ')
            line++;

        if (line[0] == '#')
            continue;

        if (CHECK_OPTION(pstr, STR_KEYCODE)) {
            line += ADD_LENGTH(pstr, STR_KEYCODE);
            strcpy(tableDict->validChars, line);
        } else if (CHECK_OPTION(pstr, STR_CODELEN)) {
            line += ADD_LENGTH(pstr, STR_CODELEN);
            tableDict->codeLength = atoi(line);

            if (tableDict->codeLength > MAX_CODE_LENGTH) {
                tableDict->codeLength = MAX_CODE_LENGTH;
                printf("Max Code Length is %d\n", MAX_CODE_LENGTH);
            }
        } else if (CHECK_OPTION(pstr, STR_IGNORECHAR)) {
            tableDict->ignoredChars += ADD_LENGTH(pstr, STR_IGNORECHAR);
            strcpy(strIgnoreChars, pstr);
        } else if (CHECK_OPTION(pstr, STR_PINYIN)) {
            pstr += ADD_LENGTH(pstr, STR_PINYIN);

            while (*pstr == ' ' && *pstr != '\0')
                pstr++;

            cPinyinKey = *pstr;
        } else if (CHECK_OPTION(pstr, STR_PROMPT)) {
            pstr += ADD_LENGTH(pstr, STR_PROMPT);

            while (*pstr == ' ' && *pstr != '\0')
                pstr++;

            cPromptKey = *pstr;
        } else if (CHECK_OPTION(pstr, STR_CONSTRUCTPHRASE)) {
            pstr += ADD_LENGTH(pstr, STR_CONSTRUCTPHRASE);

            while (*pstr == ' ' && *pstr != '\0')
                pstr++;

            cPhraseKey = *pstr;
        } else if (CHECK_OPTION(pstr, STR_PINYINLEN)) {
            pstr += ADD_LENGTH(pstr, STR_PINYINLEN);
            iPYCodeLength = atoi(pstr);
        }

        else if (CHECK_OPTION(pstr, STR_DATA))
            break;
        else if (CHECK_OPTION(pstr, STR_RULE)) {
            bRule = 1;
            break;
        }
    }

    if (iCodeLength <= 0 || !strInputCode[0]) {
        printf("Source File Format Error!\n");
        exit(1);
    }

    if (bRule) {
        /*
         * 组词规则数应该比键码长度小1
         */
        rule = (RULE *) malloc(sizeof(RULE) * (iCodeLength - 1));

        for (iTemp = 0; iTemp < (iCodeLength - 1); iTemp++) {
            l++;

            if (getline(&buffer, &len, fpDict) == -1)
                break;

            rule[iTemp].rule = (RULE_RULE *) malloc(sizeof(RULE_RULE) * iCodeLength);

            i = strlen(buffer) - 1;

            while ((i >= 0) && (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r'))
                buffer[i--] = '\0';

            pstr = buffer;

            if (*pstr == ' ')
                pstr++;

            if (pstr[0] == '#')
                continue;

            if (CHECK_OPTION(pstr, STR_DATA))
                break;

            switch (*pstr) {

            case 'e':

            case 'E':
                rule[iTemp].iFlag = 0;
                break;

            case 'a':

            case 'A':
                rule[iTemp].iFlag = 1;
                break;

            default:
                printf("2   Phrase rules are not suitable!\n");
                printf("\t\t%s\n", buffer);
                exit(1);
            }

            pstr++;

            char* p = pstr;

            while (*p && *p != '=')
                p++;

            if (!(*p)) {
                printf("3   Phrase rules are not suitable!\n");
                printf("\t\t%s\n", buffer);
                exit(1);
            }

            strncpy(strTemp, pstr, p - pstr);

            strTemp[p - pstr] = '\0';
            rule[iTemp].iWords = atoi(strTemp);

            p++;

            for (i = 0; i < iCodeLength; i++) {
                while (*p == ' ')
                    p++;

                switch (*p) {

                case 'p':

                case 'P':
                    rule[iTemp].rule[i].iFlag = 1;
                    break;

                case 'n':

                case 'N':
                    rule[iTemp].rule[i].iFlag = 0;
                    break;

                default:
                    printf("4   Phrase rules are not suitable!\n");
                    printf("\t\t%s\n", buffer);
                    exit(1);
                }

                p++;

                rule[iTemp].rule[i].iWhich = *p++ - '0';
                rule[iTemp].rule[i].iIndex = *p++ - '0';

                while (*p == ' ')
                    p++;

                if (i != (iCodeLength - 1)) {
                    if (*p != '+') {
                        printf("5   Phrase rules are not suitable!\n");
                        printf("\t\t%s  %d\n", buffer, iCodeLength);
                        exit(1);
                    }

                    p++;
                }
            }
        }

        if (iTemp != iCodeLength - 1) {
            printf("6  Phrase rules are not suitable!\n");
            exit(1);
        }

        for (iTemp = 0; iTemp < (iCodeLength - 1); iTemp++) {
            l++;

            if (getline(&buffer, &len, fpDict) == -1)
                break;

            i = strlen(buffer) - 1;

            while ((i >= 0) && (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r'))
                buffer[i--] = '\0';

            pstr = buffer;

            if (*pstr == ' ')
                pstr++;

            if (pstr[0] == '#')
                continue;

            if (CHECK_OPTION(pstr, STR_DATA))
                break;
        }
    }

    if (iPYCodeLength < iCodeLength)
        iPYCodeLength = iCodeLength;

    if (!CHECK_OPTION(pstr, STR_DATA)) {
        printf("Source File Format Error!\n");
        exit(1);
    }

    while (getline(&buffer, &len, fpDict) != -1) {
        l++;
        if (buf1)
            free(buf1);
        buf1 = fcitx_utils_trim(buffer);
        char *p = buf1;

        while (*p && !isspace(*p))
            p ++;

        if (*p == '\0')
            continue;

        while (isspace(*p)) {
            *p = '\0';
            p ++;
        }

        char* strHZ = p;

        if (!IsValidCode(buf1[0])) {
            printf("Invalid Format: Line-%d  %s %s\n", l, buf1, strHZ);

            exit(1);
        }

        if (((buf1[0] != cPinyinKey) && (strlen(buf1) > iCodeLength))
            || ((buf1[0] == cPinyinKey) && (strlen(buf1) > (iPYCodeLength + 1)))
            || ((buf1[0] == cPhraseKey) && (strlen(buf1) > (iCodeLength + 1)))
            || ((buf1[0] == cPromptKey) && (strlen(buf1) > (iPYCodeLength + 1)))
        ) {
            printf("Delete:  %s %s, Too long\n", buf1, strHZ);
            continue;
        }

        size_t hzLen = fcitx_utf8_strlen(strHZ);
        if (buf1[0] == cPhraseKey && hzLen != 1) {
            printf("Delete:  %s %s, Too long\n", buf1, strHZ);
            continue;
        }

        type = RECORDTYPE_NORMAL;

        pstr = buf1;

        if (buf1[0] == cPinyinKey) {
            pstr ++;
            type = RECORDTYPE_PINYIN;
        } else if (buf1[0] == cPhraseKey) {
            pstr ++;
            type = RECORDTYPE_CONSTRUCT;
        } else if (buf1[0] == cPromptKey) {
            pstr ++;
            type = RECORDTYPE_PROMPT;
        }

        //查找是否重复
        temp = current;

        if (temp != head) {
            if (strcmp(temp->strCode, pstr) >= 0) {
                while (temp != head && strcmp(temp->strCode, pstr) >= 0) {
                    if (!strcmp(temp->strHZ, strHZ) && !strcmp(temp->strCode, pstr) && temp->type == type) {
                        printf("Delete:  %s %s\n", pstr, strHZ);
                        goto _next;
                    }

                    temp = temp->prev;
                }

                if (temp == head)
                    temp = temp->next;

                while (temp != head && strcmp(temp->strCode, pstr) <= 0)
                    temp = temp->next;
            } else {
                while (temp != head && strcmp(temp->strCode, pstr) <= 0) {
                    if (!strcmp(temp->strHZ, strHZ) && !strcmp(temp->strCode, pstr) && temp->type == type) {
                        printf("Delete:  %s %s\n", pstr, strHZ);
                        goto _next;
                    }

                    temp = temp->next;
                }
            }
        }

        //插在temp的前面
        newRec = (RECORD *) fcitx_utils_malloc0(sizeof(RECORD));

        newRec->strCode = (char *) fcitx_utils_malloc0(sizeof(char) * (iPYCodeLength + 1));

        newRec->strHZ = (char *) fcitx_utils_malloc0(sizeof(char) * strlen(strHZ) + 1);

        strcpy(newRec->strCode, pstr);

        strcpy(newRec->strHZ, strHZ);

        newRec->type = type;

        newRec->iHit = 0;

        newRec->iIndex = 0;

        temp->prev->next = newRec;

        newRec->next = temp;

        newRec->prev = temp->prev;

        temp->prev = newRec;

        current = newRec;

        s++;

    _next:
        continue;

    }


    if (buffer)
        free(buffer);
    if (buf1)
        free(buf1);

    fclose(fpDict);

    printf("\nReading %d records.\n\n", s);

    fpNew = fopen(argv[2], "w");

    if (!fpNew) {
        printf("\nCan not create target file!\n\n");
        exit(3);
    }

    int8_t iInternalVersion = INTERNAL_VERSION;

    //写入版本号--如果第一个字为0,表示后面那个字节为版本号
    fcitx_utils_write_uint32(fpNew, 0);
    fwrite(&iInternalVersion, sizeof(int8_t), 1, fpNew);

    iTemp = (uint32_t)strlen(strInputCode);
    fcitx_utils_write_uint32(fpNew, iTemp);
    fwrite(strInputCode, sizeof(char), iTemp + 1, fpNew);
    fwrite(&iCodeLength, sizeof(unsigned char), 1, fpNew);
    fwrite(&iPYCodeLength, sizeof(unsigned char), 1, fpNew);
    iTemp = (uint32_t)strlen(strIgnoreChars);
    fcitx_utils_write_uint32(fpNew, iTemp);
    fwrite(strIgnoreChars, sizeof(char), iTemp + 1, fpNew);

    fwrite(&bRule, sizeof(unsigned char), 1, fpNew);

    if (bRule) {
        for (i = 0; i < iCodeLength - 1; i++) {
            fwrite(&(rule[i].iFlag), sizeof(unsigned char), 1, fpNew);
            fwrite(&(rule[i].iWords), sizeof(unsigned char), 1, fpNew);

            for (iTemp = 0; iTemp < iCodeLength; iTemp++) {
                fwrite(&(rule[i].rule[iTemp].iFlag), sizeof(unsigned char), 1, fpNew);
                fwrite(&(rule[i].rule[iTemp].iWhich), sizeof(unsigned char), 1, fpNew);
                fwrite(&(rule[i].rule[iTemp].iIndex), sizeof(unsigned char), 1, fpNew);
            }
        }
    }

    fcitx_utils_write_uint32(fpNew, s);

    current = head->next;

    while (current != head) {
        fwrite(current->strCode, sizeof(char), iPYCodeLength + 1, fpNew);
        s = strlen(current->strHZ) + 1;
        fcitx_utils_write_uint32(fpNew, s);
        fwrite(current->strHZ, sizeof(char), s, fpNew);
        fwrite(&(current->type), sizeof(int8_t), 1, fpNew);
        fcitx_utils_write_uint32(fpNew, current->iHit);
        fcitx_utils_write_uint32(fpNew, current->iIndex);
        current = current->next;
    }

    fclose(fpNew);

    return 0;
}

boolean fcitx_tabledict_save_text(FcitxTableDict* tableDict, FILE* fp)
{

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
        fcitx_multi_radix_tree_add(tableDict->radix, strCode, recTemp);
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
    tableDict->radix = fcitx_multi_radix_tree_new(offsetof(RECORD, list), NULL, NULL);
    
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

