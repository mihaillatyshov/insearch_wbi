import os
import re
from typing import TypedDict

import pandas as pd

from shared import df_select_group, get_onedrive_path, is_cell_none
from shared_inserts import symb_to_iwsc

IN_PATH = "yg1_not_present.xlsx"
OUT_PATH = "turning_holders/yg1_not_present_Universal_Turning_Holders_TR.xlsx"

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


class DataList(TypedDict):
    model: list[str]
    constr: list[str]
    dc: list[float | None]
    dcx: list[float | None]
    dcon: list[float | None]
    zefp: list[int | None]
    cict: list[int | None]
    dconms: list[float | None]
    dconws: list[float | None]
    lf: list[float | None]
    bd1: list[float | None]
    bmc: list[str | None]
    apmx: list[float | None]                                                                                            # ! can parse from model ???
    cutintmaster: list[str | None]
    machiningtype: list[int | None]
    iwsc: list[int | None]
    cnsc: list[int]
    cxsc: list[int]
    multiplatform: list[str | None]
    adintms: list[str | None]
    adintws: list[str | None]
    cst: list[str | None]
    cw: list[float | None]
    ceatc: list[str | None]
    oal: list[float | None]
    kapr: list[float | None]
    thszms: list[str | None]
    re: list[float | None]
    mgro: list[str | None]                                                                                              # ! only for ctd_sha1
    sep: list[int | None]


null_cols = [
    "serie", "cp", "bbd", "dcn", "dcsfms", "bd2", "bd3", "lb1", "lb2", "lb3", "bhta1", "bhta2", "wt", "cedc", "zipx",
    "cwx", "cwn", "cdx", "kww", "stragged", "tmins", "lu", "rpmx", "dn", "dbc", "kdp", "dbc2", "adj", "kwy", "nof"
]

dl: DataList = {
    "model": [],
    "constr": [],
    "dc": [],
    "dcx": [],
    "dcon": [],
    "zefp": [],
    "cict": [],
    "dconms": [],
    "dconws": [],
    "lf": [],
    "bd1": [],
    "bmc": [],
    "apmx": [],
    "cutintmaster": [],
    "machiningtype": [],
    "iwsc": [],
    "cnsc": [],
    "cxsc": [],
    "multiplatform": [],
    "adintms": [],
    "adintws": [],
    "cst": [],
    "cw": [],
    "ceatc": [],
    "oal": [],
    "kapr": [],
    "thszms": [],
    "re": [],
    "mgro": [],
    "sep": [],
}

group = "Indexable Turning"
line = "Universal Holder (TR)"
df_no_props = df_select_group(
    pd.read_excel(os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/in", IN_PATH),
                  index_col=None,
                  engine="openpyxl",
                  sheet_name="NoProps"),
    line=line,
    group=group,
)
df_not_present = df_select_group(
    pd.read_excel(os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/in", IN_PATH),
                  index_col=None,
                  engine="openpyxl",
                  sheet_name="NotPresent"),
    line=line,
    group=group,
)
df = pd.concat([df_no_props, df_not_present], ignore_index=True, sort=False)


def get_insert_reg_search(insert: str):
    return re.search(r"([A-Z][A-Z])([A-Z][A-Z])?(\d\d)(\w\w)?", insert)


def parse_round_insert_re(raw_insert: str):
    tr = get_insert_reg_search(raw_insert)
    if tr is None:
        print("Error parsing insert:", raw_insert)
        exit(1)

    cle = tr.group(3)

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

    return None


def parse_cutintmaster(raw_insert: str):
    tr = get_insert_reg_search(raw_insert)
    if tr is None:
        print("Error parsing insert:", raw_insert)
        exit(1)

    if tr.group(4) is not None:
        return f"{tr.group(1)}{tr.group(2) if tr.group(2) is not None else '..'}{tr.group(3)}{tr.group(4)}"
    return None


def parse_adintws(raw_insert: str):
    tr = get_insert_reg_search(raw_insert)
    if tr is None:
        print("Error parsing insert:", raw_insert)
        exit(1)

    if tr.group(4) is not None:
        return f"ISO_{tr.group(1)}{tr.group(3)}{tr.group(4)}H"
    return None


