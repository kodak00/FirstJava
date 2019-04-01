/* ================================================================================ */
//
// 레이더 에코 영상 GIS 소스
//
// 2016. 07. 29 수정 : Choi Hyun-Jun
//
/* ================================================================================ */
// INCLUDE

#include "cgi_wind_cmm.h"
#include "cgi_wind_data.h"

#include "cgi_cmm_util.h"
#include "cgi_cmm_color.h"
#include "cgi_cmm_map_ini.h"

#include "main.h"
#include "parameter.h"
#include "disp.h"

/* ================================================================================ */
// GLOBAL

float**         g_pWindData = NULL;
float**         g_pImgData  = NULL;
st_Option       g_option;
st_MultiWind    g_multiwind;

/* ================================================================================ */
// LOCAL FUNCTION
static void fnInitProc(void)
{
    memset(&g_option, 0x00, sizeof(st_Option));
    memset(&g_multiwind,   0x00, sizeof(st_MultiWind));
}

static int fnSetWindFileName(void)
{
    int nSetTime    = 0;
    int nSix_Hour   = 360;
    struct tm CurFileTime;

    CurFileTime = g_option.m_tFileTime;

    for(nSetTime = 0; nSetTime < nSix_Hour; nSetTime++)
    {
        //oskim 20190304, 20190328
        if(g_option.m_nFormatType == WISSDOM)
        {
            strftime(g_option.m_szWindFileName, sizeof(g_option.m_szWindFileName), CGI_WISSDOM_WIND_FILE_PATH, &CurFileTime);
            if(!access(g_option.m_szWindFileName, 0))
            {
                //oskim 20190328
                //g_option.m_nFormatType = WISSDOM;
                break;
            }
        }
        else
        {
            strftime(g_option.m_szWindFileName, sizeof(g_option.m_szWindFileName), CGI_MULTI_WIND_FILE_PATH, &CurFileTime);
            if(!access(g_option.m_szWindFileName, 0))
            {
                //oskim 20190328
                //g_option.m_nFormatType = MULTI_WIND;
                break;
            }
        }

       CurFileTime = fnGetIncMin(CurFileTime, -1);
    }

    if(access(g_option.m_szWindFileName, 0))
        return -1;

    return 0;
}

