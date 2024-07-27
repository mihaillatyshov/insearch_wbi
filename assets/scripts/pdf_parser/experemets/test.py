import pytesseract
from matplotlib import pyplot as plt
from matplotlib.patches import Rectangle
import cv2
from matplotlib import image as mpimg
from img2table.document import Image

from TabelExtractor import TableExtractor

pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'

IMG_SRC = "./out/png_split/6.png"

IMG_TEST = "./out/test_a_3.png"

imgcv = cv2.imread(IMG_SRC)

imgcv = cv2.cvtColor(imgcv, cv2.COLOR_BGR2GRAY)
imgcv = cv2.GaussianBlur(imgcv, (3, 3), 0)
imgcv = cv2.threshold(imgcv, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)[1]

kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
imgcv = cv2.morphologyEx(imgcv, cv2.MORPH_OPEN, kernel, iterations=1)
imgcv = 255 - imgcv

cv2.imwrite(IMG_TEST, imgcv)

# te = TableExtractor(IMG_SRC)
# te.execute()

# ================================================================================================================
# ================================================================================================================
# ================================================================================================================

imgcv = cv2.imread(IMG_SRC)
template1 = cv2.imread("./test/pattern_a_TL.png")
template2 = cv2.imread("./test/pattern_a_BR.png")

imgcv2 = cv2.Canny(imgcv, 0, 10)
cv2.imwrite(IMG_TEST, imgcv2)

result = cv2.matchTemplate(imgcv, template1, cv2.TM_CCORR_NORMED)
(_, maxVal, _, maxLoc) = cv2.minMaxLoc(result)
print(maxVal, maxLoc)
print("Shape1:", template1.shape)
imgcv = cv2.circle(imgcv, maxLoc, radius=10, color=(255, 0, 0), thickness=-1)

result = cv2.matchTemplate(imgcv, template2, cv2.TM_CCORR_NORMED)
(_, maxVal, _, maxLoc) = cv2.minMaxLoc(result)
print(maxVal, maxLoc)
print("Shape2:", template2.shape)
x = maxLoc[0] + template2.shape[1]
y = maxLoc[1] + template2.shape[0] // 4 * 3
imgcv = cv2.circle(imgcv, (x, y), radius=10, color=(255, 0, 0), thickness=-1)

cv2.imwrite("./out/test_a_3_circle.png", imgcv)

# Instantiation of the image
img = Image(src=IMG_TEST)
img_tables = img.extract_tables()

   # ================================================================================================================
   # ================================================================================================================
   # ================================================================================================================

   # fig, ax = plt.subplots()
   # fig.subplots_adjust(left=0.3)
   # ax.set_xlabel("X pixel scaling")
   # ax.set_ylabel("Y pixels scaling")
   # image = mpimg.imread(IMG_SRC)
   # ax.set_title("6.png")
   # ax_img = ax.imshow(image)

   # rects = []

   # for table in img_tables:
   #     print("\n===============================================================")
   #     print(table)
   #     rects.append(
   #         ax.add_patch(
   #             Rectangle((table.bbox.x1, table.bbox.y1),
   #                       table.bbox.x2 - table.bbox.x1,
   #                       table.bbox.y2 - table.bbox.y1,
   #                       edgecolor='blue',
   #                       facecolor='none',
   #                       lw=1)))

   # plt.show()
