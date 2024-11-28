import re
from typing import TypedDict

import pandas as pd

from shared import df_select_group, is_cell_none
from shared_inserts import symb_to_ifs, symb_to_iwsc

IN_PATH = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/in/yg1_not_present.xlsx"
OUT_PATH = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/out_tmp/yg1_not_present_Universal_Turning_Insert.xlsx"

# item = "Патрон фрезерный гидравлический SK50-HC25G-110 . Конус SK40 DIN 69871. Посадочный диаметр 25 мм, Длина до торца 110 мм; Система охлаждения AD, класс балансировки G2.5/25,000 об/мин"
# fixed_item = re.sub(r'(G[0-9]+.[0-9]+/[0-9]+),([0-9]+)', r'\1\2', fixed_item)
# fixed_item = re.sub(r'Сменная\s*твердосплавная\s*фрезерная\s*пластина\s*([0-9]+)', r'\1.\2', fixed_item)
# fixed_item_arr = re.split(r"\s*;\s*|\s*\.\s+|\s*,\s*", fixed_item)
# print(fixed_item_arr)

INS_IC_3_97 = 3.97
INS_IC_4_76 = 4.76
INS_IC_5_56 = 5.56
INS_IC_6_35 = 6.35
INS_IC_7_94 = 7.94
INS_IC_8 = 8
INS_IC_9_52 = 9.52
INS_IC_10 = 10
INS_IC_12 = 12
INS_IC_12_7 = 12.7
INS_IC_15_88 = 15.88
INS_IC_16 = 16
INS_IC_19_05 = 19.05
INS_IC_20 = 20
INS_IC_25 = 25
INS_IC_25_4 = 25.4
INS_IC_32 = 32


def get_ic_from_cle_symb(sc: str, cle: str | None) -> float | None:
    if (cle is None):
        return None

    # ? NONE: yg1
    if sc == "A":
        if cle == "10": return 6.7
        if cle == "11": return 6.2
        if cle == "12": return 6.6
        if cle == "15": return 9.7
        if cle == "16": return 9.4
    # if sc == "B":

    if sc == "C":
        if cle == "03": return INS_IC_3_97
        if cle == "04": return INS_IC_4_76
        if cle == "05": return INS_IC_5_56
        if cle == "06": return INS_IC_6_35
        if cle == "08": return INS_IC_7_94
        if cle == "09": return INS_IC_9_52
        if cle == "12": return INS_IC_12_7
        if cle == "16": return INS_IC_15_88
        if cle == "19": return INS_IC_19_05
        if cle == "25": return INS_IC_25_4

    if sc == "D":
        if cle == "04": return INS_IC_3_97
        if cle == "05": return INS_IC_4_76
        if cle == "06": return INS_IC_5_56
        if cle == "07": return INS_IC_6_35
        if cle == "09": return INS_IC_7_94
        if cle == "11": return INS_IC_9_52
        if cle == "15": return INS_IC_12_7
        if cle == "19": return INS_IC_15_88
        if cle == "23": return INS_IC_19_05
        if cle == "31": return INS_IC_25_4

    # ? NOTE: only for yg1?
    if sc == "E":
        if cle == "06": return 6.3
        if cle == "09": return 9

    # if sc == "H":
    # if cle == "05": return INS_IC_12_7

    if sc == "K":
        if cle == "16": return INS_IC_9_52

    # if sc == "L":

    if sc == "O":
        if cle == "05": return 12.73
        if cle == "06": return 15.9
        if cle == "07": return 18.05
        if cle == "08": return 20.2

    if sc == "P":
        # if cle == "04": return INS_IC_6_35
        # if cle == "05": return INS_IC_7_94
        # if cle == "07": return INS_IC_9_52
        # if cle == "09": return INS_IC_12_7
        # if cle == "11": return INS_IC_15_88
        # ? NOTE: yg1
        if cle == "12": return 14
        # if cle == "13": return INS_IC_19_05
        # if cle == "18": return INS_IC_25_4
        # if cle == "23": return INS_IC_32

    if sc == "R":
        if cle == "08": return INS_IC_8
        if cle == "09": return INS_IC_9_52
        if cle == "10": return INS_IC_10
        # ? NOTE: can be 12.7
        if cle == "12": return INS_IC_12
        if cle == "15": return INS_IC_15_88
        if cle == "16": return INS_IC_16
        if cle == "19": return INS_IC_19_05
        if cle == "20": return INS_IC_20
        # ? NOTE: can be 25.4
        if cle == "25": return INS_IC_25
        if cle == "32": return INS_IC_32

    if sc == "S":
        if cle == "03": return INS_IC_3_97
        if cle == "04": return INS_IC_4_76
        if cle == "05": return INS_IC_5_56
        if cle == "06": return INS_IC_6_35
        if cle == "07": return INS_IC_7_94
        if cle == "09": return INS_IC_9_52
        if cle == "12": return INS_IC_12_7
        if cle == "13": return 13.4
        if cle == "15": return INS_IC_15_88
        if cle == "16": return INS_IC_15_88
        if cle == "19": return INS_IC_19_05
        if cle == "25": return INS_IC_25_4

    if sc == "T":
        if cle == "06": return INS_IC_3_97
        if cle == "08": return INS_IC_4_76
        if cle == "09": return INS_IC_5_56
        if cle == "11": return INS_IC_6_35
        if cle == "13": return INS_IC_7_94
        if cle == "16": return INS_IC_9_52
        if cle == "22": return INS_IC_12_7
        if cle == "27": return INS_IC_15_88
        if cle == "33": return INS_IC_19_05
        if cle == "44": return INS_IC_25_4

    if sc == "V":
        if cle == "08": return INS_IC_4_76
        if cle == "09": return INS_IC_5_56
        if cle == "11": return INS_IC_6_35
        if cle == "13": return INS_IC_7_94
        if cle == "16": return INS_IC_9_52
        if cle == "22": return INS_IC_12_7
        if cle == "27": return INS_IC_15_88
        if cle == "33": return INS_IC_19_05

    if sc == "W":
        if cle == "02": return INS_IC_3_97
        if cle == "03": return INS_IC_5_56
        if cle == "04": return INS_IC_6_35
        if cle == "05": return INS_IC_7_94
        if cle == "06": return INS_IC_9_52
        if cle == "08": return INS_IC_12_7
        if cle == "10": return INS_IC_15_88
        if cle == "13": return INS_IC_19_05

    print(f"Error: unknown cle for sc {sc}:", cle)
    return None


