import json
import os
import sys
import traceback

import pandas as pd
from pydantic import BaseModel

from base import ArgsBase, parse_args, print_to_cpp


class Args(ArgsBase):
    xlsx_path: str
    save_path: str
    rules_path: str
    per_page_img_folder: str
    per_page_rule_img_folder: str


class GlobalAddListObject(BaseModel):
    name: str
    value: int | float | str | bool
    ignore_warns: bool = False
    pos: int | None = None


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


class PerPageCalcListValueObject(BaseModel):
    index: list[int]
    exec: str


class PerPageCalcListObject(BaseModel):
    name: str
    values: list[PerPageCalcListValueObject]


class PerPageSimpleRuleImgListValueItemObject(BaseModel):
    cmp: str
    cmp_value: str
    img_filename_hash: str


class PerPageSimpleRuleImgListValueObject(BaseModel):
    index: list[int]
    list: list[PerPageSimpleRuleImgListValueItemObject]


class PerPageSimpleRuleImgListObject(BaseModel):
    name: str
    values: list[PerPageSimpleRuleImgListValueObject]


class ExtraInfoRules(BaseModel):
    global_add_list: list[GlobalAddListObject]
    simple_add_list: list[SimpleAddListObject]
    simple_rename_list: list[SimpleRenameListObject] | None = None
    constr_rename_list: dict[str, str] | None = None  # ConstrRenameList
    per_page_calc_list: list[PerPageCalcListObject]
    per_page_simple_rule_img_list: list[PerPageSimpleRuleImgListObject]
    per_constr_calc_list: dict[str, str] | None = None  # PerConstrCalcList


def get_simple_add_list(
        index: int, simple_add_list_values: list[SimpleAddListValueObject]):
    for val_obj in simple_add_list_values:
        is_found = index == val_obj.index if isinstance(
            val_obj.index, int) else index in val_obj.index
        if is_found:
            return val_obj.value

    return None


def get_simple_rename_list(index: int,
                           simple_rename_list: list[SimpleRenameListObject]):
    result: dict[str, str] = {}

    for rename_list_obj in simple_rename_list:
        if index in rename_list_obj.index:
            result[rename_list_obj.src_name] = rename_list_obj.dst_name

    return result


def get_per_page_calc_list_result(
        index: int, df: pd.DataFrame,
        per_page_calc_list_item: PerPageCalcListObject):

    result: list[None | int | float | str] = []
    for val_obj in per_page_calc_list_item.values:
        if index in val_obj.index:
            for i in df.index:
                exec(val_obj.exec, {"df": df, "i": i, "result": result})
            return result

    return None


def get_per_page_simple_rule_img_list_result(
        index: int, df: pd.DataFrame,
        per_page_simple_rule_img_list_item: PerPageSimpleRuleImgListObject):

    result: list[None | str] = []
    for val_obj in per_page_simple_rule_img_list_item.values:
        if index in val_obj.index:
            for i in df.index:
                res = None
                for rule in val_obj.list:
                    if df[per_page_simple_rule_img_list_item.
                          name][i] == rule.cmp_value:
                        res = rule.img_filename_hash
                        break
                result.append(res)
            return result

    return None


# TODO: Create per field check (like check by serie)


