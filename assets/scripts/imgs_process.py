import hashlib
import json
import os
import random
import shutil
import string

import pandas as pd
import pydantic
import torch
from base import (ArgsBase, log_info_to_cpp, log_space_to_cpp, log_trace_to_cpp, start_program)
# pip install -e "git+https://github.com/PramaLLC/BEN2.git#egg=ben2"
from ben2 import BEN_Base                                                                                               # type: ignore
from PIL import Image, ImageChops


class Args(ArgsBase):
    xlsx_path: str = pydantic.Field(description="Папка исходных файлов с Excel")
    xlsx_save_path: str = pydantic.Field(description="Папка для сохранения Excel файлов")
    prev_imgs_hash_and_map_filepath: str = pydantic.Field(
        description=
        "Папка с предыдущими хэшами (исходных файлов) и картой изображений (для пропуска уже обработанных изображений)")
    imgs_save_path: str = pydantic.Field(description="Папка для сохранения изображений")


class PrevImgsHashAndMapItem(pydantic.BaseModel):
    img_hash: str
    img_new_path: str


class PrevImgsHashAndMap(pydantic.BaseModel):
    imgs: dict[str, PrevImgsHashAndMapItem]


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


def sha512_org(fname: str):
    hash_sha = hashlib.sha512()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_sha.update(chunk)
    hashname = hash_sha.hexdigest()

    return hashname


def process_images(args: Args):
    log_info_to_cpp("Начало обработки изображений...")

    prev_imgs_hash_and_map: PrevImgsHashAndMap | None = None
    log_info_to_cpp(f"Открытие файла картой предыдущих изображений: '{args.prev_imgs_hash_and_map_filepath}'")
    os.makedirs(os.path.dirname(args.prev_imgs_hash_and_map_filepath), exist_ok=True)
    if os.path.isfile(args.prev_imgs_hash_and_map_filepath):
        with open(args.prev_imgs_hash_and_map_filepath, encoding="utf-8") as conf_file:
            json_data = json.load(conf_file)
            if json_data is not None:
                prev_imgs_hash_and_map = PrevImgsHashAndMap(**json_data)
            else:
                prev_imgs_hash_and_map = PrevImgsHashAndMap(imgs={})
    else:
        prev_imgs_hash_and_map = PrevImgsHashAndMap(imgs={})

    log_info_to_cpp("Удаление старых xlsx файлов с обработанными картинками и создание папок")
    shutil.rmtree(args.xlsx_save_path, ignore_errors=True)
    os.makedirs(args.xlsx_save_path, exist_ok=True)
    os.makedirs(args.imgs_save_path, exist_ok=True)

    log_trace_to_cpp("Выбор устройства для обработки изображений (удаление фона)")
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    log_trace_to_cpp(f"Используется устройство: {device}, {torch.cuda.is_available()}")

    log_info_to_cpp("Загрузка модели для удаления фона изображений")
    model = BEN_Base.from_pretrained("PramaLLC/BEN2")
    # model = AutoModel.from_pretrained("PramaLLC/BEN2", )
    model.to(device).eval()

    img_pic_set: set[str] = set()
    img_drw_set: set[str] = set()
    img_new_name_set: set[str] = set()

    log_info_to_cpp(f"Создание списка изображений из файлов в папке: {args.xlsx_path}")
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file() and not filename.name.startswith("~$"):
            log_info_to_cpp(f"Прочитан файл: {filename.name}")
            df = pd.read_excel(filename.path, dtype=str)
            for col_name, img_set in [("img_pic", img_pic_set), ("img_drw", img_drw_set)]:
                if col_name in df.columns:
                    img_set.update(df[col_name].replace('', pd.NA).dropna().astype(str))

    log_info_to_cpp("Преобразование изображений и создание карты замены имен для изображений")
    for img_set, suffix in [(img_pic_set, "_pic"), (img_drw_set, "_drw")]:
        for img_path in img_set:
            file_hash = sha512_org(img_path)
            if img_path in prev_imgs_hash_and_map.imgs and file_hash == prev_imgs_hash_and_map.imgs[img_path].img_hash:
                log_info_to_cpp(f"Пропуск обработки изображения (уже обработано ранее): {img_path}")
                continue

            log_info_to_cpp(f"Открытие изображения: {img_path}")
            image_base = Image.open(img_path)

            log_info_to_cpp(f"Удаление фона для изображения: {img_path}")
            img_extracted = model.inference(
                image_base,
                refine_foreground=True,
            ) if suffix == "_pic" else image_base

            if img_extracted is None:
                raise RuntimeError(f"[ERROR] Не удалось обработать изображение: {img_path}")

            log_info_to_cpp(f"Обрезка изображения: {img_path}")
            img_cropped = trim(image_base.convert('RGB'), img_extracted)

            new_name = generate_new_image_name(img_new_name_set)
            save_file_path = os.path.join(args.imgs_save_path, f"{new_name}.webp")
            prev_imgs_hash_and_map.imgs[img_path] = PrevImgsHashAndMapItem(
                img_hash=file_hash,
                img_new_path=save_file_path,
            )
            img_cropped.save(save_file_path, format='WEBP')

            log_info_to_cpp(f"Изображение обработано и сохранено: {save_file_path}")
            log_space_to_cpp()

    log_info_to_cpp(f"Сохранение обработанных xlsx файлов в папку: {args.xlsx_save_path}")
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file() and not filename.name.startswith("~$"):
            log_info_to_cpp(f"Прочитан файл: {filename.name}")
            df = pd.read_excel(filename.path, dtype=str)
            for col_name in ["img_pic", "img_drw"]:
                if col_name in df.columns:
                    df[col_name] = df[col_name].map(lambda x: prev_imgs_hash_and_map.imgs[x].img_new_path
                                                    if pd.notna(x) else x)

            save_path = os.path.join(args.xlsx_save_path, filename.name)

            writer = pd.ExcelWriter(save_path, engine="xlsxwriter")                                                     # pylint: disable=abstract-class-instantiated
            df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 0), index=False)
            worksheet = writer.sheets["sm"]
            worksheet.autofit()
            writer.close()

            log_info_to_cpp(f"Файл сохранен: {save_path}")

    log_info_to_cpp("Сохранение файла с картой предыдущих изображений")
    # print(os.access(args.prev_imgs_hash_and_map_filepath, os.W_OK))
    with open(args.prev_imgs_hash_and_map_filepath, "w", encoding="utf-8") as conf_file:
        conf_file.write(prev_imgs_hash_and_map.model_dump_json(indent=4))
    log_info_to_cpp("Файл сохранен.")

    log_info_to_cpp("Обработка завершена успешно.")


if __name__ == "__main__":
    start_program(process_images, Args)
