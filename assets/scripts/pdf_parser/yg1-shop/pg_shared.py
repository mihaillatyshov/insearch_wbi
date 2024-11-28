from collections import namedtuple
from typing import Any, List, Optional, Union

import psycopg2


def open_db_connection():
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


def get_db_all_rows(connection, query: str, args: Union[tuple, dict] = None) -> List[tuple]:
    cursor = connection.cursor()
    cursor.execute(query, args)
    columns = [col[0] for col in cursor.description]
    RowType = namedtuple("Row", columns)
    data = [RowType(*row) for row in cursor.fetchall()]

    cursor.close()
    return (data)


def get_db_one_or_none_row(connection, query: str, args: Union[tuple, dict] = None) -> Optional[tuple]:
    cursor = connection.cursor()
    cursor.execute(query, args)
    columns = [col[0] for col in cursor.description]
    RowType = namedtuple("Row", columns)
    fetch_data = cursor.fetchone()
    if fetch_data is None:
        return None
    data = RowType(*fetch_data)

    cursor.close()
    return (data)


def get_db_one_or_none_cell(connection, query: str, args: Union[tuple, dict] = None) -> Optional[Any]:
    cursor = connection.cursor()
    cursor.execute(query, args)
    fetch_data = cursor.fetchone()
    if fetch_data is None:
        return None
    data = fetch_data[0]

    cursor.close()
    return (data)
