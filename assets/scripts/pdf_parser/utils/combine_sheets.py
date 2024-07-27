from typing import cast

import openpyxl
import pandas as pd

df = pd.concat(pd.read_excel('./utils/in/ОтсутствуютПараметры.xlsx', sheet_name=None), ignore_index=True)

writer = pd.ExcelWriter("./utils/out/ОтсутствуютПараметры.xlsx", engine="xlsxwriter")                                   # pylint: disable=abstract-class-instantiated
df.to_excel(writer, sheet_name="Sheet1", freeze_panes=(1, 1), index=False)

workbook = cast(openpyxl.Workbook, writer.book)
worksheet = writer.sheets["Sheet1"]

worksheet.autofit()
writer.close()
