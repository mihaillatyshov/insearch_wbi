from abc import ABC, abstractmethod
import os
import re
from typing import TypedDict

from pydantic import BaseModel
import pandas as pd

from shared import df_select_group, get_onedrive_path, is_cell_none, open_file_in_default_app

from shared_yg1 import YG1_Parsers

IN_PATH = "yg1_not_present.xlsx"
OUT_PATH = "turning_holders/yg1_not_present_Universal_Turning_HoldersTR.xlsx"

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


class CtdModelThe(CtdBaseModel):
    constr: str = "ctd_the"
    serie: str | None = None
    family: str | None = None
    clampdirection: str | None = None
    clfam: str | None = None
    kapr: float | None = None
    psir: float | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    cutintmaster: str | None = None
    adintms: str | None = None
    adintws: str | None = None
    rmpx: float | None = None
    baws: float | None = None
    bams: float | None = None
    basement: str | None = None
    cutdir: str | None = None
    hand: str | None = None
    swma: bool | None = None
    ifs: int | None = None
    ohx: float | None = None
    dpc: bool | None = None
    cnsc: int | None = None
    cxsc: int | None = None
    b: float | None = None
    h: float | None = None
    wf: float | None = None
    lf: float | None = None
    l2: float | None = None
    hf: float | None = None
    allowswitchinsert: None = None
    gamo: float | None = None
    lams: float | None = None
    b2: float | None = None
    h2: float | None = None
    h3: float | None = None
    bmc: str | None = None
    wt: float | None = None
    sep: bool | None = None


class CtdModelThi(CtdBaseModel):
    constr: str = "ctd_thi"
    serie: str | None = None
    family: str | None = None
    clampdirection: str | None = None
    clfam: str | None = None
    kapr: float | None = None
    psir: float | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    cutintmaster: str | None = None
    adintms: str | None = None
    adintws: str | None = None
    rmpx: float | None = None
    dmin: float | None = None
    dcon: float | None = None
    baws: float | None = None
    bams: float | None = None
    basement: str | None = None
    cutdir: str | None = None
    hand: str | None = None
    ifs: int | None = None
    ohx: float | None = None
    dpc: bool | None = None
    cnsc: int | None = None
    cxsc: int | None = None
    cp: float | None = None
    hpcool: bool | None = None
    b: float | None = None
    h: float | None = None
    wf: float | None = None
    lf: float | None = None
    lu: float | None = None
    hf: float | None = None
    gamo: float | None = None
    lams: float | None = None
    bd2: float | None = None
    bmc: str | None = None
    mgro: str | None = None
    wt: float | None = None
    sep: bool | None = None


class CtdModelTavsArbor(CtdBaseModel):
    constr: str = "ctd_tavs_arbor"
    serie: str | None = None
    dcon: float | None = None
    bd: float | None = None
    bd1: float | None = None
    bdx: float | None = None
    lu: float | None = None
    lbx: float | None = None
    lf: float | None = None
    lpr: float | None = None
    oal: float | None = None
    cnsc: int | None = None
    cxsc: int | None = None
    dpc: bool | None = None
    bmc: str | None = None
    thid: str | None = None
    adintms: str | None = None
    adintws: str | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    bsg: str | None = None
    rpmx: float | None = None
    wt: float | None = None
    sep: bool | None = None


class CtdModelTca(CtdBaseModel):
    constr: str = "ctd_tca"
    family: str | None = None
    clampdirection: str | None = None
    clfam: str | None = None
    kapr: float | None = None
    psir: float | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    cutintmaster: str | None = None
    adintms: str | None = None
    adintws: str | None = None
    rmpx: float | None = None
    basement: str | None = None
    cutdir: str | None = None
    hand: str | None = None
    ifs: int | None = None
    cnsc: int | None = None
    cxsc: int | None = None
    dmin: float | None = None
    wf3: float | None = None
    wf: float | None = None
    lf: float | None = None
    sc: str | None = None
    oal: float | None = None
    hf: float | None = None
    cict: int | None = None
    allowswitchinsert: bool | None = None
    gamo: float | None = None
    lams: float | None = None
    wt: float | None = None
    sep: bool | None = None


