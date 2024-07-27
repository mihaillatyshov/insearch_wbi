import os
import math

import pandas as pd

from shared import ArgsBase, parse_args


class Args(ArgsBase):
    xlsx_path: str
    save_path: str


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


def fix_pl_single(xlsx_path: os.DirEntry[str], save_path: str):
    print(xlsx_path)

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")

    del df["pl"]
    del df["lf"]

    # * Calc List
    for key, func in calc_list.items():
        df.insert(len(df.columns), key, func(df))

    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name), engine="xlsxwriter")
    df.to_excel(writer, sheet_name="ctd_ds1", freeze_panes=(1, 1), index=False)
    worksheet = writer.sheets["ctd_ds1"]
    worksheet.autofit()
    writer.close()


def fix_pl(args: Args):
    os.makedirs(args.save_path, exist_ok=True)
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            print(filename.path, filename.name)
            if (filename.name.startswith("~$")): continue
            fix_pl_single(filename, args.save_path)


fix_pl(parse_args(Args))