sc_symb_to_pna: dict[str, int | None] = {
    "A": 85,
    "B": 82,
    "C": 80,
    "D": 55,
    "E": None,                                                                                                          # yg1 problem?
    "H": 120,
    "K": 55,
    "L": 90,
    "M": None,
    "O": 135,
    "P": 108,
    "R": None,
    "S": 90,
    "T": 60,
    "V": 35,
    "W": 80,
    "N": 55,
}

can_to_an: dict[str, int | None] = {"N": 0, "B": 5, "C": 7, "P": 11, "D": 15, "E": 20, "F": 25, "O": None}

cs1_to_s1: dict[str, float] = {
    "01": 1.59,
    "T1": 1.98,
    "02": 2.38,
    "T2": 2.78,
    "03": 3.18,
    "T3": 3.97,
    "04": 4.76,
    "05": 5.56,
    "06": 6.35,
    "07": 7.94,
    "09:": 9.52,
}

symb_to_re: dict[str, float] = {
    "01": 0.1,
    "02": 0.2,
    "03": 0.3,
    "04": 0.4,
    "05": 0.5,
    "08": 0.8,
    "12": 1.2,
    "16": 1.6,
    "20": 2,
    "24": 2.4,
    "32": 3.2,
}

grade_to_app: dict[str, str] = {
    "YG602": "PMKNS",
    "YG712": "P",
    "YG500": "N",
    "YG622": "PK",
    "YG50": "N",
    "YG603": "M",
    "YG501": "K",
    "YG5020": "K",
    "YG501G": "K",
    "YG613": "PM",
    "YG713": "P",
    "YG612": "PMS",
    "YG012": "PH",
    "YG801": "P",
    "YG1001": "PK",
    "YG10": "N",
    "YG3020": "P",
    "YG3010": "P",
    "YG3030": "P",
    "YG100": "N",
    "YG211": "M",
    "YG213": "M",
    "YG214": "M",
    "YG3015": "P",
    "YG1010": "K",
    "YG401": "S",
    "YC0041": "S",
    "YG3115": "P",
    "YG2025": "M",
    "YT100": "PMK",
}

