import os

from xlsxwriter.workbook import Worksheet, Workbook

import pandas as pd

from shared import ArgsBase, parse_args


class Args(ArgsBase):
    xlsx_path: str
    save_path: str


ROW_OFFSET = 1
COL_OFFSET = 0


def check_simple_single(xlsx_path: os.DirEntry[str], save_path: str):
    print("Start reading: ", xlsx_path)
    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")
    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name), engine="xlsxwriter")                               # pylint: disable=abstract-class-instantiated
    df.to_excel(writer, sheet_name="Sheet1", freeze_panes=(1, 1), index=False)
    workbook: Workbook = writer.book
    worksheet: Worksheet = writer.sheets["Sheet1"]
    format_error = workbook.add_format({'bg_color': '#FFC7CD', 'font_color': '#9C0006'})
    format_good = workbook.add_format({'bg_color': '#C6EFCD', 'font_color': '#006100'})
    format_warn = workbook.add_format({'bg_color': '#FCF4A3', 'font_color': '#000000'})

    for col_num, column in enumerate(df.columns[1:]):
        for row_num, (value, isnull) in enumerate(zip(df[column], df[column].isnull())):
            if isnull:
                worksheet.write(row_num + ROW_OFFSET, col_num + COL_OFFSET + 1, "", format_error)
                continue

            if row_num == 0:
                continue

            if value < df[column][row_num - 1]:
                worksheet.write(row_num + ROW_OFFSET, col_num + 1, value, format_warn)
                continue

            worksheet.write(row_num + ROW_OFFSET, col_num + 1, value, format_good)

    worksheet.autofit()
    writer.close()


def check_simple_xlsx(args: Args):
    os.makedirs(args.save_path, exist_ok=True)

    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")):
                continue
            check_simple_single(filename, args.save_path)


check_simple_xlsx(parse_args(Args))

# ! python 1.2_check_simple_rules.py C:\Coding\works\wbi\ProjectsISC\Amati_2023_10_30\data\catalog\xlsx_fix_ocr C:\Coding\works\wbi\ProjectsISC\Amati_2023_10_30\data\catalog\xlsx_test
