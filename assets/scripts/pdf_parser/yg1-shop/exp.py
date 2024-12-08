import os

import numpy as np
import pandas as pd

from shared import df_select_group, get_onedrive_path

IN_PATH = os.path.join(get_onedrive_path(), "Work/wbi/yg1-shop/parsing/in", "yg1_full_02102023.xlsx")
df = pd.read_excel(IN_PATH, index_col=None, engine="openpyxl")
# a_df = df.drop_duplicates(['Группа товаров', 'Линейка'])[['Группа товаров', 'Линейка']]
# a_df = df.groupby(['Группа товаров', 'Линейка'])
a_df = df.filter(items=['Группа товаров', 'Линейка']).drop_duplicates()
print(a_df)
# for val in a_df:
#     print(val)

with pd.ExcelWriter("./out/exp.xlsx", engine="xlsxwriter") as writer:
    a_df.to_excel(writer, sheet_name="Desc", index=False)