static int fnDataDispCompMultiwind(MULTIWIND **stWind)
{
    gdImagePtr  pImg            = NULL;
    int         nYIdx           = 0;
    int         nXIdx           = 0;
    float       ws              = 0.0;
    float       wd              = 0.0;
    int         nP_Size         = 8;
    int         nWingSize       = 8;
    int         nFreq           = 0;
    //oskim 20190304 , 20190328
    float       fLon            = g_option.m_nFormatType == WISSDOM ? 121.34 : 122.150940; //좌하단 경도
    float       fLat            = g_option.m_nFormatType == WISSDOM ? 31.5001 : 30.846172; //좌하단 위도
    float       fX_Point        = 0.0;
    float       fY_Point        = 0.0;
    float       fImgXGridM      = 0.0;
    float       fImgYGridM      = 0.0;
    //oskim 20190304
    float       fWindGrid_Km    = g_option.m_nFormatType == WISSDOM ? 1.0 : 2.0;
    int         nDiff_X         = 0;
    int         nDiff_Y         = 0;
    int         nResult_X       = 0;
    int         nResult_Y       = 0;
    int         nWindColor      = 0;
    int         nTransparent    = 0;
    float       fImgLU_x        = 0.0;
    float       fImgLU_y        = 0.0;
    st_LAMC_PARAMETER Lamc_Parameter;
    st_LAMC_VAR       Lamc_Var;
    unsigned short GRID_X_CNT   = g_option.m_nFormatType == WISSDOM ? WIS_GRID_X_CNT : MUL_GRID_X_CNT ; 
    unsigned short GRID_Y_CNT   = g_option.m_nFormatType == WISSDOM ? WIS_GRID_Y_CNT : MUL_GRID_Y_CNT ; 
    


    pImg = gdImageCreateTrueColor(g_option.m_nImgXdim, g_option.m_nImgYdim);
    gdImageAlphaBlending(pImg, 0);
    gdImageSaveAlpha(pImg, 1);
    nWindColor = gdImageColorAllocate(pImg, 255, 0, 0);
    nTransparent = gdTrueColorAlpha(50, 50, 50, 127);
    gdImageFilledRectangle(pImg, 0, 0, g_option.m_nImgXdim, g_option.m_nImgYdim, nTransparent);

    Lamc_Parameter = fnCgiGetMapInfo(GIS_MAP_RE, 30.0, 60.0, 126.0, 38.0, 0.001, 0, 0);
    Lamc_Var.m_nFirst = 0;
    fnCgiLamcproj(&g_option.m_fLU_lon, &g_option.m_fLU_lat, &fImgLU_x, &fImgLU_y, 0, &Lamc_Parameter, &Lamc_Var);
    fnCgiLamcproj(&fLon, &fLat, &fX_Point, &fY_Point, 0, &Lamc_Parameter, &Lamc_Var);

    fImgXGridM = g_option.m_fXDist/g_option.m_nImgXdim;
    fImgYGridM = g_option.m_fYDist/g_option.m_nImgYdim;
    g_option.m_fImgGridKm = fImgXGridM / 1000;
    nDiff_X = (int)((fX_Point - fImgLU_x)/fImgXGridM);
    nDiff_Y = (int)((fImgLU_y - fY_Point)/fImgYGridM);

////////////////////////////////////////////////////////////////////////////////////
    //oskim 20190304 , for debuf
    int nStep = 0;
    /* 확대 축소 별 바람장 크기와 바람장 개수 조절 */
    if((g_option.m_fImgGridKm < 0.6) && (g_option.m_fImgGridKm >= 0.3))
    {
        nFreq       = g_option.m_nFrequency;
        nP_Size     = (int)(nP_Size / (g_option.m_fImgGridKm * 2));
        nWingSize   = (int)(nWingSize / (g_option.m_fImgGridKm * 2));
        nStep = 1;
    }
    else if((g_option.m_fImgGridKm < 0.3) && (g_option.m_fImgGridKm >= 0.05))
    {
        nFreq       = g_option.m_nFrequency / 2;
        nP_Size     = (int)(nP_Size / (g_option.m_fImgGridKm * 4));
        nWingSize   = (int)(nWingSize / (g_option.m_fImgGridKm * 4));
        nStep = 2;
    }
    else if(g_option.m_fImgGridKm < 0.05)
    {
        nFreq       = g_option.m_nFrequency / 4;
        nP_Size     = (int)(nP_Size / (g_option.m_fImgGridKm * 8));
        nWingSize   = (int)(nWingSize / (g_option.m_fImgGridKm * 8));
        nStep = 3;
    }
    else    //  기본
    {
        nFreq       = g_option.m_nFrequency * 2;
        nP_Size     = (int)(nP_Size / g_option.m_fImgGridKm);
        nWingSize   = (int)(nWingSize / g_option.m_fImgGridKm);
    }
////////////////////////////////////////////////////////////////////////////////////

    //oskim 20190304 , Decimation(filter)
    if(g_option.m_nFormatType == WISSDOM)
    {
        //nFreq = nFreq * 2;
        if((g_option.m_fImgGridKm < 10.0) && (g_option.m_fImgGridKm >= 9.6))
            nFreq = 256; //Step 4
        else if((g_option.m_fImgGridKm < 9.6) && (g_option.m_fImgGridKm >= 4.8))
            nFreq = 128; //Step 5
        else if((g_option.m_fImgGridKm < 4.8) && (g_option.m_fImgGridKm >= 2.4))
            nFreq = 64; //Step 6
        else if((g_option.m_fImgGridKm < 2.4) && (g_option.m_fImgGridKm >= 1.2))
            nFreq = 32; //Step 7
        else if((g_option.m_fImgGridKm < 1.2) && (g_option.m_fImgGridKm >= 0.6))
            nFreq = 16; //Step 8
        else if((g_option.m_fImgGridKm < 0.6) && (g_option.m_fImgGridKm >= 0.3))
            nFreq = 8;  //Step 9
        else if((g_option.m_fImgGridKm < 0.3) && (g_option.m_fImgGridKm >= 0.15))
            nFreq = 4;  //Step 10
        else if((g_option.m_fImgGridKm < 0.15) && (g_option.m_fImgGridKm >= 0.07))
            nFreq = 2;  //Step 11
        else if((g_option.m_fImgGridKm < 0.07))
            nFreq = 1;  //Step 12
    }

    //oskim 20190304 , for debuf
    char pStringBuf[1024] = {0,};
    sprintf(pStringBuf, "nFreq:%d, m_fImgGridKm:%f, nP_Size:%d, nWingSize:%d, nStep:%d", \
            nFreq, g_option.m_fImgGridKm, nP_Size, nWingSize, nStep);
    fnTextOut(pImg, 10, 4, pStringBuf);

    int ix = 0, iy = 0;
    for(nYIdx = 0; nYIdx < GRID_Y_CNT; nYIdx+=nFreq)
    {
        for(nXIdx = 0; nXIdx < GRID_X_CNT; nXIdx+=nFreq)
        {
            if (stWind[nYIdx][nXIdx].u != BAD_VALUE_F && stWind[nYIdx][nXIdx].v != BAD_VALUE_F)
            {
                ws = fnUV_To_S(stWind[nYIdx][nXIdx].u, stWind[nYIdx][nXIdx].v);
                wd = fnUV_To_D(stWind[nYIdx][nXIdx].u, stWind[nYIdx][nXIdx].v);

                nResult_Y = (nDiff_Y - ((nYIdx*(fWindGrid_Km*1000.))/fImgYGridM));
                nResult_X = ((nXIdx*(fWindGrid_Km*1000.))/fImgXGridM) + nDiff_X;

                if(g_option.m_cUnitFlag == 'W')
                {
                    nP_Size = 7;
                    nWingSize = 12;
                    nP_Size *= g_option.m_fVectorSize * 2;
                    //oskim 20190328
                    //fnWind_Draw_New(pImg, nResult_X, nResult_Y, ws, wd, nWindColor, nP_Size, nWingSize);

                    if(g_option.m_fImgGridKm >= 0.038) 
                    {
                        fnWind_Draw_New(pImg, nResult_X, nResult_Y, ws, wd, nWindColor, nP_Size, nWingSize);
                    }
                    else
                    {
                        nFreq = 32;
                        //oskim 20190401, interpolation
                        /*
                        if((g_option.m_fImgGridKm < 0.038) && (g_option.m_fImgGridKm >= 0.019))
                            nFreq = 32; //Step 13, 27 ok
                        if((g_option.m_fImgGridKm < 0.019) && (g_option.m_fImgGridKm >= .0095))
                            nFreq = 32; //Step 14
                        if((g_option.m_fImgGridKm < .0095) && (g_option.m_fImgGridKm >= .0047))
                            nFreq = 32; //Step 15
                        if((g_option.m_fImgGridKm < .0047) && (g_option.m_fImgGridKm >= .0023))
                            nFreq = 32; //Step 16
                        if((g_option.m_fImgGridKm < .0023) && (g_option.m_fImgGridKm >= .0011))
                            nFreq = 32; //Step 17
                        if((g_option.m_fImgGridKm < .0011) && (g_option.m_fImgGridKm >= .0005))
                            nFreq = 32; //Step 18
                        if((g_option.m_fImgGridKm < .0005) && (g_option.m_fImgGridKm >= .0002))
                            nFreq = 32; //Step 19
                        if((g_option.m_fImgGridKm < .0002) && (g_option.m_fImgGridKm >= .0001))
                            nFreq = 32; //Step 20
                        */

                        for(ix = nResult_X; ix < (int)(((nXIdx+nFreq)*(fWindGrid_Km*1000.))/fImgXGridM) + nDiff_X; ix += nFreq)
                        {
                            for(iy = nResult_Y; iy > (int)(nDiff_Y - (((nYIdx+nFreq)*(fWindGrid_Km*1000.))/fImgYGridM)); iy -= nFreq)
                            {
                                fnWind_Draw_New(pImg, ix, iy, ws, wd, nWindColor, nP_Size, nWingSize);
                            }
                        }
                   }
                }
                else if(g_option.m_cUnitFlag == 'V')
                {
                    nP_Size = 2;    //oskim 20190328 size change 4 -> 2
                    nWingSize = 8;

                    nP_Size = ((float)nP_Size * ws);
                    nP_Size *= g_option.m_fVectorSize;

                    fnVector_Draw(pImg, nResult_X, nResult_Y, ws, wd, nWindColor, nP_Size, nWingSize);
                }
            }
            else
                continue;
        }
    }

    for(nYIdx = 0; nYIdx < GRID_Y_CNT; nYIdx++)
        free(stWind[nYIdx]);

    free(stWind);
    fprintf(stdout, "Content-type: image/png\r\n\r\n");
    gdImagePng(pImg,stdout);
    gdImageDestroy(pImg);
    return 0;
}


