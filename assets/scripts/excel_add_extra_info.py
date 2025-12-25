import json
import os
import traceback
from pathlib import Path

import numpy as np
import pandas as pd
import pydantic
from base import ArgsBase, parse_args_new, print_to_cpp
from pydantic import BaseModel

IMGS_FOLDER_SUFFIX_PRIORITY = ["_no_bg_webp_crop", "_webp_crop", "_no_bg_webp", "_no_bg", ""]
IMG_EXTENSIONS = ['.png', '.jpg', '.jpeg', '.webp', '.gif', '.bmp']


class Args(ArgsBase):
    xlsx_path: str = pydantic.Field(description="Папка исходных файлов")
    save_path: str = pydantic.Field(description="Папка для сохранения")
    rules_path: str = pydantic.Field(description="Путь к файлу с правилами")
    per_page_img_folder: str = pydantic.Field(description="Папка с картинками по странице")
    per_page_rule_img_folder: str = pydantic.Field(description="Папка с картинками по правилам")
    extra_parser_type: str = pydantic.Field(default="", description="Выбор дополнительного обработчика полей")


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
    constr_rename_list: dict[str, str] | None = None                                                                    # ConstrRenameList
    per_page_calc_list: list[PerPageCalcListObject] | None = None
    per_page_simple_rule_img_list: list[PerPageSimpleRuleImgListObject] | None = None
    per_constr_calc_list: dict[str, str] | None = None                                                                  # PerConstrCalcList


def get_simple_add_list(index: int, simple_add_list_values: list[SimpleAddListValueObject]):
    for val_obj in simple_add_list_values:
        is_found = index == val_obj.index if isinstance(val_obj.index, int) else index in val_obj.index
        if is_found:
            return val_obj.value

    return None


def get_simple_rename_list(index: int, simple_rename_list: list[SimpleRenameListObject]):
    result: dict[str, str] = {}

    for rename_list_obj in simple_rename_list:
        if index in rename_list_obj.index:
            result[rename_list_obj.src_name] = rename_list_obj.dst_name

    return result


def get_per_page_calc_list_result(index: int, df: pd.DataFrame, per_page_calc_list_item: PerPageCalcListObject):

    result: list[None | int | float | str] = []
    for val_obj in per_page_calc_list_item.values:
        if index in val_obj.index:
            for i in df.index:
                exec(val_obj.exec, {"df": df, "i": i, "result": result})                                                # pylint: disable=exec-used
            return result

    return None


def get_per_page_simple_rule_img_list_result(index: int, df: pd.DataFrame,
                                             per_page_simple_rule_img_list_item: PerPageSimpleRuleImgListObject):

    result: list[None | str] = []
    for val_obj in per_page_simple_rule_img_list_item.values:
        if index in val_obj.index:
            for i in df.index:
                res = None
                for rule in val_obj.list:
                    if df[per_page_simple_rule_img_list_item.name][i] == rule.cmp_value:
                        res = rule.img_filename_hash
                        break
                result.append(res)
            return result

    return None


# TODO: Create per field check (like check by serie)


def get_img_with_prefix(img_full_path: str, extra_parser_type: str):
    if extra_parser_type is None:
        return img_full_path.replace("\\", "/")

    if extra_parser_type == "yg1-shop":
        return os.path.join("http://194.113.153.157/nameduploads/",
                            Path(img_full_path).parent.parent.parent.parent.name,
                            Path(img_full_path).parent.name,
                            Path(img_full_path).name).replace("\\", "/")

    return img_full_path.replace("\\", "/")


def get_col_index(df: pd.DataFrame, col: str) -> int:
    loc = df.columns.get_loc(col)
    if isinstance(loc, (int, np.integer)):
        return int(loc)
    if isinstance(loc, slice):
        return int(loc.start)
    if isinstance(loc, (list, np.ndarray)):
        return int(loc[0])

    raise TypeError(f"Неизвестный тип результата: {type(loc)}")


