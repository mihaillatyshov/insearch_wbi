import sys
from typing import TypeVar

import cv2
import numpy as np
from pydantic import BaseModel as ArgsBase


def print_to_cpp(string: str):
    print(string, file=utf8stdout, flush=True)


TArgs = TypeVar("TArgs", bound=ArgsBase)


def parse_args(args_type: type[TArgs]) -> TArgs:
    if (len(sys.argv) - 1) < len(args_type.__fields__.keys()):
        print_to_cpp("Скрипт не выполнен (не хватает агрументов)")
        exit(-1)

    try:
        args_input = {}
        for i, key in enumerate(args_type.__fields__):
            args_input[key] = str(sys.argv[i + 1])
        return args_type(**args_input)
    except Exception:
        print("Аргументы скрипта имеют ошибки!")
        sys.exit(-1)


utf8stdout = open(1, 'w', encoding='utf-8', closefd=False)


def file_format_id(id: int) -> str:
    return f"{id:0>4}"


def file_format_img(id: int) -> str:
    return file_format_id(id) + ".png"


def read_cv_file(path: str):
    with open(path, "rb") as f:
        chunk = f.read()
        chunk_arr = np.frombuffer(chunk, dtype=np.uint8)
        return cv2.imdecode(chunk_arr, cv2.IMREAD_COLOR)