static MULTIWIND** fnRead_Wind_File(char* szFileName, int nAltitude)
{
    gzFile      pFp             = NULL;
    char        szStr[MAX_STR]  = {0,};
    float       fU_Val          = 0.0f;
    float       fV_Val          = 0.0f;
    float       fW_Val          = 0.0f;
    int         nYdim           = 0;
    int         nXdim           = 0;
    int         nZdim           = 0;
    int         nXIdx           = 0;
    int         nYIdx           = 0;
    int         nSkipLine       = 0;
    MULTIWIND** wind            = NULL;

    if (((pFp = gzopen(szFileName, "rt")) == NULL) || (szFileName == NULL))
    {
        return NULL;
    }
    gzgets(pFp, szStr, MAX_STR);
    sscanf(szStr, "%d %d %d", &nXdim, &nYdim, &nZdim);

    wind = malloc(nYdim * sizeof(MULTIWIND *));
    for (nYIdx = 0; nYIdx < nYdim; nYIdx++) {
        wind[nYIdx] = malloc(nXdim * sizeof(MULTIWIND));
        memset(wind[nYIdx], 0, nXdim * sizeof(MULTIWIND));
    }

    nSkipLine = nXdim * nYdim * nAltitude * 49;
    gzseek(pFp, nSkipLine, SEEK_CUR);

    for (nYIdx = 0; nYIdx < nYdim; nYIdx++)
    {
        for (nXIdx = 0; nXIdx < nXdim; nXIdx++)
        {
            if (gzgets(pFp, szStr, MAX_STR) == Z_NULL)
            {
                gzclose(pFp);
                return NULL;
            }

            sscanf(szStr, "%f %f %f", &fU_Val, &fV_Val, &fW_Val);

            if (fU_Val < -990.0f || fV_Val < -990.0f)
            {
                wind[nYIdx][nXIdx].u = BAD_VALUE_F;
                wind[nYIdx][nXIdx].v = BAD_VALUE_F;
            }
            else if(fU_Val == 0.0f && fV_Val == 0.0f && fW_Val == 0.0f)
            {
                wind[nYIdx][nXIdx].u = BAD_VALUE_F;
                wind[nYIdx][nXIdx].v = BAD_VALUE_F;
            }
            else
            {
                wind[nYIdx][nXIdx].u = fU_Val;
                wind[nYIdx][nXIdx].v = fV_Val;
            }
        }
    }
    gzclose(pFp);
    return wind;
}