grade_to_method: dict[str, str] = {
    "YG602": "PVD",
    "YG712": "PVD",
    "YG500": "PVD",
    "YG622": "PVD",
    "YG50": "NONE",
    "YG603": "PVD",
    "YG501": "PVD",
    "YG5020": "CVD",
    "YG501G": "PVD",
    "YG613": "PVD",
    "YG713": "PVD",
    "YG612": "PVD",
    "YG012": "PVD",
    "YG801": "PVD",
    "YG1001": "CVD",
    "YG10": "NONE",
    "YG3020": "CVD",
    "YG3010": "CVD",
    "YG3030": "CVD",
    "YG100": "DLC",
    "YG211": "PVD",
    "YG213": "PVD",
    "YG214": "PVD",
    "YG3015": "CVD",
    "YG1010": "CVD",
    "YG401": "PVD",
    "YC0041": "cvd",
    "YG3115": "CVD",
    "YG2025": "CVD",
    "YT100": "NONE",
}


class DataList(TypedDict):
    model: list[str]
    sc: list[str | None]
    pna: list[int | None]
    can: list[str | None]
    an: list[float | int | None]
    tc: list[str | None]
    ifs: list[int | None]
    iwsc: list[int | None]
    cle: list[str | None]
    ic: list[float | None]
    cs1: list[str | None]
    s1: list[float | None]
    re: list[float | None]
    hand: list[str | None]
    chipbreaker: list[str | None]
    grade: list[str | None]
    mgro: list[str | None]
    multiplatform: list[str | None]
    basis: list[str | None]
    punze: list[str | None]
    adintms: list[str | None]
    app1: list[str | None]
    method: list[str | None]


dl: DataList = {
    "model": [],
    "sc": [],
    "pna": [],
    "can": [],
    "an": [],
    "tc": [],
    "ifs": [],
    "iwsc": [],
    "cle": [],
    "ic": [],
    "cs1": [],
    "s1": [],
    "re": [],
    "hand": [],
    "chipbreaker": [],
    "grade": [],
    "mgro": [],
    "multiplatform": [],
    "basis": [],
    "punze": [],
    "adintms": [],
    "app1": [],
    "method": [],
}

line = "Universal Turning Insert"
group = None
df_no_props = df_select_group(
    pd.read_excel(IN_PATH, index_col=None, engine="openpyxl", sheet_name="NoProps"),
    line=line,
    group=group,
)
df_not_present = df_select_group(
    pd.read_excel(IN_PATH, index_col=None, engine="openpyxl", sheet_name="NotPresent"),
    line=line,
    group=group,
)
df = pd.concat([df_no_props, df_not_present], ignore_index=True, sort=False)