class CtdModelTsmbTurn(CtdBaseModel):
    constr: str = "ctd_tsmb_turn"
    serie: str | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    app1: str | None = None
    app2: str | None = None
    dmin: float | None = None
    dcon: float | None = None
    dn: float | None = None
    wb: float | None = None
    wf: float | None = None
    wf2: float | None = None
    wf3: float | None = None
    lu: float | None = None
    lf: float | None = None
    apmx: float | None = None
    re: float | None = None
    hand: str | None = None
    ohx: float | None = None
    ohn: float | None = None
    h: float | None = None
    psir: float | None = None
    kapr: float | None = None
    bapr: float | None = None
    rpmx: float | None = None
    adintms: str | None = None
    adintws: str | None = None
    grade: str | None = None
    coating: str | None = None
    substrate: str | None = None
    mgro: str | None = None
    bsg: str | None = None
    cnsc: int | None = None
    cxsc: int | None = None
    wt: float | None = None
    sep: bool | None = None


class CtdModelTsmbFaceChamfer(CtdBaseModel):
    constr: str = "ctd_tsmb_face_chamfer"
    serie: str | None = None
    axgsup: int | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    app1: str | None = None
    app2: str | None = None
    dcon: float | None = None
    dn: float | None = None
    wb: float | None = None
    dmin: float | None = None
    wf: float | None = None
    wf3: float | None = None
    cdx: float | None = None
    lu: float | None = None
    lf: float | None = None
    lpr: float | None = None
    re: float | None = None
    hand: str | None = None
    ohx: float | None = None
    ohn: float | None = None
    h: float | None = None
    psir: float | None = None
    kapr: float | None = None
    adintms: str | None = None
    adintws: str | None = None
    grade: str | None = None
    coating: str | None = None
    substrate: str | None = None
    mgro: str | None = None
    bsg: str | None = None
    cnsc: int | None = None
    wt: float | None = None
    sep: bool | None = None


class CtdModelTsmbFaceGroove(CtdBaseModel):
    constr: str = "ctd_tsmb_face_groove"
    serie: str | None = None
    app1: str | None = None
    app2: str | None = None
    dmin: float | None = None
    dcon: float | None = None
    daxn: float | None = None
    daxx: float | None = None
    surfacepropose: str | None = None
    axgsup: int | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    dn: float | None = None
    wb: float | None = None
    wf: float | None = None
    wf3: float | None = None
    lu: float | None = None
    lf: float | None = None
    cw: float | None = None
    cdx: float | None = None
    re: float | None = None
    hand: str | None = None
    ohx: float | None = None
    ohn: float | None = None
    h: float | None = None
    rar: float | None = None
    ral: float | None = None
    adintms: str | None = None
    adintws: str | None = None
    grade: str | None = None
    coating: str | None = None
    substrate: str | None = None
    mgro: str | None = None
    bsg: str | None = None
    cnsc: int | None = None
    cxsc: int | None = None
    wt: float | None = None
    sep: bool | None = None


class CtdModelTsmbBush(CtdBaseModel):
    constr: str = "ctd_tsmb_bush"
    serie: str | None = None
    dconms: float | None = None
    dconws: float | None = None
    db: float | None = None
    bd1: float | None = None
    bd2: float | None = None
    dn: float | None = None
    lb1: float | None = None
    lb2: float | None = None
    h: float | None = None
    b: float | None = None
    lsh: float | None = None
    oaw: float | None = None
    oah: float | None = None
    oal: float | None = None
    cnsc: int | None = None
    cp: float | None = None
    hpcool: bool | None = None
    thid: None = None
    adintms: str | None = None
    adintws: str | None = None
    multiplatform: str | None = None
    plvendor: str | None = None
    dpc: bool | None = None
    bsg: str | None = None
    wt: float | None = None
    sep: bool | None = None


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


