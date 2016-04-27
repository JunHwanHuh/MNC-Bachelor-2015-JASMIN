#include <iostream>
#include <cstdio>
#include <vector>
#include <list>
#include <stdlib.h>

// g++ -ggdb `pkg-config --cflags opencv` -o `basename main.cpp .cpp` main.cpp `pkg-config --libs opencv`
//#include "highgui.h"
//#include "cv.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
//#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
//#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/wait.h>
//#include <netdb.h>
//#include <unistd.h>

#include <opencv/cvaux.h>
#include <opencv/cxcore.h>

//#include "cvaux.h"
//#include "cxcore.h"

//#define NAME "car.mp4"
//#define NAME "opencv3.mp4"
//제도관 뒤
//#define NAME "20151103_640_360.avi"

//기숙사
//#define NAME "gisuksa.avi"

//계단
//#define NAME "upstair.avi"

//#define NAME "result_movie.avi"

//어두워지는 화면
//#define NAME "natbam.mp4"

//#define NAME "bam.avi"
//#define NAME "natbam2.avi"
//#define NAME "3.avi"


//#define NAME "ffront.avi"
#define NAME "fffff.avi"
//#define NAME "1f.avi"
#define PI 3.14159265
//#define MASKSIZE 20
#define DIRECTION_A 0
#define DIRECTION_B 1
#define DIRECTION_C 2
#define DIRECTION_D 3

#define SA      struct sockaddr
#define MAXLINE 4096
#define MAXSUB  200
#define LISTENQ         1024

typedef unsigned char BYTE;
#define CV_GET_B(img,x,y) CV_IMAGE_ELEM((img), BYTE, (y), (x) * 3 + 0)
#define CV_GET_G(img,x,y) CV_IMAGE_ELEM((img), BYTE, (y), (x) * 3 + 1)
#define CV_GET_R(img,x,y) CV_IMAGE_ELEM((img), BYTE, (y), (x) * 3 + 2)

typedef struct roi{
	int id;
	int leftTop_x;
	int leftTop_y;
	int rightBot_x;
	int rightBot_y;
	CvPoint past;
	int inDirection;
	int outDirection;
	int lifetime;
}ROI;

using namespace cv;
using namespace std;

extern int h_errno;
double MASKSIZE = 10000;
list<ROI> ROIs;
list<ROI>::iterator itor;
int ROI_num = 0;

double rad2deg(double radian)
{
	return radian*180/PI;
}

