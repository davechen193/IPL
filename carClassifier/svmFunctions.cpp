#include "classifier.h"

float tThresh;
CvSVM svm;


void svm_train(Mat trainingData, Mat trainingClasses) {
    CvSVMParams param = CvSVMParams();

    param.svm_type = CvSVM::C_SVC;
    param.kernel_type = CvSVM::RBF; //CvSVM::RBF, CvSVM::LINEAR ...
    param.degree = 0; // for poly
    param.gamma = 20; // for poly/rbf/sigmoid
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
    svm.train_auto(trainingData, trainingClasses, Mat(), Mat(), param,k_fold);
    svm.save("svm.dat");
    // prediction
}

float experiment(string typePos, string typeNeg, string data_folder, int nTest, bool is_random, 
    bool use_indecision_boundary){
    if(is_random){
        randomTestCreate(typePos, typeNeg, data_folder, nTest);
    }
    Mat trainPos = createDataMatrix2(data_folder+typePos);
    Mat trainNeg = createDataMatrix2(data_folder+typeNeg);
    Mat testData = createDataMatrix2(data_folder+"test");
    Mat testLabels = createTestLabels(data_folder+"test", typePos);
    int featureDim = IMG_WIDTH * IMG_HEIGHT;
    int Np = trainPos.rows;
    int Nn = trainNeg.rows;
    Mat trainData = combineData(trainPos, trainNeg);
    Mat trainLabels = combineData(Mat::ones(Np,1,CV_32F),Mat::ones(Nn,1,CV_32F)*-1);

    svm_train(trainData, trainLabels);    

    float accuracy =  prediction_result(testData, testLabels, Np, Nn, use_indecision_boundary);
    if(is_random){
        redoRandom(typePos, typeNeg, data_folder, nTest);
    }
    return accuracy;
}

float prediction_result(Mat testData, Mat testClasses, int Np, int Nn, bool use_indecision_boundary){
    Mat predicted(testClasses.rows, 1, CV_32F);
    Mat predicted2(testClasses.rows, 1, CV_32F);
    float threshold = 0.0;
    if(use_indecision_boundary){
        threshold = getThresh(testData, testClasses, Np, Nn, false);
    }
    for(int i = 0; i < testData.rows; i++) {
        Mat sample = testData.row(i);
        predicted.at<float>(i, 0) = svm.predict(sample);
        predicted2.at<float>(i, 0) = -svm.predict(sample,true);
    }

    //results
    int tp = 0;
    int fp = 0;
    for(int i = 0; i < testClasses.rows; i++){
        float f = predicted2.at<float>(i,0);
        float c = testClasses.at<float>(i,0);
        if( abs(f) < threshold){
            predicted.at<float>(i,0) = 0;
        }else if((f > 0.0 && c == 1) || (f <= 0.0 &&  c == -1)) {
            tp++;
        } else {
            fp++;
        }
    }
    float accuracy =  (tp * 1.0) / (tp + fp);
    cout << "Accuracy_{SVM} = " << accuracy << endl;
    cout << "given class labels = " << testClasses << endl;
    cout << "prediction result = " << predicted << endl;
    return accuracy;
}

//Np: number of positives.
//Nn: number of Negatives.
float getThresh(Mat data, Mat labels, int Np, int Nn, bool debug){
    std::fstream inFile ("thresh.dat", std::ios_base::in);
    int Ntimes = 0;
    float tThresh = 0.0;
    if(inFile.is_open()){
        inFile >> Ntimes >> tThresh ;
    }
    inFile.close();

    Mat decVal(labels.size(), CV_32F);
    float idThresh = 0;
    float fMin = 1e3;
    float fMax = 0;
    int N = labels.rows;
    for(int i = 0; i < N; ++i){
        Mat instance = data.row(i);
        float f = -svm.predict(instance, true);// When we get the real decision value, the sign is flipped.
        decVal.at<float>(i, 0) = f;
        if(abs(f) < fMin) fMin = abs(f);
        if(abs(f) > fMax) fMax = abs(f);
    }
    if(debug){
        printf("Train indecision region of SVM #%d\n");
        printf("%-9s%-6s%-6s%-6s%-9s%-9s\n", "f_ID", "TP", "FP", "ID", "benefit", "marginal");
    }
    float beta = (fMax - fMin) * 0.025;
    int tpFull = -1;
    float maxBenefit = 0, maxMarginalBenefit = 0;
    int TP = 0, FP = 0, ID = 0;
    int loopMax = 1000;
    int j = 0;
    for(float fID = fMin; fID <= fMax; fID += beta){
        int tp = 0, fp = 0, id = 0;
        for(int i = 0; i < N; ++i){
            float f = decVal.at<float>(i, 0);
            int l = labels.at<float>(i, 0);
            if(abs(f) < fID){
                ++id;
            }else{
                if(f > 0 && l == 1 || f < 0 && l == -1) 
                    ++tp;
                else
                    ++fp;
            }
        }
        if(tpFull < 0) tpFull = tp;
        int M = N - id;
        float benefitFull = 2*(float)tpFull/N - 1.0;
        float benefit =  2*(float)tp/N - (float)M/N;
        float marginalBenefit = benefit - benefitFull;

        if(debug) printf("%-9.4f%-6d%-6d%-6d%-9.4f%-9.4f\n", fID, tp, fp, id, benefit, marginalBenefit);
        if(benefit > maxBenefit){
                idThresh = fID;
                maxBenefit = benefit;
                maxMarginalBenefit = marginalBenefit;
                TP = tp;
                FP = fp;
                ID = id;
        }
        if(j < loopMax) j++;
        else break;
    }
    cout << "final threshold: " << idThresh << endl;
    cout << "number of true positives: " << TP << endl;
    cout << "number of false positives: " << FP << endl;
    cout << "number of indecisions: " << ID << endl;
    cout << "maximum benefit value: " << maxBenefit << endl;
    cout << "maximum marginal benefit value: " << maxMarginalBenefit << endl;
    cout << endl;
    
    //update the trained threshold.
    tThresh = (tThresh* Ntimes + idThresh)/(Ntimes + 1);
    Ntimes = Ntimes + 1;
    ofstream outFile ("thresh.dat");
    outFile << Ntimes << endl;
    outFile << tThresh << endl;
    outFile.close();
    return tThresh;
}