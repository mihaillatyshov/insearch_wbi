import os
import random
import shutil
import string
import traceback

import pandas as pd
import pydantic
import torch
from base import ArgsBase, parse_args_new, print_to_cpp
# pip install -e "git+https://github.com/PramaLLC/BEN2.git#egg=ben2"
from ben2 import BEN_Base                                                                                               # type: ignore
from PIL import Image, ImageChops


class Args(ArgsBase):
    xlsx_path: str = pydantic.Field(description="Папка исходных файлов с Excel")
    xlsx_save_path: str = pydantic.Field(description="Папка для сохранения Excel файлов")
    imgs_save_path: str = pydantic.Field(description="Папка для сохранения изображений")


def trim(im_raw, im):
    bg = Image.new(im_raw.mode, im_raw.size, (255, 255, 255))
    diff = ImageChops.difference(im_raw, bg)

    # Применяем пороговое значение (эпсилон = 15)
    def threshold(pixel):
        return 0 if pixel < 15 else pixel

    diff = diff.point(threshold)
    bbox = diff.getbbox()
    if bbox:
        return im.crop(bbox)
    return im


def generate_new_image_name(img_new_name_set: set[str]) -> str:
    while True:
        new_name = ''.join(random.choices(string.ascii_letters + string.digits, k=64))
        if new_name not in img_new_name_set:
            img_new_name_set.add(new_name)
            return new_name


def process_images(args: Args):
    print_to_cpp("Начало обработки изображений...")

    print_to_cpp("Отчистка и создание папок для сохранения")
    os.makedirs(args.xlsx_save_path, exist_ok=True)
    shutil.rmtree(args.imgs_save_path, ignore_errors=True)
    os.makedirs(args.imgs_save_path, exist_ok=True)

    print_to_cpp("Выбор устройства для обработки изображений (удаление фона)")
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print_to_cpp(f"Используется устройство: {device}, {torch.cuda.is_available()}")

    print_to_cpp("Загрузка модели для удаления фона изображений")
    model = BEN_Base.from_pretrained("PramaLLC/BEN2")
    # model = AutoModel.from_pretrained("PramaLLC/BEN2", )
    model.to(device).eval()

    img_pic_set: set[str] = set()
    img_drw_set: set[str] = set()
    img_new_name_set: set[str] = set()
    img_replace_map: dict[str, str] = {}

    print_to_cpp(f"Создание списка изображений из файлов в папке: {args.xlsx_path}")
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file() and not filename.name.startswith("~$"):
            print_to_cpp(f"Прочитан файл: {filename.name}")
            df = pd.read_excel(filename.path, dtype=str)
            for col_name, img_set in [("img_pic", img_pic_set), ("img_drw", img_drw_set)]:
                if col_name in df.columns:
                    img_set.update(df[col_name].replace('', pd.NA).dropna().astype(str))

    print_to_cpp("Преобразование изображений и создание карты замены имен для изображений")
    for img_set, suffix in [(img_pic_set, "_pic"), (img_drw_set, "_drw")]:
        for img_path in img_set:
            print_to_cpp(f"Открытие изображения: {img_path}")
            image_base = Image.open(img_path)

            print_to_cpp(f"Удаление фона для изображения: {img_path}")
            img_extracted = model.inference(
                image_base,
                refine_foreground=True,
            ) if suffix == "_pic" else image_base

            if img_extracted is None:
                raise RuntimeError(f"[ERROR] Не удалось обработать изображение: {img_path}")

            print_to_cpp(f"Обрезка изображения: {img_path}")
            img_cropped = trim(image_base.convert('RGB'), img_extracted)

            new_name = generate_new_image_name(img_new_name_set)
            save_file_path = os.path.join(args.imgs_save_path, f"{new_name}.webp")
            img_replace_map[img_path] = save_file_path
            img_cropped.save(save_file_path, format='WEBP')

            print_to_cpp(f"Изображение обработано и сохранено: {save_file_path}")

    for filename in os.scandir(args.xlsx_path):
        if filename.is_file() and not filename.name.startswith("~$"):
            print_to_cpp(f"Прочитан файл: {filename.name}")
            df = pd.read_excel(filename.path, dtype=str)
            for col_name in ["img_pic", "img_drw"]:
                if col_name in df.columns:
                    df[col_name] = df[col_name].map(lambda x: img_replace_map.get(x, x) if pd.notna(x) else x)

            save_path = os.path.join(args.xlsx_save_path, filename.name)

            writer = pd.ExcelWriter(save_path, engine="xlsxwriter")                                                     # pylint: disable=abstract-class-instantiated
            df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 0), index=False)
            worksheet = writer.sheets["sm"]
            worksheet.autofit()
            writer.close()


try:
    process_images(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:                                                                                                  # pylint: disable=broad-exception-caught
    formatted_traceback = traceback.format_exc()                                                                        # pylint: disable=invalid-name
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")

# exc_type, exc_value, exc_traceback = sys.exc_info()
