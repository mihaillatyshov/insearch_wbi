from pdf2image import convert_from_path
from psutil import cpu_count
from PIL import Image

import sys

utf8stdout = open(1, 'w', encoding='utf-8', closefd=False)                                                              # fd 1 is stdout


def print_to_cpp(string: str):
    print(string, file=utf8stdout, flush=True)


if (len(sys.argv) < 5):
    print_to_cpp("Скрипт не выполнен (не хватает агрументов)")
    exit(-1)

try:
    CATALOG_PATH = str(sys.argv[1])
    SAVE_PATH = str(sys.argv[2])
    MULTI = int(sys.argv[3])
    NEED_SPLIT = bool(int(sys.argv[4]))
except Exception:
    print_to_cpp("Аргументы скрипта имеют ошибки!")
    exit(-1)


def save_images(pages: list[Image.Image], page_id, save_path: str, need_split: bool):
    print_to_cpp(f"Сохраняется страница ({page_id})")
    img = pages[page_id]
    if need_split:
        width, height = img.size
        split = width // 2

        box1 = (0, 0, split, height)
        img1 = pages[page_id].crop(box1)
        img1.save(f"{save_path}{page_id * 2 + 0}.png", "PNG")

        box2 = (split, 0, width, height)
        img2 = pages[page_id].crop(box2)
        img2.save(f"{save_path}{page_id * 2 + 1}.png", "PNG")
    else:
        pages[page_id].save(f"{save_path}{page_id}.png", "PNG")


def prepare_img_raw(catalog_path: str, save_path: str, multi: int, need_split: bool):
    threads_to_use = max(1, cpu_count() - 1)
    # print("Threads will be used: ", threads_to_use)

    print_to_cpp("PDF Преобразуется (это может занять несколько минут)")
    pages = convert_from_path(catalog_path, 100 * multi, thread_count=threads_to_use)

    for i in range(len(pages)):
        save_images(pages, i, save_path, need_split)

    print_to_cpp("Преобразование завершено успешно")


prepare_img_raw(CATALOG_PATH, SAVE_PATH, MULTI, NEED_SPLIT)