import os

import pandas as pd

from shared import ArgsBase, parse_args


class Args(ArgsBase):
    xlsx_path: str
    xlsx_naming_path: str
    save_path: str


py_args = parse_args(Args)


def get_new_names(now_list: list[str], old_list: list[str], new_list: list[str]) -> tuple[list[str], list[int]]:
    result: list[str] = []
    errors_list: list[int] = []
    for i, item in enumerate(now_list):
        try:
            result.append(new_list[list(old_list).index(item)])
        except Exception as e:
            result.append(item)
            errors_list.append(i)

    return result, errors_list


def fix_single_file(xlsx_path: os.DirEntry[str], df_naming: pd.DataFrame, save_path: str):
    print("Start reading: ", xlsx_path)
    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")
    df.rename(columns={"编号": "model", "d1": "dc", "DH6": "dcon", "L": "oal", "L1": "lcf"}, inplace=True)

    df["model"], errors_list = get_new_names(df["model"], df_naming["Geefun"], df_naming["Amati"])

    writer = pd.ExcelWriter(save_path + xlsx_path.name, engine="xlsxwriter")
    df.to_excel(writer, sheet_name="Sheet1", freeze_panes=(1, 1), index=False)
    workbook = writer.book
    worksheet = writer.sheets["Sheet1"]
    format_error = workbook.add_format({'bg_color': '#FFC7CD', 'font_color': '#9C0006'})
    format_good = workbook.add_format({'bg_color': '#C6EFCD', 'font_color': '#006100'})
    for row_num, value in enumerate(df["model"]):
        worksheet.write(row_num + 1, 0, value, format_error if row_num in errors_list else format_good)

    worksheet.autofit()
    writer.close()


def fix_xlsx_naming(xlsx_path: str, xlsx_naming_path, save_path: str):
    df_naming = pd.read_excel(xlsx_naming_path, index_col=None, engine="openpyxl")

    for filename in os.scandir(xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")): continue
            fix_single_file(filename, df_naming, save_path)


fix_xlsx_naming(**py_args.dict())

# * python fix_xlsx_naming.py C:/Coding/works/wbi/amati_drill/xlsx_start/ C:/Coding/works/wbi/amati_drill/Translator.xlsx C:/Coding/works/wbi/amati_drill/xlsx_fix_naming/
