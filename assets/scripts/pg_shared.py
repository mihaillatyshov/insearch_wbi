import sys

from base import print_to_cpp
from sqlalchemy import create_engine
from sqlalchemy.engine import Engine
from sqlalchemy.exc import SQLAlchemyError


def create_sqlalchemy_engine(user: str, password: str, host: str, port: int) -> Engine:
    database = "toolz370_01"
    try:
        return create_engine(
            f"postgresql+psycopg2://{user}:{password}@{host}:{port}/{database}",
            echo=False,
        )
    except SQLAlchemyError as error:
        print_to_cpp(f"Can not create sqlalchemy engine {error}")
        sys.exit(1)
