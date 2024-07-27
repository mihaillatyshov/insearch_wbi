import cv2

from shared import get_img_cv_path, get_img_path, load_config, get_boundaries

config = load_config()


def prepare_img_cv(page_id) -> bool:
    print(f"Start read img {page_id}")
    imgcv = cv2.imread(get_img_path(page_id))
    if imgcv is None:
        return True
    # print("Start fix color")
    # imgcv = cv2.cvtColor(imgcv, cv2.COLOR_BGR2RGB)
    skip_first, skip_second, start_x_first, end_x_first, start_y_first, end_y_first, start_x_second, end_x_second, start_y_second, end_y_second = get_boundaries(
        page_id)

    imgcv = cv2.cvtColor(imgcv, cv2.COLOR_BGR2GRAY)
    imgcv = cv2.GaussianBlur(imgcv, (3, 3), 0)
    imgcv = cv2.threshold(imgcv, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)[1]

    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
    imgcv = cv2.morphologyEx(imgcv, cv2.MORPH_OPEN, kernel, iterations=1)
    imgcv = 255 - imgcv

    if not skip_first:
        print(f"Start Split {page_id * 2 + 0}")
        imgcv1 = imgcv[start_y_first:end_y_first, start_x_first:end_x_first]
        print("Start write cv img")
        cv2.imwrite(get_img_cv_path(page_id * 2 + 0), imgcv1)
    if not skip_second:
        print(f"Start Split {page_id * 2 + 1}")
        imgcv2 = imgcv[start_y_second:end_y_second, start_x_second:end_x_second]
        print("Start write cv img")
        cv2.imwrite(get_img_cv_path(page_id * 2 + 1), imgcv2)

    return False


i = 0
while not prepare_img_cv(i):
    i += 1
