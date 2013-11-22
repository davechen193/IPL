#include "classifier.h"

vector<path> ls(string directory, bool debug=false)
{    
    typedef vector<path> vec;             // store paths,
    vec v;              // so we can sort them later
    path p (directory);   // abbreviate a long word.
    try{
        if (exists(p)){    // does p actually exist?
          if (is_regular_file(p)){        // is p a regular file?
            cout << p << " size is " << file_size(p) << '\n';
          }else if (is_directory(p)){      // is p a directory?
            if(debug) cout << p << " is a directory containing:\n";
            
            copy(directory_iterator(p), directory_iterator(), back_inserter(v));
            sort(v.begin(), v.end());             // sort, since directory iteration
                                                  // is not ordered on some file systems
            for (vec::const_iterator it (v.begin()); it != v.end(); ++it){
                if(debug) cout << "   " << *it << '\n';
            }
          }
          else
            cout << p << " exists, but is neither a regular file nor a directory\n";
        }
        else
          cout << p << " does not exist\n";
      }

      catch (const filesystem_error& ex)
      { 
        cout << ex.what() << '\n';
      }
      return v;
}

Mat createDataMatrix2(string directory){
    vector<path> dataPaths = ls(directory);
    int nData = dataPaths.size();
    int data_size = IMG_WIDTH * IMG_HEIGHT;
    Mat dataMatrix(nData,data_size, CV_32F);
    for (int i=0 ; i<nData ; i++) {
        path p = dataPaths[i];
        FILE* file = fopen(p.string().c_str(), "r");
        if(IMG_WIDTH != 2 & IMG_HEIGHT != 60){
            cout<<"Please recheck the size set for a single data frame." << endl;
        }
        for(int y = 0; y < IMG_HEIGHT; y++) {
           float row[2] = {0};
           fscanf(file, "%f\t%f\n", &row[0], &row[1]);
           dataMatrix.at<float>(i, y*IMG_WIDTH) = row[0];
           dataMatrix.at<float>(i, y*IMG_WIDTH+1) = row[1];   
        }
        fclose(file);
    }
    return dataMatrix;
}

//Mat createDataMatrix(string directory){
//
//    vector<path> dataPaths = ls(directory);
//    int nData = dataPaths.size();
//    int data_size = IMG_WIDTH * IMG_HEIGHT;
//
//    Mat dataMatrix(nData,data_size, CV_32F);
//    for (int i=0 ; i<nData ; i++){
//        path p = dataPaths[i];
//        Mat image = imread(p.string(), CV_LOAD_IMAGE_GRAYSCALE);
//        Size s = Size(IMG_WIDTH,IMG_HEIGHT);
//        resize(image,image,s);
//        Canny(image,image,100,100);
//        Sobel(image, image, CV_8UC1, 1, 0, 3);
//        for(int y = 0; y < image.rows; y++){
//           for(int x = 0; x < image.cols; x++){
//               dataMatrix.at<float>(i, y*image.cols+x) = image.at<uchar>(y,x);
//           }
//        }
//    }
//    return dataMatrix;
//}

Mat createTestLabels(string directory, string pos_class){
    vector<path> dataPaths = ls(directory);
    int nData = dataPaths.size();
    int test_length = pos_class.length();

    Mat classLabels = Mat::ones(nData,1,CV_32F);

    for(int i=0 ; i<nData ; i++){
        string fileName = dataPaths[i].filename().string();
        if(fileName.substr(0,test_length) == pos_class){
           classLabels.at<float>(i,0)= 1;
        }else{
           classLabels.at<float>(i,0)= -1;
        }
    }
    return classLabels;
}

Mat combineData(Mat data1, Mat data2){
    if(data1.cols!= data2.cols){
        cout << "featureDimensions don't match!" << endl;
    }
    int featureDim = data1.cols;

    Mat result(data1.rows+data2.rows , featureDim, CV_32F);
    for (int y = 0; y < data1.rows+data2.rows; y++){
        for (int x = 0; x < featureDim; x++) {
            if (y < data1.rows) {
                result.at<float>(y,x) = data1.at<float>(y,x);
            }
            else {
                result.at<float>(y,x) = data2.at<float>(y-data1.rows,x);
            }
        }    
    }
    return result;
}

void randomTestCreate(string typePos, string typeNeg, string directory, int nTest){
    vector<path> dataPos = ls(directory+typePos,false);
    vector<path> dataNeg = ls(directory+typeNeg,false);
    String testDir = directory + "test";
    int nPos = dataPos.size();
    int nNeg = dataNeg.size();
    if(nTest > nPos/2 || nTest > nNeg/2){
        cout << "number of training data is not sufficient!";
    }
    srand ( time(NULL) );
    for (int i=0 ; i< nTest ; i++){
        //copy the data to test folder.
        int iPos = rand() % dataPos.size();
        int iNeg = rand() % dataNeg.size();
        path srcPos = dataPos[iPos];
        path srcNeg = dataNeg[iNeg];
        copy_file(srcPos.string(),testDir+"\\"+srcPos.filename().string());
        copy_file(srcNeg.string(),testDir+"\\"+srcNeg.filename().string());
        //delete the data in the train folder.
        remove(srcPos.string());
        remove(srcNeg.string());
        dataPos.erase(dataPos.begin()+iPos);
        dataNeg.erase(dataNeg.begin()+iNeg);
    }
}

void redoRandom(string typePos, string typeNeg, string directory, int nTest){
    string testDir = directory+"test";
    string type1, type2;
    vector<path> dataTest = ls(testDir, false);
    
    if(dataTest[0].filename().string().substr(0,typePos.length()) == typePos){
        type1 = typePos;
        type2 = typeNeg;
    }else{
        type1 = typeNeg;
        type2 = typePos;
    }
        
    for(int i=0 ; i< nTest ; i++){
        path src = dataTest[i];
        copy_file(src.string(),directory+type1+"\\"+src.filename().string());
        //delete the data in the train folder.
        remove(src.string());
    }
        for(int i=nTest ; i< 2*nTest ; i++){
        path src = dataTest[i];
        copy_file(src.string(),directory+type2+"\\"+src.filename().string());
        //delete the data in the train folder.
        remove(src.string());
    }
}