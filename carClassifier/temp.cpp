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
    vector<path> dataTest = ls(testDir, true);
    
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

Mat createDataMatrix2(string directory){
    vector<path> dataPaths = ls(directory);
    int nData = dataPaths.size();
    int data_size = IMG_WIDTH * IMG_HEIGHT;

    Mat dataMatrix(nData,data_size, CV_32F);
    for (int i=0 ; i<nData ; i++) {
        path p = dataPaths[i];
        FILE* file = fopen(p.string().c_str(), "r");
        for(int y = 0; y < IMG_HEIGHT; y++) {
           float row[2] = {0};
           fscanf(file, "%f\t%f\n", &row[0], &row[1]);
           dataMatrix.at<float>(i, y*IMG_WIDTH  ) = row[0];
           dataMatrix.at<float>(i, y*IMG_WIDTH+1) = row[1];
        }
        fclose(file);
    }
    return dataMatrix;
}

Mat createDataMatrix(string directory){

    vector<path> dataPaths = ls(directory);
    int nData = dataPaths.size();
    int data_size = IMG_WIDTH * IMG_HEIGHT;

    Mat dataMatrix(nData,data_size, CV_32F);
    for (int i=0 ; i<nData ; i++){
        path p = dataPaths[i];
        Mat image = imread(p.string(), CV_LOAD_IMAGE_GRAYSCALE);
        Size s = Size(IMG_WIDTH,IMG_HEIGHT);
        resize(image,image,s);
        //Canny(image,image,100,100);
        Sobel(image, image, CV_8UC1, 1, 0, 3);
        for(int y = 0; y < image.rows; y++){
           for(int x = 0; x < image.cols; x++){
               dataMatrix.at<float>(i, y*image.cols+x) = image.at<uchar>(y,x);
           }
        }
    }
    return dataMatrix;
}

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

// accuracy
float evaluate(cv::Mat& predicted, cv::Mat& actual) {
    assert(predicted.rows == actual.rows);
    int t = 0;
    int f = 0;
    for(int i = 0; i < actual.rows; i++) {
        float p = predicted.at<float>(i,0);
        float a = actual.at<float>(i,0);
        if((p > 0.0 && a > 0.0) || (p <= 0.0 &&  a <= 0.0)) {
            t++;
        } else {
            f++;
        }
    }
    return (t * 1.0) / (t + f);
}

//Np: number of positives.
//Nn: number of Negatives.
//float getThresh(Mat data, Mat labels, int Np, int Nn, bool debug){
//    Mat decVal(labels.size(), CV_32F);
//    float idThresh;
//    float fMin = 1e3;
//    float fMax = 0;
//    int N = labels.rows;
//    for(int i = 0; i < N; ++i){
//        Mat instance = data.row(i);
//        float f = svm.predict(instance, true);
//        decVal.at<float>(i, 0) = f;
//        if(abs(f) < fMin) fMin = abs(f);
//        if(abs(f) > fMax) fMax = abs(f);
//    }
//
//    printf("Train indecision region of SVM #%d\n");
//    printf("%-9s%-6s%-6s%-6s%-9s%-9s\n", "f_ID", "TP", "FP", "ID", "benefit", "marginal");
//    float beta = (fMax - fMin) * 0.025;
//    int tpFull = -1;
//    float maxBenefit = 0, maxMarginalBenefit;
//    int TP, FP, ID;
//    for(float fID = fMin; fID <= fMax; fID += beta){
//        int tp = 0, fp = 0, id = 0;
//        for(int i = 0; i < N; ++i){
//            float f = decVal.at<float>(i, 0);
//            int l = labels.at<float>(i, 0);
//            if(abs(f) < fID){
//                ++id;
//            }else{
//                if(f > 0 && l == 1 || f < 0 && l == -1)
//                    ++tp;
//                else
//                    ++fp;
//            }
//        }
//        if(tpFull < 0) tpFull = tp;
//        int M = N - id;
//        float benefitFull = 2*(float)tpFull/N - 1.0;
//        float benefit =  2*(float)tp/N - (float)M/N;
//        float marginalBenefit = benefit - benefitFull;
//
//        if(debug) printf("%-9.4f%-6d%-6d%-6d%-9.4f%-9.4f\n", fID, tp, fp, id, benefit, marginalBenefit);
//        if(benefit > maxBenefit){
//                idThresh = fID;
//                maxBenefit = benefit;
//                maxMarginalBenefit = marginalBenefit;
//                TP = tp;
//                FP = fp;
//                ID = id;
//        }
//    }
//    printf("%-9.4f%-6d%-6d%-6d%-9.4f%-9.4f\n", idThresh, TP, FP, ID, maxBenefit, maxMarginalBenefit);
//    return idThresh;
//}