def handle_glob_add_list(df: pd.DataFrame, page_id: int, global_add_list: list[GlobalAddListObject]):
    for glob_add_list_obj in global_add_list:
        to_add_value = glob_add_list_obj.value
        if to_add_value is None:
            print_to_cpp(f"[WARN]: Для столбца '{glob_add_list_obj.name}' на странице '{page_id}' не найдено значение")
            continue

        pos: None | int = None
        if glob_add_list_obj.name in df.columns:
            # print_to_cpp("[WARN]:", f"Столбец '{glob_add_list_obj.name}' уже есть на странице '{page_id}'")
            pos = get_col_index(df, glob_add_list_obj.name)
            df.drop(glob_add_list_obj.name, axis=1, inplace=True)

        if pos is None:
            pos = glob_add_list_obj.pos if glob_add_list_obj.pos is not None else len(df.columns)
        df.insert(pos, glob_add_list_obj.name, to_add_value)


def handle_simple_add_list(df: pd.DataFrame, page_id: int, simple_add_list: list[SimpleAddListObject]):
    for simple_add_list_obj in simple_add_list:
        to_add_value = get_simple_add_list(page_id, simple_add_list_obj.values)
        if to_add_value is None:
            # if not simple_add_list_obj.ignore_warns:
            #     print_to_cpp(
            #         f"[WARN]: Для столбца '{simple_add_list_obj.name}' на странице '{page_id}' не найдено значение"
            #     )
            continue
        pos: None | int = None
        if simple_add_list_obj.name in df.columns:
            # print_to_cpp("[WARN]:", f"Столбец '{simple_add_list_obj.name}' уже есть на странице '{page_id}'")
            pos = get_col_index(df, simple_add_list_obj.name)
            df.drop(simple_add_list_obj.name, axis=1, inplace=True)

        if pos is None:
            pos = simple_add_list_obj.pos if simple_add_list_obj.pos is not None else len(df.columns)
        df.insert(pos, simple_add_list_obj.name, to_add_value)


def handle_per_page_calc_list(df: pd.DataFrame, page_id: int, per_page_calc_list: list[PerPageCalcListObject] | None):
    if per_page_calc_list is None:
        return

    for per_page_calc_list_obj in per_page_calc_list:
        calc_to_add_value = get_per_page_calc_list_result(page_id, df, per_page_calc_list_obj)

        if calc_to_add_value is not None:
            if not (per_page_calc_list_obj.name in df.columns):
                # print_to_cpp("[WARN]:", f"Столбец '{per_page_calc_list_obj.name}' уже есть на странице '{page_id}'")
                # df.drop(per_page_calc_list_obj.name, axis=1, inplace=True)
                df.insert(len(df.columns), per_page_calc_list_obj.name, None)
            for i in df.index:
                if calc_to_add_value[i] is not None:
                    df.at[i, per_page_calc_list_obj.name] = calc_to_add_value[i]


def find_img_file_with_any_extension(base_path: str, base_name: str) -> str | None:
    for ext in IMG_EXTENSIONS:
        candidate = os.path.join(base_path, f"{base_name}{ext}")
        if os.path.exists(candidate):
            return candidate
    return None


def handle_per_page_img_folder(df: pd.DataFrame, xlsx_path_name: str, per_page_img_folder: str, extra_parser_type: str):
    img_pic = find_img_file_with_any_extension(per_page_img_folder, f"{Path(xlsx_path_name).stem}_pic")
    img_drw = find_img_file_with_any_extension(per_page_img_folder, f"{Path(xlsx_path_name).stem}_drw")

    df["img_pic"] = get_img_with_prefix(img_pic, extra_parser_type) if img_pic is not None else None
    df["img_drw"] = get_img_with_prefix(img_drw, extra_parser_type) if img_drw is not None else None


