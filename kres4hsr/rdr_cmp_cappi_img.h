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
���̴��ռ������� ó���� �ִ� ���� (24bytes)
*/

struct RDR_CMP_HEAD
{
    short  kind;                /* ���� "10" */
    short  YY;                  /* �� */
    char   MM;                  /* �� */
    char   DD;                  /* �� */
    char   HH;                  /* �� */
    char   MI;                  /* �� */
    struct TIME_SS tm_in;       /* �����ð� */
    char   num_stn;             /* �ռ��� ���̴� ������ */
    char   rem[8];              /* ���� */
};

/*
���̴��ռ������� HEAD ������ �ִ� �ռ��� ���� ���̴����� ��� (24*29bytes)
*/

struct RDR_CMP_STN_LIST
{
    char   stn_cd[8];           /* ���̴� �����ڵ� */
    struct TIME_SS tm;          /* ���̴� �����ð� */
    char   num1;                /* ���� */
    struct TIME_SS tm_in;       /* ���̴� �ڷ� �����ð� */
    char   num2;                /* ���� */
};