//oskim 20190304 , create new function
static MULTIWIND** fnRead_WISSDOM_File(char* szFileName, int nAltitude)
{
#define MEM_FREE()      if(pU!=NULL)free(pU); if(pV!=NULL)free(pV); if(pW!=NULL)free(pW);
#define MEM_FREE_ALL()  if(wind!=NULL)free(wind); MEM_FREE()

    gzFile      pFp             = NULL;
    char        szStr[MAX_STR]  = {0,};
    float       fU_Val          = 0.0f;
    float       fV_Val          = 0.0f;
    float       fW_Val          = 0.0f;
    int         nYdim           = 0;
    int         nXdim           = 0;
    int         nZdim           = 0;
    int         nXIdx           = 0;
    int         nYIdx           = 0;
    int         nZIdx           = 0;
    int         nSkipLine       = 0;
    MULTIWIND** wind            = NULL;

    short       *pU             = NULL;
    short       *pV             = NULL;
    short       *pW             = NULL;
    long        lOneDimIdx      = 0;
    long        lBufSize        = 0;

    FILE		*flog			= NULL;
    long		lflog			= 0;
    short		nTemp			= 0;

    WISSD_HEADER    hdr;
    memset (&hdr, 0, sizeof(hdr));

    if (((pFp = gzopen(szFileName, "rb")) == NULL) || (szFileName == NULL))
    {
        return NULL;
    }

    if(gzbuffer(pFp, 1024*1024*16) != Z_OK)
    {
        gzclose(pFp);
        return NULL;
    }

    gzread(pFp, &hdr, sizeof(hdr));

    nXdim = hdr.nx-0;
    nYdim = hdr.ny-0;
    nZdim = hdr.nz-0;

    wind = malloc(nYdim * sizeof(MULTIWIND *));
    for (nYIdx = 0; nYIdx < nYdim; nYIdx++) {
        wind[nYIdx] = malloc(nXdim * sizeof(MULTIWIND));
        memset(wind[nYIdx], 0, nXdim * sizeof(MULTIWIND));
    }

    lBufSize = nXdim*nYdim*nZdim * sizeof(short);
    pU = calloc(nXdim*nYdim*nZdim , sizeof(short));
    if(pU == NULL || gzread(pFp, pU, lBufSize) < 0)
    {
        if(pFp != NULL) gzclose(pFp);
        MEM_FREE_ALL()
        return NULL;
    }

    pV = calloc(nXdim*nYdim*nZdim , sizeof(short));
    if(pV == NULL || gzread(pFp, pV, lBufSize) < 0)
    {
        if(pFp != NULL) gzclose(pFp);
        MEM_FREE_ALL()
        return NULL;
    }
/*  //for dump test
    //fprintf(stderr, "sizeof header:%d\n",sizeof(hdr));
    flog = fopen("V-LOG", "wt");
    for(nZIdx = 0; nZIdx < nZdim; nZIdx++)
    {
        for (nYIdx = 0; nYIdx < nYdim; nYIdx++)
        {
            for (nXIdx = 0; nXIdx < nXdim; nXIdx++)
            {
                    nTemp = pV[lflog++];
                    
                    if(nTemp == -30000)
                        fprintf(flog, "O");
                    else if(nTemp == 0)
                        fprintf(flog, "o");
                    else
                        fprintf(flog, ".");
            }
            fprintf(flog, "\n");
        }
        fprintf(flog, "\n\n");
    }
    fclose(flog);
    MEM_FREE_ALL()
    return NULL;
//*/
    pW = calloc(nXdim*nYdim*nZdim , sizeof(short));
    if(pW == NULL || gzread(pFp, pW, lBufSize) < 0)
    {
        if(pFp != NULL) gzclose(pFp);
        MEM_FREE_ALL()
        return NULL;
    }

    for(nZIdx = 0; nZIdx < nZdim; nZIdx++)
    {
        for (nYIdx = 0; nYIdx < nYdim; nYIdx++)
        {
            for (nXIdx = 0; nXIdx < nXdim; nXIdx++)
            {
                //oskim 20190328,
                if(nZIdx != (int)(nAltitude/200) - 1) continue;

                lOneDimIdx = (nZIdx*nYdim + nYIdx)*nXdim + nXIdx;

                fU_Val = ((pU[lOneDimIdx] - hdr.data_minus) / (float)hdr.data_scale);
                fV_Val = ((pV[lOneDimIdx] - hdr.data_minus) / (float)hdr.data_scale);
                fW_Val = ((pW[lOneDimIdx] - hdr.data_minus) / (float)hdr.data_scale);

                if (pU[lOneDimIdx] == -30000 || fU_Val < -990.0f )
                    wind[nYIdx][nXIdx].u = BAD_VALUE_F;
                else
                    wind[nYIdx][nXIdx].u = fU_Val;

                if (pV[lOneDimIdx] == -30000 || fV_Val < -990.0f)
                    wind[nYIdx][nXIdx].v = BAD_VALUE_F;
                else
                    wind[nYIdx][nXIdx].v = fV_Val;
                
                //it happend very rarely
                if(fU_Val == 0.0f || fV_Val == 0.0f)
                {
                    wind[nYIdx][nXIdx].u = BAD_VALUE_F;
                    wind[nYIdx][nXIdx].v = BAD_VALUE_F;
                }

            }
        }
    }

    gzclose(pFp);
    MEM_FREE()
    return wind;
}

