import os
from typing import Literal

import pandas as pd

from shared import flatten, frange

PATH = "C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/xlsx_fixed/"
SAVE_PATH = "C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/xlsx_fixed_2/"

AMATI_EMPTY_SYMBOL = "—"


def get_page_app1(id: int) -> str:
    if id in range(9, 22 + 1) or id in range(36, 39 + 1) or id in [46] or id in range(47, 58 + 1):
        return "PMK"
    if id in range(23, 35 + 1):
        return "PMKH"
    if id in range(40, 43 + 1):
        return "PMKS"
    if id in range(44, 45 + 1):
        return "N"
    if id in [47]:
        return "MK"

    raise ValueError(f"No id ({id}) in list get_page_ctd")


def get_coating(id: int) -> str:
    if id in frange(9, 22) or id in frange(46, 56):
        return "TISIN"

    if id in frange(23, 29):
        return "ALCRTIN"

    if id in frange(30, 43):
        return "ALCRSIN"

    if id in frange(44, 45) or id in frange(57, 58):
        return "NONE"

    raise ValueError(f"No id ({id}) in list get_coating")


def get_varom(id: int) -> Literal[0, 1, ""]:
    if id in frange(9, 35) or id in frange(38, 43) or id in [55]:
        return 0

    if id in frange(36, 37) or id in frange(44, 45):
        return 1

    if id in frange(45, 54) or id in frange(56, 58):
        return ""

    raise ValueError(f"No id ({id}) in list get_varom")


def get_ccc(id: int) -> Literal[0, 1]:
    if id in frange(9, 39) or id in frange(42, 58):
        return 1

    if id in frange(40, 41):
        return 0

    raise ValueError(f"No id ({id}) in list get_ccc")


def get_zefp(id: int) -> int:
    if id in [9, 10] or id in frange(13, 17) or id in frange(20, 24) or id in frange(27, 37) \
                     or id in frange(40, 43) or id in frange(52, 56):
        return 4

    if id in [44, 45]:
        return 3

    if id in flatten([[11, 12], [18, 19], [25, 26], [38, 39]]) or id in frange(46, 51):
        return 2

    # ! Возможно другое значение
    if id in [57]:
        return 2

    if id in [58]:
        return 1

    raise ValueError(f"No id ({id}) in list get_zefp")


def get_cpdf(id: int) -> Literal[0, 1]:
    if id in frange(9, 39) or id in [55]:
        return 1

    if id in frange(40, 54) or id in frange(56, 58):
        return 0

    raise ValueError(f"No id ({id}) in list get_cpdf")


def get_fha(id: int) -> int | Literal[""]:
    if id in flatten([[9, 10], [13, 14, 15], [23, 24], [27, 28, 29]]):
        return 35

    if id in flatten([[11, 12], [18, 19], [25, 26], [32, 33], [38, 39]]):
        return 30

    if id in flatten([[16, 17], [20, 21, 22]]):
        return 40

    if id in flatten([[30, 31], [34, 35], [55]]):
        return 45

    if id in flatten([[40, 41, 42, 43]]):
        return 38

    # * Unknown angle
    if id in frange(46, 54) or id in [56, 57, 58]:
        return ""

    # ! Angle from 35 to 38
    if id in flatten([[36, 37]]):
        return 35

    # ? Angle 45 - 50
    if id in flatten([[44, 45]]):
        return 45

    raise ValueError(f"No id ({id}) in list get_fha")


add_list = {
    "app1": get_page_app1,
    "coating": get_coating,
    "varom": get_varom,
    "ccc": get_ccc,
    "zefp": get_zefp,
    "cpdf": get_cpdf,
    "fha": get_fha,
    "grade": lambda _: "Super MG",
    "substrate": lambda _: "Super MG",
    "cst": lambda _: "HA",
    "tcdcon": lambda _: "h6",
    "adintws": lambda _: "WORKPIECE",
    "mgro": lambda _: "VHM",
    "releasepack": lambda _: "2023/Amati Форма Точности",
    "sep": lambda _: 0,
    "cnsc": lambda _: 0,
    "cxsc": lambda _: 0,
}

# ? "dc": # d1
# ? "re": # R
# ? "re1": # R

# ? "apmx": # L1
# ! remove this "—"


def calc_adintms(dcon_col: list[int | float], cst_col: list[str]) -> list[str]:
    return [f"{cst}-{int(dcon) if int(dcon) == dcon else dcon}" for dcon, cst in zip(dcon_col, cst_col)]


