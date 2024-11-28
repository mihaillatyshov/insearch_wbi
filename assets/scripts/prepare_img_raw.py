import os

from pdf2image import convert_from_bytes, pdfinfo_from_bytes
from PIL import Image

from psutil import cpu_count                                                                                            # type: ignore

from base import print_to_cpp, file_format_img, ArgsBase, parse_args

PREV_MAX_SIZE = 2560, 1440


class Args(ArgsBase):
    catalog_path: str
    save_path: str
    prev_save_path: str
    multi: int
    need_split: bool


# if (len(sys.argv) < 5):
#     print_to_cpp("Скрипт не выполнен (не хватает агрументов)")
#     exit(-1)

# try:
#     CATALOG_PATH = str(sys.argv[1])
#     SAVE_PATH = str(sys.argv[2])
#     MULTI = int(sys.argv[3])
#     NEED_SPLIT = bool(int(sys.argv[4]))
# except Exception:
#     print_to_cpp("Аргументы скрипта имеют ошибки!")
#     exit(-1)


def save_images(img: Image.Image, page_id: int, save_path: str, prev_save_path: str, need_split: bool):
    print_to_cpp(f"Сохраняется страница ({page_id})")
    if need_split:
        width, height = img.size
        split = width // 2

        box1 = (0, 0, split, height)
        img1 = img.crop(box1)
        img1.save(os.path.join(save_path, file_format_img(page_id * 2 + 0)), "PNG")

        img1.thumbnail(PREV_MAX_SIZE, Image.Resampling.LANCZOS)
        img1.save(os.path.join(prev_save_path, file_format_img(page_id * 2 + 0)), "PNG")

        box2 = (split, 0, width, height)
        img2 = img.crop(box2)
        img2.save(os.path.join(save_path, file_format_img(page_id * 2 + 1)), "PNG")

        img2.thumbnail(PREV_MAX_SIZE, Image.Resampling.LANCZOS)
        img2.save(os.path.join(prev_save_path, file_format_img(page_id * 2 + 1)), "PNG")

    else:
        img.save(os.path.join(save_path, file_format_img(page_id)), "PNG")

        img.thumbnail(PREV_MAX_SIZE, Image.Resampling.LANCZOS)
        img.save(os.path.join(prev_save_path, file_format_img(page_id)), "PNG")


def prepare_img_raw(args: Args):
    threads_to_use = max(1, cpu_count() - 1)
    print_to_cpp("PDF Преобразуется (это может занять несколько минут)")

    pdf_file = open(args.catalog_path, "rb")
    pdf_bytes = pdf_file.read()
    pdf_info = pdfinfo_from_bytes(pdf_bytes)
    # * print("pdfinfo_from_bytes: ", pdfinfo_from_bytes(pdf_bytes))
    start_page = 1                                                                                                      # * pdfinfo_from_bytes["Page_rot"] ???

    for i in range(pdf_info["Pages"]):
        print_to_cpp(f"Конвертируется страница ({i})")
        img = convert_from_bytes(pdf_bytes,
                                 100 * args.multi,
                                 thread_count=threads_to_use,
                                 first_page=start_page + i,
                                 last_page=start_page + i + 1)[0]

        save_images(img, i, args.save_path, args.prev_save_path, args.need_split)

    print_to_cpp("Преобразование завершено успешно")


prepare_img_raw(parse_args(Args))
