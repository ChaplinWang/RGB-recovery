#include <iostream>
using namespace std;
#include <cv.h>
#include <highgui.h>
#include <math.h>

CvScalar Find_trans(IplImage* M, IplImage* S);
CvScalar SSD_IMP(IplImage* M, IplImage* S, float xbase, float ybase);
void Cons_Pyr(IplImage* original, IplImage** Pyr);
void Color_image(IplImage* imageR,IplImage* imageG,IplImage* imageB);
void Trans_image(IplImage* imageR,IplImage* imageG,IplImage* imageB, CvScalar transR, CvScalar transG);
void adjustments(IplImage* result);
CvScalar CCOOR_IMP(IplImage* M, IplImage* S, float xbase, float ybase);

int small_range_search = 0;
int main ( int argc, char **argv )
{

if(argc < 2){
	printf("usage: ./prog data/file_name.jpg\n");
	return 0;
}

int pyr = 0;
if(argv[1][10] == 'l'){
	pyr = 1;
	printf("large image processing activating pyramid\n");
}

IplImage* image=0;
image = cvLoadImage(argv[1],-1);

CvRect roi;
CvSize size;

size = cvGetSize(image);

roi.x = 0;
roi.y = 0*size.height/3;
roi.width = size.width;
roi.height = (size.height)/3;
cvSetImageROI(image, roi);
cvSaveImage("program_data/B.jpg",image);
cvResetImageROI(image);

roi.x = 0;
roi.y = 1*size.height/3;
roi.width = size.width;
roi.height = (size.height)/3;
cvSetImageROI(image, roi);
cvSaveImage("program_data/G.jpg",image);
cvResetImageROI(image);

roi.x = 0;
roi.y = 2*size.height/3;
roi.width = size.width;
roi.height = (size.height)/3;
cvSetImageROI(image, roi);
cvSaveImage("program_data/R.jpg",image);
cvResetImageROI(image);
cvReleaseImage(&image);

IplImage* imageR=0;
imageR = cvLoadImage("program_data/R.jpg",-1);

IplImage* imageG=0;
imageG = cvLoadImage("program_data/G.jpg",-1);

IplImage* imageB=0;
imageB = cvLoadImage("program_data/B.jpg",-1);

CvScalar transG;
CvScalar transR;

CvScalar Final_transG;
CvScalar Final_transR;

// if it is a small resolution image;
if(pyr == 0){
	printf("BG");
	transG = Find_trans(imageB, imageG);
	Final_transG = transG;
	printf("BR");
	transR = Find_trans(imageB, imageR);
	Final_transR = transR;
}else{
// if it is a large resolution image;

	IplImage* PyrR[4];
	IplImage* PyrG[4];
	IplImage* PyrB[4];

	Cons_Pyr(imageR, PyrR);
	Cons_Pyr(imageG, PyrG);
	Cons_Pyr(imageB, PyrB);


	printf("Pyramid high trans:");
	printf("BG:\n");
	transG = Find_trans(PyrB[1], PyrG[1]);
	transG.val[0] = transG.val[0]*4;
	transG.val[1] = transG.val[1]*4;
	printf("BR:\n");
	transR = Find_trans(PyrB[1], PyrR[1]);
	transR.val[0] = transR.val[0]*4;
	transR.val[1] = transR.val[1]*4;
	Trans_image(imageR,imageG,imageB,transR,transG);

	Final_transG.val[0] = transG.val[0];
	Final_transG.val[1] = transG.val[1];
	Final_transR.val[0] = transR.val[0];
	Final_transR.val[1] = transR.val[1];

	imageR = cvLoadImage("program_data/R.jpg",-1);
	imageG = cvLoadImage("program_data/G.jpg",-1);
	imageB = cvLoadImage("program_data/B.jpg",-1);
	small_range_search = 1;
	printf("Final Trans:\n");
	printf("BG:\n");
	transG = SSD_IMP(imageB, imageG, imageG->width/2, imageG->height/2);
	printf("[%f,%f]\n",transG.val[0],transG.val[1]);
	printf("BR:\n");
	transR = SSD_IMP(imageB, imageR, imageR->width/2, imageR->height/2);
	printf("[%f,%f]\n",transR.val[0],transR.val[1]);

	Final_transG.val[0] += transG.val[0];
	Final_transG.val[1] += transG.val[1];
	Final_transR.val[0] += transR.val[0];
	Final_transR.val[1] += transR.val[1];
}

imageR = cvLoadImage("program_data/R.jpg",-1);
imageG = cvLoadImage("program_data/G.jpg",-1);
imageB = cvLoadImage("program_data/B.jpg",-1);

printf("Final offset G [%f,%f]\n",Final_transG.val[0],Final_transG.val[1]);
printf("Final offset R [%f,%f]\n",Final_transR.val[0],Final_transR.val[1]);

Trans_image(imageR,imageG,imageB,transR,transG);
Color_image(imageR,imageG,imageB);

IplImage* result = cvLoadImage("program_data/result.jpg",-1);
// do some smoothing and contrast
adjustments(result);

return 0;
}

