#include <fcitx-im/table/tabledict.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

char data[] = ""
"KeyCode=abcdefghijklmnopqrstuvwxy\n"
"Length=4\n"
"Pinyin=@\n"
"PinyinLength=4\n"
"[Rule]\n"
"e2=p11+p12+p21+p22\n"
"e3=p11+p21+p31+p32\n"
"a4=p11+p21+p31+n11\n"
"[Data]\n"
"a 工\n"
"a 戈\n"
"a 或\n"
"a 其\n"
"aa 式\n"
"aa 戒\n"
"aaa 工\n"
"aaaa 工\n"
"aaaa 恭恭敬敬\n"
"aaad 工期\n"
"aaae 黄花菜\n"
"aaae 黄芽菜\n"
"aaag 工巧\n"
"aaah 葡萄牙\n"
"aaal 花花世界\n"
"aaan 工艺\n"
"aaaq 工区\n"
"aaar 工匠\n"
"aaar 菚\n"
"aaau 工薪\n"
"aaaw 斯蒂芬\n"
"aaaw 欺世惑众\n"
"aaay 劳苦功高\n"
"aabb 式子\n"
"aabg 草草了事\n"
"aabi 芙蓉出水\n"
"aabk 工职\n"
"aabn 工薪阶层\n"
"aabt 医药卫生\n"
"aabu 茕茕孑立\n"
"aabw 戒除\n"
"aacn 萨其马\n"
"aacw 落落难合\n"
"aad 式\n"
"aad 葚\n"
"aad 匞\n"
"aadc 工友\n"
"aade 蘛\n"
"aadg 工厂\n"
"aadi 落荒而逃\n"
"aadi 藄\n"
"aadk 匿\n"
"aadk 暱\n"
"aadm 花花太岁\n"
"aadn 慝\n"
"aadn 葚\n"
"aadr 茙\n"
"yygn 方\n"
"tmkd 向\n"
"telf 盘\n"
;

int main(int argc, char* argv[])
{
    FcitxTableDict* tableDict = fcitx_table_dict_new();
    FILE* fp = fmemopen(data, FCITX_ARRAY_SIZE(data), "r");
    assert(fp);
    assert(fcitx_table_dict_load_text(tableDict, fp));
    fclose(fp);
    assert(strcmp(fcitx_table_dict_get_valid_chars(tableDict), "abcdefghijklmnopqrstuvwxy") == 0);
    assert(fcitx_table_dict_get_ignored_chars(tableDict) == NULL);
    assert(fcitx_table_dict_get_code_length(tableDict) == 4);

    char code[5] = {'\0', };
    assert(fcitx_table_dict_construct_word(tableDict, "方向盘", code));
    assert(strcmp(code, "ytte") == 0);

    fcitx_table_dict_unref(tableDict);
    return 0;
}
