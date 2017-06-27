import argparse as ap
import os
import numpy as np
import subprocess
from sklearn.svm import SVC
from sklearn.model_selection import KFold
from sklearn.model_selection import StratifiedKFold
from sklearn.model_selection import GridSearchCV

def readFeatures(imageName, paralFeatures):
  if paralFeatures:
    x = subprocess.Popen(['./paral_hist', imageName], stdout=subprocess.PIPE).communicate()[0]
  else:
    x = subprocess.Popen(['./serial_hist', imageName], stdout=subprocess.PIPE).communicate()[0]
  x = np.array(x.split()).astype(np.float)
  return x

def preprocess(filePath, paralFeatures = False):
  trainingClasses = os.listdir(filePath)
  x = []
  y = []
  classLabel = 0
  for person in trainingClasses:
    imageNames = os.listdir(filePath + '/' + person)
    for imageName in imageNames:
      feature = readFeatures(filePath + '/' + person + '/' + imageName, paralFeatures)
      x.append(feature)
      y.append(classLabel)
    classLabel += 1
  x = np.array(x)
  y = np.array(y)
  return x, y

def faceRecognition(x, y):
  print "Face Recognition"
  params = {'kernel':['rbf', 'linear'], 'C' : [2**(-5), 2**(0), 2**(5), 2**(10)], 'gamma': [2**(-15), 2**(-10), 2**(-5), 2**(0), 2**(5)]}
  #create stratified k-folds
  kf = StratifiedKFold(n_splits = 5, random_state = 1)
  kf.get_n_splits(x, y)

  accuracy_mean = 0
  for train_index, test_index in kf.split(x, y):
    #set train and test data
    x_train, x_test = x[train_index], x[test_index]
    y_train, y_test = y[train_index], y[test_index]

    #run gridSearch
    gridSearch = GridSearchCV(SVC(random_state = 1), params, cv = 3, n_jobs = -1)
    gridSearch.fit(x_train, y_train)

    #create model with best params
    clf = SVC(**gridSearch.best_params_).fit(x_train, y_train)

    #test created model
    nested_accuracy = clf.score(x_test, y_test)
    accuracy_mean += nested_accuracy
    print "\tC: ",gridSearch.best_params_, "nested acc:", nested_accuracy

  print "Mean acc: %.5f" % (accuracy_mean/5.0)

if __name__ == '__main__':
  #set flag for parallel feature extraction
  parser = ap.ArgumentParser()
  parser.add_argument("-p", "--parallel", default = "false", help="Use parallel computation")
  args = vars(parser.parse_args())

  if args['parallel'] == "false":
    paralFeatures = False
    print "Not using parallel features"
  else:
    paralFeatures = True
    print "Using parallel features"


  filePath = 'orl_faces'
  x, y = preprocess(filePath, paralFeatures)
  print y
  faceRecognition(x, y)
'''
OUTPUT:
Face Recognition
	C:  {'kernel': 'rbf', 'C': 1024, 'gamma': 32} nested acc: 0.9625
	C:  {'kernel': 'rbf', 'C': 1024, 'gamma': 32} nested acc: 0.925
	C:  {'kernel': 'rbf', 'C': 1024, 'gamma': 32} nested acc: 0.9625
	C:  {'kernel': 'rbf', 'C': 1024, 'gamma': 32} nested acc: 0.9375
	C:  {'kernel': 'rbf', 'C': 1024, 'gamma': 32} nested acc: 0.975
Mean acc: 0.95250

'''
