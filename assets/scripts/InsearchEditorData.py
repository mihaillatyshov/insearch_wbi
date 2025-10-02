import os
import pandas.io.sql as sql
import pandas as pd
import json

import time as t
import openpyxl
from openpyxl.comments import Comment
from openpyxl.styles import PatternFill, Font

import psycopg2
from collections import namedtuple
from shared import ArgsBase, parse_args


class Args(ArgsBase):
    save_path: str


parsed_args = parse_args(Args)

EXPORT_FOLDER = parsed_args.save_path

# TODO: other way to get constr_set (from db)

constr_set = set([
    "ctd_arb_busheccenter", "ctd_arb_extension", "ctd_arbm_drillingchuck", "ctd_bs_adapter1", "ctd_bs_cart_head",
    "ctd_bs_flange", "ctd_ds3", "ctd_arb_bush", "ctd_jse_m", "ctd_mc4", "ctd_mhc1", "ctd_sha2", "ctd_rm4", "ctd_the",
    "ctd_bs_arbor", "ctd_ma2", "ctd_inserts", "ctd_arbm_pullstud", "ctd_arbm_colletchuck", "ctd_mie_gears",
    "ctd_arbm_weldonchuck", "ctd_inserts_gears", "ctd_tsmb_tuplethread", "ctd_mis_gears", "ctd_mib_gears",
    "ctd_tc_head", "ctd_bs_adapter_coolantb", "ctd_mib_cartmill1", "ctd_tsmb_radial_doubledir", "ctd_tavs_head",
    "ctd_mih_mfunc", "ctd_grinding_wheel", "ctd_mb3", "ctd_pass_cutter", "ctd_mhr", "ctd_tge_module_holder",
    "ctd_tsmb_face_chamfer", "ctd_mib_t", "ctd_mcr", "ctd_mic_90", "ctd_mko_radial_groove", "ctd_me2", "ctd_mhb",
    "ctd_me1", "ctd_die_f1", "ctd_mha1", "ctd_mb1", "ctd_mb2", "ctd_mh1", "ctd_burr", "ctd_inserts_drilling", "ctd_mr1",
    "ctd_mr2", "ctd_tge_face", "ctd_sha1", "ctd_tthe_holder", "ctd_tgm_face", "ctd_jse_hand_set", "ctd_mt2",
    "ctd_tgm_radial", "ctd_mt3", "ctd_tavs_arbor", "ctd_bs_head", "ctd_tsmb_face_fullround", "ctd_tsmb_face_groove",
    "ctd_tsmb_tupleback_boring", "ctd_tsmb_thread", "ctd_tthi_holder", "ctd_inserts_grooving", "ctd_inserts_dh1",
    "ctd_inserts_threading", "ctd_inserts_threading_2surf", "ctd_tge_radial", "ctd_tca", "ctd_tgi_radial", "ctd_jfe_m",
    "ctd_tmfe_holder", "ctd_jse_hand_each", "ctd_tgb_radial", "ctd_tko_radial_fullround", "ctd_mt4",
    "ctd_eths_holder_drill", "ctd_eths_squareholder", "ctd_mis_disk", "ctd_tsmb_turn", "ctd_mthr",
    "ctd_tko_radial_groove", "ctd_collet_tapping", "ctd_ds5", "ctd_tsmb_back_boring", "ctd_tsmb_chamfer_preparting",
    "ctd_tsmb_radial_fullround", "ctd_tsmb_radial_groove", "ctd_graver", "ctd_tsmb_multi_functional",
    "ctd_tsmb_tuplechamfer_preparting", "ctd_tsmb_tupleturn", "ctd_mc1", "ctd_mb4", "ctd_mc2", "ctd_rm1", "ctd_mha3",
    "ctd_mhcb", "ctd_mib_3075", "ctd_mib_90", "ctd_mib_chamf", "ctd_mib_corn901", "ctd_mib_hf", "ctd_mib_rou",
    "ctd_mie_t", "ctd_mie_chamf", "ctd_mie_corn901", "ctd_mie_hf", "ctd_zip", "ctd_mhcr", "ctd_mie_rou",
    "ctd_mie_single", "ctd_mih_3075", "ctd_mih_90", "ctd_mih_hf", "ctd_mih_rou", "ctd_mih_single", "ctd_mic_corn902",
    "ctd_mko_tslots", "ctd_mr3", "ctd_rm_head", "ctd_mgb", "ctd_ne1_countersink", "ctd_bs_bridge", "ctd_bs_carte",
    "ctd_bs_carti_solid", "ctd_tko_face_groove", "ctd_arbm_hydroholdchuck", "ctd_tko_chamfer_preparting",
    "ctd_tkoi_holder", "ctd_tko_thread", "ctd_tsmb_bush", "ctd_ethd_corner", "ctd_collet_spring", "ctd_arbm_shellchuck",
    "ctd_tmfi_holder", "ctd_die_2", "ctd_arbm_tappingchuck", "ctd_dsp", "ctd_eths_blank", "ctd_tgb_face",
    "ctd_tmf_module", "ctd_tsmb_holdersquare", "ctd_eths_roundholder", "ctd_ds1", "ctd_mih_fe",
    "ctd_tko_face_fullround", "ctd_tko_turn", "ctd_ethd_straight", "ctd_mi_cartmill1", "ctd_mie_threadmill",
    "ctd_arbm_powerchuck", "ctd_arbm_shrinkchuck", "ctd_arbm_wnotchchuck", "ctd_arbm_tappingattachment",
    "ctd_arbt_2sidecolletholder", "ctd_tge_face_assembly", "ctd_mt1", "ctd_ne4_countersink", "ctd_arbm_morse",
    "ctd_tge_arbor_module_holder", "ctd_tge_radial_assembly", "ctd_tge_radial_sub", "ctd_tgi_radial_assembly",
    "ctd_dih_f1", "ctd_ds4", "ctd_ethd_corner_2sides", "ctd_mie_fe", "ctd_bs_adapter_coolantholer", "ctd_tgi_face",
    "ctd_ecr", "ctd_arbt_colletturning", "ctd_bs_cart_adj", "ctd_jdi", "ctd_tko_back_boring", "ctd_tkoe_holder",
    "ctd_tsmb_bush2side", "ctd_tsmb_holderarbor", "ctd_eths_arborholder", "ctd_inserts_plungturning",
    "ctd_tmfe_round_holder", "ctd_tgi_module_holder", "ctd_tkoi_holder_flat", "ctd_die_c2", "ctd_die_o1",
    "ctd_tge_blade_holder", "ctd_bs_carti_indexable", "ctd_mi_cap", "ctd_thi", "ctd_collet_round",
    "ctd_tsmb_tupleradial_groove", "ctd_ds2", "ctd_ds6", "ctd_tsmb_tupleradial_fullround", "ctd_tsmb_tupleface_groove",
    "ctd_mie_per", "ctd_me4", "ctd_mie_mfunc", "ctd_mib_mfunc", "ctd_ma1", "ctd_mib_threadmill", "ctd_mih_threadmill",
    "ctd_mr4", "ctd_mib_disk", "ctd_mcs", "ctd_mie_3075", "ctd_mie_90", "ctd_mih_chamf", "ctd_met",
    "ctd_ne2_countercone", "ctd_ne3_counterbore"
])
print("Constr to process: ", constr_set)


