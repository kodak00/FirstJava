#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <math.h>
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>
#include <gdfontl.h>
#include <gdfontg.h>
#include <zlib.h>
#include <zconf.h>
//oskim
/*
#include <cgiutil.h>
#include <nrutil.h>
#include <aws_data.h>
#include <stn_inf.h>
#include <map_ini.h>
#include <grid_img.h>
#include <disp_htm.h>

#define  IMG_DIR1      "/www/mis/web/tmp/rdr"
#define  IMG_DIR2      "/www/mis/web/tmp/rdr"
#define  CGI_DIR       "/cgi-bin/rdr"
#define  CMP_DIR       "/DATA/RDR/CMP"
*/
#define  BLANK1  -30000     /* No Area */
#define  BLANK2  -25000     /* No Data */
#define  BLANK3  -20000     /* Minimum Data */

struct TIME_SS
{
    short  YY;
    char   MM;
    char   DD;
    char   HH;
    char   MI;
    char   SS;
};

/*
레이더합성파일의 처음에 있는 정보 (24bytes)
*/

struct RDR_CMP_HEAD
{
    short  kind;                /* 종류 "10" */
    short  YY;                  /* 년 */
    char   MM;                  /* 월 */
    char   DD;                  /* 일 */
    char   HH;                  /* 시 */
    char   MI;                  /* 분 */
    struct TIME_SS tm_in;       /* 생성시각 */
    char   num_stn;             /* 합성된 레이더 지점수 */
    char   rem[8];              /* 예비 */
};

/*
레이더합성파일의 HEAD 다음에 있는 합성에 사용된 레이더정보 목록 (24*29bytes)
*/

struct RDR_CMP_STN_LIST
{
    char   stn_cd[8];           /* 레이더 지점코드 */
    struct TIME_SS tm;          /* 레이더 관측시각 */
    char   num1;                /* 예비 */
    struct TIME_SS tm_in;       /* 레이더 자료 생성시각 */
    char   num2;                /* 예비 */
};
