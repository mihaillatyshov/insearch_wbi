import os
import re
import sys
from openpyxl import Workbook

import pandas as pd

from custom_regex import full_regex

if (len(sys.argv) < 3):
    print("Скрипт не выполнен (не хватает агрументов)")
    exit(-1)

try:
    SOURCE_PATH = str(sys.argv[1])
    SAVE_PATH = str(sys.argv[2])
except Exception:
    print("Аргументы скрипта имеют ошибки!")
    exit(-1)

CHECK_START_WITH_6 = False


def get_code_prefix(id: int) -> str:
    if id in range(6, 12 + 1): return "E-POR"
    if id in range(13, 19 + 1): return "E-PEN"
    if id in range(20, 26 + 1): return "E-HEK"
    if id in range(27, 32 + 1): return "E-HES"
    if id in range(33, 36 + 1): return "E-MUS"
    if id in range(37, 40 + 1): return "E-TIS"
    if id in range(41, 42 + 1): return "E-ALM"
    if id in range(43, 44 + 1): return "E-LIT"
    if id in range(45, 48 + 1): return "E-LNC"
    if id in range(49, 50 + 1): return "E-TSL"
    if id in range(51, 51 + 1): return "E-ICR"
    if id in range(52, 52 + 1): return "E-HRM"
    if id in range(53, 53 + 1): return "E-CHM"
    if id in range(54, 54 + 1): return "E-TPM"
    if id in range(55, 55 + 1): return "E-SBM"

    return "None"


def get_code_regex(id: int) -> str:
    if id in range(6, 7 + 1): return full_regex["float-MRd"]
    if id in range(8, 9 + 1): return full_regex["float-BRd"]
    if id in range(10, 12 + 1): return full_regex["float-MRd"]

    if id in range(13, 14 + 1): return full_regex["float-MRd"]
    if id in range(15, 16 + 1): return full_regex["float-BRd"]
    if id in range(17, 19 + 1): return full_regex["float-MRd"]

    if id in range(20, 21 + 1): return full_regex["float-MRd"]
    if id in range(22, 23 + 1): return full_regex["float-BRd"]
    if id in range(24, 26 + 1): return full_regex["float-MRd"]

    if id in range(27, 28 + 1): return full_regex["float-MRd"]
    if id in range(29, 30 + 1): return full_regex["float-BRd"]
    if id in range(31, 32 + 1): return full_regex["float-MRd"]

    if id in range(33, 34 + 1): return full_regex["float-MRd"]
    if id in range(35, 36 + 1): return full_regex["float-BRd"]

    if id in range(37, 38 + 1): return full_regex["float-MRd"]
    if id in range(39, 40 + 1): return full_regex["float-BRd"]

    if id in range(41, 42 + 1): return full_regex["float-MRd"]

    if id in range(43, 43 + 1): return full_regex["float-MRd"]
    if id in range(44, 44 + 1): return full_regex["float-BRd"]

    if id in range(45, 46 + 1): return full_regex["float-MRd"]
    if id in range(47, 48 + 1): return full_regex["float-BRd"]

    if id in range(49, 50 + 1): return full_regex["float-T"]

    if id in range(51, 51 + 1): return full_regex["float-CRRd"]

    if id in range(52, 52 + 1): return full_regex["float-Rd"]

    if id in range(53, 53 + 1): return full_regex["float-D"]

    if id in range(54, 54 + 1): return full_regex["float-KRd"]

    if id in range(55, 55 + 1): return full_regex["float-MRd"]

    return "^$"


code_replace_rules_eng = {"O": "0", "o": "0", "&": ""}
code_replace_rules_rus = {"О": "0", "о": "0", "З": "3", "з": "3"}
code_replace_rules_rus_to_eng = {"М": "M", "В": "B", "С": "C"}


def code_replace_by_rules(string: str) -> str:
    for key, value in (code_replace_rules_eng | code_replace_rules_rus | code_replace_rules_rus_to_eng).items():
        string = string.replace(key, value)
    return string


value_replace_rules_eng = {"O": "0", "o": "0", "M": "", "m": "", "B": "", "R": "", "K": "", "D": "", "l": "1", "!": "1"}
value_replace_rules_eng.update({"T": "7", "I": "1", "S": "5"})

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
    if is_int(value): return int(value)
    if is_float(value): return float(value)

    raise ValueError(f"Wrong type {value}")


def value_replace_by_rules(string: str) -> str:
    for key, value in (value_replace_rules_eng | value_replace_rules_rus).items():
        string = string.replace(key, value)
    return string


def fix_single(xlsx_path: os.DirEntry[str], save_path: str):
    print("Start reading: ", xlsx_path)
    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")
    writer = pd.ExcelWriter(save_path + xlsx_path.name, engine="xlsxwriter")
    df.to_excel(writer, sheet_name="Sheet1", freeze_panes=(1, 1), index=False)
    workbook = writer.book
    worksheet = writer.sheets["Sheet1"]
    format_error = workbook.add_format({'bg_color': '#FFC7CD', 'font_color': '#9C0006'})
    format_good = workbook.add_format({'bg_color': '#C6EFCD', 'font_color': '#006100'})
    format_warn = workbook.add_format({'bg_color': '#FCF4A3', 'font_color': '#000000'})

    for column in df.columns[:1]:
        for row_num, (value, isnull) in enumerate(zip(df[column], df[column].isnull())):
            if isnull:
                worksheet.write(row_num + 1, 0, "", format_error)
            elif not isinstance(value, str):
                worksheet.write(row_num + 1, 0, value, format_error)
            else:
                cell_id = int(xlsx_path.name.split('.')[0])
                prefix = get_code_prefix(cell_id)
                regex = get_code_regex(cell_id)
                fixed_substr = code_replace_by_rules(value[len(prefix):])
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
            else:
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


def fix_xlsx(xlsx_path: str, save_path: str):
    for filename in os.scandir(xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")): continue
            fix_single(filename, save_path)


fix_xlsx(SOURCE_PATH, SAVE_PATH)
