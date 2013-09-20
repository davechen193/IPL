//#include "classifer.h"

#include "classifier.h"

int main() {
    string space;
    classifier_example();
    getline(cin,space);
    cv::waitKey(0);
    return 0;
}

void classifier_example(){
    // set up the experiments.
    string data_folder = "..\\..\\textfile_data\\side_view\\set3\\";
    int nTest = 10;
    int maxTrial = 100;
    string typePos = "truck";
    string typeNeg = "sedan";
    float total1 = 0.0;
    float total2 = 0.0;
    if(boost::filesystem::exists("svm.dat")){
        svm.load("svm.dat");  
    }
    for(int i=0 ; i<maxTrial ; i++){
        total1 += experiment("sedan","truck", data_folder,nTest,true,true);
        cout<<endl;
    }
    for(int i=0 ; i<maxTrial ; i++){
        total2 += experiment("sedan","truck", data_folder,nTest,true,false);
        cout<<endl;
    }
    cout<< "The average accuracy of "<< maxTrial << " experiments is: "<< total1/maxTrial << "(with indecision-boundary)" <<endl;
    cout << "average indecision boundary: " << tThresh<<endl;
    cout<< "The average accuracy of "<< maxTrial << " experiments is: "<< total2/maxTrial << endl;
}

//void chained_classifier(){
//    vector<string> types = {"sedan", "truck", "SUV"};
//    string data_folder = "";
//    string first_layer_type = "";
//    string second_layer_type = ""
//    //first layer training.
//    
//    //second layer training.
//    //first layer testing.
//    //second layer testing.
//}
// 