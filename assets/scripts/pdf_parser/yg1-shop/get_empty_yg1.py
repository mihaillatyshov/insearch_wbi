import pg_shared
import pandas as pd

IN_PATH = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/in/yg1_full_02102023.xlsx"
OUT_PATH_PRESENT = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/in/yg1_present.xlsx"
OUT_PATH_NO_PROPS = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/in/yg1_no_props.xlsx"
OUT_PATH_NOT_PRESENT = "C:/works/wbi/InSearchCreator/assets/scripts/pdf_parser/yg1-shop/in/yg1_not_present.xlsx"

conn, _ = pg_shared.open_db_connection()

df = pd.read_excel(IN_PATH, index_col=None, engine="openpyxl")

df_present = pd.DataFrame()
df_no_props = pd.DataFrame()
df_not_present = pd.DataFrame()

no_props_models: list[str] = []

for index, row in df.iterrows():
    # print(index)
    if (index % 100 == 0):
        print(f"EDP №: {row["EDP №"]}")
    res_not_present = pg_shared.get_db_one_or_none_row(conn, "select gt.model from tools.gen_tools gt where codem = %s and gt.manuf = 'YG1';", (str(row["EDP №"]),))
    res_no_props = pg_shared.get_db_one_or_none_row(conn, "select gt.model from tools.gen_tools gt join tools.gen_items gi on gt.id = gi.tool_id where codem = %s and gt.manuf = 'YG1';", (str(row["EDP №"]),))

    if res_not_present is not None and res_no_props is not None:
        df_present = df_present._append(df.loc[index], ignore_index = True)
    elif res_not_present is None:
        df_not_present = df_not_present._append(df.loc[index], ignore_index = True)
    else:
        no_props_models.append(res_not_present.model)
        df_no_props = df_no_props._append(df.loc[index], ignore_index = True)
        
        
df_no_props.insert(1, "model", no_props_models)

with pd.ExcelWriter(OUT_PATH_NOT_PRESENT, engine="xlsxwriter") as writer:
    df_not_present.to_excel(writer, sheet_name="NotPresent", index=False)
    df_no_props.to_excel(writer, sheet_name="NoProps", index=False)
    df_present.to_excel(writer, sheet_name="Present", index=False)
