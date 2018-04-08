#ifndef CGI_COMP_ORIGIN_DATA_H
#define CGI_COMP_ORIGIN_DATA_H

/* ================================================================================ */
// DEFINE

//oskim
//#define COMP_XDIM_240KM     960
#define COMP_XDIM_240KM     1152
#define COMP_XDIM_480KM     526
#define COMP_YDIM_480KM     576
#define COMP_XDIM_CJ3       751

/* ================================================================================ */
// STRUCT

typedef  struct 
{
    int     rgnforecast_period[10];
    int     nforecasts;
} stForecast_hed;

typedef struct 
{    
    int     nYear;
    int     nMonth;
    int     nDay;
    int     nHours;
    int     nMinutes;       
    int     nx;
    int     ny;       
    float   fClat;
    float   fClon;
    float   fYminl;
    float   fYmaxl;
    float   fXminl;
    float   fXmaxl;
    int     nForecast_period;
    int     nMaptype; 
} stArrayhed;

/* ================================================================================ */
// FUNCTION PROTO

float**     fnGetCompData(char szF_name[], int nDataXdim, int nDataYdim);
float**     fnGetCompVsrfData(char szF_name[], int nDataXdim, int nDataYdim, int nFcstTime);
float**     fnGetCompMapleData(char szF_name[], int nDataXdim, int nDataYdim, int nFcstTime);
float**     fnGetComp480Data(char szF_name[], int nDataXdim, int nDataYdim, unsigned char szIn_bound[576][526]);
float**     fnGetCompCj3Data(char szF_name[], int nDataXdim, int nDataYdim, int nFileFlag);

/* ================================================================================ */

#endif /* CGI_COMP_ORIGIN_DATA_H */
