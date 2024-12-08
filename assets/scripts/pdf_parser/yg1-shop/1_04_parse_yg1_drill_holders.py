from abc import ABC, abstractmethod
import os
import re
from typing import TypedDict

from pydantic import BaseModel
import pandas as pd

from shared import df_select_group, get_onedrive_path, is_cell_none, open_file_in_default_app

from shared_yg1 import YG1_Parsers

IN_PATH = "yg1_not_present.xlsx"
OUT_PATH = "turning_holders/yg1_not_present_Universal_Drill_HoldersTR.xlsx"

GROUP = "Indexable Drilling"
LINE = "Universal Holder (TR)"

_RE_COMBINE_WHITESPACE = re.compile(r"\s+")
_RE_STRIP_WHITESPACE = re.compile(r"(?a:^\s+|\s+$)")

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


class CtdBaseModel(BaseModel):
    model: str
    manuf: str = "YG1"


class CtdModelDie2(CtdBaseModel):
    constr: str = "ctd_the"
    serie: str | None = None
    platform: str | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    dc: float | None = None
    radismax: float | None = None
    dcx: float | None = None
    dcn: float | None = None
    centin: str | None = None
    peripin: str | None = None
    cutintmaster: str | None = None
    ifs: int | None = None
    tcha: float | None = None
    tchal: float | None = None
    tchau: float | None = None
    lu: float | None = None
    uldr: float | None = None
    adintms: str | None = None
    adintws: str | None = None
    cedc: int | None = None
    cnsc: int | None = None
    cxsc: int | None = None
    cst: str | None = None
    cp: float | None = None
    dcon: float | None = None
    kapr: float | None = None
    pl: float | None = None
    oal: float | None = None
    lf: float | None = None
    ifs: float | None = None
    lsh: float | None = None
    lb1: float | None = None
    rpmx: float | None = None
    wt: float | None = None
    sep: bool | None = False


class CtdModelArbBusheccenter(CtdBaseModel):
    serie: str | None = None
    oal: float | None = None
    lsh: float | None = None
    l2: float | None = None
    df: float | None = None
    adjcentupp: float | None = None
    adjcentlow: float | None = None
    adjradupp: float | None = None
    adjradlow: float | None = None
    multiplatform: str | None = None
    adintws: str | None = None
    adintms: str | None = None
    plvendor: str | None = None
    dcon: float | None = None
    dconws: float | None = None
    bsg: str | None = None
    wt: float | None = None
    sep: bool | None = False


class ParserBase(ABC):
    _model: str

    def __init__(self, model: str):
        self._model = model

    @abstractmethod
    def can_parse(self) -> bool:
        pass

    @abstractmethod
    def parse(self) -> CtdBaseModel:
        pass


class ParserDie2(ParserBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"YG(\w\w)(\d)-(\d+\.?\d*)d(\d\d)F(\d\d\d)-(\d\d)", self._model)

        return self._regex_res is not None

    def parse(self) -> CtdModelDie2:
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelDie2 model")

        return CtdModelDie2(
            model=self._regex_res.group(0),
            plvendor="YG1",
            dc=float(self._regex_res.group(3)),
            lu=float(self._regex_res.group(5)),
            adintms=f"ISO_9766:{self._regex_res.group(4)}",
            cnsc=4,
            cxsc=4,
            cst="HB",
            dcon=float(self._regex_res.group(4)),
            sep=False,
        )


class ParserArbBusheccenter(ParserBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"YGE-(\d\d)(\d\d)", self._model)

        return self._regex_res is not None

    def parse(self) -> CtdModelArbBusheccenter:
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelArbBusheccenter model")

        return CtdModelArbBusheccenter(model=self._regex_res.group(0),
                                                                                                                        # TODO: stoped here
                                       )


group = GROUP
line = LINE
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

models_list: list[str] = []
ctd_items: list[CtdBaseModel] = []

ctd_parsers = [
    ParserDie2,
]

for index, row in df.iterrows():
    if (isinstance(index, int) and index % 100 == 0):
        print(f"EDP №: {row['EDP №']}")

    model = row["model"]
    if is_cell_none(model):
        model_reg = re.search(r"\s?([A-Z].*)", row["Описание"])
        if model_reg is None:
            print("Error parsing:", row["Описание"])
            exit(1)
        model = model_reg.group(1)

    model = _RE_COMBINE_WHITESPACE.sub(" ", model)
    model = _RE_STRIP_WHITESPACE.sub("", model)

    if model in [
            "YGAV-16-25", "YGAV-32-60", "SER 2020K 16C", "SIR S10H 11", "SCFCR 10CA 09", "STFCR 10CA 11",
            "YGD2.25-12XC06"
    ]:
        continue

    if model == "Universal":
        print("Model is 'Universal':", model)
        continue
    if "С" in model or "M" in model or "T" in model or "В" in model:
        print("Model contains 'С', 'Т', 'В' or 'М':", model)
        model = model.replace("С", "C").replace("М", "M").replace("Т", "T").replace("В", "B")
    if model in models_list:
        print("Duplicate model:", model)
        continue

    models_list.append(model)

    is_parsed = False
    for parser in ctd_parsers:
        p = parser(model)
        if p.can_parse():
            ctd_items.append(p.parse())
            is_parsed = True
            break

    if not is_parsed:
        print("Error parsing model (no valid parser):", model)
        exit(1)

# new_df = pd.DataFrame(dl)
# new_df.insert(1, "manuf", "YG1")
# new_df.insert(2, "plvendor", "ISO")
# new_df.insert(len(new_df.columns), "constr", "ctd_inserts")
# new_df.insert(len(new_df.columns), "adintws", "WORKPIECE")

new_df = pd.DataFrame([x.model_dump() for x in ctd_items])

out_path = os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/out_tmp", OUT_PATH)

new_df.to_excel(out_path, index=False)

open_file_in_default_app(out_path)

print(new_df)
