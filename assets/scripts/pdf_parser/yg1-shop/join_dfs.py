import os

import pandas as pd

from shared import get_onedrive_path

NAME = "turning_holders"

xls = pd.ExcelFile(os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/bd_tables", f"{NAME}.xlsx"))

dfs = []
for sheet_name in xls.sheet_names:
    if sheet_name == "sm" or sheet_name == "repr_fields":                                                               # or sheet_name == "ctd_sha1":
        continue
    dfs.append(pd.read_excel(xls, sheet_name))

df = pd.concat(dfs, ignore_index=True, sort=False, join="outer")
df.to_excel(os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/tables_fields", f"{NAME}_outer.xlsx"),
            index=False)

df = pd.concat(dfs, ignore_index=True, sort=False, join="inner")
df.to_excel(os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/tables_fields", f"{NAME}_inner.xlsx"),
            index=False)
