import json
import math
import os
import sys
from typing import TypeVar
import subprocess, platform

import pandas as pd
from openpyxl.utils import get_column_letter
from pydantic import BaseModel as ArgsBase

TArgs = TypeVar("TArgs", bound=ArgsBase)


def parse_args(args_type: type[TArgs]) -> TArgs:
    if (len(sys.argv) - 1) < len(args_type.__fields__.keys()):
        print("Скрипт не выполнен (не хватает агрументов)")
        exit(-1)

    try:
        args_input = {}
        for i, key in enumerate(args_type.__fields__):
            args_input[key] = str(sys.argv[i + 1])
        return args_type(**args_input)
    except Exception:
        print("Аргументы скрипта имеют ошибки!")
        exit(-1)


def get_img_filename(page_id: int) -> str:
    return f'{page_id}.png'


def get_img_path(page_id: int) -> str:
    return f"./out/png/{get_img_filename(page_id)}"


def get_img_preview(page_id: int) -> str:
    return f"./out/png_preview/{page_id}.jpg"


def get_img_cv_path(page_id: int) -> str:
    return f"./out/png_cv/{get_img_filename(page_id)}"


def get_text_path(page_id: int) -> str:
    return f"./out/text/{page_id}.txt"


def load_config():
    config_file = open("./config.json")
    data = json.load(config_file)
    config_file.close()
    return data


def flatten(list_of_lists: list[list]) -> list:
    return [item for sublist in list_of_lists for item in sublist]


def frange(first: int, second: int) -> range:
    return range(first, second + 1)


def get_boundaries(page_id: int, multi: int = 1):
    config = load_config()
    skip_first = False
    skip_second = False

    start_x_first = config["first"]["startx"] // multi
    end_x_first = config["first"]["endx"] // multi
    start_y_first = config["first"]["starty"] // multi
    end_y_first = config["first"]["endy"] // multi

    start_x_second = config["second"]["startx"] // multi
    end_x_second = config["second"]["endx"] // multi
    start_y_second = config["second"]["starty"] // multi
    end_y_second = config["second"]["endy"] // multi

    if page := config.get(str(page_id)):
        if first := page.get("first"):
            if val := first.get("startx"): start_x_first = val // multi
            if val := first.get("endx"): end_x_first = val // multi
            if val := first.get("starty"): start_y_first = val // multi
            if val := first.get("endy"): end_y_first = val // multi
            skip_first = first.get("skip", False)

        if second := page.get("second"):
            if val := second.get("startx"): start_x_second = val // multi
            if val := second.get("endx"): end_x_second = val // multi
            if val := second.get("starty"): start_y_second = val // multi
            if val := second.get("endy"): end_y_second = val // multi
            skip_second = second.get("skip", False)

    return skip_first, skip_second, start_x_first, end_x_first, start_y_first, end_y_first, start_x_second, end_x_second, start_y_second, end_y_second


def openpyxl_ws_autofit(ws):
    dims = {}

    for row in ws.rows:
        for cell in row:
            if cell.value:
                dims[cell.column_letter] = max((dims.get(cell.column_letter, 0), len(str(cell.value)))) + 0.2
    for col, value in dims.items():
        ws.column_dimensions[col].width = value


def df_select_group(df: pd.DataFrame, group: str | None, line: str | None) -> pd.DataFrame:
    new_df = df

    # if group is not None and line is not None:
    #     return df[df["Группа товаров"] == group and df["Линейка"] == line]

    if group is not None:
        new_df = new_df[new_df["Группа товаров"] == group]

    if line is not None:
        new_df = new_df[new_df["Линейка"] == line]

    return new_df


def is_cell_none(value) -> bool:
    return (type(value) is int or type(value) is float) and math.isnan(value)


def get_onedrive_path() -> str:
    res = os.getenv("OneDrive")
    if res is None:
        print("No env variable for 'OneDrive'")
        raise Exception("No env variable for 'OneDrive'")
    return res


def open_file_in_default_app(filepath: str):
    if platform.system() == 'Darwin':                                                                                   # macOS
        subprocess.call(('open', filepath))
    elif platform.system() == 'Windows':                                                                                # Windows
        os.startfile(filepath)
    else:                                                                                                               # linux variants
        subprocess.call(('xdg-open', filepath))
