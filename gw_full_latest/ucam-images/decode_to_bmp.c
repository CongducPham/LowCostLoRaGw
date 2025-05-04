/*
 *  Copyright (C) 2016 Congduc Pham
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

char* originalFile;
// for onlydisplay
char* file_to_decode;

// decode original or received .dat file?
bool decode_received_dat=false;
// vertically flip or not
bool vflip=false;

int SN=0;
int srcAddr=0;
int camid=0;

uint8_t str2hex(char* str)
{
        int aux=0, aux2=0;

        if( (*str>='0') && (*str<='9') )
        {
                aux=*str++-'0';
        }
        else if( (*str>='A') && (*str<='F') )
        {
                aux=*str++-'A'+10;
        }
        if( (*str>='0') && (*str<='9') )
        {
                aux2=*str-'0';
        }
        else if( (*str>='A') && (*str<='F') )
        {
                aux2=*str-'A'+10;
        }
        return aux*16+aux2;
}


// begin from CRAN

/************************************************************************
 *									*
 *	Fast DCT for image compression scheme				*
 *									*
 *	Author: Vincent LECUIRE, CRAN UMR 7039, Nancy-Université, CNRS	*
 *	Date: march, 16 2012						*
 *									*
 ************************************************************************/

#include "bmp.h"
#include "mqc.h"

#define CRAN_ENCODING_IMAGE_TYPE -1

#define a4   1.38703984532215
#define a7  -0.275899379282943
#define a47  0.831469612302545
#define a5   1.17587560241936
#define a6  -0.785694958387102
#define a56  0.98078528040323
#define a2   1.84775906502257
#define a3   0.765366864730179
#define a23  0.541196100146197
#define a32  1.306562964876376
#define rc2  1.414213562373095

double OriginalLuminanceJPEGTable[8][8] = {
  16.0,  11.0,  10.0,  16.0,  24.0,  40.0,  51.0,  61.0,
  12.0,  12.0,  14.0,  19.0,  26.0,  58.0,  60.0,  55.0,
  14.0,  13.0,  16.0,  24.0,  40.0,  57.0,  69.0,  56.0,
  14.0,  17.0,  22.0,  29.0,  51.0,  87.0,  80.0,  62.0,
  18.0,  22.0,  37.0,  56.0,  68.0, 109.0, 103.0,  77.0,
  24.0,  35.0,  55.0,  64.0,  81.0, 104.0, 113.0,  92.0,
  49.0,  64.0,  78.0,  87.0, 103.0, 121.0, 120.0, 101.0,
  72.0,  92.0,  95.0,  98.0, 112.0, 100.0, 103.0,  99.0
};

double LuminanceJPEGTable[8][8] = {
  16.0,  11.0,  10.0,  16.0,  24.0,  40.0,  51.0,  61.0,
  12.0,  12.0,  14.0,  19.0,  26.0,  58.0,  60.0,  55.0,
  14.0,  13.0,  16.0,  24.0,  40.0,  57.0,  69.0,  56.0,
  14.0,  17.0,  22.0,  29.0,  51.0,  87.0,  80.0,  62.0,
  18.0,  22.0,  37.0,  56.0,  68.0, 109.0, 103.0,  77.0,
  24.0,  35.0,  55.0,  64.0,  81.0, 104.0, 113.0,  92.0,
  49.0,  64.0,  78.0,  87.0, 103.0, 121.0, 120.0, 101.0,
  72.0,  92.0,  95.0,  98.0, 112.0, 100.0, 103.0,  99.0
};

struct position { int row;	int col;
	} ZigzagCoordinates[8*8]=	// Matrice Zig-Zag
	{0, 0, 0, 1, 1, 0, 2, 0, 1, 1, 0, 2, 0, 3, 1, 2, 2, 1, 3, 0,
	 4, 0, 3, 1, 2, 2, 1, 3, 0, 4, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1,
	 5, 0, 6, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1, 5, 0, 6, 0, 7, 1, 6,
	 2, 5, 3, 4, 4, 3, 5, 2, 6, 1, 7, 0, 7, 1, 6, 2, 5, 3, 4, 4,
	 3, 5, 2, 6, 1, 7, 2, 7, 3, 6, 4, 5, 5, 4, 6, 3, 7, 2, 7, 3,
	 6, 4, 5, 5, 4, 6, 3, 7, 4, 7, 5, 6, 6, 5, 7, 4, 7, 5, 6, 6,
	 5, 7, 6, 7, 7, 6, 7, 7};