class ParserThBase(ParserBase):
    def _symb_to_lf(self, symb: str) -> float | None:
        return {
            "E": 70,
            "F": 80,
            "H": 100,
            "K": 125,
            "M": 150,
            "P": 170,
            "Q": 180,
            "R": 200,
            "S": 250,
            "T": 300,
            "U": 350,
            "V": 400,
        }.get(symb)

    def _symb_to_clfam(self, symb: str) -> str | None:
        return {
            "C": "C",
            "M": "M",
            "P": "P",
            "S": "S",
            "D": "D",
            "T": "D",
            "A": "D",
        }.get(symb)

    def _symb_to_kapr(self, symb: str) -> float | None:
        return {
            "D": 45,
            "S": 45,
            "T": 60,
            "N": 62.5,
            "V": 72.5,
            "B": 75,
            "K": 75,
            "A": 90,
            "G": 91,
            "F": 91,
            "J": 93,
            "U": 93,
            "L": 95,
            "H": 107.5,
            "Q": 107.5,
        }.get(symb)

    def _symb_to_psir(self, symb: str) -> float | None:
        kapr = self._symb_to_kapr(symb)
        if kapr is None:
            return None

        return 90 - kapr


class ParserThi(ParserThBase):
    _regex_res: (re.Match[str] | None)

    def _symb_to_mgro(self, symb: str) -> str | None:
        if symb in ["A", "S"]:
            return "STL"
        if symb in ["E", "C"]:
            return "VHM"

        return None

    def _symb_to_bmc(self, symb: str) -> str | None:
        if symb in ["A", "S"]:
            return "Steel"
        if symb in ["E", "C"]:
            return "Carbide"

        return None

    def _symb_to_cnsc(self, symb: str) -> int | None:
        if symb in ["A", "E"]:
            return 1
        if symb in ["S", "C"]:
            return 0

        return None

    def _symb_to_cxsc(self, symb: str) -> int | None:
        if symb in ["A", "E"]:
            return 3
        if symb in ["S", "C"]:
            return 0

        return None

    def _raw_dcon_to_dcon(self, symb: str) -> float:
        return float(symb)

    def _raw_dcon_to_adintms(self, symb: str) -> str:
        return f"ISO_5609:{symb}"

    def _get_basement(self) -> str | None:
        if self._regex_res is None:
            return None
        return f"{self._regex_res.group(2)}{self._regex_res.group(3)}{self._regex_res.group(5)}{self._regex_res.group(6)}{self._regex_res.group(7)}{self._regex_res.group(8)}{self._regex_res.group(9)}"

    def _get_family(self) -> str | None:
        if self._regex_res is None:
            return None
        return f"{self._regex_res.group(4)}{self._regex_res.group(5)}{self._regex_res.group(6)}{self._regex_res.group(7)}{self._regex_res.group(8)}"

    def can_parse(self) -> bool:
        self._regex_res = re.search(
            r"([AESC])(\d\d)(?:\d\d)?([EFHKMPQRSTUVC])(?:\s|-)(\w)(\w)(\w)(\w)([RLN])(?:\s|-)?(\d\d)(C)?", self._model)
        return self._regex_res is not None

    def parse(self) -> CtdModelThi:
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelThi model")

        return CtdModelThi(
            model=self._regex_res.group(0),
            family=self._get_family(),
            clampdirection="N",
            clfam=self._symb_to_clfam(self._regex_res.group(4)),
            kapr=self._symb_to_kapr(self._regex_res.group(6)),
            psir=self._symb_to_psir(self._regex_res.group(6)),
            plvendor="ISO",
            adintms=self._raw_dcon_to_adintms(self._regex_res.group(2)),
            dcon=self._raw_dcon_to_dcon(self._regex_res.group(2)),
            basement=self._get_basement(),
            hand=self._regex_res.group(8),
            cnsc=self._symb_to_cnsc(self._regex_res.group(1)),
            cxsc=self._symb_to_cxsc(self._regex_res.group(1)),
            lf=self._symb_to_lf(self._regex_res.group(3)),
            bmc=self._symb_to_bmc(self._regex_res.group(1)),
            mgro=self._symb_to_mgro(self._regex_res.group(1)),
            sep=False,
        )


