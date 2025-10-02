import os
import traceback
from pathlib import Path

import pandas as pd
from base import ArgsBase, parse_args_new, print_to_cpp
import pydantic

constr_to_lvl = {
    "ctd_jse_m": {
        "l1": "Метчики",
        "l2": "Метчики метрические"
    },
    "ctd_arbm_pullstud": {
        "l1": "Оснастка",
        "l2": "Штревели"
    },
    "ctd_arbm_colletchuck": {
        "l1": "Оснастка",
        "l2": "Фрезерные патроны и оправки"
    },
    "ctd_arbm_shellchuck": {
        "l1": "Оснастка",
        "l2": "Фрезерные патроны и оправки"
    },
    "ctd_me1": {
        "l1": "Фрезы",
        "l2": "Фрезы монолитные",
        "l3": "Фрезы с плоским торцом"
    },
    "ctd_me2": {
        "l1": "Фрезы",
        "l2": "Фрезы монолитные",
        "l3": "Фрезы с плоским торцом"
    },
    "ctd_mr1": {
        "l1": "Фрезы",
        "l2": "Фрезы монолитные",
        "l3": "Фрезы с плоским торцом"
    },
    "ctd_mr2": {
        "l1": "Фрезы",
        "l2": "Фрезы монолитные",
        "l3": "Фрезы с плоским торцом"
    },
    "ctd_ma3": {
        "l1": "Фрезы",
        "l2": "Фрезы монолитные",
        "l3": "Фрезы для обработки фасок"
    },
    "ctd_me5": {
        "l1": "Фрезы",
        "l2": "Фрезы монолитные",
        "l3": "Фрезы насадные"
    },
}


class Args(ArgsBase):
    xlsx_path: str = pydantic.Field(description="Папка исходных файлов")
    save_path: str = pydantic.Field(description="Папка для сохранения")


def get_img_with_prefix(img_full_path: str):
    return os.path.join("http://194.113.153.157/nameduploads/",
                        Path(img_full_path).parent.parent.parent.parent.name,
                        Path(img_full_path).parent.name,
                        Path(img_full_path).name).replace("\"", "/")


def process_row(row):
    drw = str(row["img_drw"]).strip()
    pic = str(row["img_pic"]).strip()

    is_drw_not_empty = drw and drw.lower() != "nan" and drw.strip() != ""
    is_pic_empty = not pic or pic.lower() == "nan" or pic.strip() == ""
    if is_drw_not_empty and is_pic_empty:
        img_list = [p.strip() for p in drw.split(";") if p.strip()]
        if img_list:
            first_img = img_list.pop(0)
            row["img_pic"] = first_img
            row["img_drw"] = ";".join(img_list)

    lvl = constr_to_lvl.get(row["constr"])
    if lvl is not None:
        row["Раздел 1"] = lvl.get("l1")
        row["Раздел 2"] = lvl.get("l2")
        row["Раздел 3"] = lvl.get("l3")

    return row


def add_extra_info(args: Args):
    os.makedirs(args.save_path, exist_ok=True)
    all_dfs = []
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")):
                continue

            print_to_cpp(filename.name)

            df = pd.read_excel(filename, index_col=None, engine="openpyxl")
            df.insert(0, "Раздел 3", None)
            df.insert(0, "Раздел 2", None)
            df.insert(0, "Раздел 1", None)
            df = df.apply(process_row, axis=1)
            all_dfs.append(df)

    combined_df = pd.concat(all_dfs, ignore_index=True)

    writer = pd.ExcelWriter(os.path.join(args.save_path, "ALL.xlsx"), engine="xlsxwriter")                              # pylint: disable=abstract-class-instantiated
    combined_df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 0), index=False)
    worksheet = writer.sheets["sm"]
    worksheet.autofit()
    writer.close()

    print_to_cpp("Все файлы обработаны успешно!")


try:
    add_extra_info(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:
    # exc_type, exc_value, exc_traceback = sys.exc_info()
    formatted_traceback = traceback.format_exc()
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")

# python excel_add_extra_info.py `
# --xlsx_path W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\xlsx_add_info `
# --save_path W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\xlsx_add_info-shop `
