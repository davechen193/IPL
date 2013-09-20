#define _USE_MATH_DEFINES
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include<iostream>
#include<vector>
#include <iterator>
#include <algorithm>
#include<math.h>

using namespace cv;
using namespace boost::filesystem;
using namespace std;
vector<path> ls(string directory); //getting the file paths under a directory