import os
import sys

import cv2
from cv2.typing import Point

from base import print_to_cpp

if (len(sys.argv) < 6):
    print_to_cpp("Скрипт не выполнен (не хватает агрументов)")
    exit(-1)

try:
    RAW_IMG_PATH = str(sys.argv[1])
    PATTERN_1 = str(sys.argv[2])
    PATTERN_2 = str(sys.argv[3])
    SAVE_PATH = str(sys.argv[4])
    TRESHOLD = float(sys.argv[5])
except Exception:
    print_to_cpp("Аргументы скрипта имеют ошибки!")
    exit(-1)


def find_point(img, pattern) -> tuple[float, Point]:
    result = cv2.matchTemplate(img, pattern, cv2.TM_CCORR_NORMED)
    (_, max_val, _, max_loc) = cv2.minMaxLoc(result)
    return (max_val, max_loc)


def split_img(img_path: os.DirEntry[str], pattern_1, pattern_2, save_path: str, treshold: float):
    print_to_cpp(f"Обрезается файл: {img_path.name}")
    imgcv = cv2.imread(img_path.path)

    max_val_1, max_loc_1 = find_point(imgcv, pattern_1)
    max_val_2, max_loc_2 = find_point(imgcv, pattern_2)
    print(max_val_1, max_val_2)

    min_x = min(max_loc_1[0], max_loc_2[0])
    min_y = min(max_loc_1[1], max_loc_2[1])

    max_x = min(max(max_loc_1[0] + pattern_1.shape[1], max_loc_2[0] + pattern_2.shape[1]), imgcv.shape[1])
    max_y = min(max(max_loc_1[1] + pattern_1.shape[0], max_loc_2[1] + pattern_2.shape[0]), imgcv.shape[0])

    res_img = imgcv[min_y:max_y, min_x:max_x]

    res_img = cv2.cvtColor(res_img, cv2.COLOR_BGR2GRAY)
    res_img = cv2.GaussianBlur(res_img, (3, 3), 0)
    res_img = cv2.threshold(res_img, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)[1]

    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
    res_img = cv2.morphologyEx(res_img, cv2.MORPH_OPEN, kernel, iterations=1)
    res_img = 255 - res_img

    cv2.imwrite(save_path + img_path.name, res_img)

    if max_val_1 < treshold or max_val_2 < treshold:
        return False

    return True


def split_by_pattern(raw_img_path: str, pattern_path_1: str, pattern_path_2: str, save_path: str, treshold: float):
    result: list[str] = []

    pattern_1 = cv2.imread(pattern_path_1)
    pattern_2 = cv2.imread(pattern_path_2)
    for filename in os.scandir(raw_img_path):
        if filename.is_file():
            if (not split_img(filename, pattern_1, pattern_2, save_path, treshold)):
                print_to_cpp(f"Есть неточности в: {filename.name}")
                result.append(filename.name.split(".")[0])

    print_to_cpp(" ".join(result))


#  python .\cut_by_pattern.py C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/raw_img/ C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/patterns/cut_1.png C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/patterns/cut_2.png C:/Coding/works/wbi/insearch_wbi/assets/projects/2023_08_15__18_31_58/cut_img/ 0.999
split_by_pattern(RAW_IMG_PATH, PATTERN_1, PATTERN_2, SAVE_PATH, TRESHOLD)