import os
import sys
import traceback

import cv2
import pydantic
import torch
from base import ArgsBase, parse_args_new, print_to_cpp
from ben2 import BEN_Base                                                                                               # type: ignore
from PIL import Image

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

# print(torch.version.cuda)
# print(torch.cuda.is_available())
print_to_cpp(f"Используется устройство: {device}, {torch.cuda.is_available()}")

# sys.exit(0)

# pip install -e "git+https://github.com/PramaLLC/BEN2.git#egg=ben2"
model = BEN_Base.from_pretrained("PramaLLC/BEN2")
# model = AutoModel.from_pretrained("PramaLLC/BEN2", )
model.to(device).eval()


class Args(ArgsBase):
    img_dirs: str = pydantic.Field(description="Папка исходных файлов; Можно использовать ';' для нескольких папок")
    save_paths: str = pydantic.Field(description="Папка для сохранения; Можно использовать ';' для нескольких папок")


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
            # Проверяем по суффиксу в имени файла (без расширения)
            lower_filename = filename.lower()
            stem = os.path.splitext(lower_filename)[0]

            if lower_filename.endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif')) and stem.endswith("_pic"):
                print_to_cpp(f"Обрабатывается pic файл: {filename}")
                img_path = os.path.join(img_dir, filename)

                image = Image.open(img_path)

                extracted_img = model.inference(
                    image,
                    refine_foreground=True,
                )

                if extracted_img is not None:
                    name, _ = os.path.splitext(filename)
                    extracted_img.save(os.path.join(save_path, f"{name}.png"))
            elif lower_filename.endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif')) and stem.endswith("_drw"):
                print_to_cpp(f"Обрабатывается drw файл: {filename}")
                img_path = os.path.join(img_dir, filename)
                name, _ = os.path.splitext(filename)
                file_read = cv2.imread(img_path)
                if file_read is None:
                    raise ValueError(f"Не удалось прочитать файл изображения: {img_path}")
                cv2.imwrite(os.path.join(save_path, f"{name}.png"), file_read)


try:
    process_images(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:                                                                                                  # pylint: disable=broad-exception-caught
    formatted_traceback = traceback.format_exc()                                                                        # pylint: disable=invalid-name
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")

# exc_type, exc_value, exc_traceback = sys.exc_info()