/*------------------------------- Compression JPEG ----------------------------------------*/
void QTinitialization(int Quality)
{
 double Qs;

 if (Quality <= 0)  Quality = 1;
 if (Quality > 100) Quality = 100;
 if (Quality < 50)   Qs = 50.0 / (double) Quality;
	else	     Qs = 2.0 - (double) Quality/50.0;


 // Calcul des coefficients de la table de quantification
 for (int u=0; u<8; u++)
   for (int v=0; v<8; v++)
	{
	 LuminanceJPEGTable[u][v] = OriginalLuminanceJPEGTable[u][v] * Qs;
	 if (LuminanceJPEGTable[u][v] < 1.0) LuminanceJPEGTable[u][v]=1.0;
	 if (LuminanceJPEGTable[u][v] > 255.0) LuminanceJPEGTable[u][v]=255.0;
	}

 return;
}


void JPEGdecoding(BMPImageStruct *InputImage , BMPImageStruct *OutputImage)
{
   double Block[8][8];
   double tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
   double tmp10, tmp11, tmp12, tmp13, tmp20, tmp23;
   double tmp;

   // Decodage bloc par bloc
   for (int i=0; i<InputImage->imageVsize; i=i+8)
	 for (int j=0; j<InputImage->imageHsize; j=j+8)
	{
	 for (int u=0; u<8; u++)
	   for (int v=0; v<8; v++) Block[u][v]=InputImage->data[i+u][j+v];

	// Quantification inverse
	for (int u=0; u<8; u++)
	   for (int v=0; v<8; v++)
	   Block[u][v] = Block[u][v] * LuminanceJPEGTable[u][v];

	// DCT inverse
	Block[0][0]+=1024.0;

	for (int u=0; u<8; u++)
	  {
	   // 1ère étape: Partie paire
	   tmp0=Block[u][0]+Block[u][4];
	   tmp1=Block[u][0]-Block[u][4];
	   tmp=(Block[u][2]+Block[u][6])*a23;
	   tmp2=tmp-(a2*Block[u][6]);
	   tmp3=tmp+(a3*Block[u][2]);

	   tmp10=tmp0+tmp3;
	   tmp13=tmp0-tmp3;
	   tmp11=tmp1+tmp2;
	   tmp12=tmp1-tmp2;

	   // 1ème étape: Partie impaire
	   tmp0=Block[u][1]-Block[u][7];
	   tmp3=Block[u][1]+Block[u][7];
	   tmp1=Block[u][3]*rc2;
	   tmp2=Block[u][5]*rc2;
	   tmp4=tmp0+tmp2;
	   tmp6=tmp0-tmp2;
	   tmp7=tmp3+tmp1;
	   tmp5=tmp3-tmp1;

	   // 2ème étape
	   tmp=(tmp4+tmp7)*a47;
	   tmp20=tmp-(a4*tmp7);
	   tmp23=tmp+(a7*tmp4);
	   Block[u][0]=tmp10+tmp23;
	   Block[u][7]=tmp10-tmp23;
	   Block[u][3]=tmp13+tmp20;
	   Block[u][4]=tmp13-tmp20;

	   tmp=(tmp5+tmp6)*a56;
	   tmp20=tmp-(a5*tmp6);
	   tmp23=tmp+(a6*tmp5);
	   Block[u][2]=tmp12+tmp20;
	   Block[u][5]=tmp12-tmp20;
	   Block[u][1]=tmp11+tmp23;
	   Block[u][6]=tmp11-tmp23;
	  }

	for (int v=0; v<8; v++)
	  {
	   // 1ère étape: Partie paire
	   tmp0=Block[0][v]+Block[4][v];
	   tmp1=Block[0][v]-Block[4][v];
	   tmp=(Block[2][v]+Block[6][v])*a23;
	   tmp2=tmp-(a2*Block[6][v]);
	   tmp3=tmp+(a3*Block[2][v]);

	   tmp10=tmp0+tmp3;
	   tmp13=tmp0-tmp3;
	   tmp11=tmp1+tmp2;
	   tmp12=tmp1-tmp2;

	   // 1ème étape: Partie impaire
	   tmp0=Block[1][v]-Block[7][v];
	   tmp3=Block[1][v]+Block[7][v];
	   tmp1=Block[3][v]*rc2;
	   tmp2=Block[5][v]*rc2;
	   tmp4=tmp0+tmp2;
	   tmp6=tmp0-tmp2;
	   tmp7=tmp3+tmp1;
	   tmp5=tmp3-tmp1;

	   // 2ème étape
	   tmp=(tmp4+tmp7)*a47;
	   tmp20=tmp-(a4*tmp7);
	   tmp23=tmp+(a7*tmp4);
	   Block[0][v]=tmp10+tmp23;
	   Block[7][v]=tmp10-tmp23;
	   Block[3][v]=tmp13+tmp20;
	   Block[4][v]=tmp13-tmp20;

	   tmp=(tmp5+tmp6)*a56;
	   tmp20=tmp-(a5*tmp6);
	   tmp23=tmp+(a6*tmp5);
	   Block[2][v]=tmp12+tmp20;
	   Block[5][v]=tmp12-tmp20;
	   Block[1][v]=tmp11+tmp23;
	   Block[6][v]=tmp11-tmp23;
	  }

	// On vérifie que cela ne déborde pas
	for (int u=0; u<8; u++)
	   for (int v=0; v<8; v++)
	{
	   Block[u][v]/=8.0;
	   if (Block[u][v] < 0.0) Block[u][v]=0.0;
	   if (Block[u][v] > 255.0) Block[u][v]=255.0;
	}

	// On range le résultat dans l'image de sortie
	 for (int u=0; u<8; u++)
	   for (int v=0; v<8; v++)
		OutputImage->data[i+u][j+v]=Block[u][v];
   }

   return;
}


