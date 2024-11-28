import os
from PIL import Image

from base import print_to_cpp, file_format_img, ArgsBase, parse_args


class Args(ArgsBase):
    in_path: str
    save_path: str


def convert_img(args: Args):
    print_to_cpp("Img Преобразуется (это может занять несколько минут)")

    for filename in os.scandir(args.in_path):
        if filename.is_file():
            print_to_cpp(filename.path)
            image = Image.open(filename.path)
            image.save(os.path.join(args.save_path, ".".join(filename.name.split(".")[:-1]) + ".webp"), 'webp')

    print_to_cpp("Преобразование завершено успешно")


convert_img(parse_args(Args))