# NOTE: rows_count = len(df.index)
# NOTE: cols_count = len(df.columns)
# NOTE: cols_names = list(df.columns)
def add_extra_info_single(xlsx_path: os.DirEntry[str], save_path: str,
                          extra_info_rules: ExtraInfoRules,
                          per_page_img_folder: str,
                          per_page_rule_img_folder: str):
    print_to_cpp(str(xlsx_path))
    page_id = int(xlsx_path.name.split('.')[0].split("_")[0])

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")

    # NOTE: Global add List
    for glob_add_list_obj in extra_info_rules.global_add_list:
        to_add_value = glob_add_list_obj.value
        if to_add_value is None:
            print_to_cpp(
                f"[WARN]: Для столбца '{glob_add_list_obj.name}' на странице '{page_id}' не найдено значение"
            )
            continue

        pos: None | int = None
        if glob_add_list_obj.name in df.columns:
            # print_to_cpp("[WARN]:", f"Столбец '{glob_add_list_obj.name}' уже есть на странице '{page_id}'")
            # TODO: add check for type and conver to int
            pos = df.columns.get_loc(glob_add_list_obj.name)
            df.drop(glob_add_list_obj.name, axis=1, inplace=True)

        if pos is None:
            pos = glob_add_list_obj.pos if glob_add_list_obj.pos is not None else len(
                df.columns)
        df.insert(pos, glob_add_list_obj.name, to_add_value)

    # NOTE: Simple add List
    for simp_add_list_obj in extra_info_rules.simple_add_list:
        to_add_value = get_simple_add_list(page_id, simp_add_list_obj.values)
        if to_add_value is None:
            # if not simp_add_list_obj.ignore_warns:
            #     print_to_cpp(
            #         f"[WARN]: Для столбца '{simp_add_list_obj.name}' на странице '{page_id}' не найдено значение"
            #     )
            continue
        pos: None | int = None
        if simp_add_list_obj.name in df.columns:
            # print_to_cpp("[WARN]:", f"Столбец '{simp_add_list_obj.name}' уже есть на странице '{page_id}'")
            pos = df.columns.get_loc(simp_add_list_obj.name)
            df.drop(simp_add_list_obj.name, axis=1, inplace=True)

        if pos is None:
            pos = simp_add_list_obj.pos if simp_add_list_obj.pos is not None else len(
                df.columns)
        df.insert(pos, simp_add_list_obj.name, to_add_value)

    # NOTE: Simple rename list
    # df.rename(columns=get_simple_rename_list(
    #     page_id, extra_info_rules.simple_rename_list),
    #           inplace=True)

    # NOTE: Per page calc list
    for per_page_calc_object in extra_info_rules.per_page_calc_list:
        calc_to_add_value = get_per_page_calc_list_result(
            page_id, df, per_page_calc_object)

        if calc_to_add_value is not None:
            pos: None | int = None
            if not (per_page_calc_object.name in df.columns):
                # print_to_cpp("[WARN]:", f"Столбец '{per_page_calc_object.name}' уже есть на странице '{page_id}'")
                # df.drop(per_page_calc_object.name, axis=1, inplace=True)
                df.insert(len(df.columns), per_page_calc_object.name, None)
            for i in df.index:
                if calc_to_add_value[i] is not None:
                    df.at[i, per_page_calc_object.name] = calc_to_add_value[i]

    # ? TODO: Handle per_constr_calc_list

    img_pic = os.path.join(per_page_img_folder, f"{xlsx_path.name}_pic.png")
    img_drw = os.path.join(per_page_img_folder, f"{xlsx_path.name}_drw.png")

    # NOTE: Handle imgs in folder

    df["img_pic"] = str(img_pic) if os.path.exists(img_pic) else None
    df["img_drw"] = str(img_drw) if os.path.exists(img_drw) else None

    # NOTE: Handle per_page_simple_rule_img_list
    for per_page_simple_rule_img_list_object in extra_info_rules.per_page_simple_rule_img_list:
        calc_to_add_value = get_per_page_simple_rule_img_list_result(
            page_id, df, per_page_simple_rule_img_list_object)

        if calc_to_add_value is not None:
            for i in df.index:
                if calc_to_add_value[i] is not None:
                    img_pic = os.path.join(per_page_rule_img_folder,
                                           f"{calc_to_add_value[i]}_pic.png")
                    img_drw = os.path.join(per_page_rule_img_folder,
                                           f"{calc_to_add_value[i]}_drw.png")
                    if os.path.exists(img_pic):
                        df.at[i, "img_pic"] = str(img_pic)
                    if os.path.exists(img_drw):
                        df.at[i, "img_drw"] = str(img_drw)

    # !TODO: Add remove list

    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name),
                            engine="xlsxwriter")  # pylint: disable=abstract-class-instantiated
    df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 1), index=False)
    worksheet = writer.sheets["sm"]
    worksheet.autofit()
    writer.close()


def add_extra_info(args: Args):
    extra_info_rules: ExtraInfoRules | None = None
    with open(args.rules_path, encoding="utf-8") as conf_file:
        extra_info_rules = ExtraInfoRules(**json.load(conf_file))

    if extra_info_rules == None:
        print_to_cpp(
            f"[ERROR] Не удалось открыть файл с правилами: '{args.rules_path}'"
        )
        return

    os.makedirs(args.save_path, exist_ok=True)
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            print_to_cpp(f"{filename.path} {filename.name}")
            if (filename.name.startswith("~$")): continue
            add_extra_info_single(filename, args.save_path, extra_info_rules,
                                  args.per_page_img_folder,
                                  args.per_page_rule_img_folder)


try:
    add_extra_info(parse_args(Args))
except KeyboardInterrupt:
    pass
except Exception as e:
    exc_type, exc_value, exc_traceback = sys.exc_info()
    formatted_traceback = traceback.format_exc()
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")

# * python add_extra_info_drill.py C:/Coding/works/wbi/amati_drill/xlsx_fix_values/ C:/Coding/works/wbi/amati_drill/xlsx_add_info/

# python excel_add_extra_info.py W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\startup `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\xlsx_add_info `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\extra_info.json `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\img_raw `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\img_simple