def handle_per_page_simple_rule_img_list(df: pd.DataFrame, page_id: int, per_page_rule_img_folder: str,
                                         extra_parser_type: str,
                                         per_page_simple_rule_img_list: list[PerPageSimpleRuleImgListObject] | None):
    if per_page_simple_rule_img_list is None:
        return

    for per_page_simple_rule_img_list_object in per_page_simple_rule_img_list:
        calc_to_add_value = get_per_page_simple_rule_img_list_result(page_id, df, per_page_simple_rule_img_list_object)

        if calc_to_add_value is not None:
            for i in df.index:
                if calc_to_add_value[i] is not None:
                    img_pic = find_img_file_with_any_extension(per_page_rule_img_folder, f"{calc_to_add_value[i]}_pic")
                    img_drw = find_img_file_with_any_extension(per_page_rule_img_folder, f"{calc_to_add_value[i]}_drw")
                    if img_pic is not None:
                        df.at[i, "img_pic"] = get_img_with_prefix(img_pic, extra_parser_type)
                    if img_drw is not None:
                        df.at[i, "img_drw"] = get_img_with_prefix(img_drw, extra_parser_type)


def select_img_folder_with_suffix(base_folder: str) -> str:
    for suffix in IMGS_FOLDER_SUFFIX_PRIORITY:
        candidate_folder = (base_folder.removesuffix("/") if base_folder.endswith("/") else base_folder) + suffix
        if os.path.exists(candidate_folder) and os.path.isdir(candidate_folder):
            return candidate_folder

    return base_folder


# NOTE: rows_count = len(df.index)
# NOTE: cols_count = len(df.columns)
# NOTE: cols_names = list(df.columns)
def add_extra_info_single(xlsx_path: os.DirEntry[str], save_path: str, extra_info_rules: ExtraInfoRules,
                          per_page_img_folder: str, per_page_rule_img_folder: str, extra_parser_type: str):
    print_to_cpp(xlsx_path.name)
    page_id = int(xlsx_path.name.split('.')[0].split("_")[0])

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")

    handle_glob_add_list(df, page_id, extra_info_rules.global_add_list)
    handle_simple_add_list(df, page_id, extra_info_rules.simple_add_list)

    # NOTE: Simple rename list
    # df.rename(columns=get_simple_rename_list(page_id, extra_info_rules.simple_rename_list), inplace=True)

    handle_per_page_calc_list(df, page_id, extra_info_rules.per_page_calc_list)

    # ? TODO: Handle per_constr_calc_list

    handle_per_page_img_folder(df, xlsx_path.name, per_page_img_folder, extra_parser_type)
    handle_per_page_simple_rule_img_list(df, page_id, per_page_rule_img_folder, extra_parser_type,
                                         extra_info_rules.per_page_simple_rule_img_list)

    # !TODO: Add remove list

    writer = pd.ExcelWriter(os.path.join(save_path, xlsx_path.name), engine="xlsxwriter")                               # pylint: disable=abstract-class-instantiated
    df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 0), index=False)
    worksheet = writer.sheets["sm"]
    worksheet.autofit()
    writer.close()


def add_extra_info(args: Args):
    extra_info_rules: ExtraInfoRules | None = None
    with open(args.rules_path, encoding="utf-8") as conf_file:
        extra_info_rules = ExtraInfoRules(**json.load(conf_file))

    if extra_info_rules == None:
        print_to_cpp(f"[ERROR] Не удалось открыть файл с правилами: '{args.rules_path}'")
        return

    per_page_img_folder = select_img_folder_with_suffix(args.per_page_img_folder)
    per_page_rule_img_folder = select_img_folder_with_suffix(args.per_page_rule_img_folder)

    os.makedirs(args.save_path, exist_ok=True)
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            # print_to_cpp(f"{filename.path} {filename.name}")
            if (filename.name.startswith("~$")): continue
            add_extra_info_single(filename, args.save_path, extra_info_rules, per_page_img_folder,
                                  per_page_rule_img_folder, args.extra_parser_type)

    print_to_cpp("Все файлы обработаны успешно!")


try:
    add_extra_info(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:
    # exc_type, exc_value, exc_traceback = sys.exc_info()
    formatted_traceback = traceback.format_exc()
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")

# * python add_extra_info_drill.py C:/Coding/works/wbi/amati_drill/xlsx_fix_values/ C:/Coding/works/wbi/amati_drill/xlsx_add_info/

# python excel_add_extra_info.py W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\startup `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\xlsx_add_info `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\extra_info.json `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\img_raw `
# W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\img_simple