static int fnMakeMultiwind(void)
{
    int nAltitude = 0;

    if(fnSetWindFileName() < 0)
    {
        return -1;
    }

    if(g_option.m_fWindAlt == 1.0f)
        nAltitude = 0;
    else if(g_option.m_fWindAlt == 1.5f)
        nAltitude = 1;
    else
        nAltitude = (int)g_option.m_fWindAlt;

    //oskim 20190328
    if(g_option.m_nFormatType == WISSDOM)
    {
        nAltitude = (int)g_option.m_fWindAlt * 1000; //meter
    }

    //oskim 20190304
    if(g_option.m_nFormatType == WISSDOM)
        g_multiwind.m_ppWind = fnRead_WISSDOM_File(g_option.m_szWindFileName, nAltitude);
    else
        g_multiwind.m_ppWind = fnRead_Wind_File(g_option.m_szWindFileName, nAltitude);

    if(g_multiwind.m_ppWind == NULL)
        return -1;
    fnDataDispCompMultiwind(g_multiwind.m_ppWind);

    return 0;
}

/* ================================================================================ */
// FUNCTION

/* ================================================================================ */
// MAIN

int main(int argc, char** argv)
{
#define MAIN_FREE() \
        fnFreeMatrixFloat(g_pImgData, g_option.m_nImgYdim); \
        fnFreeMatrixFloat(g_pWindData, g_multiwind.m_nWindYdim);

    alarm(60);
    fnInitProc();   //g_option, g_multiwind memset

    if(fnParamSet() < 0)    //파라미터 분리
    {
        fnDumpDisp();
        MAIN_FREE()
        return -1;
    }

    if(fnMakeMultiwind() < 0)
    {
        fnDumpDisp();
        MAIN_FREE()
        return -1;
    }

    MAIN_FREE()
    alarm(0);
    return 0;
}
/* ================================================================================ */
