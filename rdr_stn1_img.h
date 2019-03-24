#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iconv.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <math.h>
#include <zlib.h>
#include <zconf.h>
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>
#include <gdfontl.h>
#include <gdfontg.h>
#include <netcdf.h>
#include <hdf5.h>

#include "cgiutil.h"
#include "nrutil.h"
#include "stn_inf.h"
#include "aws3_data.h"
#include "map_ini.h"
#include "url_io.h"
#include "rsl_wrc.h"
#include "rdr_file_header.h"

#define  DEGRAD   3.1415927/180.0
#define  RADDEG   180.0/3.1415927
#define  RE  6371008.77   // 지구변경(m)
#define  MAX_SWEEP  20    // 레이더 최대 Sweep수
#define  MAX_GOV  100     // AWS관련 유관기관 최대수
#define  MAX_STN  5000    // AWS관련 최대 지점수
#define  NUM_AWS  1000    // 기상청AWS 최대 지점수
#define  RATE_RAY 2

//#define  RDR_RAW_DIR   "/DATA/RDR/RAW"        // 레이더 지점별 RAW파일
#define  RDR_RAW_DIR   "/DATA/INPUT/NCV"        // 레이더 지점별 RAW파일
#define  RDR_QCD_DIR   "/DATA/RDR/QCD"        // 레이더 지점별 QCD파일
#define  RDR_HSR_DIR   "/DATA/RDR/HSR"        // 레이더 지점별 HSR파일
#define  RDR_HCI_DIR   "/DATA/RDR/HCI"        // 레이더 지점별 HCI파일
#define  RDR_LNG_DIR   "/DATA/RDR/LNG"        // 레이더 지점별 480km파일
#define  RDR_VER_DIR   "/DATA/RDR/VER"        // 레이더 지점별 수직관측파일
#define  RDR_CMP_DIR   "/DATA/RDR/CMP"        // 레이더합성파일 누년저장소
#define  COLOR_SET_DIR "/rdr/REF/COLOR/"      // 색상표 파일 임시저장소
#define  MAP_DIR       "/DATA/GIS/MAP/dat"    // 지도파일이 있는 디저장소

// 합성된 자료에 저장된 기본값
#define  BLANK1  -30000     // No Area
#define  BLANK2  -25000     // No Data
#define  BLANK3  -20000     // Minimum Data

// 이미지 영역중 일부 기본값
#define  LEG_pixel   35     // 범례표시영역
#define  TITLE_pixel 20     // 제목표시영역

// 사용할 한글TTF
#define  FONTTTF  "/usr/share/fonts/korean/TrueType/gulim.ttf"

// Volume 목록
struct VOL_LIST {
  int  voln;
  char vol_cd[4];
};

//------------------------------------------------------------------------------
// 사용자 입력 변수
struct INPUT_VAR {
  int   seq_now;      // 현재시각 SEQ(분)
  int   seq;          // 기준시각 SEQ(분)
  char  stn[8];       // 레이더 지점코드
  char  rdr[8];       // HSR, HCI, 480 등
  char  vol[8];       // UF자료 변수 코드
  char  cpi[8];       // CPP(CAPPI), PPI(PPI)
  float stn_lon;      // 레이더사이트 경도(deg)
  float stn_lat;      // 레이더사이트 위도(deg)
  float stn_ht;       // 레이더사이트 해발고도(m)
  int   voln;         // obs에 해당하는 voln
  int   voln2;        // 2번째 voln
  int   voln3;        // 3번째 voln
  int   swpn;         // Sweep Number
  int   swpn2;        // 두번째 Sweep Number
  float swp_deg;      // Sweep 고도각(deg)
  float cappi_ht;     // CAPPI 고도(m)
  float pulse_width;  // 펄스폭(us)
  int   area;         // 영역
  float ang;          // RHI인경우 각도(deg)
  float range;        // 표출반경(m)
  char  map[8];       // 사용할 지도코드
  char  color[8];     // 요청한 색상표
  float ZRa;          // Z-R관계식의 계수(기본:200)  Z=aR^b
  float ZRb;          // Z-R관계식의 지수(기본:1.6)
  float grid;         // 격자크기(km)
  int   zoom_level;   // 확대횟수
  int   zoom_rate;    // 확대비율
  char  zoom_x[16];   // X방향 확대
  char  zoom_y[16];   // Y방향 확대
  char  auto_man;     // 자동여부

  // 화면표출용
  int   size;         // 이미지크기(픽셀) : NI에 해당
  int   legend;       // 범례표시(1) 여부
  int   aws;          // 지점정보 포함(0:지점표출없음, 1:지점명, 2:지점번호, 3:관측값)
  int   sms;          // 평활화 여부(횟수)

  // 산출된 자료
  int   num_gov;      // 유관기관 기관수
  char  gov_cd[MAX_GOV][8];  // 포함할 유관기관 TAG 문자열
  int   num_aws_stn;  // AWS 지점수
  int   num_area_stn; // 표출영역내 AWS 지점수
  int   NX, NY;       // 격자수, 따라서 격자점수는 각각에 1을 더하면 됨
  int   NI, NJ;       // 자료표출이미지 영역(픽셀)
  int   GI, GJ;       // 전체 이미지 영역(픽셀)
  int   num_color;    // 색상수

  // 파일정보
  char  fname[120];   // 파일명
  int   cdf;          // 1(nectCDF), 0(UF)
  int   dir_mode;     // 1 이면 RDR_FTP_DIR 사용
  int   HSR_mask;     // 1이면 마스킹 가능
};

// 지점 자료
struct STN_VAL {
  int   stn_id;       // 지점번호
  char  stn_ko[32];   // 지점명(한글)
  char  stn_sp[32];   // 지점득성
  float x, y;         // 위치
  float ht;           // 해발고도(m)
  float d;            // 값
  float v;            // 값(바람벡터시 V풍속 사용)
  float wd;           // 풍향
  float ws;           // 풍속
  int   re;           // 강수유무(강수감지 또는 15분강수 있으면)
  float s;            // 임시저장소
};
