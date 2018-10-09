// Student ID:
// Name      :
// Date      : 2017.11.03

#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0
#define TIME	5

int FILTER_SIZE;
int FILTER_SCALE;
int *filter_GX,*filter_GY;

const char *inputfile_name[5] = {
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};
const char *outputSobel_name[5] = {
	"Sobel1.bmp",
	"Sobel2.bmp",
	"Sobel3.bmp",
	"Sobel4.bmp",
	"Sobel5.bmp"
};

void *PROCESS_1(void *);
void *PROCESS_2(void *);
void *PROCESS_3(void *);


typedef struct type{
	int number;
	unsigned char *pic_in, *pic_grey ,*pic_final ;
	int imgWidth;
	int imgHeight;
	sem_t sem;
}pic_content;


unsigned char RGB2grey(int w, int h ,int imgWidth ,unsigned char *pic_in)
{
	int tmp = (
		pic_in[3 * (h*imgWidth + w) + MYRED] +
		pic_in[3 * (h*imgWidth + w) + MYGREEN] +
		pic_in[3 * (h*imgWidth + w) + MYBLUE] )/3;

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char GaussianFilterX(int w, int h,int imgWidth ,int imgHeight,unsigned char *pic_grey)
{
	int tmp = 0;
	int a, b;
	int ws = (int)sqrt((float)FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;

		tmp += filter_GX[j*ws + i] * pic_grey[b*imgWidth + a];
	}

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}
unsigned char GaussianFilterY(int w, int h,int imgWidth ,int imgHeight,unsigned char *pic_grey)
{
	int tmp = 0;
	int a, b;
	int ws = (int)sqrt((float)FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;

		tmp += filter_GY[j*ws + i] * pic_grey[b*imgWidth + a];
	}

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

void *PROCESS_1(void *ptr){
		pic_content *temp=(pic_content *)ptr;
		int imgWidth, imgHeight;
		unsigned char *pic_in, *pic_grey, *pic_final;
		BmpReader* bmpReader = new BmpReader();
		pic_in = bmpReader->ReadBMP(inputfile_name[temp->number], &imgWidth, &imgHeight);
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));
		pic_content content[4];
		pthread_t process2[4];
		pthread_t process3[4];

		for(int i=0;i<4;i++){
			content[i].number=i;
			content[i].imgWidth=imgWidth;
			content[i].imgHeight=imgHeight;
			content[i].pic_grey=pic_grey;
			content[i].pic_in=pic_in;
			content[i].pic_final=pic_final;
		}
		for(int i=0;i<4;i++){
			sem_init(&(content[i].sem),0,0);
			pthread_create(&(process2[i]) , NULL , &PROCESS_2 , (void *)&content[i]);
			sem_post(&(content[i].sem));
			
		}
		for(int i=0;i<4;i++){
			pthread_join(process2[i] , NULL);
			pthread_create(&(process3[i]) , NULL , &PROCESS_3 , (void *)&content[i]);
		}
		for(int i=0;i<4;i++){
			pthread_join(process3[i] , NULL);
		}	

		bmpReader->WriteBMP(outputSobel_name[temp->number], imgWidth, imgHeight, pic_final);

		free(pic_in);
		free(pic_grey);
		free(pic_final);

		return NULL;
}

void *PROCESS_2(void *ptr){
	pic_content *temp=(pic_content *)ptr;
	sem_wait(&(temp->sem));
	switch(temp->number){
		case 0 :{
			for (int j = 0; j<temp->imgHeight/4; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					temp->pic_grey[j*temp->imgWidth + i] = RGB2grey(i , j , temp->imgWidth , temp->pic_in);
				}
			}
			break;
		}
		case 1 :{
			for (int j = temp->imgHeight/4; j<temp->imgHeight/2; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					temp->pic_grey[j*temp->imgWidth + i] = RGB2grey(i , j , temp->imgWidth , temp->pic_in);
				}
			}
			break;
		}
		case 2 :{
			for (int j = temp->imgHeight/2; j<3*temp->imgHeight/4; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					temp->pic_grey[j*temp->imgWidth + i] = RGB2grey(i , j , temp->imgWidth , temp->pic_in);
				}
			}
			break;
		}
		case 3:{
			for (int j = 3*temp->imgHeight/4; j<temp->imgHeight; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					temp->pic_grey[j*temp->imgWidth + i] = RGB2grey(i , j , temp->imgWidth , temp->pic_in);
				}
			}
			break;
		}
	}
	sem_post(&(temp->sem));
	return NULL;	
}


void *PROCESS_3(void *ptr){
	pic_content *temp=(pic_content *)ptr;
	sem_wait(&(temp->sem));
	switch(temp->number){
		case 0 :{
			for (int j = 0; j<temp->imgHeight/4; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					unsigned char x=GaussianFilterX(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					unsigned char y=GaussianFilterY(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					int tmp=sqrt(x*x+y*y);
					if(tmp>255)
						tmp=255;
					else if(tmp<0)
						tmp=0;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYRED] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYGREEN] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYBLUE] = (unsigned char)tmp;
				}
			}
			break;
		}
		case 1 :{
			for (int j = temp->imgHeight/4; j<temp->imgHeight/2; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					unsigned char x=GaussianFilterX(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					unsigned char y=GaussianFilterY(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					int tmp=sqrt(x*x+y*y);
					if(tmp>255)
						tmp=255;
					else if(tmp<0)
						tmp=0;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYRED] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYGREEN] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYBLUE] = (unsigned char)tmp;
				}
			}
			break;
		}
		case 2 :{
			for (int j = temp->imgHeight/2; j<3*temp->imgHeight/4; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					unsigned char x=GaussianFilterX(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					unsigned char y=GaussianFilterY(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					int tmp=sqrt(x*x+y*y);
					if(tmp>255)
						tmp=255;
					else if(tmp<0)
						tmp=0;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYRED] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYGREEN] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYBLUE] = (unsigned char)tmp;
				}
			}
			break;
		}
		case 3:{
			for (int j = 3*temp->imgHeight/4; j<temp->imgHeight; j++) {
				for (int i = 0; i<temp->imgWidth; i++){
					unsigned char x=GaussianFilterX(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					unsigned char y=GaussianFilterY(i , j , temp->imgWidth , temp->imgHeight , temp->pic_grey);
					int tmp=sqrt(x*x+y*y);
					if(tmp>255)
						tmp=255;
					else if(tmp<0)
						tmp=0;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYRED] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYGREEN] = (unsigned char)tmp;
					temp->pic_final[3 * (j*temp->imgWidth + i) + MYBLUE] = (unsigned char)tmp;
				}
			}
			break;
		}
	}
	sem_post(&(temp->sem));
	return NULL;	
}

int main()
{
	// read mask file
	FILE* mask;
	mask = fopen("mask_Sobel.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);

	filter_GX = new int[FILTER_SIZE];
	filter_GY = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_GX[i]);
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_GY[i]);
	fclose(mask);

	pthread_t process1[TIME];
	pic_content temp[TIME];
	
	for (int k = 0; k<TIME; k++){
		temp[k].number=k;
		pthread_create(&process1[k] , NULL , &PROCESS_1 , (void *)&temp[k]);
	}
	for(int k=0;k<TIME;k++)
		pthread_join(process1[k] , NULL);

	delete [] filter_GX;
	delete [] filter_GY;

	return 0;
}