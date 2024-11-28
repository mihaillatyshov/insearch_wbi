import pandas as pd

IN_PATH = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/in/yg1_full_02102023.xlsx"
OUT_PATH = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/in/yg1_simple.xlsx"

df = pd.read_excel(IN_PATH, index_col=None, engine="openpyxl")

raw_descr = df["Линейка"].unique()

print(len(raw_descr))
print(raw_descr)