def open_connection():
    config = {
        'user': 'postgres',
        'password': 'Radius314',
        'host': '192.168.111.115',
        'port': '5432',
        'database': 'toolz370_01'
    }
    try:
        conn = psycopg2.connect(**config)
        cursor = conn.cursor()
    except:
        print('Не могу подключиться к базе')
    return (conn, cursor)


def get_db_rows(curs, sql, args=None):                                                                                  # sql как параметр
    curs.execute(sql, args)
    columns = [col[0] for col in curs.description]
    RowType = namedtuple('Row', columns)
    data = [RowType(*row) for row in curs.fetchall()]
    return (data)                                                                                                       # стандартный разбор Sql запроса.


co, cu = open_connection()

tables_list = list(constr_set)

const_cols = dict()
for table in tables_list:
    if table == '':
        continue
    try:
        query = "SELECT * FROM tools." + table + " ORDER BY random() LIMIT 5"
        df = sql.read_sql(query, co)
        # export the data into the excel sheet
        df.drop(columns=["id", "tool_id", "item_id"], errors='ignore', inplace=True)
        with pd.ExcelWriter(path=os.path.join(EXPORT_FOLDER, "examples", f"{table}.xlsx"), engine='openpyxl') as writer:
            df.to_excel(writer, sheet_name="ctd", index=False)

        const_cols[table] = list(df)

    except Exception as exc:
        print(f"Somethin wrong with table: '{table}'", f"Error: {exc}")
        continue

