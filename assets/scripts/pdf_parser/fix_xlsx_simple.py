import os

import pandas as pd

from shared import ArgsBase, parse_args


class Args(ArgsBase):
    xlsx_path: str
    save_path: str


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


def fix_single_file(xlsx_path: os.DirEntry[str], save_path: str):
    print("Start reading: ", xlsx_path)
    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")
    writer = pd.ExcelWriter(save_path + xlsx_path.name, engine="xlsxwriter")
    df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 1), index=False)
    workbook = writer.book
    worksheet = writer.sheets["sm"]
    format_error = workbook.add_format({'bg_color': '#FFC7CD', 'font_color': '#9C0006'})
    format_good = workbook.add_format({'bg_color': '#C6EFCD', 'font_color': '#006100'})
    format_warn = workbook.add_format({'bg_color': '#FCF4A3', 'font_color': '#000000'})

    # ! serie = xlsx_path.name.split('.')[0].split("_")[1]

    for col_num, column in enumerate(df.columns[1:]):
        for row_num, (value, isnull) in enumerate(zip(df[column], df[column].isnull())):
            if isnull:
                worksheet.write(row_num + 1, col_num + 1, "", format_error)
            else:
                fixed_value = str(value).replace("Φ", "")

                if is_float_or_int(value):
                    worksheet.write(row_num + 1, col_num + 1, to_float_or_int(str(value)), format_good)
                elif is_float_or_int(fixed_value):
                    worksheet.write(row_num + 1, col_num + 1, to_float_or_int(fixed_value), format_warn)
                else:
                    worksheet.write(row_num + 1, col_num + 1, fixed_value, format_error)

    worksheet.autofit()
    writer.close()


def fix_xlsx_simple(args: Args):
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")): continue
            fix_single_file(filename, args.save_path)


fix_xlsx_simple(parse_args(Args))

# * python fix_xlsx_simple.py C:/Coding/works/wbi/amati_drill/xlsx_fix_naming/ C:/Coding/works/wbi/amati_drill/xlsx_fix_values/
