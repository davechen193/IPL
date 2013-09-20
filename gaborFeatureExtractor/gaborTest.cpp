/*
Gabor Feautre Extraction is a biologically motivated texture analysis method.
This is a test of the gabor extraction process on a single image.
It consists of 4 stages:
    1. object detection
        - histogram equalization process is imported from library.
    2. obect segmentation
    3. feature extraction
    4. jet matching
*/
#include "gaborTest.h"
#include <iostream>
#include <fstream>

ofstream outfile("output.txt");


Mat lowpass(Mat img){
    //  noise reduction with lowpass filter
    //  the filter is just averaging out the area of the pixel. other lowpass filters can be applied.
    Mat newImage;
    Mat filt = Mat::ones(3,3,CV_32F)/9;
    Point anchor = (-1,-1);
    filter2D(img, newImage,-1,filt,anchor,0,BORDER_REPLICATE);
    return newImage;
}

Mat gaborKernel(int ksize[], float bw, float gamma, float theta, float lambda, float psi, bool isReal){
    float sig = lambda/CV_PI*sqrt(log(2.0)/2)*(pow(bw,2)+1)/(pow(bw,2)-1);
    float sigX = sig;
    float sigY = sig/gamma;
    int nstds = 3;
    int xmin, xmax, ymin, ymax;
    if (ksize[0] > 0){
        xmax = ksize[0]/2;
    }else{
        xmax = max(fabs(nstds*sigX*cos(theta)),abs(nstds*sigY*sin(theta)));
    }
    if  (ksize[1] > 0){
        ymax = ksize[1]/2;
    }else{
        ymax = max(fabs(nstds*sigX*sin(theta)),abs(nstds*sigY*cos(theta)));
    }
    xmin = -xmax; ymin = -ymax;
    int cols = xmax - xmin + 1;
    int rows = ymax - ymin + 1;
    Mat kernel = Mat(rows, cols, CV_32F);
     
    for ( int y = ymin ; y <= ymax ; y++){
        for ( int x = xmin ; x <= xmax ; x++){
            float x_theta = x*cos(theta)+y*sin(theta);
            float y_theta = -x*sin(theta)+y*cos(theta);
            float trigValue;
            if (isReal){
                trigValue = cos(M_PI*2/lambda*x_theta+psi);
            }else{
                trigValue = sin(M_PI*2/lambda*x_theta+psi);
            }
            if(pow(sigY,2) == 0 || pow(sigX,2) == 0){
                cout<<"No pow!"<<endl;
            }
            float quantity = cv::exp(-.5*(pow(x_theta,2) / pow(sigX,2) + pow(y_theta,2) / pow(sigY,2)))*trigValue;
            kernel.at<float>(ymax-y, xmax-x) = quantity;        
        }
    }
    return kernel;
}

Vector<Mat> gaborImages(Mat image, int nOrientations, int nLambda, string type, bool imageSuppressed){
    int ksize[]= {image.cols,image.rows};
    float bw = 0.8;
    float gamma = 1;
    float psi = 0;
    float theta;
    float lambda;
    float lamb_step = 0.5;
    Vector<Mat> resultImages = Vector<Mat>(nOrientations * nLambda);

    for (int i=1; i<= nOrientations ; i++){
        for (int j=1 ; j<=nLambda ; j++){
            theta = i*CV_PI/nOrientations; 
            lambda = 40 + lamb_step*j;
            string str_th = static_cast<ostringstream*>( &(ostringstream() << theta) )->str();
            string str_lamb = static_cast<ostringstream*>( &(ostringstream() << lambda) )->str();
            string title = "Gabor filtered image, theta = " + str_th+"   lambda = " + str_lamb;
            Mat gReal, gImag, tempMat, magMat, mag1, mag2, resultMat;
            Mat kernel = gaborKernel(ksize, bw, gamma, theta, lambda, psi, true);
            Mat imag = gaborKernel(ksize, bw, gamma, theta, lambda, psi, false);
            filter2D(image,gReal,CV_32F,kernel);
            filter2D(image,gImag,CV_32F,imag);
            pow(gReal,2,mag1);
            pow(gImag,2,mag2);
            pow(mag1+mag2,0.5,magMat);
            if (type == "kernel"){
                resultMat = kernel;
            }else if(type == "response"){
                resultMat = gReal;
            }else{
                resultMat = magMat;
            }
            resultImages[(i-1)*nLambda + (j-1)] = resultMat;
            // show image.
            if (!imageSuppressed){
                namedWindow(title, CV_WINDOW_AUTOSIZE); 
                imshow(title, resultMat);
            }
        }
    }
    return resultImages;
}


