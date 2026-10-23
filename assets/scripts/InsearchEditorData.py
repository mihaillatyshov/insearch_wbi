import json
import os
import shutil

import pandas as pd
import pydantic
from base import ArgsBase, log_info_to_cpp, log_trace_to_cpp, log_warning_to_cpp, start_program
from pg_shared import create_sqlalchemy_engine
from sqlalchemy import text
from sqlalchemy.engine import Engine

# Таблицы, которых может не быть в БД, но поля для них нужны в редакторе
FALLBACK_CONST_COLS = {
    "ctd_ma3": [
        "model", "manuf", "serie", "app1", "app2", "dc", "dctol", "dcn", "dcx", "re", "kapr", "apmx", "apmxpfw", "zefp",
        "adintms", "adintws", "tcdcon", "grade", "substrate", "coating", "bsg", "cnsc", "cxsc", "dcon", "lf", "lu",
        "lsh", "cst", "rpmx", "wt", "sep", "releasepack", "mgro", "varom", "cpdf", "ccc"
    ],
    "ctd_me5": [
        "model", "manuf", "serie", "app1", "app2", "dc", "dctol", "dctoll", "dctolu", "dcf", "kch", "chw", "apmx",
        "apmxpfw", "ccc", "apmxffw", "lu", "zefp", "adintms", "adintws", "rmpxffw", "tcdcon", "grade", "substrate",
        "coating", "bsg", "cnsc", "cxsc", "dcon", "lf", "bhta", "fha", "gamf", "gamp", "norgmx", "rpmx", "wt", "sep",
        "dcsfms", "kdp", "kww", "releasepack", "mgro", "varom", "cpdf"
    ],
}

EXTRA_GEN_FIELDS = [
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
]


class Args(ArgsBase):
    save_path: str = pydantic.Field(description="Папка для сохранения данных")
    db_host: str = pydantic.Field(description="Хост БД")
    db_port: int = pydantic.Field(description="Порт БД", default=5432)
    db_user: str = pydantic.Field(description="Пользователь БД")
    db_password: str = pydantic.Field(description="Пароль БД")
    db_name: str = pydantic.Field(description="Имя БД", default="toolz370_01")


def prepare_export_folder(export_folder: str):
    os.makedirs(export_folder, exist_ok=True)
    for entry in os.listdir(export_folder):
        entry_path = os.path.join(export_folder, entry)
        if os.path.isdir(entry_path) and not os.path.islink(entry_path):
            shutil.rmtree(entry_path)
        else:
            os.remove(entry_path)
    os.makedirs(os.path.join(export_folder, "examples"), exist_ok=True)


def get_ctd_tables(engine: Engine) -> list[str]:
    query = text("""
        SELECT table_name
        FROM information_schema.tables
        WHERE table_schema = 'tools'
        AND table_type = 'BASE TABLE'
        AND table_name LIKE 'ctd!_%' ESCAPE '!'
        ORDER BY table_name
    """)
    with engine.connect() as connection:
        return [row[0] for row in connection.execute(query)]


def export_ctd_fields(engine: Engine, export_folder: str):
    tables = get_ctd_tables(engine)
    log_info_to_cpp(f"Найдено CTD таблиц: {len(tables)}")

    const_cols: dict[str, list[str]] = {}
    for table in tables:
        try:
            log_trace_to_cpp(f"Обработка таблицы: {table}")
            df = pd.read_sql(text(f'SELECT * FROM tools."{table}" ORDER BY random() LIMIT 5'), engine)
            df.drop(columns=["id", "tool_id", "item_id"], errors="ignore", inplace=True)
            with pd.ExcelWriter(path=os.path.join(export_folder, "examples", f"{table}.xlsx"),
                                engine="openpyxl") as writer:
                df.to_excel(writer, sheet_name="ctd", index=False)

            const_cols[table] = list(df)
        except Exception as exc:                                                                                        # pylint: disable=broad-exception-caught
            log_warning_to_cpp(f"Ошибка при обработке таблицы '{table}': {exc}")

    for table, cols in FALLBACK_CONST_COLS.items():
        const_cols.setdefault(table, cols)

    with open(os.path.join(export_folder, "ctd_fields.json"), "w", encoding="utf-8") as f:
        json.dump(const_cols, f)

    log_info_to_cpp("Создан ctd_fields.json")


def export_constructions_list(engine: Engine, export_folder: str):
    query = text("""
        SELECT constr, img_link, drw_link, constr_rushort, constr_ru
        FROM tools.gen_constructions
        ORDER BY constr
    """)
    with engine.connect() as connection:
        constructions = [{
            "key": row.constr,
            "img_link": row.img_link,
            "drw_link": row.drw_link,
            "constr_rushort": row.constr_rushort,
            "text": row.constr_ru,
        } for row in connection.execute(query)]

    with open(os.path.join(export_folder, "constructions_list.json"), "w", encoding="utf-8") as f:
        json.dump(constructions, f, ensure_ascii=False)

    log_info_to_cpp(f"Создан constructions_list.json (конструкций: {len(constructions)})")


# TODO: export_constructions_tree (constructions_tree.json: entities -> toolgroups -> constrs)


def export_gen_fields(engine: Engine, export_folder: str):
    df = pd.read_sql(
        text("SELECT fieldname, typ, unitru, rudescription, allownulls, ifbooleantrue, ifbooleanfalse, qtyp, is_descr "
             "FROM tools.gen_fields"), engine)
    df = pd.concat([df, pd.DataFrame.from_records(EXTRA_GEN_FIELDS)], ignore_index=True)
    df["descr"] = df.apply(lambda row: f"{row['rudescription']} ({row['unitru']})"
                           if pd.notnull(row['unitru']) else row['rudescription'],
                           axis=1)

    with open(os.path.join(export_folder, "gen_fields.json"), "w", encoding="utf-8") as f:
        f.write(df.to_json(orient="records"))

    log_info_to_cpp(f"Создан gen_fields.json (полей: {len(df)})")


def export_repr_fields(engine: Engine, export_folder: str):
    query = text("""
        SELECT tablename
        FROM pg_catalog.pg_tables
        WHERE schemaname = 'tools'
        AND tablename LIKE 'descr!_%' ESCAPE '!'
    """)
    with engine.connect() as connection:
        orange_fields = [str(row[0]).replace("descr_", "", 1) for row in connection.execute(query)]

    log_info_to_cpp(f"Найдено полей с описанием: {len(orange_fields)}")

    orange_descrs = {}
    for orange_field in orange_fields:
        df = pd.read_sql(
            text(f'SELECT "{orange_field}" AS key, "{orange_field}_rushort" AS rushort, "{orange_field}_ru" AS ru '
                 f'FROM tools."descr_{orange_field}"'), engine)
        orange_descrs[orange_field] = json.loads(df.to_json(orient="records"))

    with open(os.path.join(export_folder, "repr_fields.json"), "w", encoding="utf-8") as f:
        json.dump(orange_descrs, f)

    log_info_to_cpp("Создан repr_fields.json")


def main(args: Args):
    export_folder = args.save_path
    prepare_export_folder(export_folder)

    engine = create_sqlalchemy_engine(args.db_user, args.db_password, args.db_host, args.db_port, args.db_name)
    try:
        export_ctd_fields(engine, export_folder)
        export_constructions_list(engine, export_folder)
        export_gen_fields(engine, export_folder)
        export_repr_fields(engine, export_folder)
    finally:
        engine.dispose()

    log_info_to_cpp("Готово")


if __name__ == "__main__":
    start_program(main, Args)
