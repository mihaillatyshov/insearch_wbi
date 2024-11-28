import pandas as pd

xls = pd.ExcelFile("../out/3_export_from_toolz.xlsx")

dfs = []
for sheet_name in xls.sheet_names:
    if sheet_name == "sm" or sheet_name == "repr_fields":                                                               # or sheet_name == "ctd_sha1":
        continue
    dfs.append(pd.read_excel(xls, sheet_name))

df = pd.concat(dfs, ignore_index=True, sort=False, join="outer")

df.to_excel("./out/t2.xlsx", index=False)
