import os
import cv2
import pytesseract

from shared import get_img_cv_path

pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'

dir_path = r'./out/png_cv'


def parse_page(filename: str):
    try:
        print("Start read img")
        imgcv = cv2.imread(get_img_cv_path(int(filename)))

        print("Start converting to string")
        print("=============================================================")
        text = pytesseract.image_to_string(imgcv, lang='eng', config='--psm 6')                                         #+rus
        print(text)
        with open(f"./out/text/{filename}.txt", "w") as file:
            file.write(text)
    except KeyboardInterrupt:
        exit()
    except Exception as e:
        print(e)


# [os.path.isfile(os.path.join(dir_path, path)) for path in os.listdir(dir_path)]

# for path in os.listdir(dir_path):
#     if os.path.isfile(os.path.join(dir_path, path)):
#         parse_page(path.split(".")[0])

parse_page("22")
parse_page("23")

# for i in range(len(pages)):
#     parse_page(pages, i)