int JPEGdepacketization(BMPImageStruct *OutputImage, FILE* TRACEFILE)
{
	int Block[8][8];
	unsigned int BlockOffset, row, col, row_mix, col_mix, packetsize, index, q, r, K;
   	opj_mqc_t mqobjet;
   	opj_mqc_t *objet=NULL;
   	unsigned char buffer[MQC_NUMCTXS];
	
	// On lit la taille du prochain packet
	if (fscanf(TRACEFILE, "%4X", &packetsize)==EOF) return 0;

	fscanf(TRACEFILE, "%2X %2X", &row, &col);
	BlockOffset = row * 256 + col;
	packetsize -= 2;
	
	for (int x=0; x<packetsize; x++) fscanf(TRACEFILE, "%2X", &buffer[x]);
	//packetcount++;
	objet=&mqobjet;
	mqc_init_dec(objet, buffer, packetsize);
	mqc_resetstates(objet);
	
   	while (mqc_numbytes(objet) < packetsize) {
		// On décode
		q=0;
		while(mqc_decode(objet)==1) q++;
		r=mqc_decode(objet);
		K=q*2+r;
		for (int x=0; x<K; x++)
		{
		q=0;
		while(mqc_decode(objet)==1) q++;
		r=mqc_decode(objet);
		index=q*2+r;
		if ((index % 2) == 0)
			{  Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]=index / 2;	}
		   else {  Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]=0-((index+1) / 2);	}
		}
		for (int x=K; x<64; x++) Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]=0;
		// On copie le bloc décodé à sa place dans l'image
		row = (BlockOffset * 8) / OutputImage->imageHsize * 8;
		col = (BlockOffset * 8) % OutputImage->imageHsize;
		row_mix = ((row * 5) + (col *  8)) % (OutputImage->imageHsize);
		col_mix = ((row * 8) + (col * 13)) % (OutputImage->imageVsize);
		for (int u=0; u<8; u++)
		   for (int v=0; v<8; v++) OutputImage->data[row_mix+u][col_mix+v]=(double) Block[u][v];
		BlockOffset++;
   	}
		
   	if (BlockOffset != (OutputImage->imageHsize * OutputImage->imageVsize / 64)) {
		q=0;
		while((mqc_decode(objet)==1) && (q < 32)) q++;
		r=mqc_decode(objet);
		K=q*2+r;
		if (K > 64) return packetsize;
		for (int x=0; x<K; x++)
		{
		q=0;
		while((mqc_decode(objet)==1) && (q < 32)) q++;
		r=mqc_decode(objet);
		index=q*2+r;
		if ((index % 2) == 0)
			{  Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]=index / 2;	}
		   else {  Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]=0-((index+1) / 2);	}
		}
		for (int x=K; x<64; x++) Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]=0;
		// On copie le bloc décodé à sa place dans l'image
		row = (BlockOffset * 8) / OutputImage->imageHsize * 8;
		col = (BlockOffset * 8) % OutputImage->imageHsize;
		row_mix = ((row * 5) + (col *  8)) % (OutputImage->imageHsize);
		col_mix = ((row * 8) + (col * 13)) % (OutputImage->imageVsize);
		for (int u=0; u<8; u++)
		   for (int v=0; v<8; v++) OutputImage->data[row_mix+u][col_mix+v]=(double) Block[u][v];
		BlockOffset++;
   	}

   return packetsize;
}

// end from CRAN


