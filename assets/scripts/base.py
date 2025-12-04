import argparse
import sys
import traceback
from typing import TypeVar

import cv2
import numpy as np
from pydantic import BaseModel as ArgsBase


def print_to_cpp(string: str):
    print(string, file=utf8stdout, flush=True)


TArgs = TypeVar("TArgs", bound=ArgsBase)


def parse_args(args_type: type[TArgs]) -> TArgs:
    if (len(sys.argv) - 1) < len(args_type.model_fields.keys()):
        print_to_cpp("Скрипт не выполнен (не хватает агрументов)")
        exit(-1)

    try:
        args_input = {}
        for i, key in enumerate(args_type.model_fields):
            args_input[key] = str(sys.argv[i + 1])
        return args_type(**args_input)
    except Exception:
        print("Аргументы скрипта имеют ошибки!")
        sys.exit(-1)


def parse_args_add_model(parser, model: type[TArgs]):
    "Add Pydantic model to an ArgumentParser"
    fields = model.model_fields
    for name, field in fields.items():
        parser.add_argument(
            f"--{name}",
            dest=name,
            type=field.annotation,
            default=field.default,
            help=field.description,
        )


def parse_args_new(args_type: type[TArgs]) -> TArgs:
    try:
        parser = argparse.ArgumentParser()
        parse_args_add_model(parser, args_type)
        args = parser.parse_args()
        item = args_type(**vars(args))
        return item
    except Exception:
        # exc_type, exc_value, exc_traceback = sys.exc_info()
        formatted_traceback = traceback.format_exc()
        print_to_cpp(f"Аргументы скрипта имеют ошибки:\n{formatted_traceback}")
        sys.exit(-1)


utf8stdout = open(1, 'w', encoding='utf-8', closefd=False)


def file_format_id(index: int) -> str:
    return f"{index:0>4}"


def file_format_img(index: int) -> str:
    return file_format_id(index) + ".png"


def read_cv_file(path: str):
    with open(path, "rb") as f:
        chunk = f.read()
        chunk_arr = np.frombuffer(chunk, dtype=np.uint8)
        return cv2.imdecode(chunk_arr, cv2.IMREAD_COLOR)


def remove_suffix(input_string, suffix):
    if suffix and input_string.endswith(suffix):
        return input_string[:-len(suffix)]
    return input_string


def remove_model_suffix(val) -> str:
    result = val
    for suf in ["_AMATI", "_ASKUP", "_DEREK", "_HT", "_PL", "_LIK", "_ТИЗ"]:
        result = remove_suffix(result, suf)

    return result
