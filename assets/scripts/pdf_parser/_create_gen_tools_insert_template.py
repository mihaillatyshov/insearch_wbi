import os

import pandas as pd

from shared import ArgsBase, parse_args


class Args(ArgsBase):
    xlsx_path: str
    save_path: str


def create_insert_template(args: Args):
    os.makedirs(args.save_path, exist_ok=True)
    model_col = []

    for filename in os.scandir(args.xlsx_path):
        if filename.is_file():
            if (filename.name.startswith("~$")): continue

            df = pd.read_excel(filename.path, index_col=None, engine="openpyxl")
            model_col += list(df["model"].values)

    df_result = pd.DataFrame(
        data={
            "manuf": "AMATI",
            "model": model_col,
            "edp": "",
            "lcs": 1,
            "edp_pl": "",
            "codem": "",
            "fulldescription": "",
            "moq": 1,
            "pricevalue": "",
            "pricecurrency": "",
            "priceyear": ""
        })

    writer = pd.ExcelWriter(os.path.join(args.save_path, "_gen_tools_insert_template.xlsx"), engine="xlsxwriter")       # pylint: disable=abstract-class-instantiated
    df_result.to_excel(writer, freeze_panes=(1, 2), index=False)
    writer.close()


create_insert_template(parse_args(Args))

# * python create_gen_tools_insert_template.py C:/Coding/works/wbi/amati_drill/xlsx_add_info/ C:/Coding/works/wbi/amati_drill/xlsx_sm/
