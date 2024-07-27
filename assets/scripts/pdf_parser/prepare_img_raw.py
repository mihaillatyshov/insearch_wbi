from pdf2image import convert_from_path
from psutil import cpu_count
from PIL import Image

from shared import get_img_path, get_img_preview


def save_images(pages: list[Image.Image], page_id, need_split: bool):
    print(f"Start save img raw ({page_id})")
    img = pages[page_id]
    if need_split:
        width, height = img.size
        split = width // 2

        box1 = (0, 0, split, height)
        img1 = pages[page_id].crop(box1)
        img1.save(get_img_path(page_id * 2 + 0), "PNG")

        box2 = (split, 0, width, height)
        img2 = pages[page_id].crop(box2)
        img2.save(get_img_path(page_id * 2 + 1), "PNG")
    else:
        pages[page_id].save(get_img_path(page_id), "PNG")

    print("End save img")


def prepare_img_raw(multi: int, need_split: bool):
    threads_to_use = max(1, cpu_count() - 1)
    print("Threads will be used: ", threads_to_use)

    print("Start converting from path (this take a while)")
    pages = convert_from_path("./pdf/5_Amati_Каталог_07.08.2023.pdf", 100 * multi, thread_count=threads_to_use)
    print("End converting from path")

    for i in range(len(pages)):
        save_images(pages, i, need_split)


# def prepare_img_preview(pages: list[Image.Image], page_id):
#     print(f"Start save img preview ({page_id})")
#     pages[page_id].save(get_img_preview(page_id), "JPEG")
#     print("End save img")

# print("Start converting from path (this take a while)")
# pages = convert_from_path("./pdf/5_Amati_Каталог_07.08.2023.pdf", 100 * MULTI, thread_count=threads_to_use)
# print("End converting from path")

# for i in range(len(pages)):
#     prepare_img_raw(pages, i)

# print("Start converting from path")
# pages = convert_from_path("./Catalog-2023.pdf", 100)
# print("End converting from path")

# for i in range(len(pages)):
#     prepare_img_preview(pages, i)
