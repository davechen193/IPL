#define _USE_MATH_DEFINES
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <math.h>
#include <string>
using namespace std;
using namespace cv;
using namespace boost::filesystem;

//fields
extern float tThresh; // trained Threshold.
extern vector<string> types;
extern CvSVM svm;

// constants
const int IMG_WIDTH = 2;//40;
const int IMG_HEIGHT = 60;//15;

// example.cpp
void classifier_example();

// utilities.cpp
void randomTestCreate(string typePos, string typeNeg, string directory, int nTest);
void redoRandom(string typePos, string typeNeg, string directory, int nTest);
Mat createDataMatrix2(string directory);
Mat combineData(Mat data1, Mat data2);
Mat createTestLabels(string directory, string pos_class);
//Mat createDataMatrix(string directory);
vector<path> ls(string directory, bool debug); //getting the file paths under a directory.

// svmFunctions.cpp
void svm_train(Mat trainingData, Mat trainingClasses, Mat testData, Mat testClasses);
float prediction_result(Mat testData, Mat testClasses,int Np, int Nn, bool use_indecision_boundary);
float experiment(string typePos, string typeNeg, string data_folder, int nTest, bool is_random, bool use_indecision_boundary);
float getThresh(Mat data, Mat labels, int Np, int Nn, bool debug);

