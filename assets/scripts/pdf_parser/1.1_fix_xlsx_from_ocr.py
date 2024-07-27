import json
import os
import re
from typing import Literal, cast

import openpyxl
import pandas as pd
import pydantic

from shared import ArgsBase, flatten, frange, parse_args


class Args(ArgsBase):
    xlsx_path: str
    save_path: str
    rules_path: str


CHECK_START_WITH_6 = True


class CodePrefixRuleSimple(pydantic.BaseModel):
    type: Literal["simple"]
    index: list[int]
    value: str


class CodeRegex(pydantic.BaseModel):
    index: list[int]
    regex: str


class FixRules(pydantic.BaseModel):
    code_prefix: list[CodePrefixRuleSimple]
    code_replace_rules_sets: dict[str, dict[str, str]]
    code_regex: list[CodeRegex]


def get_code_prefix(index: int, code_prefix_rules: list[CodePrefixRuleSimple]) -> str:
    for code_prefix in code_prefix_rules:
        if index in code_prefix.index:
            return code_prefix.value

    raise ValueError(f"No id ({index}) in list get_code_prefix")


def get_code_regex(index: int, code_regex: list[CodeRegex]) -> str:
    for reg in code_regex:
        if index in reg.index:
            return reg.regex

    return "^$"


# code_replace_rules_eng = {"O": "0", "o": "0", "&": "", "Z": "7", "S": "5", "T": "7", "G": "6", "A": "4"}
# code_replace_rules_rus = {"О": "0", "о": "0", "З": "3", "з": "3"}
# code_replace_rules_rus_to_eng = {"М": "M", "В": "B", "С": "C"}
# code_replace_rules_combined = code_replace_rules_eng | code_replace_rules_rus | code_replace_rules_rus_to_eng


def code_replace_by_rules(string: str, code_replace_rules_batch) -> str:
    for key, value in code_replace_rules_batch.items():
        string = string.replace(key, value)
    return string


value_replace_rules_eng = {"O": "0", "o": "0", "M": "", "m": "", "B": "", "R": "", "K": "", "D": ""}
value_replace_rules_eng.update({"T": "7", "I": "1", "S": "5", "l": "1", "!": "1", "Z": "7"})

value_replace_rules_rus = {"О": "0", "о": "0", "М": "", "м": "", "В": "", "в": "", "К": "", "Ф": "", "ф": ""}
value_replace_rules_rus.update({"З": "3", "з": "3", "б": "6", "Т": "7"})


def is_int(value) -> bool:
    try:
        int(value)
    except Exception:
        return False
    return True


def is_float(value) -> bool:
    try:
        float(value)
    except Exception:
        return False
    return True


def is_float_or_int(value) -> bool:
    return is_float(value) or is_int(value)


def to_float_or_int(value) -> float | int:
    if is_int(value):
        return int(value)
    if is_float(value):
        return float(value)

    raise ValueError(f"Wrong type {value}")


def value_replace_by_rules(string: str) -> str:
    for key, value in (value_replace_rules_eng | value_replace_rules_rus).items():
        string = string.replace(key, value)
    return string


def fix_single(
        xlsx_path: os.DirEntry[str],
        save_path: str,
        fix_rules: FixRules,                                                                                            #
        code_replace_rules_batch: dict[str, str]):
    print("Start reading: ", xlsx_path)
    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")
    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name), engine="xlsxwriter")                               # pylint: disable=abstract-class-instantiated
    df.to_excel(writer, sheet_name="Sheet1", freeze_panes=(1, 1), index=False)
    workbook = cast(openpyxl.Workbook, writer.book)
    worksheet = writer.sheets["Sheet1"]
    format_error = workbook.add_format({'bg_color': '#FFC7CD', 'font_color': '#9C0006'})                                # type: ignore[attr-defined]
    format_good = workbook.add_format({'bg_color': '#C6EFCD', 'font_color': '#006100'})                                 # type: ignore[attr-defined]
    format_warn = workbook.add_format({'bg_color': '#FCF4A3', 'font_color': '#000000'})                                 # type: ignore[attr-defined]

    for column in df.columns[:1]:
        for row_num, (value, isnull) in enumerate(zip(df[column], df[column].isnull())):
            if isnull:
                worksheet.write(row_num + 1, 0, "", format_error)
            elif not isinstance(value, str):
                worksheet.write(row_num + 1, 0, value, format_error)
            else:
                cell_id = int(xlsx_path.name.split('.')[0])
                prefix = get_code_prefix(cell_id, fix_rules.code_prefix)
                regex = get_code_regex(cell_id, fix_rules.code_regex)
                fixed_substr = code_replace_by_rules(value[len(prefix):], code_replace_rules_batch)

                is_good = value.startswith(prefix) and re.search(regex, fixed_substr) is not None
                new_value = prefix + fixed_substr
                if not is_good:
                    worksheet.write(row_num + 1, 0, new_value, format_error)
                elif re.search(r"00\d*\.\d+", fixed_substr) is not None:
                    worksheet.write(row_num + 1, 0, new_value, format_warn)
                else:
                    worksheet.write(row_num + 1, 0, new_value, format_good)

    for col_num, column in enumerate(df.columns[1:]):
        for row_num, (value, isnull) in enumerate(zip(df[column], df[column].isnull())):
            if isnull:
                worksheet.write(row_num + 1, col_num + 1, "", format_error)
                continue

            fixed_value = value_replace_by_rules(str(value))

            if is_float_or_int(value):
                xl_format = format_good if not str(value).startswith("6") else format_warn
                if not CHECK_START_WITH_6: xl_format = format_good
                worksheet.write(row_num + 1, col_num + 1, to_float_or_int(str(value)), xl_format)
            elif is_float_or_int(fixed_value):
                worksheet.write(row_num + 1, col_num + 1, to_float_or_int(fixed_value), format_warn)
            else:
                worksheet.write(row_num + 1, col_num + 1, fixed_value, format_error)

    worksheet.autofit()
    writer.close()


def fix_xlsx(args: Args):
    os.makedirs(args.save_path, exist_ok=True)

    fix_rules: FixRules | None = None
    with open(args.rules_path, encoding="utf-8") as conf_file:
        fix_rules = FixRules(**json.load(conf_file))

    if fix_rules == None:
        print(f"[ERROR] Не удалось открыть файл с правилами: '{args.rules_path}'")
        return

    code_replace_rules_batch: dict[str, str] = {}
    for _, code_replace_rules in fix_rules.code_replace_rules_sets.items():
        code_replace_rules_batch = code_replace_rules_batch | code_replace_rules

    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")):
                continue
            fix_single(
                filename,
                args.save_path,
                fix_rules=fix_rules,
                code_replace_rules_batch=code_replace_rules_batch,
            )


fix_xlsx(parse_args(Args))

# ! python 1.1_fix_xlsx_from_ocr.py c:\Coding\works\wbi\ProjectsISC\Amati_2023_10_30\data\catalog\easy_xlsx_small c:\Coding\works\wbi\ProjectsISC\Amati_2023_10_30\data\catalog\xlsx_fix_1
