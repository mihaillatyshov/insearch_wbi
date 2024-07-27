import os

import pandas as pd

from shared import flatten

PATH = "C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/xlsx_checks2_c/"
SAVE_PATH = "C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/xlsx_fixed/"


def get_page_title(id: int) -> list[str]:
    id += 3
    if id in flatten([[9, 10], [16, 17], [23, 24], [44, 45, 46], [55, 56, 57, 58]]):
        return ["d1", "L1", "Dh6", "L"]
    if id in [11, 12, 18, 19, 25, 26, 47]:
        return ["R", "L1", "Dh6", "L"]
    if id in [13, 14, 15, 20, 21, 22, 27, 28, 29]:
        return ["d1", "R", "L1", "Dh6", "L"]
    if id in [30, 31, 36, 37, 39, 52, 53]:
        return ["d1", "d2", "L1", "L2", "Dh6", "L"]
    if id in [34, 35, 40, 41]:
        return ["d1", "R", "d2", "L1", "L2", "Dh6", "L"]
    if id in [32, 33, 38, 39, 42, 43]:
        return ["R", "d2", "L1", "L2", "Dh6", "L"]
    if id in [48, 49]:
        return ["d1", "L1", "L2", "Dh6", "L"]
    if id in [50, 51]:
        return ["R", "L1", "L2", "Dh6", "L"]
    if id in [54]:
        return ["R", "d1", "d2", "L1", "Dh6", "L"]

    raise ValueError(f"No id ({id}) in list get_page_title")


def get_page_ctd(id: int) -> str:
    id += 3

    # * Плоский торец без обнижения
    if id in [9, 10, 16, 17, 23, 24, 44, 45]:
        return "me1"

    # * Полусферический торец без обнижения
    if id in [11, 12, 18, 19, 25, 26]:
        return "mb1"

    # * Плоский торец с радиусом без обнижения
    if id in [13, 14, 15, 20, 21, 22, 27, 28, 29]:
        return "mr1"

    # * Плоский торец с обнижением
    if id in [36, 37]:
        return "me2"

    # * Полусферический торец с обнижением
    if id in [32, 33, 38, 39, 42, 43]:
        return "mb2"

    # * Плоский торец с радиусом и обнижением
    if id in [34, 35, 40, 41]:
        return "mr2"
    # ! В каталоге отсутствует радиус, поэтому меняем mr2 на me2
    if id in [30, 31]:
        return "me2"

    # * Плоский торец, мелкоразмерные
    # ! Возможно другое обозначение (не me1)
    if id in [46]:
        return "me1"

    # * Полусферический торец, мелкоразмерные
    # ! Возможно другое обозначение (не mb1)
    if id in [47]:
        return "mb1"

    # * Плоский торец, мелкоразмерные с обнижением
    # ! Возможно другое обозначение (не me2)
    if id in [48, 49]:
        return "me2"

    # * Полусферический торец, мелкоразмерные с обнижением
    # ! Возможно другое обозначение (не mb2)
    if id in [50, 51]:
        return "mb2"

    # * T-Образная
    # ! Возможно другое обозначение (не met)
    if id in [52, 53]:
        return "met"

    # * Обратный радиус
    # ! Возможно другое обозначение (не ma3)
    if id in [54]:
        return "ma3"

    # * Плоский торец с рифлями
    if id in [55]:
        return "mc1"

    # * Плоский торец с рифлями
    if id in [56]:
        return "ma1"

    # * Коническая
    if id in [57]:
        return "mb3"

    # * Однозубая
    # ! Возможно другое обозначение (не me1)
    if id in [58]:
        return "me1"

    raise ValueError(f"No id ({id}) in list get_page_ctd")


def add_xlsx_headers_single(xlsx_path: os.DirEntry[str], save_path: str):
    print(xlsx_path)

    df = pd.read_excel(xlsx_path.path, index_col=None, engine="openpyxl")

    page_id = int(xlsx_path.name.split('.')[0])
    # ! rows_count = len(df.index)

    df.insert(1, "manuf", "amati")
    df.insert(2, "constr", f"ctd_{get_page_ctd(page_id)}")
    writer = pd.ExcelWriter(save_path + xlsx_path.name, engine="xlsxwriter")
    header = get_page_title(page_id)
    header.insert(0, "model")
    header.insert(1, "manuf")
    header.insert(2, "constr")

    df.to_excel(writer, sheet_name="Sheet1", freeze_panes=(1, 1), index=False, header=header)

    worksheet = writer.sheets["Sheet1"]
    worksheet.autofit()
    writer.close()


def add_xlsx_headers(xlsx_path: str, save_path: str):
    for filename in os.scandir(xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")): continue
            add_xlsx_headers_single(filename, save_path)


add_xlsx_headers(PATH, SAVE_PATH)