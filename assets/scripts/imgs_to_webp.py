import os
import traceback

import pydantic
from base import ArgsBase, parse_args_new, print_to_cpp
from PIL import Image


class Args(ArgsBase):
    img_dirs: str = pydantic.Field(description="Папка исходных файлов; Можно использовать ';' для нескольких папок")
    save_paths: str = pydantic.Field(description="Папка для сохранения; Можно использовать ';' для нескольких папок")


def convert_image_to_webp(input_filename: str, img_dir: str, save_path: str):
    with Image.open(os.path.join(img_dir, input_filename)) as img:
        img.save(os.path.join(save_path, f"{os.path.splitext(input_filename)[0]}.webp"), format='WEBP')


def process_images(args: Args):
    print_to_cpp("Начало обработки изображений...")

    img_dirs = args.img_dirs.split(';')
    save_paths = args.save_paths.split(';')
    if len(img_dirs) != len(save_paths):
        raise ValueError("Количество папок с изображениями и папок для сохранения не совпадает!")

    for save_path in save_paths:
        os.makedirs(save_path, exist_ok=True)

    for img_dir, save_path in zip(img_dirs, save_paths):
        print_to_cpp(f"Начало обработки изображений в папке: {img_dir}")
        for filename in os.listdir(img_dir):
            print_to_cpp(f"Просматривается файл: {filename}")
            if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif')):
                try:
                    convert_image_to_webp(filename, img_dir, save_path)
                    print_to_cpp(f"Файл обработан: {filename}")
                except Exception as e:                                                                                  # pylint: disable=broad-exception-caught
                    print(f'Error processing {filename}: {e}')


try:
    process_images(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:                                                                                                  # pylint: disable=broad-exception-caught
    formatted_traceback = traceback.format_exc()                                                                        # pylint: disable=invalid-name
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")

# exc_type, exc_value, exc_traceback = sys.exc_info()