void adjustments(IplImage* result){

	IplImage* adj_img= cvCreateImage(cvSize(result->width,result->height), IPL_DEPTH_8U, 3);

double 	alpha = 1.1;
int 	beta = 15;
 for (int i = 0; i < adj_img->height; i++){
    for (int j = 0; j < adj_img->width; j++){
    		for (int c = 0; c < adj_img->nChannels; c++){
				 		adj_img->imageData[i*adj_img->widthStep + j*3 + c] = cv::saturate_cast<uchar>((uchar)(alpha*(result->imageData[i*adj_img->widthStep + j*3 + c])) + beta);
				}
		}
	}
cvSmooth( adj_img, adj_img, CV_MEDIAN, 1, 1 );
cvSaveImage("program_data/adj_result.jpg",adj_img);

}


void Trans_image(IplImage* imageR,IplImage* imageG,IplImage* imageB, CvScalar transR, CvScalar transG){

	double t_matR[] = { 1,  0,  transR.val[0], 0,  1,  transR.val[1]};

	double t_matG[] = { 1,  0,  transG.val[0], 0,  1,  transG.val[1]};

	CvMat TMatR=cvMat(2, 3, CV_64FC1, t_matR);
	CvMat TMatG=cvMat(2, 3, CV_64FC1, t_matG);

	cvWarpAffine(imageR, imageR, &TMatR, 0, cvScalarAll(0));
	cvWarpAffine(imageG, imageG, &TMatG, 0, cvScalarAll(0));
	cvSaveImage("program_data/G.jpg",imageG);
	cvSaveImage("program_data/B.jpg",imageB);

}

void Color_image(IplImage* imageR,IplImage* imageG,IplImage* imageB){

	IplImage* result= cvCreateImage(cvSize(imageR->width,imageR->height), IPL_DEPTH_8U, 3);

	for (int i = 0; i < result->height; i++){
    for (int j = 0; j < result->width; j++){

        unsigned char red = imageR->imageData[i * imageR->widthStep + j];
        result->imageData[i * result->widthStep + j*result->nChannels + 2] = red;

        unsigned char blue = imageB->imageData[i * imageB->widthStep + j];
        result->imageData[i * result->widthStep + j*result->nChannels] = blue;

        unsigned char green = imageG->imageData[i * imageG->widthStep + j];
        result->imageData[i * result->widthStep + j*result->nChannels + 1] = green;
    }
	}
	cvSaveImage("program_data/result.jpg",result);
}

void Cons_Pyr(IplImage* original, IplImage** Pyr){

	int i, base = 1;

	for(i = 0; i < 4; i++){
		base = base*2;
		Pyr[i] = cvCreateImage(cvSize(original->width / base,original->height / base),
                                         original->depth,
                                         original->nChannels);
	}

	cvPyrDown(original, Pyr[0]);
	cvPyrDown(Pyr[0], Pyr[1]);
	cvPyrDown(Pyr[1], Pyr[2]);
	cvPyrDown(Pyr[2], Pyr[3]);

}


CvScalar Find_trans(IplImage* M, IplImage* S){

	CvScalar result = cvScalar(0,0);
	CvScalar temp = cvScalar(0,0);
	int x = 0;
	int y = 0;
	int max = 0;
	int Zerox = S->width/2;
	int Zeroy = S->height/2;
	int most_possible_count[S->width][S->height];

	// sample 200 points to get correct trans match
	for(int i = 0; i < 200; i++){

		float rx = 0.2 + (float)rand()/((float)RAND_MAX/(0.6));
		float ry = 0.2 + (float)rand()/((float)RAND_MAX/(0.6));

		// use 70 out of 200 samples using SSD

		if(i < 50){
			temp = SSD_IMP(M, S, rx, ry);
		// use 130 out of 200 samples using CCOOR
		}else{
			temp = CCOOR_IMP(M, S, rx, ry);
		}
		x = temp.val[0] + Zerox;
		y = temp.val[1] + Zeroy;

		most_possible_count[x][y]++;
		if(most_possible_count[x][y] > max){
			max = most_possible_count[x][y];
			result = temp;
			printf("[%f,%f]\n",result.val[0],result.val[1]);
		}

	}

	printf("[%f,%f]\n",result.val[0],result.val[1]);
	return result;
}

