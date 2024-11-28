from collections import namedtuple

import pandas as pd
import psycopg2

constr_names = {
    "ctd_arb_extension": "Удлинитель-переходник",
    "ctd_ds3": "Сверло цельное трёхзубое",
    "ctd_jse_m": "Метчик цельный машинный",
    "ctd_inserts": "Пластина сменная многогранная режущая",
    "ctd_arbm_pullstud": "Штревель",
    "ctd_arbm_colletchuck": "Патрон фрезерный цанговый",
    "ctd_arbm_weldonchuck": "Патрон фрезерный с прямым закреплением винтом",
    "ctd_mb3": "Фреза цельная с полусферическим торцом и коническим обнижением",
    "ctd_mhr": "Головка фрезерная с радиусом",
    "ctd_mcr": "Фреза цельная с рифлями и радиусом",
    "ctd_me2": "Фреза цельная с плоским торцом и с обнижением",
    "ctd_mhb": "Головка фрезерная полусферическая",
    "ctd_me1": "Фреза цельная с плоским торцом",
    "ctd_mb1": "Фреза цельная полусферическая",
    "ctd_mb2": "Фреза цельная полусферическая с обнижением",
    "ctd_mh1": "Головка фрезерная с плоским торцом",
    "ctd_mr1": "Фреза цельная с радиусом",
    "ctd_mr2": "Фреза цельная с радиусом и обнижением",
    "ctd_sha1": "Хвостовик для ввинчиваемого инструмента",
    "ctd_inserts_grooving": "Пластины устанавливаемые на державки для обработки канавок",
    "ctd_jfe_m": "Раскатник цельный - безстружечный метчик",
    "ctd_ds5": "Сверло центровочное",
    "ctd_mc1": "Фреза цельная с рифлями",
    "ctd_mc2": "Фреза цельная с рифлями с обнижением",
    "ctd_rm1": "Развёртка цельная машинная концевая",
    "ctd_mib_hf": "Фреза сборная для высоких подач насадная",
    "ctd_mie_hf": "Фреза сборная для высоких подач концевая",
    "ctd_mih_hf": "Фреза сборная для высоких подач ввинчиваемая",
    "ctd_arbm_hydroholdchuck": "Патрон фрезерный гидропластовый",
    "ctd_arbm_shellchuck": "Оправка фрезерная для насадных фрез",
    "ctd_arbm_tappingchuck": "Патрон для метчиков фрезерный",
    "ctd_ds1": "Сверло цельное спиральное",
    "ctd_arbm_powerchuck": "Патрон фрезерный силовой",
    "ctd_arbm_shrinkchuck": "Термопатрон фрезерный",
    "ctd_arbm_tappingattachment": "Вставка метчиковая",
    "ctd_mt1": "Фреза цельная резьбовая",
    "ctd_arbm_morse": "Патрон фрезерный для конических хвостовиков Морзе",
    "ctd_ds4": "Сверло цельное спиральное с коническим хвостовиком",
    "ctd_collet_round": "Цанга цилиндрическая",
    "ctd_me4": "Фреза цельная с плоским торцом и конусом Морзе",
    "ctd_ma1": "Фреза цельная для обработки фасок",
    "ctd_mcs": "Фреза цельная с рифлями и радиусом и обнижением",
    "ctd_met": "Фреза цельная для обработки канавок",
}

constr_entities = {
    "ctd_arb_extension": "Оснастка",
    "ctd_ds3": "Сверла",
    "ctd_jse_m": "Метчики",
    "ctd_inserts": "Пластины",
    "ctd_arbm_pullstud": "Оснастка",
    "ctd_arbm_colletchuck": "Оснастка",
    "ctd_arbm_weldonchuck": "Оснастка",
    "ctd_mb3": "Фрезы",
    "ctd_mhr": "Фрезы",
    "ctd_mcr": "Фрезы",
    "ctd_me2": "Фрезы",
    "ctd_mhb": "Фрезы",
    "ctd_me1": "Фрезы",
    "ctd_mb1": "Фрезы",
    "ctd_mb2": "Фрезы",
    "ctd_mh1": "Фрезы",
    "ctd_mr1": "Фрезы",
    "ctd_mr2": "Фрезы",
    "ctd_sha1": "Оснастка",
    "ctd_inserts_grooving": "Пластины",
    "ctd_jfe_m": "Метчики",
    "ctd_ds5": "Сверла",
    "ctd_mc1": "Фрезы",
    "ctd_mc2": "Фрезы",
    "ctd_rm1": "Развертки",
    "ctd_mib_hf": "Фрезы",
    "ctd_mie_hf": "Фрезы",
    "ctd_mih_hf": "Фрезы",
    "ctd_arbm_hydroholdchuck": "Оснастка",
    "ctd_arbm_shellchuck": "Оснастка",
    "ctd_arbm_tappingchuck": "Оснастка",
    "ctd_ds1": "Сверла",
    "ctd_arbm_powerchuck": "Оснастка",
    "ctd_arbm_shrinkchuck": "Оснастка",
    "ctd_arbm_tappingattachment": "Оснастка",
    "ctd_mt1": "Фрезы",
    "ctd_arbm_morse": "Оснастка",
    "ctd_ds4": "Сверла",
    "ctd_collet_round": "Оснастка",
    "ctd_me4": "Фрезы",
    "ctd_ma1": "Фрезы",
    "ctd_mcs": "Фрезы",
    "ctd_met": "Фрезы",
}

