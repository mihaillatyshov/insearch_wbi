# exclude_test_data 0.png,1.png,102.png,2.png,3.png,4.png,5.png
import os
import sys

from img2table.document import PDF, Image
from img2table.ocr import TesseractOCR
from psutil import cpu_count

from base import print_to_cpp

if (len(sys.argv) < 5):
    print_to_cpp("Скрипт не выполнен (не хватает агрументов)")
    exit(-1)

try:
    IMG_PATH = str(sys.argv[1])
    SAVE_PATH = str(sys.argv[2])
    MIN_CONFIDENCE = int(sys.argv[3])
    EXCLUDE_FILES = sys.argv[4].split(",") if sys.argv[4] != "none" else []
except Exception:
    print_to_cpp("Аргументы скрипта имеют ошибки!")
    exit(-1)

threads_to_use = max(1, cpu_count() - 1)
ocr = TesseractOCR(n_threads=threads_to_use, lang="eng+rus", psm=6)


def extract_from_img(img_path: os.DirEntry[str], save_path: str, min_confidence: int):
    print_to_cpp(f"Обрабатывается файл: {img_path.name}")

    img = Image(src=img_path.path)

    try:
        img.to_xlsx(save_path + f"{img_path.name.split('.')[0]}.xlsx",
                    ocr=ocr,
                    borderless_tables=True,
                    min_confidence=min_confidence)
    except Exception:
        return False

    return True


def extract_tables_to_xlsx(img_path: str, save_path: str, min_confidence: int, exclude_files: list[str]):
    result: list[str] = []

    for filename in os.scandir(img_path):
        if filename.is_file():
            if filename.name in exclude_files:
                continue
            if (not extract_from_img(filename, save_path, min_confidence)):
                print_to_cpp(f"Не удалось обработать: {filename.name}")
                result.append(filename.name.split(".")[0])

    print_to_cpp(" ".join(result))


# python .\extract_tables_to_xlsx.py C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/cut_img/ C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/xlsx_raw/ 6 0.png,1.png,102.png,2.png,3.png,4.png,5.png
extract_tables_to_xlsx(IMG_PATH, SAVE_PATH, MIN_CONFIDENCE, EXCLUDE_FILES)