//static inline double computeSquare (float x) { return x*x; }
// calculate the similarity value between magnitudes of two images.
// make sure the images are of the same sizes.
// at this moment I'm just averaging the similarity values.
float similarity_value(Vector<Mat> magVec1, Vector<Mat> magVec2){
    float simSum = 0;
    float simVal = 0;
    float numerator, denominator = 0;
    if(magVec1[1].rows != magVec2[1].rows || magVec1[1].cols!= magVec2[1].cols){
        cout<< "the sizes of two images do not match!"<<endl;
        return -1;
    }

    Vector<float> mag1 = Vector<float>(magVec1.size());
    Vector<float> mag2 = Vector<float>(magVec2.size());
    int rows = min(magVec1[1].rows,magVec2[1].rows);
    int cols = min(magVec1[1].cols,magVec2[1].cols);
    
    cout << "calculating similarity..." << endl;
    for(int i=0 ; i< rows*cols ; i++){
        int y = i / cols;
        int x = i % cols;
        for(int j=0 ; j<magVec1.size() ; j++){
            mag1[j] = magVec1[j].at<float>(y,x);
            mag2[j] = magVec2[j].at<float>(y,x);
        }
        numerator = dot(mag1,mag2);
        denominator = sqrt(dot(mag1,mag1)*dot(mag2,mag2));
        simSum += numerator / denominator;
    }
    if(rows*cols == 0){
        cout<<"No sim!"<<endl;
    }
    simVal += simSum / (rows*cols);

    return simVal;
}


//crop the two images to the same sizes
void cropImages(Mat &image1, Mat &image2){
    int rows = min(image1.rows,image2.rows);
    int cols = min(image1.cols,image2.cols);
    
    image1 = image1(Rect(image1.cols/2 - cols/2,image1.rows/2 - rows/2, cols, rows));
    image2 = image2(Rect(image2.cols/2 - cols/2,image2.rows/2 - rows/2, cols, rows));
}

//deal image processing to the given image.
Mat process(Mat image){
    //  histogram equalization process
    equalizeHist(image, image);

    //  noise reduction with lowpass filter
    image = lowpass(image);
    return image;
}

void matching(Mat image1, Mat image2, string name1, string name2){
    image1 = process(image1);
    image2 = process(image2);

    if(!image1.data){                              // Check for invalid input
        cout <<  "Could not open "+name1 << std::endl ;
    }
    if(!image2.data){                              // Check for invalid input
        cout <<  "Could not open "+name2 << std::endl ;
    }

    // gabor filter
    Vector<Mat> jet1 = gaborImages(image1, 8, 4, "magnitude", true); // options: kernel, response, diff, magnitude
    Vector<Mat> jet2 = gaborImages(image2, 8, 4, "magnitude", true);
    for(int i=0 ; i<jet1.size() ; i++){
        // cropping images to the same size for calculating similarity values.
        cropImages(jet1[i], jet2[i]);
    }
    float simVal = similarity_value(jet1,jet2);
    cout << "Comparing "+name1+" "+name2+" we get: " << endl;
    cout << simVal << endl;
    // write it into file.
    outfile <<"Comparing "+name1+" "+name2+" we get:";//here
    outfile << simVal;
    outfile << "\n";
}
int main( int argc, char** argv){
    
    string carTypes[] = {"uhaul","truck","sedan"};
    Mat image1, image2, image3;
    string name1, name2;
    string data_folder = "dataforexperiment\\realistic";
    int size = sizeof(carTypes)/sizeof(carTypes[0]);
    for(int i=0 ; i<size ; i++){ 
        string type1 = carTypes[i];
        vector<path> paths1 = ls(data_folder+"\\"+type1);
        for(int j = i; j<size ; j++){
            string type2 = carTypes[j];
            vector<path> paths2 = ls(data_folder+"\\"+type2);
            for each(path p1 in paths1){
                for each(path p2 in paths2){
                    image1 = imread(p1.string(),CV_LOAD_IMAGE_GRAYSCALE);
                    image2 = imread(p2.string(),CV_LOAD_IMAGE_GRAYSCALE);
                    name1 = p1.filename().string();
                    name2 = p2.filename().string();
                    matching(image1, image2, name1, name2);
                }
            }
        }
    }

    //realistic_unsegmented
    

    string input;
    outfile.close();
    waitKey(0);
    return 0;
}