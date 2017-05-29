import cv2 #opencv must be installed
import os

def convertImage(imageName):
  #name of the input image
  img_input_name = imageName
  #name of the ouput image
  img_out_name = imageName[:imageName.rfind('.')]+ ".ppm"

  img = cv2.imread(img_input_name)
  cv2.imwrite(img_out_name, img)
  print "Done ... out file in", img_out_name

def convertORL_faces(filePath):
  trainingClasses = os.listdir(filePath)
  trainingClasses.sort()
  for person in trainingClasses:
    imageNames = os.listdir(filePath + '/' + person)
    print '\t', filePath + '/' + person
    for imageName in imageNames:
      print '\t', filePath + '/' + person + '/' + imageName
      convertImage(filePath + '/' + person + '/' + imageName)
      os.remove(filePath + '/' + person + '/' + imageName)

convertORL_faces('orl_faces')
