import pandas as pd

FILENAME = "C:/Coding/works/wbi/insearch_scripts/xlsx/single/1_cvs.xlsx"
SAVE_FILENAME = "C:/Coding/works/wbi/insearch_scripts/xlsx/multiple/ctd_list.txt"

df = pd.read_excel(FILENAME, index_col=None, engine="openpyxl")
print(df.columns)

constr_set = set(df["constr"])
print(constr_set)

with open(SAVE_FILENAME, "w") as constr_file:
    constr_file.write("\n".join(constr_set))