void startDecodeImage(FILE* theFile, int Q, int SN, char* originalFile, int srcAddr, int camId) {


    BMPImageStruct OriginalImage;
    char tmpString[80];
    char bmpFile[80];
    int psize, npkt=0, totalsize=0;

    // Read the original BMP file to get useful informations
    int err = ReadBitmapFile(originalFile, &OriginalImage);

    if (err) {
            fprintf(stderr, "CANNOT read original BMP file %s\n", originalFile);

    }
    else {
            // reset content
            for (int i=0; i<OriginalImage.imageVsize; i++)
                    for (int j=0; j<OriginalImage.imageHsize; j++) OriginalImage.data[i][j]=0.0;

			fprintf(stderr, "Start JPEGdepacketization\n");
			
            // JPEG decoding
            while ((psize=JPEGdepacketization(&OriginalImage, theFile))) {
				totalsize+=psize;
  				npkt++;
	    	}

			fprintf(stderr, "Start JPEGdecoding\n");
			
            JPEGdecoding(&OriginalImage, &OriginalImage);

	    	fprintf(stderr, "Encoded file size is %d, npkt is %d\n", totalsize, npkt);

            sprintf(bmpFile,"ucam_%d-node_%04X-cam_%d-Q%d-P%d-S%d.bmp", SN, srcAddr, camId, Q, npkt, totalsize);

            err = WriteBitmapFile(bmpFile, &OriginalImage, vflip);

            if (err) {
                    fprintf(stderr, "CANNOT write ucam BMP file from received encoded image.\n");
                    printf("error\n");
            }
            else
            	printf("%s\n", bmpFile);
            
    }
}


void * printERROR(char *argv[])
{
   fprintf(stderr, "USAGE:\t%s -vflip -original/-received -SN sn -src src -camid camid -Q q orig_image_file_name\n", argv[0]);
   fprintf(stderr, "USAGE:\t-vflip, flip vertically the image\n");
   fprintf(stderr, "USAGE:\t-original img_file.dat, only decode the .dat file (produced by the encoder)\n");
   fprintf(stderr, "USAGE:\t-received img_file.dat, only decode the .dat file (previously received)\n");
   fprintf(stderr, "USAGE:\t-SN sn, use sn as image sequence number\n");
   fprintf(stderr, "USAGE:\t-src src, use src as source node address\n"); 
   fprintf(stderr, "USAGE:\t-camid camid, use camid as camera id (index)\n");          
   fprintf(stderr, "USAGE:\t-Q 40, use 40 as Quality Factor, default is 50\n");
   fprintf(stderr, "USAGE:\t orig_image_file_name, give the original bmp file\n");
   return 0;
}


int main(int argc, char *argv[]) {

    int qualityFactor=50;

    if (argc < 2) {
       printERROR(argv);
       exit (-1);
    }

    for (int arg = 1; arg < argc-1; arg++) {

        if (!strcmp(argv[arg], "-vflip"))
        {
            vflip=true;
        }

        if (!strcmp(argv[arg], "-original"))
        {
	    	decode_received_dat=false;
            file_to_decode=argv[arg+1];
        }

        if (!strcmp(argv[arg], "-received"))
        {
	    	decode_received_dat=true;
            file_to_decode=argv[arg+1];
        }

        if (!strcmp(argv[arg], "-Q")) {
            qualityFactor=atoi(argv[arg+1]);
        }
        
        
        if (!strcmp(argv[arg], "-SN")) {
            SN=atoi(argv[arg+1]);
        }
        
        
        if (!strcmp(argv[arg], "-src")) {
            srcAddr=atoi(argv[arg+1]);
        }
        
        
        if (!strcmp(argv[arg], "-camid")) {
            camid=atoi(argv[arg+1]);
        }
    }

	originalFile=argv[argc-1];

	FILE* pFile = fopen(file_to_decode, "r");

	unsigned int tmp;

	if (!decode_received_dat) {
		fprintf(stderr, "Decoding original image\n");
		
		// skip the field that are not usefull for display, just keep the quality Factor
		fscanf(pFile, "%04X", &tmp);
		fscanf(pFile, "%04X", &tmp);
		fscanf(pFile, "%04X", &tmp);
		fscanf(pFile, "%04X", &tmp);
		
		qualityFactor=tmp;	
	}
	else
		fprintf(stderr, "Decoding received image\n");

	// Initialize the quantization matrix
	QTinitialization(qualityFactor);

	fprintf(stderr, "Decode image %s, original BMP file is %s, original QualityFactor is %d\n", file_to_decode, originalFile, qualityFactor);

	startDecodeImage(pFile, qualityFactor, SN, originalFile, srcAddr, camid);

	fclose(pFile);
}
