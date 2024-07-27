import json
import os
import math
from typing import Any, Callable

import pandas as pd
from pandas import Series
from pydantic import BaseModel

from shared import ArgsBase, flatten, frange, parse_args

from thread_params import thread_extra_list, thread_coarse_list, thread_fine_list


class Args(ArgsBase):
    xlsx_path: str
    save_path: str
    rules_path: str


class SimpleAddListValueObject(BaseModel):
    index: list[int]
    value: int | float | str | bool


class SimpleAddListObject(BaseModel):
    name: str
    values: list[SimpleAddListValueObject]
    ignore_warns: bool = False
    pos: int | None = None


class SimpleRenameListObject(BaseModel):
    index: list[int]
    src_name: str
    dst_name: str


class PerPageCalcListObject(BaseModel):
    name: str
    index: list[int]
    exec: str


class ExtraInfoRules(BaseModel):
    simple_add_list: list[SimpleAddListObject]
    simple_rename_list: list[SimpleRenameListObject]
    constr_rename_list: dict[str, str]                                                                                  # ConstrRenameList
    per_page_calc_list: list[PerPageCalcListObject]
    per_constr_calc_list: dict[str, str]                                                                                #PerConstrCalcList


def get_simple_add_list(index: int, simple_add_list_values: list[SimpleAddListValueObject]):
    for val_obj in simple_add_list_values:
        if index in val_obj.index:
            return val_obj.value

    return None


def get_simple_rename_list(index: int, simple_rename_list: list[SimpleRenameListObject]):
    result: dict[str, str] = {}

    for rename_list_obj in simple_rename_list:
        if index in rename_list_obj.index:
            result[rename_list_obj.src_name] = rename_list_obj.dst_name

    return result


def get_per_page_calc_list_item(index: int, df: pd.DataFrame, per_page_calc_list_item: PerPageCalcListObject):
    if index in per_page_calc_list_item.index:
        result: list[None | int | float | str] = []

        for i in range(len(df.index)):
            exec(per_page_calc_list_item.exec, {"df": df, "i": i, "result": result})

        return result

    return None


# NOTE: rows_count = len(df.index)
# NOTE: cols_count = len(df.columns)
# NOTE: cols_names = list(df.columns)
def add_extra_info_single(xlsx_path: os.DirEntry[str], save_path: str, extra_info_rules: ExtraInfoRules):
    print(xlsx_path)
    page_id = int(xlsx_path.name.split('.')[0])

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")

    # * NOTE: Simple add List
    for add_list_obj in extra_info_rules.simple_add_list:
        to_add_value = get_simple_add_list(page_id, add_list_obj.values)
        if to_add_value is None:
            if not add_list_obj.ignore_warns:
                print("[WARN]:", f"Для столбца '{add_list_obj.name}' на странице '{page_id}' не найдено значение")
            continue
        if add_list_obj.name in df:
            print("[WARN]:", f"Столбец '{add_list_obj.name}' уже есть на странице '{page_id}'")
            continue

        pos = add_list_obj.pos if add_list_obj.pos is not None else len(df.columns)

        df.insert(pos, add_list_obj.name, to_add_value)

    # * NOTE: Simple ename list
    df.rename(columns=get_simple_rename_list(page_id, extra_info_rules.simple_rename_list), inplace=True)

    # * Per page calc list
    for per_page_calc_object in extra_info_rules.per_page_calc_list:
        to_add_value = get_per_page_calc_list_item(page_id, df, per_page_calc_object)

        if to_add_value is not None:
            df.insert(len(df.columns), per_page_calc_object.name, to_add_value)

    # !TODO: Add remove list

    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name), engine="xlsxwriter")                               # pylint: disable=abstract-class-instantiated
    df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 1), index=False)
    worksheet = writer.sheets["sm"]
    worksheet.autofit()
    writer.close()


def add_extra_info(args: Args):
    extra_info_rules: ExtraInfoRules | None = None
    with open(args.rules_path, encoding="utf-8") as conf_file:
        extra_info_rules = ExtraInfoRules(**json.load(conf_file))

    if extra_info_rules == None:
        print(f"[ERROR] Не удалось открыть файл с правилами: '{args.rules_path}'")
        return

    os.makedirs(args.save_path, exist_ok=True)
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            print(filename.path, filename.name)
            if (filename.name.startswith("~$")): continue
            add_extra_info_single(filename, args.save_path, extra_info_rules)


add_extra_info(parse_args(Args))

# * python add_extra_info_drill.py C:/Coding/works/wbi/amati_drill/xlsx_fix_values/ C:/Coding/works/wbi/amati_drill/xlsx_add_info/