double deg2rad(double degree)
{
	return degree*PI/180;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//ssize_t process_http(int sockfd, char *host, char *page, char *poststr)
//{
//	char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
//	ssize_t n;
//	snprintf(sendline, MAXSUB,
//		 "POST %s HTTP/1.0\r\n"
//		 "Host: %s\r\n"
//		 "Content-type: application/json\r\n"
//		 "Content-length: %d\r\n\r\n"
//		 "%s", page, host, strlen(poststr), poststr);
//
//	write(sockfd, sendline, strlen(sendline));
//	while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
//		recvline[n] = '\0';
//		printf("%s", recvline);
//	}
//	return n;
//
//}

void subBackg(){
	CvCapture *capture = cvCaptureFromFile(NAME);
	//CvCapture *capture = cvCaptureFromCAM(0);
	// 비디오 파일과 연결된 포인터 capture이 비어있다면 비디오를 불러오는것에 실패한 것 이므로 프로그램을 종료
	if (!capture)    {
		cout << "The video file was not found.";
		exit(0);
	}
	// cvGetCaptureProperty 함수를 사용하여 비디오 파일과 연결된 포인터 capture로부터
	// 프레임의 가로와 세로 크기 정보를 읽어서 width와 height에 저장한다.
	int width = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);

	// OpenCV의 자료구조인 CvSize 의 변수 frameSize를 생성
	// CvSize의 멤버변수로는 height와 width가 있는데, 이 멤버변수의 width와 height의 값을 업데이트 시켜줌
	CvSize frameSize = cvSize(width, height);
	// 이미지를 불러오기 위한 iplimage 구조체를 생성
	// 생성된 구조체에 image를 생성하여 생성해줌
	// 그레이 스케일 영상을 저장할 grayImage와 cvAcc 함수를 이용하여 누적하기 위한 sumImage영상을 생성
	// cvZero 함수를 사용하여 sumImage를 0으로 초기화한다.
	IplImage *grayImage = cvCreateImage(frameSize, IPL_DEPTH_8U, 1);
	IplImage *sumImage = cvCreateImage(frameSize, IPL_DEPTH_32F, 1);
	cvZero(sumImage);
	cvZero(grayImage);
	IplImage *frame = NULL;
	int t = 0;
	int temp = 0;
	cout << "making background 0% " ;

	while (t<500)    {
		// capture로부터 프레임을 획득하여 포인터 frame에 저장한다.
		frame = cvQueryFrame(capture);
		if (!frame)    break;
		t++;
		//std::cout << t << std::endl;
		// frame은 컬러이므로 cvCvtColor 함수를 이용하여 그레이 스케일 영상으로 변환하여 grayscale에 저장
		cvCvtColor(frame, grayImage, CV_BGR2GRAY);
		// cvAcc 함수를 사용하여 grayImage 영상을 sumImage에 누적한다
		cvAcc(grayImage, sumImage, NULL);
		if (t % 100 == 0) cout << (t / 100) * 20 << "% ";
	}
	cout << endl;
	// cvScale 함수를 사용하여 누적 영상 sumImage를 1.0/t로 스케일링하여 평균 영상을 계산하여 sumImage에 다시 저장
	cvScale(sumImage, sumImage, 1.0 / t);
	cvSaveImage("handBkg.jpg", sumImage);
	cvReleaseImage(&sumImage);
	cvReleaseImage(&grayImage);
	cvReleaseCapture(&capture);
}


bool startLine = false;
bool drawStartLine = false;
bool endLine = false;
bool drawEndLine = false;
int start1_x = 0;
int start1_y = 0;
int end1_x = 0;
int end1_y = 0;
int start2_x = 0;
int start2_y = 0;
int end2_x = 0;
int end2_y = 0;
bool startRect = false;
bool drawRect = false;
int rectLeftTop_x = 0;
int rectLeftTop_y = 0;
int rectRightBot_x = 0;
int rectRightBot_y = 0;

void on_eventhandle (int event, int x, int y, int flags, void* param) //핸들러 함수
{
	IplImage    *image;
	image = (IplImage *)param;
	switch(event){
	case CV_EVENT_MOUSEMOVE:{
		if(startLine){
			drawStartLine = true;
			end1_x = x;
			end1_y = y;
		}
		else if(endLine){
			drawEndLine = true;
			end2_x = x;
			end2_y = y;
		}
		else if(startRect){
			drawRect = true;
			rectRightBot_x = x;
			rectRightBot_y = y;
		}
							}
							break;
	case CV_EVENT_LBUTTONDOWN:{
		if((CV_EVENT_FLAG_SHIFTKEY + CV_EVENT_LBUTTONDOWN) == flags){
			drawStartLine = false;
			startLine = true;
			start1_x = x;
			start1_y = y;
		}
		else if((CV_EVENT_FLAG_CTRLKEY  + CV_EVENT_LBUTTONDOWN) == flags){
			drawEndLine = false;
			endLine = true;
			start2_x = x;
			start2_y = y;
		}
		else{
			drawRect = false;
			startRect = true;
			rectLeftTop_x = x;
			rectLeftTop_y = y;
		}
							  }
							  break;
	case CV_EVENT_LBUTTONUP:{
		if(startLine){
			startLine = false;
			drawStartLine = true;
			end1_x = x;
			end1_y = y;
			cvLine(image,cvPoint(start1_x,start1_y),cvPoint(end1_x,end1_y),CV_RGB(0,255,0),2,0);
		}

		else if(endLine){
			endLine = false;
			drawEndLine = true;
			end2_x = x;
			end2_y = y;
			cvLine(image,cvPoint(start2_x,start2_y),cvPoint(end2_x,end1_y),CV_RGB(255,0,0),2,0);
		}
		else if(startRect){
			startRect = false;
			drawRect = true;
			rectRightBot_x = x;
			rectRightBot_y = y;
			//cvRectangle(image,cvPoint(rectLeftTop_x,rectLeftTop_y),cvPoint(rectRightBot_x,rectRightBot_y),CV_RGB(255,255,255),CV_FILLED,0);
		}
							}
							break;
	}
}