def calc_dc(re1_col: list[int | float]) -> list[int | float]:
    return [re1 * 2 for re1 in re1_col]


def calc_dcf(dc_col: list[int | float], re1_col: list[int | float]) -> list[int | float]:
    return [dc - 2 * re1 for dc, re1 in zip(dc_col, re1_col)]


def calc_apmxpfw(apmx_col: list[int | float]) -> list[int | float]:
    return apmx_col.copy()


def calc_apmxffw(apmx_col: list[int | float]) -> list[int | float]:
    return apmx_col.copy()


def calc_lu(apmx_col: list[int | float], _l2_col: None | list[int | float | Literal["—"]] = None) -> list[int | float]:
    if _l2_col is None:
        return apmx_col.copy()

    return [(apmx + (0 if l2 == "—" else l2)) for apmx, l2 in zip(apmx_col, _l2_col)]


def get_tol_from_range(dc: int | float, val_lower3, val_lover10, other):
    if dc < 3: return val_lower3
    if dc <= 10: return val_lover10

    return other


def calc_tolerance(dc_col: list[int | float]) -> list[list[str] | list[float]]:
    dctol: list[str] = [get_tol_from_range(dc, "0~-0.02", "0~-0.03", "0~-0.04") for dc in dc_col]
    dctoll: list[int | float] = [get_tol_from_range(dc, -0.02, -0.03, -0.04) for dc in dc_col]
    dctolu: list[int | float] = [0 for _ in dc_col]

    return [dctol, dctoll, dctolu]


def fix_d2(d2_col: list[int | float | Literal["—"]]) -> list[int | float | Literal[""]]:
    return [("" if d2 == "—" else d2) for d2 in d2_col]


# ! rows_count = len(df.index)
# ! cols_count = len(df.columns)
# ! cols_names = list(df.columns)
def add_extra_info_single(xlsx_path: os.DirEntry[str], save_path: str):
    print(xlsx_path)

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")
    df.rename(columns={"L": "lf", "Dh6": "dcon", "L1": "apmx", "d1": "dc"}, inplace=True)
    df.rename(columns={"R": ("re" if "dc" in df.columns else "re1")}, inplace=True)

    page_id = int(xlsx_path.name.split('.')[0]) + 3

    df.insert(3, "serie", df["model"][0][:5])

    for key, func in add_list.items():
        df.insert(len(df.columns), key, func(page_id))

    if "dc" not in df.columns:
        df.insert(len(df.columns), "dc", calc_dc(df["re1"]))
    if "re1" in df.columns:
        df.insert(len(df.columns), "dcf", calc_dcf(df["dc"], df["re1"]))

    df.insert(len(df.columns), "adintms", calc_adintms(df["dcon"], df["cst"]))
    df.insert(len(df.columns), "apmxpfw", calc_apmxpfw(df["apmx"]))
    df.insert(len(df.columns), "apmxffw", calc_apmxffw(df["apmx"]))
    df.insert(len(df.columns), "lu", calc_lu(df["apmx"], df["L2"] if "L2" in df.columns else None))
    if "L2" in df.columns:
        del df["L2"]

    tolerance = calc_tolerance(df["dc"])
    df.insert(len(df.columns), "dctol", tolerance[0])
    df.insert(len(df.columns), "dctoll", tolerance[1])
    df.insert(len(df.columns), "dctolu", tolerance[2])

    if "d2" in df.columns:
        df.insert(len(df.columns), "dn", fix_d2(df["d2"]))
        del df["d2"]
        # df.drop("d2", inplace=True)

    del df["manuf"]
    df.insert(2, "manuf", "AMATI")

    if df["constr"][0] == "ctd_met":
        df.rename(columns={"lf": "oal", "apmx": "cdx"}, inplace=True)
        df.insert(len(df.columns), "hpcool", 0)
        df.insert(len(df.columns), "bmc", "Carbide")
        df.insert(len(df.columns), "hand", "R")

    writer = pd.ExcelWriter(save_path + xlsx_path.name, engine="xlsxwriter")
    df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 1), index=False)
    worksheet = writer.sheets["sm"]
    worksheet.autofit()
    writer.close()


def add_extra_info(xlsx_path: str, save_path: str):
    for filename in os.scandir(xlsx_path):
        if filename.is_file():
            print(filename.path, filename.name)
            if (filename.name.startswith("~$")): continue
            add_extra_info_single(filename, save_path)


add_extra_info(PATH, SAVE_PATH)