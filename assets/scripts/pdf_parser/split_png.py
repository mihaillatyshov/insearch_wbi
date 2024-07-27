import cv2

img = cv2.imread('./out/png/3.png')

print(img.shape)
height = img.shape[0]
width = img.shape[1]

# Cut the image in half
width_cutoff = width // 2
s1 = img[:, :width_cutoff]
s2 = img[:, width_cutoff:]

cv2.imwrite("./out/png_split/6.png", s1)
cv2.imwrite("./out/png_split/7.png", s2)