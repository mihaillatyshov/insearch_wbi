import math

import pandas as pd

INPUT_XLSX = "C:/Coding/works/wbi/pdf_parser/drill_fixes/xlsx/need_angle_fix.xlsx"
OUTPUT_XLSX = "C:/Coding/works/wbi/pdf_parser/drill_fixes/xlsx/angle_fix.xlsx"


def calc_pl(df: pd.DataFrame) -> list[str | int | float]:
    result: list[str | int | float] = []
    for i in range(len(df.index)):
        try:
            dc = df["dc"][i]
            sig = df["sig"][i]
            result.append(round(dc / 2 * math.tan(math.radians(180 - sig) / 2), 3))
        except Exception:
            result.append("")
    return result


def calc_lf(df: pd.DataFrame) -> list[str | int | float]:
    result: list[str | int | float] = []
    for i in range(len(df.index)):
        try:
            pl = df["pl"][i]
            oal = df["oal"][i]
            result.append(oal - pl)
        except Exception:
            result.append("")
    return result


calc_list = {
    "pl": calc_pl,
    "lf": calc_lf,
}

df = pd.read_excel(INPUT_XLSX, index_col=None, engine="openpyxl")

del df["id"]
del df["tool_id"]
del df["item_id"]

del df["sig"]
del df["pl"]
del df["lf"]

df.insert(len(df.columns), "sig", 135)

# * Calc List
for key, func in calc_list.items():
    df.insert(len(df.columns), key, func(df))

writer = pd.ExcelWriter(OUTPUT_XLSX, engine="xlsxwriter")
df.to_excel(writer, sheet_name="ctd_ds1", freeze_panes=(1, 1), index=False)
worksheet = writer.sheets["ctd_ds1"]
worksheet.autofit()
writer.close()
