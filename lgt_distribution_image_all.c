#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <zlib.h>

#include "nrutil.h"
#include "map_ini.h"
#include "lgt_distribution_image.h"

char *strptime(const char *s, const char *format, struct tm *tm);

/* 현재 시간부터 10분 누적 낙뢰 자료 읽기 */
DATA_UNIT** read_lgt_txt_data(DATA_UNIT **DATA, char* readFile, struct tm curTime , int dateMode)
{
	FILE *rfp;
	char buf[BUFFER_SIZE];
	int mon, day, yr, hr, min, itmp, num_sns;
	float sec, lat, lon, imp, ftmp;
	char type;
	float x,y;
	int ix, iy;
	
	char stmp[BUFFER_SIZE];
	long ltmp; double fftmp;

	struct lamc_parameter map;
	MAP_INFO mapInfo = comp_map_info();
	//MAP_INFO mapInfo = comp_map_info_1km();
	map = mapInfo.map;
	
	//시간 설정
	char nowTimeStr[BUFFER_SIZE];
	char agoTimeStr[BUFFER_SIZE];
	char dataTimeStr[BUFFER_SIZE];
	struct tm agoTime;

	strftime(nowTimeStr, BUFFER_SIZE, "%Y%m%d%H%M", &curTime);

	agoTime = incMin(curTime, -10);
	strftime(agoTimeStr, BUFFER_SIZE, "%Y%m%d%H%M", &agoTime);
	
	fprintf(stderr, "############ dateMode : %d all mode\n"  , dateMode);

	if( (rfp = fopen(readFile, "r")) != NULL ){
		fprintf(stderr, "\t\t TXT read file name : %s\n", readFile);
		while (fgets(buf, BUFFER_SIZE, rfp) != NULL) {	  
		
		// 구자료	
          if( dateMode == 0 )
		  {
			sscanf(buf, "%d/%d/%d %d:%d:%f %f %f %f %d %f %f %f %d %f %d %c ",
				&mon, &day, &yr,
				&hr, &min, &sec,
				&lat, &lon, &imp, &itmp, &ftmp, &ftmp, &ftmp, &itmp, &ftmp, &num_sns, &type);
			//oskim
			sprintf(dataTimeStr, "%04d%02d%02d%02d%02d", yr + 2000, mon, day, hr, min);
		  }
		  else
		  {
				sscanf(buf, "%ld %d-%d-%d %d:%d:%lf+%d:%d %d %s %f %f %f %f %f %d %d" , 
					&ltmp , &yr, &mon, &day , &hr, &min, &fftmp, &itmp , &itmp , &itmp , &stmp , &lon, &lat , &ftmp, &imp , &ftmp, &type , &num_sns);
			//oskim
			sprintf(dataTimeStr, "%04d%02d%02d%02d%02d", yr, mon, day, hr, min);
		  }

			//sprintf(dataTimeStr, "%04d%02d%02d%02d%02d", yr + 2000, mon, day, hr, min);

			x = -999.0;
			y = -999.0;
			lamcproj(&lon, &lat, &x, &y, 0, map);
			ix = (int)x;
			iy = (int)y;
			if (ix < 0 || ix > MI || iy < 0 || iy > MJ) continue;
			
			// 2013.05.09
			//DATA[iy][ix]++;
			if(num_sns > 2 && (imp < 0 || imp > 10)) 
			{
				fprintf(stderr, "-> %s %f %f %f %d %d \n", dataTimeStr , lon , lat , imp ,type , num_sns);
				//DATA[iy][ix]++;
				DATA[iy][ix].ground++;
			}
		}

		fclose(rfp);
	}else{
		printf("file not found : %s\n", readFile);
	}

	return DATA;
}

// 낙뢰 자료 합산 후 쓰기 
int write_lgt_data(DATA_UNIT **DATA, char* writeFile)
{
	FILE *rfp, *wfp;
	DATA_UNIT **TDATA;
	int i,j,n;
	int ibuf[MI];
	
	char command[BUFFER_SIZE];
	char tmpString[BUFFER_SIZE];

	TDATA = umatrix(0, MJ, 0, MI);
	TDATA = data_init(TDATA);

	// create directory
	strncpy(tmpString, writeFile, strlen(writeFile)-strlen(strrchr( writeFile, '/')));
	tmpString[strlen(writeFile)-strlen(strrchr( writeFile, '/'))] = '\0';
	sprintf(command, "mkdir -p %s", tmpString);
	system(command);

	//file read
	if( (rfp = gzopen(writeFile, "r")) != NULL ){
		//printf("\t\t read file name : %s\n", writeFile);
		for(j=0; j<MJ; j++){
			n = gzread(rfp, ibuf, MI*sizeof(DATA_UNIT));
			for(i=0; i<MI; i++){
				//TDATA[j][i] = ibuf[i];
				memcpy(&TDATA[j][i], &ibuf[i], sizeof(DATA_UNIT));
			}
		}
		gzclose(rfp);
	}


	for(j=0; j<MJ; j++){
		for(i=0; i<MI; i++){
			//TDATA[j][i] += DATA[j][i];
			TDATA[j][i].ground 	+= DATA[j][i].ground;
			TDATA[j][i].cloud 	+= DATA[j][i].cloud;
			TDATA[j][i].strength+= DATA[j][i].strength;
		}
	}

	if((wfp = gzopen(writeFile, "w")) != NULL){
		//printf("\t\t DAY write file name : %s\n", writeFile);
		for(j=0; j<MJ; j++){
			for(i=0; i<MI; i++){
				if(gzwrite(wfp, &TDATA[j][i], sizeof(DATA_UNIT)) < 0){
					printf("write file error!\n");
					return -1;
				}
			}
		}
		gzclose(wfp);
	}
	
	free_umatrix(TDATA, 0, MJ, 0, MI);

	return 0;
}

DATA_UNIT **umatrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
    int i;
    DATA_UNIT   **m;

    m=(DATA_UNIT **)malloc((unsigned) (nrh-nrl+1)*sizeof(DATA_UNIT*));
    if (!m) nrerror("allocation failure 1 in imatrix()");
    m -= nrl;

    for(i=nrl;i<=nrh;i++)
    {
        m[i]=(DATA_UNIT *)malloc((unsigned) (nch-ncl+1)*sizeof(DATA_UNIT));
        if (!m[i]) nrerror("allocation failure 2 in imatrix()");
        m[i] -= ncl;
    }
    return m;
}

void free_umatrix(m,nrl,nrh,ncl,nch)
DATA_UNIT **m;
int nrl,nrh,ncl,nch;
{
    int i;

    for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
    free((char*) (m+nrl));
}

DATA_UNIT** data_init(DATA_UNIT** DATA)
{
	int i, j;

	for(j=0; j<MJ; j++){
		for(i=0; i<MI; i++){
			memset(&DATA[j][i], 0, sizeof(DATA_UNIT));
			//DATA[j][i] = 0;
		}
	}

	return DATA;
}

struct tm incMin(struct tm tm_ptr,int addMin)
{
	time_t the_time;
	struct tm new_tm_ptr;
	the_time = mktime(&tm_ptr);
	the_time = the_time + (60 * addMin);
	new_tm_ptr = *(localtime(&the_time));
	return new_tm_ptr;
}