bool isCounterClock(int a, int b, int c, int d, int e, int f){
	float i;
	i = a*d + c*f + e*b - (c*b + a*f + e*d);
	if(i >= 0)
		return 1;
	return 0;
}

int main(int argc, char** argv) {
	double count = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// int sockfd;
	//   struct sockaddr_in servaddr;

	//  char **pptr;
	//********** You can change. Puy any values here *******
	//  char *hname = "52.26.66.52";
	//char *hname = "souptonuts.soſurceforge.net";
	//   char *page = "/receive";

	//*******************************************************

	//   char str[50];
	//   struct hostent *hptr;
	/////////////////////////////////////////////////////////////////////////////////////////////////
	CvCapture *capture = cvCaptureFromFile(NAME);
	//CvCapture *capture = cvCaptureFromCAM(0);
	if (!capture)    {
		cout << "The video file was not found.";
		return 0;
	}
	srand(15);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,480);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 320);
	int width = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	CvSize frameSize = cvSize(width, height);
	IplImage *frame = cvCreateImage(frameSize, IPL_DEPTH_8U, 1);
	IplImage *grayImage = cvCreateImage(frameSize, IPL_DEPTH_8U, 1);
	IplImage *diffImage = cvCreateImage(frameSize, IPL_DEPTH_8U, 1);
	IplImage *Mask = cvCreateImage(frameSize, IPL_DEPTH_8U, 3);
	IplImage *test = cvCreateImage(frameSize, IPL_DEPTH_8U, 3);

	cvZero(grayImage);
	//cvSet(Mask,cvScalar(255,0,255));
	cvZero(Mask);

	CvSeq* contours = 0;
	CvSeq* result = 0;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 2, CV_AA);

	subBackg();
	//capture = cvCaptureFromFile(NAME);
	//capture = cvCaptureFromCAM(0);
	if (!capture)    {
		cout << "The video file was not found" << std::endl;
		return 0;
	}

	IplImage *bkgImage = cvLoadImage("handBkg.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	if (bkgImage == NULL){
		return -1;
	}

	int num;
	double area;
	char buffer[512];
	char buffer1[512];
	char buffer2[512];
	CvPoint tmp_polygon[1][4];
	CvPoint tmp_polygon1[1][4];
	CvPoint tmp_polygon2[1][4];
	int npt[] = {4};
	CvPoint tmp_X[1][4];
	int nptX[] = {4};
	int leftTop_x, rightBot_x;
	int leftTop_y, rightBot_y;
	int newLeftTop_x, newLeftTop_y, newRightBot_x, newRightBot_y;

	int R,G,B;
	int detectR,detectG,detectB;

	bool tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12;
	int inCount = 0;
	int outCount = 0;
	int inCount1 = 0;
	int outCount1 = 0;
	int inCount2 = 0;
	int outCount2 = 0;

	int frameNum=0;

	int countFlag = -1;
	////////////////////////////////////////////////////////////////////////////////////////
	int temp1 = 1;
	int temp2 = 0;
	////////////////////////////////////////////////////////////////////////////////////////
	while (1)    {
		count++;
		cout << count << endl;

		for(itor=ROIs.begin(); itor != ROIs.end(); itor++){
			if((*itor).lifetime < 0){
				cvRectangle(Mask,cvPoint((*itor).leftTop_x,(*itor).leftTop_y),cvPoint((*itor).rightBot_x,(*itor).rightBot_y),CV_RGB(0,0,0),CV_FILLED,8);
				ROIs.erase(itor);
				break;
			}
			(*itor).lifetime--;
		}

		countFlag++;
		countFlag %= 7;

		/*if(count == 3355) {
		IplImage *bkgImage = cvLoadImage("handBkg1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		}*/
		// capture로부터 프레임을 획득하여 포인터 frame에 저장한다.
		//IplImage *bkgImage = cvLoadImage("handBkg1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		frame = cvQueryFrame(capture);
		//frame = cvQueryFrame(capture);
		cvCvtColor(frame, grayImage, CV_BGR2GRAY);
		cvAbsDiff(bkgImage, grayImage, diffImage);
		cvCvtColor(frame, grayImage, CV_BGR2GRAY);
		cvAbsDiff(bkgImage, grayImage, diffImage);

		cvSmooth(diffImage,diffImage,CV_MEDIAN,17,17);

		cvMorphologyEx(diffImage,diffImage,NULL,NULL,CV_MOP_OPEN,3);
		cvMorphologyEx(diffImage,diffImage,NULL,NULL,CV_MOP_CLOSE,3);

		cvThreshold(diffImage, diffImage,20, 255, CV_THRESH_BINARY);
		/*
		if(drawStartLine){
		cvLine(frame,cvPoint(start1_x,start1_y),cvPoint(end1_x,end1_y),CV_RGB(0,255,0),2,0);
		tmp_polygon[0][0] = cvPoint(start1_x,start1_y);
		tmp_polygon[0][1] = cvPoint(end1_x,end1_y);
		sprintf(buffer,"IN : %d, OUT : %d",inCount,outCount);
		cvPutText( frame, buffer, cvPoint( start1_x-10, start1_y-10 ), &font, cvScalar( 0, 255, 0 ) );
		}
		if(drawEndLine){
		cvLine(frame,cvPoint(start2_x,start2_y),cvPoint(end2_x,end2_y),CV_RGB(255,0,0),2,0);
		tmp_polygon[0][2] = cvPoint(end2_x,end2_y);
		tmp_polygon[0][3] = cvPoint(start2_x,start2_y);
		CvPoint* pollygon[1] = { tmp_polygon[0] };
		cvFillPoly(frame,pollygon,npt,1,CV_RGB(175,255,223),8,0);
		}
		*/

		cvLine(frame,cvPoint(320,200),cvPoint(320,30),CV_RGB(0,255,0),2,0);
		tmp_polygon[0][0] = cvPoint(320,200);
		tmp_polygon[0][1] = cvPoint(320,30);
		sprintf(buffer,"IN : %d, OUT : %d",outCount,inCount);
		cvPutText( frame, buffer, cvPoint( 20, 150 ), &font, cvScalar( 0, 255, 0 ) );

		//cvLine(frame,cvPoint(550,160),cvPoint(450,30),CV_RGB(0,255,0),2,0);
		//tmp_polygon1[0][0] = cvPoint(550,160);
		//tmp_polygon1[0][1] = cvPoint(450,30);
		//sprintf(buffer1,"IN : %d, OUT : %d",outCount1,inCount1);
		//cvPutText( frame, buffer1, cvPoint( 340, 100 ), &font, cvScalar( 0, 255, 0 ) );

		//cvLine(frame,cvPoint(100,300),cvPoint(500,300),CV_RGB(0,255,0),2,0);
		//tmp_polygon2[0][0] = cvPoint(100,300);
		//tmp_polygon2[0][1] = cvPoint(500,300);
		//sprintf(buffer2,"IN : %d, OUT : %d",outCount2,inCount2);
		//cvPutText( frame, buffer2, cvPoint( 90, 290 ), &font, cvScalar( 0, 255, 0 ) );

		/*
		cvLine(frame,cvPoint(50,150),cvPoint(350,150),CV_RGB(0,255,0),2,0);
		tmp_polygon[0][0] = cvPoint(50,150);
		tmp_polygon[0][1] = cvPoint(350,150);
		sprintf(buffer,"IN : %d, OUT : %d",outCount,inCount);
		cvPutText( frame, buffer, cvPoint( 40, 140 ), &font, cvScalar( 0, 255, 0 ) );
		*/
		if(drawRect){
			//cvEllipse(diffImage,cvPoint((ellipseStart_x+ellipseEnd_x)/2,(ellipseStart_y+ellipseEnd_y)/2),cvSize(widthEllipse,heightEllipse),ellipseAngle,0,360,CV_RGB(255,255,255),CV_FILLED,0);
			cvRectangle(diffImage,cvPoint(rectLeftTop_x,rectLeftTop_y),cvPoint(rectRightBot_x,rectRightBot_y),CV_RGB(255,255,255),CV_FILLED,0);
		}
		cvShowImage("threshold",diffImage);
		cvFindContours(diffImage, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
		cvNamedWindow("Original Video");
		cvSetMouseCallback("Original Video", on_eventhandle,  (void *)grayImage);

		while(contours){

			result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.01, 0);
			area = fabs(cvContourArea(result, CV_WHOLE_SEQ));
			//printf("%f\n",area);
			if(area > 1000 && area < 150000 ){
				vector<CvPoint*> pt;
				for(int i=0;i<result->total;i++){
					pt.push_back((CvPoint*)cvGetSeqElem(result,i));
				}
				int posX = 0;
				int posY = 0;
				for(int i=0;i<pt.size();i++){
					posX += pt[i]->x;
					posY += pt[i]->y;

				}
				posX /= result->total;
				posY /= result->total;
				R = CV_GET_R(frame,posX,posY);
				G = CV_GET_G(frame,posX,posY);
				B = CV_GET_B(frame,posX,posY);
				detectR = CV_GET_R(Mask,posX,posY);
				detectG = CV_GET_G(Mask,posX,posY);
				detectB = CV_GET_B(Mask,posX,posY);

				MASKSIZE = sqrt(area)/8*5;
				MASKSIZE = MASKSIZE > 40? 40:MASKSIZE;
				//MASKSIZE = 5;

				cvCircle(frame,cvPoint(posX,posY),5,CV_RGB(rand()%255,rand()%255,rand()%255),-1);
				cvRectangle(frame,cvPoint(posX-MASKSIZE,posY-MASKSIZE),cvPoint(posX+MASKSIZE,posY+MASKSIZE),CV_RGB(rand()%255,rand()%255,rand()%255),1,8);
				//if(R == 175 && G ==255 && B == 223){
				if(detectR == 255 && detectG == 0 && detectB == 255){


					for(itor=ROIs.begin(); itor != ROIs.end(); itor++){

						if((*itor).rightBot_x > posX && posX >(*itor).leftTop_x && (*itor).rightBot_y > posY && posY > (*itor).leftTop_y){
							cvRectangle(Mask,cvPoint((*itor).leftTop_x,(*itor).leftTop_y),cvPoint((*itor).rightBot_x,(*itor).rightBot_y),CV_RGB(0,0,0),CV_FILLED,8);
							cvRectangle(Mask,cvPoint(posX-MASKSIZE,posY-MASKSIZE),cvPoint(posX+MASKSIZE,posY+MASKSIZE),CV_RGB(255,0,255),CV_FILLED,8);
							(*itor).leftTop_x = posX-MASKSIZE;
							(*itor).leftTop_y = posY-MASKSIZE;
							(*itor).rightBot_x = posX+MASKSIZE;
							(*itor).rightBot_y = posY+MASKSIZE;

							if(countFlag == 0){

								// bool tmp1 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,start1_x,start1_y);
								// bool tmp2 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,end1_x,end1_y);
								// bool tmp3 = isCounterClock(start1_x,start1_y,end1_x,end1_y,(*itor).past.x,(*itor).past.y);
								// bool tmp4 = isCounterClock(start1_x,start1_y,end1_x,end1_y,posX,posY);

		
								bool tmp1 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,320,200);
								bool tmp2 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,320,30);
								bool tmp3 = isCounterClock(320,200,320,30,(*itor).past.x,(*itor).past.y);
								bool tmp4 = isCounterClock(320,200,320,30,posX,posY);

							//	char poststr[MAXLINE + 1];

								if(tmp1 == 0 && tmp2 == 1 && tmp3 == 1 && tmp4 == 0){
									inCount++;
									cout << "incount = " << inCount << endl;
									///////////////////////////////////////////////////////////////////////////////////////////////

									/* snprintf(poststr, MAXSUB, "{\"camera_idx\":2,\"leftToright\": %d, \"rightToleft\": %d}\r\n", temp2, temp1);
									sockfd = socket(AF_INET, SOCK_STREAM, 0);
									bzero(&servaddr, sizeof(servaddr));
									servaddr.sin_family = AF_INET;
									servaddr.sin_port = htons(80);
									inet_pton(AF_INET, str, &servaddr.sin_addr);

									connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
									process_http(sockfd, hname, page, poststr);
									close(sockfd);*/
									////////////////////////////////////////////////////////////////////////////////////////////////////
								}
								if(tmp1 == 1 && tmp2 == 0 && tmp3 == 0 && tmp4 == 1){
									outCount++;
									cout << "outcount = " <<  outCount << endl;
									////////////////////////////////////////////////////////////////////////////////////////////////

									/*  snprintf(poststr, MAXSUB, "{\"camera_idx\":2,\"leftToright\": %d, \"rightToleft\": %d}\r\n", temp1, temp2);
									sockfd = socket(AF_INET, SOCK_STREAM, 0);
									bzero(&servaddr, sizeof(servaddr));
									servaddr.sin_family = AF_INET;
									servaddr.sin_port = htons(80);
									inet_pton(AF_INET, str, &servaddr.sin_addr);

									connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
									process_http(sockfd, hname, page, poststr);
									close(sockfd);*/
									///////////////////////////////////////////////////////////////////////////////////////////////
								}
//								bool tmp5 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,550,160);
//								bool tmp6 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,450,30);
//								bool tmp7 = isCounterClock(550,160,450,30,(*itor).past.x,(*itor).past.y);
//								bool tmp8 = isCounterClock(550,160,450,30,posX,posY);
//
//								if(tmp5 == 0 && tmp6 == 1 && tmp7 == 1 && tmp8 == 0){
//									outCount1++;
//									//cout << "incount = " << inCount << endl;
//									///////////////////////////////////////////////////////////////////////////////////////////////
//
//									/* snprintf(poststr, MAXSUB, "{\"camera_idx\":2,\"leftToright\": %d, \"rightToleft\": %d}\r\n", temp2, temp1);
//									sockfd = socket(AF_INET, SOCK_STREAM, 0);
//									bzero(&servaddr, sizeof(servaddr));
//									servaddr.sin_family = AF_INET;
//									servaddr.sin_port = htons(80);
//									inet_pton(AF_INET, str, &servaddr.sin_addr);
//
//									connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
//									process_http(sockfd, hname, page, poststr);
//									close(sockfd);*/
//									////////////////////////////////////////////////////////////////////////////////////////////////////
//								}
//								if(tmp5 == 1 && tmp6 == 0 && tmp7 == 0 && tmp8 == 1){
//									inCount1++;
//									//cout << "outcount = " <<  outCount << endl;
//									////////////////////////////////////////////////////////////////////////////////////////////////
//
//									/*  snprintf(poststr, MAXSUB, "{\"camera_idx\":2,\"leftToright\": %d, \"rightToleft\": %d}\r\n", temp1, temp2);
//									sockfd = socket(AF_INET, SOCK_STREAM, 0);
//									bzero(&servaddr, sizeof(servaddr));
//									servaddr.sin_family = AF_INET;
//									servaddr.sin_port = htons(80);
//									inet_pton(AF_INET, str, &servaddr.sin_addr);
//
//									connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
//									process_http(sockfd, hname, page, poststr);
//									close(sockfd);*/
//									///////////////////////////////////////////////////////////////////////////////////////////////
//								}
//
//								bool tmp9 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,100,300);
//								bool tmp10 = isCounterClock((*itor).past.x,(*itor).past.y,posX,posY,500,300);
//								bool tmp11 = isCounterClock(100,300,500,300,(*itor).past.x,(*itor).past.y);
//								bool tmp12 = isCounterClock(100,300,500,300,posX,posY);
//
////								char poststr[MAXLINE + 1];
//
//								if(tmp9 == 0 && tmp10 == 1 && tmp11 == 1 && tmp12 == 0){
//									inCount2++;
//									//cout << "incount = " << inCount2 << endl;
//									///////////////////////////////////////////////////////////////////////////////////////////////
//
//									/* snprintf(poststr, MAXSUB, "{\"camera_idx\":2,\"leftToright\": %d, \"rightToleft\": %d}\r\n", temp2, temp1);
//									sockfd = socket(AF_INET, SOCK_STREAM, 0);
//									bzero(&servaddr, sizeof(servaddr));
//									servaddr.sin_family = AF_INET;
//									servaddr.sin_port = htons(80);
//									inet_pton(AF_INET, str, &servaddr.sin_addr);
//
//									connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
//									process_http(sockfd, hname, page, poststr);
//									close(sockfd);*/
//									////////////////////////////////////////////////////////////////////////////////////////////////////
//								}
//								if(tmp9 == 1 && tmp10 == 0 && tmp11 == 0 && tmp12 == 1){
//									outCount2++;
//									//cout << "outcount = " <<  outCount2 << endl;
//									////////////////////////////////////////////////////////////////////////////////////////////////
//
//									/*  snprintf(poststr, MAXSUB, "{\"camera_idx\":2,\"leftToright\": %d, \"rightToleft\": %d}\r\n", temp1, temp2);
//									sockfd = socket(AF_INET, SOCK_STREAM, 0);
//									bzero(&servaddr, sizeof(servaddr));
//									servaddr.sin_family = AF_INET;
//									servaddr.sin_port = htons(80);
//									inet_pton(AF_INET, str, &servaddr.sin_addr);
//
//									connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
//									process_http(sockfd, hname, page, poststr);
//									close(sockfd);*/
//									///////////////////////////////////////////////////////////////////////////////////////////////
//								}
								(*itor).past = cvPoint(posX,posY);



								/* if ((hptr = gethostbyname(hname)) == NULL) {
								fprintf(stderr, " gethostbyname error for host: %s: %s",
								hname, hstrerror(h_errno));
								exit(1);
								}*/
								//printf("hostname: %s\n", hptr->h_name);
								//if (hptr->h_addrtype == AF_INET
								//    && (pptr = hptr->h_addr_list) != NULL) {
								//	printf("address: %s\n",
								//	       inet_ntop(hptr->h_addrtype, *pptr, str,
								//			 sizeof(str)));
								//} else {
								//	fprintf(stderr, "Error call inet_ntop \n");
								//}

								////////////////////////////////////////////////////////////////////////////////////////////////////////
							}

							sprintf(buffer,"%d",(*itor).id);
							sprintf(buffer1,"%d",(*itor).id);
							sprintf(buffer2,"%d",(*itor).id);

							cvPutText(frame, buffer, cvPoint(posX, posY), &font, CV_RGB(0, 255, 255));
							cvPutText(frame, buffer1, cvPoint(posX, posY), &font, CV_RGB(0, 255, 255));
							cvPutText(frame, buffer2, cvPoint(posX, posY), &font, CV_RGB(0, 255, 255));

							(*itor).lifetime++;
							break;
						}
					}
				}
				else{
					cvRectangle(Mask,cvPoint(posX-MASKSIZE,posY-MASKSIZE),cvPoint(posX+MASKSIZE,posY+MASKSIZE),CV_RGB(255,0,255),CV_FILLED,8);
					ROI tmp;
					tmp.id = ROI_num++;
					tmp.leftTop_x = posX-MASKSIZE;
					tmp.leftTop_y = posY-MASKSIZE;
					tmp.rightBot_x = posX+MASKSIZE;
					tmp.rightBot_y = posY+MASKSIZE;
					tmp.past = cvPoint(posX,posY);
					tmp.lifetime = 10;

					ROIs.push_back(tmp);
				}
				//}
			}
			contours = contours->h_next;
		}
		cvShowImage("Original Video",frame);
		cvShowImage("Mask Video",Mask);
		//cvShowImage("Test Video",test);
		if(cvWaitKey(10)== 'p')
			break;
	}
	cvReleaseCapture(&capture); //메모리해제
	cvReleaseImage(&frame);
	cvReleaseImage(&diffImage);
	cvDestroyAllWindows();
}