float svm_train(cv::Mat& trainingData, cv::Mat& trainingClasses, cv::Mat& testData, cv::Mat& testClasses) {
    CvSVMParams param;
    param = CvSVMParams();
    param.svm_type = CvSVM::C_SVC;
    param.kernel_type = CvSVM::RBF; //CvSVM::RBF, CvSVM::LINEAR ...
    param.degree = 0; // for poly
    param.gamma = 20;//20 // for poly/rbf/sigmoid
    param.coef0 = 0; // for poly/sigmoid
    param.C = 7; // for CV_SVM_C_SVC, CV_SVM_EPS_SVR and CV_SVM_NU_SVR
    param.nu = 0.0; // for CV_SVM_NU_SVC, CV_SVM_ONE_CLASS, and CV_SVM_NU_SVR
    param.p = 0.0; // for CV_SVM_EPS_SVR

    //param.class_weights = NULL; // for CV_SVM_C_SVC
    param.term_crit.type = CV_TERMCRIT_ITER;
    param.term_crit.max_iter = 500;
    param.term_crit.epsilon = 1e-7;
    int k_fold = 10;

    // SVM training (use train auto for OpenCV>=2.0)
    CvSVM svm;
    svm.train_auto(trainingData, trainingClasses, Mat(), Mat(), param,k_fold);
    // prediction
    cv::Mat predicted(testClasses.rows, 1, CV_32F);
    for(int i = 0; i < testData.rows; i++) {
        cv::Mat sample = testData.row(i);
        predicted.at<float>(i, 0) = svm.predict(sample);
    }
    //results
    float accuracy = evaluate(predicted, testClasses);
    cout << "Accuracy_{SVM} = " << accuracy << endl;
    cout << "given class labels = " << testClasses << endl;
    cout << "prediction result = " << predicted << endl;
    return accuracy;
}

float experiment(Mat trainMat1, Mat trainMat2, Mat testMat, Mat testLabels){

    int featureDim = IMG_WIDTH * IMG_HEIGHT;
    Mat trainMat = combineData(trainMat1, trainMat2);
    Mat trainLabels = combineData(Mat::ones(trainMat1.rows,1,CV_32F),Mat::ones(trainMat2.rows,1,CV_32F)*-1);
    
    //return accuracy of the individual experiment.
    return svm_train(trainMat, trainLabels, testMat, testLabels);    
}

float randomExperiment(string typePos, string typeNeg, string data_folder, int nTest){
    randomTestCreate(typePos, typeNeg, data_folder, nTest);
    Mat data_Pos = createDataMatrix2(data_folder+typePos);
    Mat data_Neg = createDataMatrix2(data_folder+typeNeg);
    Mat data_test = createDataMatrix2(data_folder+"test");
    Mat test_labels = createTestLabels(data_folder+"test", typePos);
    cout << "read in all data" << endl;
    float accuracy = experiment(data_Pos,data_Neg,
                   data_test, test_labels);
    redoRandom(typePos, typeNeg, data_folder, nTest);
    return accuracy;
}

