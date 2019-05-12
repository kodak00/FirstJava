#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <zlib.h>
#include "gd.h"

#include "make_lgt_basin_decade.h"

int main(int arc, char *argv[])
{
	short nStartYear = 0;
	short nEndYear = 0;

	if(arc != 3){
		printf("Usage:$make_lgt_basin_decade 2009 2018\n");
		return 0;
	}

	nStartYear = atoi(argv[1]);
	nEndYear = atoi(argv[2]);

	merge_year_data(nStartYear, nEndYear);

	return 0;
}

int clearBasinData(BASIN_DATA* basin_data)
{
	int i = 0;
	for(i=0; i<basin_data[0].basinCnt; i++){
		basin_data[i].lgtCnt = 0;
		basin_data[i].totImpact = 0.0;
		basin_data[i].maxImpact = 0.0;
		basin_data[i].minImpact = 999.0;
	}
	return 0;
}

int read_src_data(short nYear, BASIN_DATA* basin_data)
{	
	FILE *rfp = NULL;
	int ibuf[2];
	double dbuf[1];
	float fbuf[2];
	int basinLen = 0;
	int i = 0;
	char pSrcFile[1024] = {0,};

	sprintf(pSrcFile,"/DATA/OUTPUT/BIN/BASI/%d/LGT_KMA_BASI_%d.bin.gz", nYear, nYear); 

	if((rfp = gzopen(pSrcFile, "rb")) != NULL){
		gzread(rfp, &basinLen, sizeof(int));

		if(basinLen != BASIN_LEN){
			printf("BASIN_LEN is diff!\n");
			exit(0);
		}

		for(i=0; i<basinLen; i++)
		{
			gzread(rfp, &ibuf, sizeof(ibuf));
			basin_data[i].code = ibuf[0];
			basin_data[i].lgtCnt = ibuf[1];

			gzread(rfp, &dbuf, sizeof(dbuf));
			basin_data[i].totImpact = dbuf[0];

			gzread(rfp, &fbuf, sizeof(fbuf));

			basin_data[i].maxImpact = fbuf[0];
			basin_data[i].minImpact = fbuf[1];
		}
		gzclose(rfp);
	}
	else
	{
		printf("pSrcFile open error!\n");
		return -1;
	}

	return 0;
}

int merge_year_data(short nStart, short nEnd)
{
	FILE *wfp = NULL;
	int i, j, ret = 0;
	int lerrno = 0;

	int ibuf[2] = {0,};
	double dbuf[1] = {0,};
	float fbuf[2] = {0,};
	int basinLen = BASIN_LEN;

	char pMrgFile[1024] = {0,};

	sprintf(pMrgFile,"/DATA/OUTPUT/BIN/BASI/LGT_KMA_BASI_DECADE_%d.bin.gz", nStart); 

	BASIN_DATA *basin_tmp = (BASIN_DATA *)malloc(basinLen * sizeof(BASIN_DATA));
	basin_tmp[0].basinCnt = BASIN_LEN; 
	clearBasinData(basin_tmp);

	if((wfp = gzopen(pMrgFile, "w")) != NULL){

			for(j = 0; j <= nEnd-nStart; j++){

					BASIN_DATA *basin_data = (BASIN_DATA *)malloc(basinLen * sizeof(BASIN_DATA));
					if(read_src_data(nStart + j, basin_data) == -1){
						free(basin_data);
						break;
					}

					for(i = 0; i<basinLen; i++){

						if(j == 0)
							basin_tmp[i].code = basin_data[i].code;
							//memcpy(&basin_tmp[i],&basin_data[i],sizeof(BASIN_DATA));
						///*
						if(basin_tmp[i].code == basin_data[i].code){
							basin_tmp[i].lgtCnt += basin_data[i].lgtCnt;
							if(basin_tmp[i].lgtCnt > 0){
								basin_tmp[i].totImpact += basin_data[i].totImpact;
								if(basin_tmp[i].maxImpact < basin_data[i].maxImpact){
									basin_tmp[i].maxImpact = basin_data[i].maxImpact;
								}
								if(basin_tmp[i].minImpact > basin_data[i].minImpact){
									basin_tmp[i].minImpact = basin_data[i].minImpact;
								}
							}
						}
						//*/
					}

					free(basin_data);
			}

			if(gzwrite(wfp, &basinLen, sizeof(int)) < 0){
				printf("%s\n",gzerror(wfp, &lerrno));
			}
			for(i=0; i<basinLen; i++){
				if(gzwrite(wfp, &basin_tmp[i].code, sizeof(int)) < 0){
					printf("%s\n",gzerror(wfp, &lerrno));
				}
				if(gzwrite(wfp, &basin_tmp[i].lgtCnt, sizeof(int)) < 0){
					printf("%s\n",gzerror(wfp, &lerrno));
				}
				if(gzwrite(wfp, &basin_tmp[i].totImpact, sizeof(double)) < 0){
					printf("%s\n",gzerror(wfp, &lerrno));
				}
				if(gzwrite(wfp, &basin_tmp[i].maxImpact, sizeof(float)) < 0){
					printf("%s\n",gzerror(wfp, &lerrno));
				}
				if(gzwrite(wfp, &basin_tmp[i].minImpact, sizeof(float)) < 0){
					printf("%s\n",gzerror(wfp, &lerrno));
				}
			}
			gzclose(wfp);
	}

	free(basin_tmp);

	return 0;
}
