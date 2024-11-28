from collections import namedtuple

import pandas as pd
import psycopg2


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

xl = pd.ExcelFile('./out/Props.xlsx', engine='openpyxl')

writer = pd.ExcelWriter("out/Images.xlsx", engine='xlsxwriter')
writerMissing = pd.ExcelWriter("out/ImagesMissing.xlsx", engine='xlsxwriter')

for sheet_name in xl.sheet_names:
    df = xl.parse(sheet_name, index_col=None)