const_cols["ctd_ma3"] = [
    "model", "manuf", "serie", "app1", "app2", "dc", "dctol", "dcn", "dcx", "re", "kapr", "apmx", "apmxpfw", "zefp",
    "adintms", "adintws", "tcdcon", "grade", "substrate", "coating", "bsg", "cnsc", "cxsc", "dcon", "lf", "lu", "lsh",
    "cst", "rpmx", "wt", "sep", "releasepack", "mgro", "varom", "cpdf", "ccc"
]
const_cols["ctd_me5"] = [
    "model", "manuf", "serie", "app1", "app2", "dc", "dctol", "dctoll", "dctolu", "dcf", "kch", "chw", "apmx",
    "apmxpfw", "ccc", "apmxffw", "lu", "zefp", "adintms", "adintws", "rmpxffw", "tcdcon", "grade", "substrate",
    "coating", "bsg", "cnsc", "cxsc", "dcon", "lf", "bhta", "fha", "gamf", "gamp", "norgmx", "rpmx", "wt", "sep",
    "dcsfms", "kdp", "kww", "releasepack", "mgro", "varom", "cpdf"
]

with open(os.path.join(EXPORT_FOLDER, "ctd_fields.json"), "w", encoding="utf-8") as f:
    json.dump(const_cols, f)

print('The previous file has been created')

repr_query = ("SELECT * FROM pg_catalog.pg_tables where tablename like 'descr%';")
resu = get_db_rows(cu, repr_query)
orange_fields = list()
for ro in resu:
    field_name = str(ro.tablename).replace('descr_', '')
    orange_fields.append(field_name)

print('===============================')
print('orange fields count:', len(orange_fields))
print('orange fields:', orange_fields)
print('===============================')

df = sql.read_sql(
    "select fieldname, typ, unitru, rudescription, allownulls, ifbooleantrue, ifbooleanfalse, qtyp, is_descr from tools.gen_fields;",
    co)
print('gen fields count:', len(df))
df = pd.concat([
    df,
    pd.DataFrame.from_records([
        {
            "fieldname": "moq",
            "typ": "Integer",
            "unitru": "шт.",
            "rudescription": "Минимальное число для покупки",
            "allownulls": False,
            "ifbooleantrue": None,
            "ifbooleanfalse": None,
            "qtyp": 2,
            "is_descr": False
        },
        {
            "fieldname": "constr",
            "typ": "String",
            "unitru": None,
            "rudescription": "Название конструкции (CTD таблицы)",
            "allownulls": False,
            "ifbooleantrue": None,
            "ifbooleanfalse": None,
            "qtyp": 4,
            "is_descr": True
        },
    ])
],
               ignore_index=True)
print('gen fields count:', len(df))
df["descr"] = df.apply(lambda row: f"{row['rudescription']} ({row['unitru']})"
                       if pd.notnull(row['unitru']) else row['rudescription'],
                       axis=1)
gen_fields = df.to_json(orient="records")
with open(os.path.join(EXPORT_FOLDER, "gen_fields.json"), "w", encoding="utf-8") as f:
    f.write(gen_fields)

orange_descrs = dict()
for orange_field in orange_fields:
    df = sql.read_sql(
        f"SELECT {orange_field} as key, {orange_field}_rushort as rushort, {orange_field}_ru as ru FROM tools.descr_{orange_field}",
        co)
    descr = json.loads(df.to_json(orient="records"))
    orange_descrs[orange_field] = descr

with open(os.path.join(EXPORT_FOLDER, "repr_fields.json"), "w", encoding="utf-8") as f:
    json.dump(orange_descrs, f)

cu.close()
co.close()
print('done')
