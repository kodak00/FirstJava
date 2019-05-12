#ifndef LGT_BASIN_DECADE_H_
#define LGT_BASIN_DECADE_H_

#define BUFFER_SIZE 1024

#define MI 960
#define MJ 1200

#define NI 480
#define NJ 600

#define BASIN_LEN 247

#define INPUT_PATH "/DATA/INPUT/LGT/"
#define OUTPUT_PATH "/DATA/OUTPUT/BIN/BASI/"

//압축파일 rear / write 데이터
typedef struct{
    int basinCnt;
    int color;
    int code;           //지역 코드
    int lgtCnt;         //낙뢰 총 횟수
    double totImpact;   //강도 총합
    float maxImpact;    //최대 강도
    float minImpact;    //최소 강도
} BASIN_DATA;

int clearBasinData(BASIN_DATA* basin_data);
int read_src_data(short nYear, BASIN_DATA* basin_data);
int merge_year_data(short nStart, short nEnd);

#endif /* LGT_BASIN_DECADE_H_ */