for index, row in df.iterrows():
    if (isinstance(index, int) and index % 100 == 0):
        print(f"EDP №: {row['EDP №']}")

    model = row["model"]
    if is_cell_none(model):
        if (row["Техническое описание"].startswith("Корпус для пластин")):
            print(row["Техническое описание"])
            continue
        model_reg = re.search(r"Сменная\s*твердосплавная\s*токарная\s*пластина\s*(\w+(-\w+)?-\w+)", row["Описание"])
        if model_reg is None:
            model_reg = re.search(r"Специальная\s*сменная\s*твердосплавная\s*токарная\s*пластина\s*(\w+(-\w+)?-\w+)",
                                  row["Описание"])
        if model_reg is None:
            print("Error parsing:", row["Описание"])
            exit(1)
        model = model_reg.group(1)

    if model == "RBEX50-YG602":
        continue

    if "С" in model or "M" in model or "T" in model:
        print("Model contains 'С', 'Т' or 'М':", model)
        model = model.replace("С", "C").replace("М", "M").replace("Т", "T")

    if model in dl["model"]:
        print("Duplicate model:", model)
        continue

    dl["model"].append(model)

    tr = re.search(r"([A-Z][A-Z][A-Z][A-Z])(\d\d)(\w\w)(\d\d|M0)?[LR]?(?:\d\d)?([A-Z][A-Z][A-Z][A-Z]?|R)?(-\w+)?(-\w+)",
                   model)
    if tr is None:
        print("Error parsing model:", model)
        exit(1)
    dl["sc"].append(tr.group(1)[0])
    dl["pna"].append(sc_symb_to_pna.get(tr.group(1)[0]))
    can = tr.group(1)[1]
    dl["can"].append(can)
    dl["an"].append(can_to_an.get(can))
    dl["tc"].append(tr.group(1)[2])
    ifs_and_iwsc = tr.group(1)[3]
    dl["ifs"].append(symb_to_ifs.get(ifs_and_iwsc))
    dl["iwsc"].append(symb_to_iwsc.get(ifs_and_iwsc))
    dl["cle"].append(tr.group(2))
    ic = get_ic_from_cle_symb(tr.group(1)[0], tr.group(2))
    dl["ic"].append(ic)
    if ic is None:
        print("\t for model:", model)
    dl["cs1"].append(tr.group(3))
    dl["s1"].append(cs1_to_s1.get(tr.group(3)))

    re_raw = tr.group(4)

    if re_raw is None:
        dl["re"].append(None)
    elif re_raw == "M0" and ic is not None:
        dl["re"].append(ic / 2)
    else:
        dl["re"].append(symb_to_re.get(re_raw))

    dl["hand"].append(tr.group(5)[-1] if tr.group(5) is not None and tr.group(5)[-1] in ["L", "R"] else "N")
    dl["chipbreaker"].append(tr.group(6)[1:] if tr.group(6) is not None else None)
    grade = tr.group(7)[1:] if tr.group(7) is not None else None
    dl["grade"].append(grade)
    if grade is None:
        print("Error parsing grade:", model)
        dl["app1"].append(None)
        dl["method"].append(None)
    else:
        dl["app1"].append(grade_to_app.get(grade))
        dl["method"].append(grade_to_method.get(grade))
    dl["mgro"].append("VHM" if row["Материал"] == "CARBIDE" else "HSS")
    dl["multiplatform"].append(f"{tr.group(1)[0]}{tr.group(1)[1]}{tr.group(2)}{tr.group(3)}H")
    dl["basis"].append(
        f"{tr.group(1)[0]}{tr.group(1)[1]}{tr.group(2)}{tr.group(3)}{tr.group(4) if tr.group(4) is not None else ''}")
    dl["punze"].append(
        f"{tr.group(1)}{tr.group(2)}{tr.group(3)}{tr.group(4) if tr.group(4) is not None else ''}{tr.group(6)[1:] if tr.group(6) is not None else ''}"
    )
    dl["adintms"].append(f"ISO_{tr.group(1)[0]}{tr.group(1)[1]}{tr.group(2)}{tr.group(3)}H")

    if grade == "UL" or grade == "R16" or grade == "R10" or grade == "R20":
        print("Error grade:", grade, "for model:", model, tr.groups())

    # print(f"{tr.group(1)}{tr.group(2)}{tr.group(3)}{tr.group(4) if tr.group(4) is not None else ''}{tr.group(5)}")

new_df = pd.DataFrame(dl)

new_df.insert(1, "manuf", "YG1")
new_df.insert(2, "plvendor", "ISO")

new_df.insert(len(new_df.columns), "sep", 0)
new_df.insert(len(new_df.columns), "constr", "ctd_inserts")
new_df.insert(len(new_df.columns), "adintws", "WORKPIECE")

new_df.insert(len(new_df.columns), "serie", None)
new_df.insert(len(new_df.columns), "le", None)
new_df.insert(len(new_df.columns), "apmx", None)
new_df.insert(len(new_df.columns), "apmax", None)
new_df.insert(len(new_df.columns), "apmin", None)
new_df.insert(len(new_df.columns), "fmin", None)
new_df.insert(len(new_df.columns), "fmax", None)
new_df.insert(len(new_df.columns), "cedc", None)
new_df.insert(len(new_df.columns), "wt", None)
new_df.insert(len(new_df.columns), "rwpr", None)
new_df.insert(len(new_df.columns), "iep", None)
new_df.insert(len(new_df.columns), "wep", None)
new_df.insert(len(new_df.columns), "bs", None)
new_df.insert(len(new_df.columns), "cdx", None)
new_df.insert(len(new_df.columns), "gan", None)
new_df.insert(len(new_df.columns), "w1", None)
new_df.insert(len(new_df.columns), "m", None)
new_df.insert(len(new_df.columns), "cutintsizeshape", None)

new_df.to_excel(OUT_PATH, index=False)

print(new_df)
print(new_df["grade"].unique())
