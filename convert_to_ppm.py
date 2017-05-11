import cv2 #opencv must be installed
#name of the input image
img_input_name = "images/tmp.jpg"
#name of the ouput image
img_out_name = "images/tmp.ppm"

img = cv2.imread(img_input_name)
cv2.imwrite(img_out_name, img)
print "Done ... out file in", img_out_name 