class ParserThe(ParserThBase):
    _regex_res: (re.Match[str] | None)

    def _get_basement(self) -> str | None:
        if self._regex_res is None:
            return None
        return f"{self._regex_res.group(2)}{self._regex_res.group(3)}{self._regex_res.group(4)}{self._regex_res.group(5)}{self._regex_res.group(6)}{self._regex_res.group(7)}{self._regex_res.group(8)}{self._regex_res.group(9)}"

    def _get_family(self) -> str | None:
        if self._regex_res is None:
            return None
        return f"{self._regex_res.group(1)}{self._regex_res.group(2)}{self._regex_res.group(3)}{self._regex_res.group(4)}{self._regex_res.group(5)}"

    def _raw_h_and_b_to_adintms(self, raw_h: str, raw_b: str) -> str:
        return f"ISO_5610:{raw_h}X{raw_b}"

    def can_parse(self) -> bool:
        self._regex_res = re.search(r"(\w)(\w)(\w)(\w)(?:\s|-)?([RLN])(?:\s|-)?(\d\d)(\d\d)(?:\s|-)?(\w)(\d\d)(C)?",
                                    self._model)
        return self._regex_res is not None

    def parse(self) -> CtdModelThe:
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelThe model")

        return CtdModelThe(
            model=self._regex_res.group(0),
            family=self._get_family(),
            clampdirection="N",
            clfam=self._symb_to_clfam(self._regex_res.group(1)),
            kapr=self._symb_to_kapr(self._regex_res.group(3)),
            psir=self._symb_to_psir(self._regex_res.group(3)),
            plvendor="ISO",
            adintms=self._raw_h_and_b_to_adintms(self._regex_res.group(6), self._regex_res.group(7)),
            basement=self._get_basement(),
            hand=self._regex_res.group(5),
            cnsc=0,
            cxsc=0,
            h=float(self._regex_res.group(6)),
            b=float(self._regex_res.group(7)),
            lf=self._symb_to_lf(self._regex_res.group(8)),
            bmc="Steel",
            sep=False,
        )


class ParserTavsArbor(ParserBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"(?:YGAV)-D(\d\d)-(\d\d\d?)-(\d\d?)D-H", self._model)

        return self._regex_res is not None

    def parse(self) -> CtdModelTavsArbor:
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelCtdModelTavsArbor model")

        return CtdModelTavsArbor(
            model=self._regex_res.group(0),
            multiplatform="YG1",
            plvendor="YG1",
            adintms=f"HA-{self._regex_res.group(1)}",
            adintws=f"YG1_YGAV:{min(int(self._regex_res.group(1)), 40)}",
            cnsc=1,
            cxsc=1,
            dcon=float(self._regex_res.group(1)),
            bd=float(self._regex_res.group(1)),
            oal=float(self._regex_res.group(2)),
            sep=False,
        )


class ParserTca(ParserThBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"(?:YGAV)-A(\d\d)(?:\s|-)(\w)(\w)(\w)(\w)(\w)(?:\s|-)(\d\d)", self._model)

        return self._regex_res is not None

    def parse(self) -> CtdModelTca:
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelTca model")

        return CtdModelTca(
            model=self._regex_res.group(0),
            family=
            f"{self._regex_res.group(2)}{self._regex_res.group(3)}{self._regex_res.group(4)}{self._regex_res.group(5)}",
            clampdirection="N",
            clfam=self._symb_to_clfam(self._regex_res.group(2)),
            plvendor="YG1",
            adintms=f"YG1_YGAV:{self._regex_res.group(1)}",
            basement=
            f"{self._regex_res.group(2)}{self._regex_res.group(3)}{self._regex_res.group(4)}{self._regex_res.group(5)}{self._regex_res.group(7)}",
            hand=self._regex_res.group(6),
            cnsc=1,
            cxsc=3,
            sc=self._regex_res.group(3),
            cict=1,
            sep=False,
        )


class ParserTsmbTurn(ParserThBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"NC(?:BP|BO|PR)(\d)([RLN])-(\d\d)\.(\d\d)-(\d\d\d)-(YG812)", self._model)

        return self._regex_res is not None

    def parse(self) -> CtdModelTsmbTurn:
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelTsmbTurn model")

        raw_re = self._regex_res.group(5)

        return CtdModelTsmbTurn(
            model=self._regex_res.group(0),
            serie="NanoCut BP",
            plvendor="YG1",
            app1=YG1_Parsers.grade_to_app1(self._regex_res.group(6)),
            dmin=float(self._regex_res.group(3)) / 10.0,
            dcon=float(self._regex_res.group(1)),
            lu=float(self._regex_res.group(4)),
            re=float(f"{raw_re[:1]}.{raw_re[1:]}"),
            hand=self._regex_res.group(2),
            kapr=98,
            psir=90 - 98,
            adintms=f"HA-{self._regex_res.group(1)}",
            adintws="WORKPIECE",
            grade=self._regex_res.group(6),
            coating=YG1_Parsers.grade_to_coating(self._regex_res.group(6)),
            mgro="VHM",
            cnsc=0,
            cxsc=0,
            sep=False,
        )


