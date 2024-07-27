import os
import math
from typing import Any, Callable, Literal

import pandas as pd
from pandas import Series

from shared import ArgsBase, flatten, frange, parse_args

from thread_params import thread_extra_list, thread_coarse_list, thread_fine_list


class Args(ArgsBase):
    xlsx_path: str
    save_path: str
    rules_path: str


def get_app1(page_id: int) -> str:
    if page_id in frange(124, 134):
        return "PMK"

    raise ValueError(f"No id ({page_id}) in list get_app1")


def get_coating(page_id: int) -> str:
    if page_id in frange(124, 126):
        return "NONE"

    if page_id in frange(130, 134):
        return "ALCRSIN"

    raise ValueError(f"No id ({page_id}) in list get_coating")


def get_cnsc(page_id: int) -> int:
    if page_id in [124]:
        return 0

    if page_id in frange(125, 126):
        return 1

    if page_id in frange(130, 134):
        return 0

    raise ValueError(f"No id ({page_id}) in list get_cnsc")


def get_cxsc(page_id: int) -> int:
    if page_id in [124]:
        return 0

    if page_id in frange(125, 126):
        return 2

    if page_id in frange(130, 134):
        return 0

    raise ValueError(f"No id ({page_id}) in list get_cnsc")


def get_constr(page_id: int) -> str:
    if page_id in frange(124, 126):
        return "ctd_rm1"

    if page_id in [130, 131, 132]:
        return "ctd_mt3"

    if page_id in [133, 134]:
        return "ctd_mt1"

    raise ValueError(f"No id ({page_id}) in list get_constr")


def get_serie(page_id: int) -> str:
    if page_id in [124]:
        return "RHC"

    if page_id in [125, 126]:
        return "RHC-C"

    if page_id in [130]:
        return "M1C"

    if page_id in [131, 132]:
        return "M3C"

    if page_id in [133, 134]:
        return "M"

    raise ValueError(f"No id ({page_id}) in list get_serie")


def get_dctol(page_id: int) -> str:
    if page_id in frange(124, 126):
        return "H7"

    if page_id in frange(130, 134):
        return ""

    raise ValueError(f"No id ({page_id}) in list get_dctol")


def get_tcha(page_id: int) -> str:
    return get_dctol(page_id)


def get_hand(page_id: int) -> str:
    if page_id in frange(124, 126):
        return "R"

    if page_id in frange(130, 134):
        return ""

    raise ValueError(f"No id ({page_id}) in list get_hand")


def get_thft(page_id: int) -> str:
    if page_id in frange(124, 126):
        return ""

    if page_id in frange(130, 134):
        return "M60"

    raise ValueError(f"No id ({page_id}) in list get_thft")


def get_tctr(page_id: int) -> str:
    if page_id in frange(124, 126):
        return ""

    if page_id in frange(130, 134):
        return "Standard"

    raise ValueError(f"No id ({page_id}) in list get_tctr")


def get_nt(page_id: int) -> str | int:
    if page_id in frange(124, 126):
        return ""

    if page_id in [130]:
        return 1

    if page_id in [131, 132]:
        return 3

    if page_id in frange(133, 134):
        return ""

    raise ValueError(f"No id ({page_id}) in list get_tctr")


def get_flutehand(page_id: int) -> str:
    if page_id in frange(124, 126):
        return "R"

    if page_id in frange(130, 134):
        return "R"

    raise ValueError(f"No id ({page_id}) in list get_tctr")


def get_ribboncon(page_id: int) -> str | int:
    if page_id in frange(124, 126):
        return 0

    if page_id in frange(130, 134):
        return ""

    raise ValueError(f"No id ({page_id}) in list get_tctr")


def get_brazed(page_id: int) -> str | int:
    if page_id in frange(124, 126):
        return 0

    if page_id in frange(130, 134):
        return ""

    raise ValueError(f"No id ({page_id}) in list get_tctr")


add_list: dict[str, Callable[[int], str | int | float | bool]] = {
    "app1": get_app1,
    "coating": get_coating,
    "cnsc": get_cnsc,
    "cxsc": get_cxsc,
    "constr": get_constr,
    "serie": get_serie,
    "dctol": get_dctol,
    "tcha": get_tcha,
    "hand": get_hand,
    "thft": get_thft,
    "tctr": get_tctr,
    "nt": get_nt,
    "flutehand": get_flutehand,
    "ribboncon": get_ribboncon,
    "brazed": get_brazed,
    "grade": lambda _: "Super MG",
    "substrate": lambda _: "Super MG",
    "cst": lambda _: "HA",
    "tcdcon": lambda _: "h6",
    "adintws": lambda _: "WORKPIECE",
    "mgro": lambda _: "VHM",
    "releasepack": lambda _: "2023/Amati Форма Точности",
    "sep": lambda _: 0,
    "holetype": lambda _: "BOTH",
}

