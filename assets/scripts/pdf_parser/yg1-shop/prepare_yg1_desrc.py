import re
import os

import numpy
import pandas as pd

from shared import df_select_group, get_onedrive_path

_RE_COMBINE_WHITESPACE = re.compile(r"\s+")
_RE_STRIP_WHITESPACE = re.compile(r"(?a:^\s+|\s+$)")

OUT_PATH = "yg1_desc__Universal_Drill_Holders.xlsx"

IN_PATH = os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/in", "yg1_full_02102023.xlsx")
OUT_PATH = os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/descrs", OUT_PATH)

group = "Indexable Drilling"
line = "Universal Holder (TR)"
df = df_select_group(
    pd.read_excel(IN_PATH, index_col=None, engine="openpyxl"),
    line=line,
    group=group,
)
# raw_descr = df["Техническое описание"].unique()
raw_descr = df["Описание"].unique()

print(len(raw_descr))

fixed_descr = numpy.array([])

# item = "Патрон фрезерный гидравлический SK50-HC25G-110 . Конус SK40 DIN 69871. Посадочный диаметр 25 мм, Длина до торца 110 мм; Система охлаждения AD, класс балансировки G2.5/25,000 об/мин"
# fixed_item = _RE_COMBINE_WHITESPACE.sub(" ", item)
# fixed_item = _RE_STRIP_WHITESPACE.sub("", fixed_item)
# fixed_item = re.sub(r'(G[0-9]+.[0-9]+/[0-9]+),([0-9]+)', r'\1\2', fixed_item)
# fixed_item = re.sub(r'([+-]?[0-9]+),([0-9]+)', r'\1.\2', fixed_item)
# fixed_item_arr = re.split(r"\s*;\s*|\s*\.\s+|\s*,\s*", fixed_item)
# print(fixed_item_arr)

# fixed_descr = numpy.append(fixed_descr, fixed_item_arr)
# fixed_descr = numpy.append(fixed_descr, fixed_item_arr)

for item in raw_descr:
    fixed_item = _RE_COMBINE_WHITESPACE.sub(" ", item)
    fixed_item = _RE_STRIP_WHITESPACE.sub("", fixed_item)
    fixed_item = re.sub(r'(G[0-9]+.[0-9]+/[0-9]+),([0-9]+)', r'\1\2', fixed_item)
    fixed_item = re.sub(r'(\s+[+-]?[0-9]+),([0-9]+\s+)', r'\1.\2', fixed_item)
    fixed_item_arr = re.split(r"\s*;\s*|\s*\.\s+|\s*,\s*", fixed_item)
    fixed_descr = numpy.append(fixed_descr, fixed_item_arr)

    # print(fixed_item)
    # print(fixed_item_arr)
    # print()

fixed_descr = numpy.unique(fixed_descr)
fixed_descr = numpy.sort(fixed_descr)
# print(fixed_descr)
df_fixed_descr = pd.DataFrame()
df_fixed_descr.insert(0, "desc", fixed_descr)

with pd.ExcelWriter(OUT_PATH, engine="xlsxwriter") as writer:
    df_fixed_descr.to_excel(writer, sheet_name="Desc", index=False)