class ParserTsmbFaceChamfer(ParserThBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"NCCH(\d)([RLN])-(\d\d)\.(\d\d)-(\d\d)-(YG812)", self._model)

        return self._regex_res is not None

    def parse(self):
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelTsmbFaceChamfer model")

        return CtdModelTsmbFaceChamfer(
            model=self._regex_res.group(0),
            serie="NanoCut CH",
            axgsup=1,
            plvendor="YG1",
            app1=YG1_Parsers.grade_to_app1(self._regex_res.group(6)),
            dcon=float(self._regex_res.group(1)),
            dmin=float(self._regex_res.group(3)) / 10.0,
            lu=float(self._regex_res.group(4)),
            re=0.2,
            hand=self._regex_res.group(2),
            kapr=float(self._regex_res.group(5)),
            psir=90 - float(self._regex_res.group(5)),
            adintms=f"HA-{self._regex_res.group(1)}",
            adintws="WORKPIECE",
            grade=self._regex_res.group(6),
            coating=YG1_Parsers.grade_to_coating(self._regex_res.group(6)),
            mgro="VHM",
            cnsc=0,
            sep=False,
        )


class ParserTsmbFaceGroove(ParserThBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"NCF([IE])(\d)([RLN])-(\d\d)\.(\d\d)-(\d\d)x(\d\d)-(YG812)", self._model)

        return self._regex_res is not None

    def parse(self):
        if self._regex_res is None:
            raise Exception("Can't parse CtdModelTsmbFaceGroove model")

        return CtdModelTsmbFaceGroove(
            model=self._regex_res.group(0),
            serie=f"NanoCut F{self._regex_res.group(1)}",
            app1=YG1_Parsers.grade_to_app1(self._regex_res.group(8)),
            dmin=float(self._regex_res.group(4)) / 10.0,
            dcon=float(self._regex_res.group(2)),
            surfacepropose="EXTERNAL" if self._regex_res.group(1) == "E" else "INTERNAL",
            axgsup=2 if self._regex_res.group(1) == "E" else 1,
            plvendor="YG1",
            lu=float(self._regex_res.group(5)),
            re=0.1,
            cw=float(self._regex_res.group(6)),
            cdx=float(self._regex_res.group(7)),
            hand=self._regex_res.group(3),
            adintms=f"HA-{self._regex_res.group(2)}",
            adintws="WORKPIECE",
            grade=self._regex_res.group(8),
            coating=YG1_Parsers.grade_to_coating(self._regex_res.group(8)),
            mgro="VHM",
            cnsc=0,
            cxsc=0,
            sep=False,
        )


class ParserTsmbBush(ParserThBase):
    _regex_res: (re.Match[str] | None)

    def can_parse(self):
        self._regex_res = re.search(r"NCHI-(\d\d).(\d)", self._model)

        return self._regex_res is not None

    def parse(self):
        return CtdModelTsmbBush(
            model=self._regex_res.group(0),
            dconms=float(self._regex_res.group(1)),
            dconws=float(self._regex_res.group(2)),
            adintms=f"ISO_5609:{self._regex_res.group(1)}",
            adintws=f"HA-{self._regex_res.group(2)}",
            plvendor="YG1",
            sep=False,
        )


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

models_list: list[str] = []
ctd_items: list[CtdBaseModel] = []

ctd_parsers = [
    ParserThi,
    ParserThe,
    ParserTavsArbor,
    ParserTca,
    ParserTsmbTurn,
    ParserTsmbFaceChamfer,
    ParserTsmbFaceGroove,
    ParserTsmbBush,
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