# TODO: ctd_jse_m, ctd_inserts: нужно вставить группу руками
constr_groups = {
    "ctd_arb_extension": "Удлинители",
    "ctd_ds3": "Цельные свёрла",
    "ctd_jse_m": "",
    "ctd_inserts": "",
    "ctd_arbm_pullstud": "Штревели",
    "ctd_arbm_colletchuck": "Фрезерные патроны и оправки",
    "ctd_arbm_weldonchuck": "Фрезерные патроны и оправки",
    "ctd_mb3": "Фрезы монолитные",
    "ctd_mhr": "Фрезы со сменными головками",
    "ctd_mcr": "Фрезы монолитные",
    "ctd_me2": "Фрезы монолитные",
    "ctd_mhb": "Фрезы со сменными головками",
    "ctd_me1": "Фрезы монолитные",
    "ctd_mb1": "Фрезы монолитные",
    "ctd_mb2": "Фрезы монолитные",
    "ctd_mh1": "Фрезы со сменными головками",
    "ctd_mr1": "Фрезы монолитные",
    "ctd_mr2": "Фрезы монолитные",
    "ctd_sha1": "Хвостовики для модульного инструмента",
    "ctd_inserts_grooving": "Токарные пластины",
    "ctd_jfe_m": "Раскатники цельные",
    "ctd_ds5": "Сверла монолитные",
    "ctd_mc1": "Фрезы монолитные",
    "ctd_mc2": "Фрезы монолитные",
    "ctd_rm1": "Развертки монолитные",
    "ctd_mib_hf": "Фрезы сборные",
    "ctd_mie_hf": "Фрезы сборные",
    "ctd_mih_hf": "Фрезы сборные",
    "ctd_arbm_hydroholdchuck": "Фрезерные патроны и оправки",
    "ctd_arbm_shellchuck": "Фрезерные патроны и оправки",
    "ctd_arbm_tappingchuck": "Фрезерные патроны и оправки",
    "ctd_ds1": "Сверла монолитные",
    "ctd_arbm_powerchuck": "Фрезерные патроны и оправки",
    "ctd_arbm_shrinkchuck": "Фрезерные патроны и оправки",
    "ctd_arbm_tappingattachment": "Патроны для метчиков",
    "ctd_mt1": "Резьбофрезы",
    "ctd_arbm_morse": "Фрезерные патроны и оправки",
    "ctd_ds4": "Сверла монолитные",
    "ctd_collet_round": "Цанги",
    "ctd_me4": "Фрезы монолитные",
    "ctd_ma1": "Фрезы монолитные",
    "ctd_mcs": "Фрезы монолитные",
    "ctd_met": "Цельные фрезы для обработки канавок",
}

constr_extra_groups = {
    "ctd_arb_extension": None,
    "ctd_ds3": None,
    "ctd_jse_m": None,
    "ctd_inserts": None,
    "ctd_arbm_pullstud": None,
    "ctd_arbm_colletchuck": None,
    "ctd_arbm_weldonchuck": None,
    "ctd_mb3": "Сферические фрезы",
    "ctd_mhr": None,
    "ctd_mcr": "Радиусные фрезы",
    "ctd_me2": "Концевые фрезы",
    "ctd_mhb": None,
    "ctd_me1": "Концевые фрезы",
    "ctd_mb1": "Сферические фрезы",
    "ctd_mb2": "Сферические фрезы",
    "ctd_mh1": None,
    "ctd_mr1": "Радиусные фрезы",
    "ctd_mr2": "Радиусные фрезы",
    "ctd_sha1": None,
    "ctd_inserts_grooving": "Токарные пластины",
    "ctd_jfe_m": None,
    "ctd_ds5": "Сверла центровочные",
    "ctd_mc1": "Концевые фрезы",
    "ctd_mc2": "Концевые фрезы",
    "ctd_rm1": None,
    "ctd_mib_hf": "Обработка плоскостей",
    "ctd_mie_hf": "Обработка плоскостей",
    "ctd_mih_hf": None,
    "ctd_arbm_hydroholdchuck": None,
    "ctd_arbm_shellchuck": None,
    "ctd_arbm_tappingchuck": None,
    "ctd_ds1": None,
    "ctd_arbm_powerchuck": None,
    "ctd_arbm_shrinkchuck": None,
    "ctd_arbm_tappingattachment": None,
    "ctd_mt1": None,
    "ctd_arbm_morse": None,
    "ctd_ds4": None,
    "ctd_collet_round": None,
    "ctd_me4": "Концевые фрезы",
    "ctd_ma1": "Фасочные фрезы",
    "ctd_mcs": "Концевые фрезы",
    "ctd_met": "Обработка пазов",
}


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