def parse_multiplatform(raw_insert: str):
    tr = get_insert_reg_search(raw_insert)
    if tr is None:
        print("Error parsing insert:", raw_insert)
        exit(1)

    if tr.group(4) is not None:
        return f"{tr.group(1)}{tr.group(3)}{tr.group(4)}H"
    return None


def parse_iwsc(raw_insert: str):
    tr = get_insert_reg_search(raw_insert)
    if tr is None:
        print("Error parsing insert:", raw_insert)
        exit(1)

    if tr.group(2) is not None:
        return symb_to_iwsc.get(tr.group(2))

    return None


for index, row in df.iterrows():
    if (isinstance(index, int) and index % 100 == 0):
        print(f"EDP №: {row['EDP №']}")

    model = row["model"]
    if is_cell_none(model):
        model_reg = re.search(r"\s?([A-Z]\S+)", row["Описание"])

        if model_reg is None:
            print("Error parsing:", row["Описание"])
            exit(1)
        model = model_reg.group(1)

    if model in dl["model"]:
        print("Duplicate model:", model)
        continue

    if model == "Universal":
        print("Model is 'Universal':", model)
        continue

    if "С" in model:
        print("Model contains 'С':", model)
        model = model.replace("С", "C")

    dl["model"].append(model)

    constr = None
    cnsc = 0
    cxsc = 0
    adintws = None
    adintms = None
    dconws = None
    dconms = None
    thszms = None
    dcon = None
    lf = None
    oal = None
    cst = None
    bd1 = None
    bmc = None
    mgro = None
    machiningtype = None
    ceatc = None
    cutintmaster = None
    multiplatform = None
    dc = None
    cw = None
    iwsc = None
    apmx = None
    dcx = None
    zefp = None
    cict = None
    kapr = None
    v_re = None
    sep = None

    ## =#===============================================================================================
    ## =#===============================================================================================
    ## =#===============================================================================================
    ## =#===============================================================================================
    ## =#===============================================================================================
    ## =#===============================================================================================
    ## ================================================================================================
    ## ================================================================================================
    ## ================================================================================================
    ## ================================================================================================
    ## ================================================================================================
    ## ================================================================================================
    ## ================================================================================================

    if model.startswith("YGM-"):
        tr = re.search(r"([AES])(\d\d)([A-Z])")
        tr = re.search(r"YGM-D(\d+)-C(\d+)-M(\d+)-L(\d+)-C", model)
        if tr is None:
            print("Error parsing model (YGM-):", model)
            exit(1)

        constr = "ctd_sha1"
        cnsc = 1
        cxsc = 1
        cst = "HA"
        adintms = f"HA-{tr.group(2)}"
        adintws = f"METRIC_THREAD:M{tr.group(3)}"
        dconms = float(tr.group(2))
        dcomws = float(tr.group(1))
        bd1 = dcomws
        lf = float(tr.group(4))
        bmc = "Carbide" if row["Материал"] == "CARBIDE" else "Steel"
        mgro = "VHM" if row["Материал"] == "CARBIDE" else "STL"
        sep = 0
    elif model.startswith("YGDM-"):
        tr = re.search(r"YGDM-([A-Z]+\w+)-D(\d+)X(\d+)-d(\d+)-Z(\d+)", model)
        if tr is None:
            print("Error parsing model (YGDM-):", model)
            exit(1)

        constr = "ctd_mis_disk"
        machiningtype = 1

        cnsc = 0
        cxsc = 0
        dc = float(tr.group(2))
        cw = float(tr.group(3))
        dcon = float(tr.group(4))
        zefp = int(tr.group(5))
        cict = zefp
        ceatc = "E"
        cutintmaster = parse_cutintmaster(tr.group(1))
        multiplatform = parse_multiplatform(tr.group(1))
        adintms = f"ISO_11529:FDA01-{dcon}"
        adintws = parse_adintws(tr.group(1))
        iwsc = parse_iwsc(tr.group(1))
    else:
        tr = re.search(
            r"(YG)?(DM|T|R|CH|CM|E|F|M)(\d+\.?\d*|R|HF)?([LS])?-([A-Z]+\w+)-D(\d+\.?\d*)(?:[Xx](\d+\.?\d*))?-([CWMS])(\d+)(?:-L(\d+))?-Z(\d\d?)(\d\d)?(-C)?",
            model)
        if tr is not None:
            raw_cst = tr.group(8)
            raw_dcon = tr.group(9)
            raw_lf_or_oal = tr.group(10)
            raw_zefp = tr.group(11)
            raw_cict = tr.group(12)
            raw_constr = tr.group(2)
            raw_kapr_or_mod = tr.group(3)
            raw_insert = tr.group(5)
            raw_dc = tr.group(6)
            raw_apmx_or_cw = tr.group(7)
            raw_has_coolant = tr.group(13)
        else:
            tr = re.search(
                r"(YG)?(DM|T|R|CH|CM|E|F|M)(\d+\.?\d*|R|HF)?([LS])?-([A-Z]+\w+)-D(\d+\.?\d*)(?:[Xx](\d+\.?\d*))?Z(\d\d?)(\d\d)?([CWMS])(\d+)(?:-L(\d+))?(-C)?",
                model)
            if tr is not None:
                raw_cst = tr.group(10)
                raw_dcon = tr.group(11)
                raw_lf_or_oal = tr.group(12)
                raw_zefp = tr.group(8)
                raw_cict = tr.group(9)
                raw_constr = tr.group(2)
                raw_kapr_or_mod = tr.group(3)
                raw_insert = tr.group(5)
                raw_dc = tr.group(6)
                raw_apmx_or_cw = tr.group(7)
                raw_has_coolant = tr.group(13)
            else:
                tr = re.search(
                    r"(YG)?(DM|T|R|CH|CM|E|F|M)(\d+\.?\d*|R|HF)?([LS])?-([A-Z]+\w+)-D(\d+\.?\d*)(?:AP(\d+\.?\d*))?Z(\d\d?)(\d\d)?([CWMS])(\d+)(?:-L(\d+))?(-C)?",
                    model)
                if tr is not None:
                    raw_cst = tr.group(10)
                    raw_dcon = tr.group(11)
                    raw_lf_or_oal = tr.group(12)
                    raw_zefp = tr.group(8)
                    raw_cict = tr.group(9)
                    raw_constr = tr.group(2)
                    raw_kapr_or_mod = tr.group(3)
                    raw_insert = tr.group(5)
                    raw_dc = tr.group(6)
                    raw_apmx_or_cw = tr.group(7)
                    raw_has_coolant = tr.group(13)
                    print("Test", tr.groups())
                else:
                    print("Error parsing model:", model)
                    exit(1)

        if raw_constr == "DM": constr = "ctd_mis_disk"
        elif raw_constr == "T": constr = "ctd_mie_t"
        elif raw_constr == "R": constr = "ctd_mie_t"
        elif raw_constr == "CH": constr = "ctd_mie_chamf"
        elif raw_constr == "E" and raw_kapr_or_mod == "R": constr = "ctd_mie_rou"
        elif raw_constr == "E" and raw_kapr_or_mod == "HF": constr = "ctd_mie_hf"
        elif raw_constr == "E" and raw_kapr_or_mod == "90": constr = "ctd_mie_90"
        elif raw_constr == "F" and raw_kapr_or_mod == "R": constr = "ctd_mib_rou"
        elif raw_constr == "F" and raw_kapr_or_mod == "HF": constr = "ctd_mib_hf"
        elif raw_constr == "F" and raw_kapr_or_mod == "90": constr = "ctd_mib_90"
        elif raw_constr == "F": constr = "ctd_mib_3075"
        elif raw_constr == "M" and raw_kapr_or_mod == "R": constr = "ctd_mih_rou"
        elif raw_constr == "M" and raw_kapr_or_mod == "HF": constr = "ctd_mih_hf"
        elif raw_constr == "M" and raw_kapr_or_mod == "90": constr = "ctd_mih_90"
        elif raw_constr == "CM": constr = "ctd_mie_corn901"
        elif raw_constr == "E":
            print("Test", tr.groups())
            constr = "ctd_mie_rou"
        elif raw_constr == "M":
            print("Test", tr.groups())
            constr = "ctd_mih_rou"

        if constr == "ctd_mib_hf" or constr == "ctd_mih_hf":
            machiningtype = 3
        else:
            machiningtype = 1

        cutintmaster = parse_cutintmaster(raw_insert)
        multiplatform = parse_multiplatform(raw_insert)
        adintws = parse_adintws(raw_insert)
        iwsc = parse_iwsc(raw_insert)
        if constr == "ctd_mie_t" and raw_apmx_or_cw is not None:
            cw = float(raw_apmx_or_cw)
            apmx = cw
        elif raw_apmx_or_cw is not None:
            dcx = float(raw_apmx_or_cw)

        if raw_kapr_or_mod == "R":
            dcx = float(raw_dc)
            raw_re = parse_round_insert_re(raw_insert)
            if raw_re is not None:
                v_re = raw_re
                apmx = raw_re
                dc = dcx - v_re
        else:
            dc = float(raw_dc)

        dcon = float(raw_dcon)
        zefp = int(raw_zefp)

        if cict is not None: cict = int(raw_cict)

        if raw_cst == "C":
            cst = "HA"
            adintms = f"HA-{dcon}"
        elif raw_cst == "W":
            cst = "HB"
            adintms = f"HB-{dcon}"
        elif raw_cst == "M":
            adintms = f"METRIC_THREAD:M{dcon}"
            thszms = f"M{dcon}"
        elif raw_cst == "S":
            adintms = f"ISO_6462:FDAxx{raw_dcon.zfill(3)}"

        cnsc = 0
        cxsc = 0
        if raw_has_coolant is not None:
            cnsc = 1
            cxsc = 3

        if raw_constr == "M" and raw_lf_or_oal is not None:
            lf = float(raw_lf_or_oal)
        elif raw_lf_or_oal is not None:
            oal = float(raw_lf_or_oal)

        if constr != "ctd_mie_t":
            try:
                kapr = float(raw_kapr_or_mod)
            except Exception:
                kapr = None

    if constr is None:
        print("Error parsing model (null constr):", model)
        exit(1)

    dl["constr"].append(constr)
    dl["cnsc"].append(cnsc)
    dl["cxsc"].append(cxsc)
    dl["adintws"].append(adintws)
    dl["adintms"].append(adintms)
    dl["dconws"].append(dconws)
    dl["dconms"].append(dconms)
    dl["thszms"].append(thszms)
    dl["dcon"].append(dcon)
    dl["lf"].append(lf)
    dl["oal"].append(oal)
    dl["cst"].append(cst)
    dl["bd1"].append(bd1)
    dl["bmc"].append(bmc)
    dl["mgro"].append(mgro)
    dl["machiningtype"].append(machiningtype)
    dl["ceatc"].append(ceatc)
    dl["cutintmaster"].append(cutintmaster)
    dl["multiplatform"].append(multiplatform)
    dl["dc"].append(dc)
    dl["cw"].append(cw)
    dl["iwsc"].append(iwsc)
    dl["apmx"].append(apmx)
    dl["dcx"].append(dcx)
    dl["zefp"].append(zefp)
    dl["cict"].append(cict)
    dl["kapr"].append(kapr)
    dl["re"].append(v_re)
    dl["sep"].append(sep)

new_df = pd.DataFrame(dl)

new_df.insert(1, "manuf", "YG1")
# new_df.insert(2, "plvendor", "ISO")

# new_df.insert(len(new_df.columns), "constr", "ctd_inserts")
# new_df.insert(len(new_df.columns), "adintws", "WORKPIECE")

for val in null_cols:
    new_df.insert(len(new_df.columns), val, None)

new_df.to_excel(os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/out_tmp", OUT_PATH), index=False)

print(new_df)