CalcReturnType = str | int | float | list[Any]                                                                          # pylint: disable=unsubscriptable-object


# NOTE template for real programm
def calc_adintms(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    result: list[str | int | float] = []
    for i in range(len(df.index)):
        try:
            cst = df["cst"][i]
            dcon = df["dcon"][i]
            result.append(f"{cst}-{int(dcon) if int(dcon) == dcon else dcon}")
        except Exception:
            result.append("")
    return result


def _calc_tolerance(dc: int | float) -> str | float | int:
    if 1 <= dc <= 3:
        return 10 / 1000

    if 3 < dc <= 6:
        return 12 / 1000

    if 6 < dc <= 10:
        return 15 / 1000

    if 10 < dc <= 18:
        return 18 / 1000

    if 18 < dc <= 30:
        return 21 / 1000

    raise ValueError(f"No _calc_tolerance for dc: {dc}")


def calc_dctoll(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(130, 134):
        return ""

    return 0


def calc_tchal(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    return calc_dctoll(page_id, df)


def calc_dctolu(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(130, 134):
        return ""

    return [_calc_tolerance(dc) for dc in df["dc"]]


def calc_tchau(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    return calc_dctolu(page_id, df)


def calc_lu(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(124, 126):
        return [l + l2 for l, l2 in zip(df["l"], df["L2"])]

    if page_id in frange(130, 132):
        return df["lu"].to_list()

    return ""


def calc_lsh(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    result: list[str | int | float] = []
    for i in range(len(df.index)):
        try:
            oal = df["oal"][i]
            lu = df["lu"][i]
            result.append(oal - lu)
        except Exception:
            result.append("")
    return result


def calc_lf(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(124, 126):
        return df["oal"].to_list()

    return ""


def calc_plgl(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(124, 126):
        return df["l"].to_list()

    return ""


def calc_threadsh(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(124, 126):
        return ""

    return [f"M{tdz}X{tp}" for tdz, tp in zip(df["tdz"], df["tp"])]


def calc_tpx(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(124, 126):
        return ""

    return df["tp"].to_list()


def calc_tpn(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(124, 126):
        return ""

    return df["tp"].to_list()


def calc_thrtyp(page_id: int, df: pd.DataFrame) -> CalcReturnType:
    if page_id in frange(124, 126):
        return ""

    result: list[str | int | float] = []
    for i in range(len(df.index)):
        tdz = df["tdz"][i]
        tp = df["tp"][i]
        is_found = False

        for llv_p, llv_d in thread_extra_list:
            if llv_d == tdz and llv_p == tp:
                is_found = True
                result.append("Extra")
                break

        if is_found:
            continue

        for llv_p, llv_d in thread_coarse_list:
            if llv_d == tdz and llv_p == tp:
                is_found = True
                result.append("Coarse")
                break

        if is_found:
            continue

        for llv_p, llv_d in thread_fine_list:
            if llv_d == tdz and llv_p == tp:
                is_found = True
                result.append("Fine")
                break

        if is_found:
            continue

        raise ValueError(f"No tdz: {tdz}  and tp: {tp}  in thrtyp check lists")

    return result


calc_list: dict[str, Callable[[int, pd.DataFrame], CalcReturnType]] = {
    "adintms": calc_adintms,
    "dctoll": calc_dctoll,
    "dctolu": calc_dctolu,
    "tchal": calc_tchal,
    "tchau": calc_tchau,
    "lu": calc_lu,
    "lsh": calc_lsh,
    "lf": calc_lf,
    "plgl": calc_plgl,
    "threadsh": calc_threadsh,
    "tpx": calc_tpx,
    "tpn": calc_tpn,
    "thrtyp": calc_thrtyp,
}


# NOTE: rows_count = len(df.index)
# NOTE: cols_count = len(df.columns)
# NOTE: cols_names = list(df.columns)
def add_extra_info_single(xlsx_path: os.DirEntry[str], save_path: str):
    print(xlsx_path)
    page_id = int(xlsx_path.name.split('.')[0])

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")

    # * Add List
    for key, add_func in add_list.items():
        if key in df:
            continue
        df.insert(len(df.columns), key, add_func(page_id))

    # * Calc List
    for key, calc_func in calc_list.items():
        if key in df:
            continue
        df.insert(len(df.columns), key, calc_func(page_id, df))

    df.insert(1, "manuf", "AMATI")

    if "L2" in df:
        del df["L2"]

    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name), engine="xlsxwriter")                               # pylint: disable=abstract-class-instantiated
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