gp_conn, pg_cursor = open_connection()

# "select constr from tools.gen_items gi where manuf = 'YG1' group by constr;"


def get_constrs(curs) -> list[str]:
    curs.execute("select constr from tools.gen_items gi where manuf = 'YG1' group by constr;")
    result: list[str] = []
    for row in curs.fetchall():
        result.append(row[0])

    return result


def get_constr_columns(curs, table_name) -> str:
    curs.execute(
        """
SELECT STRING_AGG('ctd.' || column_name, ', ')
FROM information_schema.columns
WHERE table_name = %s
AND table_schema = 'tools'
AND column_name NOT IN ('id', 'releasepack', 'tool_id', 'item_id')
""", (table_name, ))

    return curs.fetchone()[0]


def get_constr_data(curs, table_name):
    curs.execute(f"""
select 
	gt.codem,
	{get_constr_columns(curs, table_name)}
from tools.gen_tools gt 
join tools.{table_name} ctd on ctd.tool_id = gt.id
where gt.manuf = 'YG1';
""")
    tuples_list = curs.fetchall()
    column_names = [col[0] for col in curs.description]

    return pd.DataFrame(tuples_list, columns=column_names)


df_inserts = pd.read_excel("./inserts_list.xlsx", index_col=None, engine="openpyxl")

writer = pd.ExcelWriter("out/Props.xlsx", engine='xlsxwriter')
# create_sheet
constrs_list = get_constrs(pg_cursor)
for constr in constrs_list:
    print(f"Parsing table {constr} . . .")

    df = get_constr_data(pg_cursor, constr)

    df.insert(3, "constr", constr)

    if (name := constr_names.get(constr)) is None:
        print(f"No name for {constr}")
        exit(1)
    df.insert(0, "name", name)

    if (constr_extra_group := constr_extra_groups.get(constr)) is not None:
        df.insert(0, "constr_extra_group", constr_extra_group)

    if (constr_group := constr_groups.get(constr)) is None:
        print(f"No group for {constr}")
        exit(1)
    elif constr_group == "":
        if constr == "ctd_inserts":
            constr_group_inserts: list[str | None] = []
            for i in range(len(df.index)):
                value = None
                for j in range(len(df_inserts.index)):
                    model_no_mrgo = "-".join(str(df["model"][i]).lower().split("-")[:-1])
                    if model_no_mrgo in str(df_inserts["name"][j]).lower():
                        if df_inserts["type"][j] == "Indexable Turning": value = "Токарные пластины"
                        if df_inserts["type"][j] == "Indexable Milling": value = "Фрезерные пластины"
                        break
                constr_group_inserts.append(value)
                if value is None:
                    print(f"Не найдена группа для модели {df["model"][i]} в табличке {constr}")
            df.insert(0, "constr_group", constr_group_inserts)
        elif constr == "ctd_jse_m":
            constr_group_jse: list[str | None] = []
            for i in range(len(df.index)):
                value = None
                if df["thft"][i] == "M60":
                    value = "Метрическая резьба"
                elif df["thft"][i] == "WH55":
                    value = "Резьба Витворта"
                elif df["thft"][i] == "UN60":
                    if "UNC" in df["threadsh"][i]:
                        value = "Дюймовая UNC"
                    elif "UNF" in df["threadsh"][i]:
                        value = "Дюймовая UNF"
                constr_group_jse.append(value)
                if value is None:
                    print(f"Не найдена группа для модели {df["model"][i]} в табличке {constr}")
            df.insert(0, "constr_group", constr_group_jse)
        else:
            print(f"    Группа {constr} будет заполняться вручную")
            df.insert(0, "constr_group", "")
    else:
        df.insert(0, "constr_group", constr_group)

    if (constr_entity := constr_entities.get(constr)) is None:
        print(f"No entity for {constr}")
        exit(1)

    df.insert(0, "constr_entity", constr_entity)

    df.to_excel(writer, sheet_name=constr, index=False)

    writer.sheets[constr].autofit()

writer.close()