//SSD inplementation
CvScalar SSD_IMP(IplImage* M, IplImage* S, float xbase, float ybase){

	CvScalar result = cvScalar(0,0);

	int window_S_start_x, window_S_start_y,window_S_end_x,window_S_end_y;
	int  window_M_start_x, window_M_start_y, window_M_end_x, window_M_end_y;

	if(small_range_search){
		window_S_start_x = S->width*xbase;
		window_S_start_y = S->height*ybase;
		window_S_end_x = window_S_start_x + 4;
		window_S_end_y = window_S_start_y + 4;

		window_M_start_x = window_S_start_x -8;
		window_M_start_y = window_S_start_y - 8;
		window_M_end_x = window_S_start_x + 8;
		window_M_end_y = window_S_start_y + 8;
		return cvScalar(0,0);
	}
	else{
		window_S_start_x = S->width*xbase;
		window_S_start_y = S->height*ybase;
		window_S_end_x = window_S_start_x + S->width/20;
		window_S_end_y = window_S_start_y + S->height/20;

		window_M_start_x = window_S_start_x -S->width/30;
		window_M_start_y = window_S_start_y - S->height/30;
		window_M_end_x = window_S_start_x + S->width/30;
		window_M_end_y = window_S_start_y + S->height/30;
	}

	int sum = 0;
	int min = 1000000000;

	unsigned char Mvar;
	unsigned char Svar;
	int diff;
	int i,j,li,lj;

if(small_range_search){
	for (li = window_M_start_y; li < window_M_end_y; li++){
  	for (lj = window_M_start_x; lj < window_M_end_x; lj++){

			sum = 0;
			for (i = window_S_start_y; i < window_S_end_y; i++){
    		for (j = window_S_start_x; j < window_S_end_x; j++){

    			Mvar = M->imageData[((li+(i - window_S_start_y)) * M->widthStep) + lj + (j - window_S_start_x)];

        	Svar = S->imageData[i * S->widthStep + j];

					diff = (Mvar - Svar);
					sum = sum + diff*diff;
    		}
			}

			if(sum < min){
				min = sum;
				result.val[1] = li - window_S_start_y;
				result.val[0] = lj - window_S_start_x;
			}
  	}
	}
	return result;
}

	for (li = window_M_start_y; li < window_M_end_y; li++){
  	for (lj = window_M_start_x; lj < window_M_end_x; lj++){

			sum = 0;
			for (i = window_S_start_y; i < window_S_end_y; i++){
    		for (j = window_S_start_x; j < window_S_end_x; j++){

    			Mvar = M->imageData[((li+(i - window_S_start_y)) * M->widthStep) + lj + (j - window_S_start_x)];

        	Svar = S->imageData[i * S->widthStep + j];

					diff = (Mvar - Svar);
					sum = sum + diff*diff;
    		}
			}

			if(sum < min){
				min = sum;
				result.val[1] = li - window_S_start_y;
				result.val[0] = lj - window_S_start_x;
			}
  	}
	}



	return result;
}

// NCC implementation
CvScalar CCOOR_IMP(IplImage* M, IplImage* S, float xbase, float ybase){

	CvRect roi;
	roi.x = S->width*xbase;
	roi.y = S->height*ybase;
	roi.width = S->width/20;
	roi.height = S->height/20;
	cvSetImageROI(S, roi);

	roi.x = (int)(S->width*xbase) - (int)S->width/30;
	roi.y = (int)(S->height*ybase) - (int)S->height/30;
	roi.width = S->width/5;
	roi.height = S->height/5;
	cvSetImageROI(M, roi);



	IplImage *CCOOR = cvCreateImage(cvSize(S->width/5 - S->width/20 + 1,S->height/5 - S->height/20 + 1),IPL_DEPTH_32F,M->nChannels);

  cvMatchTemplate(M, S, CCOOR, CV_TM_CCORR_NORMED);
	double minval,maxval;
	CvPoint MinLocation , MaxLocation;
	cvMinMaxLoc(CCOOR, &minval, &maxval, &MinLocation, &MaxLocation);

	return cvScalar((MaxLocation.x - (int)S->width/30),(MaxLocation.y - (int)S->height/30));
}
