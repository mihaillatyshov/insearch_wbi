import os
import traceback

import pydantic
from base import ArgsBase, parse_args_new, print_to_cpp
from PIL import Image, ImageChops


class Args(ArgsBase):
    img_dirs: str = pydantic.Field(description="Папка исходных файлов; Можно использовать ';' для нескольких папок")
    img_crop_ref_dirs: str = pydantic.Field(
        description="Папка исходных файлов для выбора зоны обрезки; Можно использовать ';' для нескольких папок")
    save_paths: str = pydantic.Field(description="Папка для сохранения; Можно использовать ';' для нескольких папок")


def trim(im_raw, im):
    bg = Image.new(im_raw.mode, im_raw.size, (255, 255, 255))
    diff = ImageChops.difference(im_raw, bg)

    # Применяем пороговое значение (эпсилон = 5)
    def threshold(pixel):
        return 0 if pixel < 15 else pixel

    diff = diff.point(threshold)
    bbox = diff.getbbox()
    if bbox:
        return im.crop(bbox)
    return im


def process_images(args: Args):
    print_to_cpp("Начало обработки изображений...")

    img_dirs = args.img_dirs.split(';')
    save_paths = args.save_paths.split(';')
    img_crop_ref_dirs = args.img_crop_ref_dirs.split(';')
    if len(img_dirs) != len(save_paths) or len(img_crop_ref_dirs) != len(save_paths):
        raise ValueError("Количество папок с изображениями, папок для обрезки и папок для сохранения не совпадает!")

    for save_path in save_paths:
        os.makedirs(save_path, exist_ok=True)

    for img_dir, save_path, img_crop_ref_dir in zip(img_dirs, save_paths, img_crop_ref_dirs):
        print_to_cpp(f"Начало обработки изображений в папке: {img_dir}")
        for filename in os.listdir(img_dir):
            print_to_cpp(f"Просматривается файл: {filename}")
            if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif', '.webp')):
                img_path = os.path.join(img_dir, filename)
                try:
                    base_name, _ = os.path.splitext(filename)
                    raw_img_path = None
                    for f in os.listdir(img_crop_ref_dir):
                        if os.path.splitext(f)[0] == base_name:
                            raw_img_path = os.path.join(img_crop_ref_dir, f)
                            break
                    if raw_img_path is None:
                        print_to_cpp(f'Не найден файл для обрезки: {filename}')
                        continue
                    with Image.open(raw_img_path) as im_raw:
                        im_raw = im_raw.convert('RGB')
                        with Image.open(img_path) as im:
                            cropped = trim(im_raw, im)
                            save_file_path = os.path.join(save_path, filename)
                            cropped.save(save_file_path)
                            print_to_cpp(f'Файл обработан и сохранен: {save_file_path}')
                except Exception as e:                                                                                  # pylint: disable=broad-exception-caught
                    raise RuntimeError(f'Ошибка при обработке {filename}: {e}') from e


try:
    process_images(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:                                                                                                  # pylint: disable=broad-exception-caught
    formatted_traceback = traceback.format_exc()                                                                        # pylint: disable=invalid-name
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")
