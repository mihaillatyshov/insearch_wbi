import os
import math
from typing import Callable, Literal

import pandas as pd

from shared import ArgsBase, flatten, frange, parse_args


class Args(ArgsBase):
    xlsx_path: str
    save_path: str


def get_app1(id: int) -> str:
    if id in frange(1, 10):
        return "N"

    raise ValueError(f"No id ({id}) in list get_app1")


def get_coating(id: int) -> str:
    if id in frange(1, 10):
        return "TISIN"

    raise ValueError(f"No id ({id}) in list get_coating")


def get_cnsc(id: int) -> int:
    if id in flatten([[1, 2], [7, 8]]):
        return 0

    if id in flatten([[3, 4, 5, 6], [9, 10]]):
        return 1

    raise ValueError(f"No id ({id}) in list get_cnsc")


def get_constr(id: int) -> str:
    if id in frange(1, 10):
        return "ctd_ds1"

    raise ValueError(f"No id ({id}) in list get_constr")


def get_serie(id: int) -> str:
    if id == 1:
        return "DC08AL"
    if id == 2:
        return "DC12AL"
    if id == 3:
        return "DC08AL-C"
    if id == 4:
        return "DC12AL-C"
    if id == 5:
        return "DC20AL-C"
    if id == 6:
        return "DC30AL-C"
    if id == 7:
        return "DC03AL"
    if id == 8:
        return "DC05AL"
    if id == 9:
        return "DC03AL-C"
    if id == 10:
        return "DC05AL-C"

    raise ValueError(f"No id ({id}) in list get_serie")


add_list: dict[str, Callable[[int], str | int | float | bool]] = {
    "app1": get_app1,
    "coating": get_coating,
    "cnsc": get_cnsc,
    "constr": get_constr,
    "serie": get_serie,
    "grade": lambda _: "Super MG",
    "substrate": lambda _: "Super MG",
    "cst": lambda _: "HA",
    "tcdcon": lambda _: "h6",
    "adintws": lambda _: "WORKPIECE",
    "mgro": lambda _: "VHM",
    "releasepack": lambda _: "2023/Amati Форма Точности",
    "sep": lambda _: 0,
    "dctol": lambda _: "m7",
    "hand": lambda _: "R",
}


# NOTE template for real programm
def calc_adintms(df: pd.DataFrame) -> list[str | int | float]:
    result: list[str | int | float] = []
    for i in range(len(df.index)):
        try:
            cst = df["cst"][i]
            dcon = df["dcon"][i]
            result.append(f"{cst}-{int(dcon) if int(dcon) == dcon else dcon}")
        except Exception:
            result.append("")
    return result


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
    "adintms": calc_adintms,
    "pl": calc_pl,
    "lf": calc_lf,
}


# ! rows_count = len(df.index)
# ! cols_count = len(df.columns)
# ! cols_names = list(df.columns)
def add_extra_info_single(xlsx_path: os.DirEntry[str], save_path: str):
    print(xlsx_path)
    page_id = int(xlsx_path.name.split('.')[0])

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")

    # * Add List
    for key, func in add_list.items():
        df.insert(len(df.columns), key, func(page_id))

    # * Calc List
    for key, func in calc_list.items():
        df.insert(len(df.columns), key, func(df))

    df.insert(1, "manuf", "AMATI")

    del df["moq"]

    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name), engine="xlsxwriter")
    df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 1), index=False)
    worksheet = writer.sheets["sm"]
    worksheet.autofit()
    writer.close()


def add_extra_info(args: Args):
    os.makedirs(args.save_path, exist_ok=True)
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            print(filename.path, filename.name)
            if (filename.name.startswith("~$")): continue
            add_extra_info_single(filename, args.save_path)


add_extra_info(parse_args(Args))

# * python add_extra_info_drill.py C:/Coding/works/wbi/amati_drill/xlsx_fix_values/ C:/Coding/works/wbi/amati_drill/xlsx_add_info/
