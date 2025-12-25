import logging
import sys

from sqlalchemy import create_engine
from sqlalchemy.engine import Engine
from sqlalchemy.exc import SQLAlchemyError

config = {
    'user': 'postgres',
    'password': 'Radius314',
    'host': 'localhost',
    'port': '5432',
    'database': 'toolz370_01',
}


def create_sqlalchemy_engine() -> Engine:
    try:
        return create_engine(
            'postgresql+psycopg2://' + config['user'] + ':' + config['password'] + '@' + config['host'] + ':' +
            config['port'] + '/' + config['database'],
            echo=False,
        )
    except SQLAlchemyError as error:
        logging.error('Can not create sqlalchemy engie %s', error)
        sys.exit(1)
