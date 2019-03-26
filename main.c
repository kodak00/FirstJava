/* ================================================================================ */
//
// SnK 
//
/* ================================================================================ */
// INCLUDE
#include <pthread.h>
#include "cgi_site_common.h"

#include "cgi_cmm_util.h"
#include "cgi_cmm_color.h"
#include "cgi_cmm_map_ini.h"

#include "cgi_site_uf.h"
#include "cgi_site_nc.h"
#include "cgi_site_data.h"
#include "cgi_site_calc.h"
#include "cgi_site_draw.h"
#include "cgi_site_smooth.h"

#include <sys/time.h>
#include <unistd.h>
#include "main.h"
#include "disp.h"
#include "parameter.h"

#include "rdr_common.h" 
#include "rdr_util.h"   

#include "rdr_in_out.h"

//oskim 0325 start
struct RDR_VOL_HEAD rdr;        
float  **rayf;
float  gate_size, eleva, ddeg;
float  blank1s = BLANK1*0.01, blank2s = BLANK2*0.01, blank3s = BLANK3*0.01;
int    max_ray, max_gate;
//oskim 0325 end


//khbaek (20190226)
//oskim 0318
ST_NC_DESC* fnReadNcFile(char *szFilename, char *szField, float fAzimuthCorrection);

/* ================================================================================ */
// GLOBAL

float**     g_pSiteData = NULL;
float**     g_pImgData  = NULL;
st_Option   g_option;
st_Site     g_site;

//oskim 20190318
short  *data;
int    *abins, *iSray, *iSswp, *iEswp;
float  *azim, *elev, *dist, *nyqvel, *fixed;
/* ================================================================================ */
// LOCAL FUNCTION
//oskim 20190318
static void fnAllocateNcdata(void)
{

}

static void fnFreeNcData(void)
{
    free(data);
    free(nyqvel); free(fixed); free(dist); free(elev); free(azim);
    free(iEswp); free(iSswp); free(iSray); free(abins);
}

static void fnInitProc(void)
{
    memset(&g_option, 0x00, sizeof(st_Option));
    memset(&g_site,   0x00, sizeof(st_Site));
}

static void getElapsedTime(struct timeval Tstart, struct timeval Tend, int flag)
{
    Tend.tv_usec = Tend.tv_usec - Tstart.tv_usec;
    Tend.tv_sec  = Tend.tv_sec - Tstart.tv_sec;
    Tend.tv_usec += (Tend.tv_sec*1000000);
    fprintf(stderr, "%d) Elapsed Time: %lf sec\n", flag, Tend.tv_usec / 1000000.0);
}

static int fnSetWindFileName(void)
{
    int  nSetTime               = 0;
    int  nSix_Hour              = 360;
    char szSetWindName[MAX_STR] = {0,};
    struct tm CurFileTime;

    if(strcmp(g_option.m_szSiteName, "YIT") == 0)
    {
        g_option.m_nWindChk = 0;
        return -1;
    }

    CurFileTime = g_option.m_tFileTime;

    sprintf(szSetWindName, MVVP_WIND_FILE_FORMAT, g_option.m_szSiteName);

    for(nSetTime = 0; nSetTime < nSix_Hour; nSetTime++)
    {
        strftime(g_option.m_szWindFileName, sizeof(g_option.m_szWindFileName), szSetWindName, &CurFileTime);
        if(!access(g_option.m_szWindFileName, 0))
        {
            break;
        }

        CurFileTime = fnGetIncMin(CurFileTime, -1);
    }

    return 0;
}

static int fnSetInputFileName(void)
{
    int  nSetTime                   = 0;
    int  nSix_Hour                  = 360;
    char szUF_FilePath[MAX_STR]     = {0,};
    char szDir[MAX_STR]             = {0,};
    char szQC[MAX_STR]              = {0,};
    char szQ_Type[3]                = {0,};
    struct tm CurFileTime;
    //khbaek (20190226) : for reading netcdf file.
    char szNC_FilePath[MAX_STR]     = {0,};
    char szDir_NC[MAX_STR]          = {0,};

    memset(&CurFileTime, 0, sizeof(CurFileTime));

    if(strcmp(g_option.m_szQC_Type, "FUZZYQC") == 0)
    {
        sprintf(szDir, "%s", "UFF2");
        sprintf(szQC, "%s", "_FQC");
        sprintf(szQ_Type, "%s", "FQ");
    }
    else if(strcmp(g_option.m_szQC_Type, "ORPGQC") == 0)
    {
        sprintf(szDir, "%s", "UFQ");
        sprintf(szQC, "%s", "_QCD");
        sprintf(szQ_Type, "%s", "OQ");
    }
    else if(strcmp(g_option.m_szQC_Type, "NOQC") == 0)
    {
        sprintf(szDir, "%s", "UFV");
        sprintf(szDir_NC, "%s", "NCV"); //khbaek (20190226)
        sprintf(szQC, "%s", "");
        sprintf(szQ_Type, "%s", "NQ");
     }
    else
    {
        sprintf(szDir, "%s", "UFV");
        sprintf(szDir_NC, "%s", "NCV"); //khbaek (20190226)
        sprintf(szQC, "%s", "");
        sprintf(szQ_Type, "%s", "NQ");
    }

    sprintf(szUF_FilePath, "/DATA/INPUT/%s/%%Y%%m/%%d/RDR_%s%s_%%Y%%m%%d%%H%%M.uf", 
            szDir, g_option.m_szSiteName, szQC);
    //khbaek (20190226)
    sprintf(szNC_FilePath, "/DATA/INPUT/%s/%%Y%%m/%%d/RDR_%s%s_RAW_%%Y%%m%%d%%H%%M.nc", 
            szDir_NC, g_option.m_szSiteName, szQC);

    CurFileTime = g_option.m_tFileTime;

    g_option.m_nUF_Flag = 0; g_option.m_nNC_Flag = 0;
    for(nSetTime = 0; nSetTime < nSix_Hour; nSetTime++)
    {
        //khbaek (20190226)
	    strftime(g_option.m_szNC_FileName, sizeof(g_option.m_szNC_FileName), 
                szNC_FilePath, &CurFileTime);

        if(!access(g_option.m_szNC_FileName, 0))
		    g_option.m_nNC_Flag = 1;

        if (g_option.m_nNC_Flag == 1)
        {
fprintf(stderr, "%s\n", g_option.m_szNC_FileName);
            g_option.m_tDataTime = CurFileTime;
            return 0;
        }
        //khbaek
        else
        {
            strftime(g_option.m_szUF_FileName, sizeof(g_option.m_szUF_FileName), 
                    szUF_FilePath, &CurFileTime);

            if(!access(g_option.m_szUF_FileName, 0))
                g_option.m_nUF_Flag = 1;

            if(g_option.m_nUF_Flag == 1)
            {
fprintf(stderr, "%s\n", g_option.m_szUF_FileName);
                g_option.m_tDataTime=CurFileTime;
                return 0;
            }
        }
	    CurFileTime = fnGetIncMin(CurFileTime, -1);
    }
    return -1;
}

static WIND*** fnRead_Wind_File(char* szFileName, int nDiff_WindX, int nDiff_WindY, float fImgXGridM, float fImgYGridM)
{
    WIND*** wind            = NULL;
    FILE*   pFp             = NULL;
    char    szStr[MAX_STR]  = {0,};
    int     nLine_Cnt       = 0;
    int     nBox_X          = 0;
    int     nBox_Y          = 0;
    int     nBox_Z          = 0;
    int     nYIdx           = 0;
    int     nXIdx           = 0;
    float   u               = 0.0f;
    float   v               = 0.0f;
    float   w               = 0.0f;

    if ((pFp = fopen(szFileName,"r")) == NULL)
    {
        return NULL;
    }

    nLine_Cnt = 0;
    while ((fgets(szStr,MAX_STR,pFp)) != NULL)
    {
        nLine_Cnt++;
    }

    fseek(pFp,0,SEEK_SET);

    if (nLine_Cnt != WIND_XYDIM*WIND_XYDIM*WIND_ZDIM)
    {
        return NULL;
    }

    wind = malloc(WIND_ZDIM * sizeof(WIND **));
    for (nYIdx = 0 ; nYIdx < WIND_ZDIM; nYIdx++)
    {
        wind[nYIdx] = malloc(WIND_XYDIM * sizeof(WIND *));
        for (nXIdx = 0 ; nXIdx < WIND_XYDIM ; nXIdx++ )
        {
            wind[nYIdx][nXIdx] = malloc(WIND_XYDIM * sizeof(WIND));
       }
    }

    while ((fgets(szStr,MAX_STR,pFp)) != NULL)
    {
        sscanf(szStr,"%d %d %d %f %f %f",&nBox_X,&nBox_Y,&nBox_Z,&u,&v,&w);

        wind[nBox_Z-1][nBox_Y-1][nBox_X-1].x  = ((nBox_X -1) * (WIND_DX*1000./fImgXGridM) + nDiff_WindX);
        wind[nBox_Z-1][nBox_Y-1][nBox_X-1].y  = ((((WIND_XYDIM-1)*1000.*WIND_DY)/fImgYGridM) -
                                                (((nBox_Y -1) * (WIND_DY*1000.))/fImgYGridM) + nDiff_WindY);
        wind[nBox_Z-1][nBox_Y-1][nBox_X-1].z  = nBox_Z;
        wind[nBox_Z-1][nBox_Y-1][nBox_X-1].ws = fnUV_To_S(u,v);
        wind[nBox_Z-1][nBox_Y-1][nBox_X-1].wd = fnUV_To_d(u,v);
    }
    fclose(pFp);
    return wind;
}

//oskim 0325
static void fnSetSiteValueToRDR_NC()
{
    g_site.m_fSiteLon    = g_option.m_fPoint_lon;
    g_site.m_fSiteLat    = g_option.m_fPoint_lat;
    g_site.m_nGateSize   = gate_size;
    g_site.m_nBinCount   = max_gate;
    g_site.m_fMaxRange   = (g_site.m_nGateSize * g_site.m_nBinCount)/1000. + (rdr.swp[g_option.m_nSweepNo].gate1)/1000.;
    g_site.m_fSiteGridKm = 0.25;

    if((strcmp(g_option.m_szSiteName, "MIL") == 0)||
      (strcmp(g_option.m_szSiteName, "SRI") == 0)||
      (strcmp(g_option.m_szSiteName, "DJK") == 0))
        g_site.m_fSiteGridKm = 0.15;

    g_site.m_nSiteXdim   = (g_site.m_fMaxRange / g_site.m_fSiteGridKm)*2 + 1;
    g_site.m_nSiteYdim   = (g_site.m_fMaxRange / g_site.m_fSiteGridKm)*2 + 1;
}

//oskim 0318
static void fnSetSiteValueToNC(ST_NC_DESC *pNc)
{
    if(pNc == NULL)
        return;

    g_site.m_fSiteLon    = pNc->m_fSiteLon;
    g_site.m_fSiteLat    = pNc->m_fSiteLat;
    //g_site.m_nGateSize   = pNc->m_nGateSize[g_option.m_nSweepNo][0];
    g_site.m_nGateSize   = pNc->m_nGateSize;
    //g_site.m_nBinCount   = pNc->m_nBins[g_option.m_nSweepNo][0];
    g_site.m_nBinCount   = abins[iSswp[g_option.m_nSweepNo]];
    //g_site.m_fMaxRange   = (g_site.m_nGateSize * g_site.m_nBinCount)/1000. + (pNc->m_nRangeBin[g_option.m_nSweepNo][0])/1000.;
    g_site.m_fMaxRange   = (g_site.m_nGateSize * g_site.m_nBinCount)/1000. + (pNc->m_nRangeBin)/1000.;
    g_site.m_fSiteGridKm = 0.25;

    //grid 수정 0.25 2017.12.11
    //oskim 20190130 , update(change) small radar site name
    if((strcmp(g_option.m_szSiteName, "MIL") == 0)||
      (strcmp(g_option.m_szSiteName, "SRI") == 0)||
      (strcmp(g_option.m_szSiteName, "DJK") == 0))
        g_site.m_fSiteGridKm = 0.25;
    g_site.m_nSiteXdim   = (g_site.m_fMaxRange / g_site.m_fSiteGridKm)*2 + 1;
    g_site.m_nSiteYdim   = (g_site.m_fMaxRange / g_site.m_fSiteGridKm)*2 + 1;
}

static void fnSetSiteValueToUf(ST_UF_DATA *pUf)
{
    if(pUf == NULL)
        return;

    g_site.m_fSiteLon    = pUf->m_fSiteLon;
    g_site.m_fSiteLat    = pUf->m_fSiteLat;
    g_site.m_nGateSize   = pUf->m_nGateSize[g_option.m_nSweepNo][0];
    g_site.m_nBinCount   = pUf->m_nBins[g_option.m_nSweepNo][0];
    g_site.m_fMaxRange   = (g_site.m_nGateSize * g_site.m_nBinCount)/1000. + (pUf->m_nRangeBin[g_option.m_nSweepNo][0])/1000.;
    //g_site.m_fSiteGridKm = 1.0; 
    g_site.m_fSiteGridKm = 0.25;

    //grid 수정 0.25 2017.12.11
    //oskim 20190130 , update(change) small radar site name
    if((strcmp(g_option.m_szSiteName, "MIL") == 0)||
      (strcmp(g_option.m_szSiteName, "SRI") == 0)||
      (strcmp(g_option.m_szSiteName, "DJK") == 0))
        g_site.m_fSiteGridKm = 0.25;
    g_site.m_nSiteXdim   = (g_site.m_fMaxRange / g_site.m_fSiteGridKm)*2 + 1;
    g_site.m_nSiteYdim   = (g_site.m_fMaxRange / g_site.m_fSiteGridKm)*2 + 1;
}

static int fnMakeImgData(void)
{
    int                 nXIdx           = 0;
    int                 nYIdx           = 0;
    float               fImgXGridM      = 0.0;
    float               fImgYGridM      = 0.0;
    float               fImgLU_lon      = 0.0;
    float               fImgLU_lat      = 0.0;
    float               fImgLU_x        = 0.0;
    float               fImgLU_y        = 0.0;
    float               fSiteLon        = g_site.m_fSiteLon;
    float               fSiteLat        = g_site.m_fSiteLat;
    float               fSiteToImg_x    = 0.0;
    float               fSiteToImg_y    = 0.0;
    int                 nDiff_x         = 0;
    int                 nDiff_y         = 0;
    int                 nDiff_WindX     = 0;
    int                 nDiff_WindY     = 0;
    float               fXGridScale     = 0;
    float               fYGridScale     = 0;
    st_LAMC_PARAMETER   stLamcMap;
    st_LAMC_VAR         stLamcVar;

    stLamcVar.m_nFirst = 0;
    fImgXGridM = g_option.m_fXDist/g_option.m_nImgXdim;
    fImgYGridM = g_option.m_fYDist/g_option.m_nImgYdim;
    g_option.m_fImgGridKm = fImgXGridM / 1000;

    stLamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30.0, 60.0, 126.0, 0, 0.001, 0, 0);

    fImgLU_lon = g_option.m_fLU_lon;
    fImgLU_lat = g_option.m_fLU_lat;
    fnCgiLamcproj(&fImgLU_lon, &fImgLU_lat, &fImgLU_x, &fImgLU_y, 0, &stLamcMap, &stLamcVar);
    fnCgiLamcproj(&fSiteLon, &fSiteLat, &fSiteToImg_x, &fSiteToImg_y, 0, &stLamcMap, &stLamcVar);

    nDiff_x = (int)((fSiteToImg_x - fImgLU_x)/fImgXGridM - ((g_site.m_nSiteXdim*(g_site.m_fSiteGridKm/0.001))/2)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fSiteToImg_y)/fImgYGridM - ((g_site.m_nSiteYdim*(g_site.m_fSiteGridKm/0.001))/2)/fImgYGridM);

    fXGridScale = g_site.m_fSiteGridKm/(fImgXGridM/1000);
    fYGridScale = g_site.m_fSiteGridKm/(fImgYGridM/1000);

    g_pImgData = fnGetMatrixFloat(g_option.m_nImgYdim, g_option.m_nImgXdim);

    for(nYIdx = 0; nYIdx < g_option.m_nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < g_option.m_nImgXdim; nXIdx++)
        {
            g_pImgData[nYIdx][nXIdx] = fnCalcRadarValue(g_pSiteData,
                                                        g_site.m_nSiteXdim, g_site.m_nSiteYdim,
                                                        nDiff_x, nDiff_y,
                                                        nXIdx, nYIdx,
                                                        fXGridScale, fYGridScale);
        }
    }
    
    if(g_option.m_nWindChk == 1)
    {
        nDiff_WindX = (int)((fSiteToImg_x - fImgLU_x)/fImgXGridM - (((WIND_XYDIM-1)*(WIND_DX/0.001))/2)/fImgXGridM);
        nDiff_WindY = (int)((fImgLU_y - fSiteToImg_y)/fImgYGridM - (((WIND_XYDIM-1)*(WIND_DY/0.001))/2)/fImgYGridM);

        if(fnSetWindFileName() < 0)
        {
            return 0;
        }

        g_site.m_pppWind = fnRead_Wind_File(g_option.m_szWindFileName, nDiff_WindX, nDiff_WindY, fImgXGridM, fImgYGridM);
    }
    
    return 0;
}

//oskim 0325
static float *fnKRL_SweepToCartRDR_NC(int nSweepNo, int nXdim, int nYdim, float fRange)
{
    int          nXIdx          = 0;
    int          nYIdx          = 0;
    float        fAzim          = 0.0;
    float        fResult        = 0.0;
    float        fVal           = 0.0;
    int          nCartIdx       = 0;
    int          nIdx           = 0;
    float        fRes           = 0.0;
    float        fRm            = 0.0;
    int          nAzim          = 0;
    static float *pfCartImage   = NULL;
    double       dCloseDiff     = 0.0;
    //oskim 0318
    int          index        = 0;
    int          nRayCntOfSweepNo = 0;
    float        rate, az1, s, x1, y1, x, y, dd;
    int          i,j,ia,ja;
    int          nx = 2.0*240000*0.001;
    int          ny = nx;    

    if (nXdim != nYdim || nYdim < 0 || nXdim < 0)
    {
        return NULL;
    }

    pfCartImage = (float*) calloc(nXdim*nYdim, sizeof(float));

    for (nXIdx = 0; nXIdx < nXdim * nYdim; nXIdx++ )
    {
        pfCartImage[nXIdx] = BAD_VALUE_F;
    }

    rate = nx/(float)nXdim;
    for(j = 1; j < nYdim; j++){
    for(i = 1; i < nXdim; i++){

        y1 = rate*j;
        x1 = rate*i;

        y = (y1 - 0.5*ny)*1000;
        x = (x1 - 0.5*nx)*1000;

        if (y > 0 || y < 0)
            az1 = atan2(x, y)*RADDEG;
        else
            az1 = 0;
        if (az1 < 0) az1 += 360;

        s = sqrt(x*x + y*y);
        ia = (int)(s/gate_size);
        if (ia < 0 || ia >= max_gate) continue;

        ja = (int)(az1/ddeg);
        if (ja < 0) ja += max_ray;
        if (ja >= max_ray) ja -= max_ray;

        dd = rayf[ja][ia];

        //nCartIdx =  (j + nYdim) * nYdim + (nXdim-1)-(i + nXdim);
        nCartIdx =  j * (nYdim-1) + i;
        pfCartImage[nCartIdx] = dd;
    }
    }

    return pfCartImage;
}

//oskim 0325
static int fnCreatePPIRDR_NC()
{
    float   *pfImsiData  = NULL;
    int     nXIdx        = 0;
    int     nYIdx        = 0;
    int     nImsiCount   = 0;

    pfImsiData = fnKRL_SweepToCartRDR_NC(g_option.m_nSweepNo, g_site.m_nSiteXdim, g_site.m_nSiteYdim, g_site.m_fMaxRange);
    if(pfImsiData == NULL)
    {
        return -1;
    }

    for (nYIdx = 0 ; nYIdx < g_site.m_nSiteYdim ; nYIdx++ )
    {
        for (nXIdx = 0 ; nXIdx < g_site.m_nSiteXdim ; nXIdx++ )
        {
            g_pSiteData[nYIdx][nXIdx] = pfImsiData[nImsiCount];
            nImsiCount++;
        }
    }

    free(pfImsiData);

    return 0;
}

static int fnCreatePPINC(ST_NC_DESC *pNc)
{
    float   *pfImsiData  = NULL;
    int     nXIdx        = 0;
    int     nYIdx        = 0;
    int     nImsiCount   = 0;
    st_Azimuth* table;

    if(pNc == NULL)
        return -1;

    if(g_option.m_nSweepNo >= pNc->m_nSweeps)
        return -1;

    //oskim 0318
    table = fnMakeHashTableNC(pNc, iEswp, iSswp, azim, g_option.m_nSweepNo);

    pfImsiData = fnKRL_SweepToCartNC(pNc, iEswp, iSswp, iSray, azim, data, g_site.m_fAzimuthCorrection, g_option.m_nSweepNo, g_site.m_nSiteXdim, g_site.m_nSiteYdim, g_site.m_fMaxRange, table);
    if(pfImsiData == NULL)
    {
        free(table);
        return -1;
    }

    for (nYIdx = 0 ; nYIdx < g_site.m_nSiteYdim ; nYIdx++ )
    {
        for (nXIdx = 0 ; nXIdx < g_site.m_nSiteXdim ; nXIdx++ )
        {
            g_pSiteData[nYIdx][nXIdx] = pfImsiData[nImsiCount];
            nImsiCount++;
        }
    }

    free(pfImsiData);
    free(table);

    return 0;
}

static int fnCreatePPI(ST_UF_DATA *pUf)
{
    float   *pfImsiData  = NULL;
    int     nXIdx        = 0;
    int     nYIdx        = 0;
    int     nImsiCount   = 0;
    st_Azimuth* table;

    if(pUf == NULL)
        return -1;

    if(g_option.m_nSweepNo >= pUf->m_nSweeps)
        return -1;

    table = fnMakeHashTable(pUf, g_option.m_nSweepNo);

    pfImsiData = fnKRL_SweepToCart(pUf, g_option.m_nSweepNo, g_site.m_nSiteXdim, g_site.m_nSiteYdim, g_site.m_fMaxRange, table);
    if(pfImsiData == NULL)
    {
        free(table);
        return -1;
    }

    for (nYIdx = 0 ; nYIdx < g_site.m_nSiteYdim ; nYIdx++ )
    {
        for (nXIdx = 0 ; nXIdx < g_site.m_nSiteXdim ; nXIdx++ )
        {
            g_pSiteData[nYIdx][nXIdx] = pfImsiData[nImsiCount];
            nImsiCount++;
        }
    }

    free(pfImsiData);
    free(table);

    return 0;
}

static int fnCreateMohrCAPPI(ST_UF_DATA *pUf, int nIs_Dbz)
{
    int   nXIdx             = 0;
    int   nYIdx             = 0;
    int   nImsiCount        = 0;
    float fHeightFromRadar  = 0.0;
    float fHeight           = 0.0;
    float *pfImsiData       = NULL;
    st_Azimuth** table;

    if(pUf == NULL)
        return -1;

    //해발 고도 계산
    fHeight = pUf->m_nHeight;
    fHeightFromRadar = g_option.m_fCappi_Alt-(fHeight/1000.);

    table = (st_Azimuth**)malloc(sizeof(st_Azimuth*)*(pUf->m_nSweeps));
    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        table[nYIdx] = fnMakeHashTable(pUf, nYIdx);
    }

    for(nYIdx = 0; nYIdx < g_site.m_nSiteYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < g_site.m_nSiteXdim; nXIdx++)
        {
            g_pSiteData[nYIdx][nXIdx] = OUT_BOUND_F;
        }
    }
    pfImsiData = fnMakeMohrCappi(pUf, nIs_Dbz, g_site.m_nSiteYdim, g_site.m_nSiteXdim, g_site.m_fSiteGridKm,
                                 0, g_site.m_nSiteYdim, 0, g_site.m_nSiteXdim, fHeightFromRadar, table);

    if(pfImsiData != NULL)
    {
        nImsiCount = 0;
        for (nYIdx = 0 ; nYIdx < g_site.m_nSiteYdim ; nYIdx++ )
        {
            for (nXIdx = 0 ; nXIdx < g_site.m_nSiteXdim ; nXIdx++ ){

                g_pSiteData[nYIdx][nXIdx] = pfImsiData[nImsiCount];
                nImsiCount++;
            }
        }
        free(pfImsiData);
    }

    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        free(table[nYIdx]);
    }

    free(table);
    return 0;
}

static int fnCreateBASE(ST_UF_DATA *pUf)
{
    float  *pfImsiData  = NULL;
    int     nXIdx       = 0;
    int     nYIdx       = 0;
    int     nImsiCnt    = 0;
    st_Azimuth** table;

    if(pUf == NULL)
        return -1;

    table = (st_Azimuth**)malloc(sizeof(st_Azimuth*)*(pUf->m_nSweeps));
    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        table[nYIdx] = fnMakeHashTable(pUf, nYIdx);
    }

    pfImsiData = fnVolumeToBASE(pUf, g_site.m_nSiteXdim, g_site.m_nSiteYdim, g_site.m_fMaxRange, table);

    if(pfImsiData != NULL)
    {
        for (nYIdx = 0 ; nYIdx < g_site.m_nSiteYdim ; nYIdx++ )
        {
            for (nXIdx = 0 ; nXIdx < g_site.m_nSiteXdim ; nXIdx++ )
            {
                g_pSiteData[nYIdx][nXIdx] = pfImsiData[nImsiCnt];
                nImsiCnt++;
            }
        }
        free(pfImsiData);
    }

    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        free(table[nYIdx]);
    }

    free(table);
    return 0;
}

int fnCreateVIL(ST_UF_DATA *pUf)
{
    float   *pfImsiData     = NULL;
    int     nXIdx           = 0;
    int     nYIdx           = 0;
    int     nImsiCnt        = 0;
    int     nVilBottom_h_km = 0;
    int     nVilTop_h_km    = 10;
    st_Azimuth** table;

    if(pUf == NULL)
        return -1;

    table = (st_Azimuth**)malloc(sizeof(st_Azimuth*)*(pUf->m_nSweeps));
    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        table[nYIdx] = fnMakeHashTable(pUf, nYIdx);
    }

    pfImsiData = fnVolumeToVIL(pUf, g_site.m_nSiteYdim, g_site.m_nSiteXdim, g_site.m_fMaxRange, nVilTop_h_km, nVilBottom_h_km, table);

    if(pfImsiData != NULL)
    {
        for (nYIdx = 0 ; nYIdx < g_site.m_nSiteYdim ; nYIdx++ )
        {
            for (nXIdx = 0 ; nXIdx < g_site.m_nSiteXdim ; nXIdx++ )
            {
                g_pSiteData[nYIdx][nXIdx] = pfImsiData[nImsiCnt];
                nImsiCnt++;
            }
        }
        free(pfImsiData);
    }

    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        free(table[nYIdx]);
    }

    free(table);
    return 0;
}

int fnCreateCMAX(ST_UF_DATA *pUf)
{
    float  *pfImsiData  = NULL;
    int     nXIdx       = 0;
    int     nYIdx       = 0;
    int     nImsiCnt    = 0;
    st_Azimuth** table;

    if(pUf == NULL)
        return -1;

    table = (st_Azimuth**)malloc(sizeof(st_Azimuth*)*(pUf->m_nSweeps));
    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        table[nYIdx] = fnMakeHashTable(pUf, nYIdx);
    }

    for(nYIdx = 0; nYIdx < g_site.m_nSiteYdim; nYIdx++)
        g_pSiteData[nYIdx] = (float*)malloc(sizeof(float)*g_site.m_nSiteYdim);

    pfImsiData = fnVolumeToCMAX(pUf, g_site.m_nSiteYdim, g_site.m_nSiteXdim, g_site.m_fMaxRange, table);

    if(pfImsiData != NULL)
    {
        for (nYIdx = 0; nYIdx < g_site.m_nSiteYdim; nYIdx++ )
        {
            for (nXIdx = 0; nXIdx < g_site.m_nSiteXdim; nXIdx++ )
            {
                g_pSiteData[nYIdx][nXIdx] = pfImsiData[nImsiCnt];
                nImsiCnt++;
            }
        }
        free(pfImsiData);
    }

    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        free(table[nYIdx]);
    }

    free(table);
    return 0;
}

int fnCreateETOP(ST_UF_DATA *pUf)
{
    float   *pfImsiData     = NULL;
    int     nXIdx           = 0;
    int     nYIdx           = 0;
    int     pfImsiCnt       = 0;
    float   fMinThreshold   = -15.0;
    st_Azimuth** table;

    if(pUf == NULL)
        return -1;

    table = (st_Azimuth**)malloc(sizeof(st_Azimuth*)*(pUf->m_nSweeps));
    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        table[nYIdx] = fnMakeHashTable(pUf, nYIdx);
    }
    pfImsiData = fnVolumeToETOP(pUf, g_site.m_nSiteXdim, g_site.m_nSiteYdim, g_site.m_fMaxRange, pUf->m_nHeight, fMinThreshold, g_site.m_fSiteGridKm, table);

    if(pfImsiData != NULL)
    {
        for (nYIdx = 0; nYIdx < g_site.m_nSiteYdim; nYIdx++ )
        {
            for (nXIdx = 0 ; nXIdx < g_site.m_nSiteXdim; nXIdx++ )
            {
                g_pSiteData[nYIdx][nXIdx] = pfImsiData[pfImsiCnt];
                pfImsiCnt++;
            }
        }
        free(pfImsiData);
    }

    for(nYIdx = 0; nYIdx < pUf->m_nSweeps; nYIdx++)
    {
        free(table[nYIdx]);
    }

    free(table);
    return 0;
}

static void fnSetSiteDataBound(void)
{
    int     nYIdx = 0;
    int     nXIdx = 0;
    float   fDist = 0;

    for(nYIdx = 0; nYIdx < g_site.m_nSiteYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < g_site.m_nSiteXdim; nXIdx++)
        {
            fDist = sqrt(pow(nYIdx-(g_site.m_nSiteYdim/2), 2) + pow(nXIdx-(g_site.m_nSiteXdim/2), 2)) * g_site.m_fSiteGridKm;

            if(fDist > g_site.m_fMaxRange)
            {
                g_pSiteData[nYIdx][nXIdx] = OUT_BOUND_F;
            }
        }
    }
}

static int fnGetSite_UF_Data(void)
{
    ST_UF_DATA  *pUf            = NULL;
    int         nIs_Dbz         = 0;
    char        szFieldName[3]  = {0,};

//  2017.03.31 요청에의해 주석처리    
//    if(strcmp(g_option.m_szProductType, "VIL") == 0)
//        sprintf(g_option.m_szDataType, "RN");

    if(strcmp(g_option.m_szDataType, "DZ") == 0 || strcmp(g_option.m_szDataType, "CZ") == 0)
    {
        nIs_Dbz = 1;
    }

    if(strcmp(g_option.m_szDataType, "RN") == 0 || (strcmp(g_option.m_szDataType, "SN") == 0))
    {
        sprintf(szFieldName, "%s", "CZ");
    }
    else
    {
        sprintf(szFieldName, "%s", g_option.m_szDataType);
    }
    
    if(strcmp(g_option.m_szSiteName, "IIA") == 0)
        g_site.m_fAzimuthCorrection = -7.53;
    else
        g_site.m_fAzimuthCorrection = 0;

    if((pUf = fnReadUfFile(g_option.m_szUF_FileName, szFieldName, g_site.m_fAzimuthCorrection)) == NULL)
    {
        if((strcmp(g_option.m_szDataType, "RN") == 0) || (strcmp(g_option.m_szDataType, "SN") == 0))
        {
            sprintf(szFieldName, "%s", "DZ");
            if((pUf = fnReadUfFile(g_option.m_szUF_FileName, szFieldName, g_site.m_fAzimuthCorrection)) == NULL)
                return -1;
        }
        else
            return -1;
    }

    fnSetSiteValueToUf(pUf);

    g_pSiteData = fnGetMatrixFloat(g_site.m_nSiteYdim, g_site.m_nSiteXdim);

    if(strcmp(g_option.m_szProductType, "PPI") == 0)
    {
        if(fnCreatePPI(pUf) < 0)
        {
            free(pUf);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"CAPPI") == 0)
    {
        if(fnCreateMohrCAPPI(pUf, nIs_Dbz) < 0)
        {
            free(pUf);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"BASE") == 0)
    {
        if(fnCreateBASE(pUf) < 0)
        {
            free(pUf);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"CMAX") == 0)
    {
        if(fnCreateCMAX(pUf) < 0)
        {
            free(pUf);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"ETOP") == 0)
    {
        if(fnCreateETOP(pUf) < 0)
        {
            free(pUf);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"VIL") == 0)
    {
        if(fnCreateVIL(pUf) < 0)
        {
            free(pUf);
            return -1;
        }
    }
    else
    {
        if(fnCreateMohrCAPPI(pUf, nIs_Dbz) < 0)
        {
            free(pUf);
            return -1;
        }
    }
    fnSetSiteDataBound();

    free(pUf);
    return 0;
}

//oskim 0325
static int fnGetSite_RDR_NC_Data(void)
{
    fnSetSiteValueToRDR_NC();

    g_pSiteData = fnGetMatrixFloat(g_site.m_nSiteYdim, g_site.m_nSiteXdim);

    if(strcmp(g_option.m_szProductType, "PPI") == 0)
    {
        if(fnCreatePPIRDR_NC() < 0)
        {
            return -1;
        }
    }

   return 0;
}

//khbaek (20190226) : function for reading netcdf file.
static int fnGetSite_NC_Data(void)
{
    ST_NC_DESC  *pNc            = NULL;
    int         nIs_Dbz         = 0;
    char        szFieldName[5]  = {0,};

    if(strcmp(g_option.m_szDataType, "DZ") == 0 || strcmp(g_option.m_szDataType, "CZ") == 0)
    {
        nIs_Dbz = 1;
    }

    if(strcmp(g_option.m_szDataType, "RN") == 0 || (strcmp(g_option.m_szDataType, "SN") == 0))
    {
        sprintf(szFieldName, "%s", "CZ");
    }
    else
    {
        sprintf(szFieldName, "%s", g_option.m_szDataType);
    }

    if (strcmp(szFieldName, "DZ") == 0) sprintf(szFieldName, "%s", "UH");
    if (strcmp(szFieldName, "CZ") == 0) sprintf(szFieldName, "%s", "DBZH");
    if (strcmp(szFieldName, "RH") == 0) sprintf(szFieldName, "%s", "RHOHV");
    if (strcmp(szFieldName, "DR") == 0) sprintf(szFieldName, "%s", "ZDR");
    if (strcmp(szFieldName, "PH") == 0) sprintf(szFieldName, "%s", "PHIDP");
    if (strcmp(szFieldName, "KD") == 0) sprintf(szFieldName, "%s", "KDP");

    if(strcmp(g_option.m_szSiteName, "IIA") == 0)
        g_site.m_fAzimuthCorrection = -7.53;
    else
        g_site.m_fAzimuthCorrection = 0;

    if((pNc = fnReadNcFile(g_option.m_szNC_FileName, szFieldName, g_site.m_fAzimuthCorrection)) == NULL)
    {
        if((strcmp(g_option.m_szDataType, "RN") == 0) || (strcmp(g_option.m_szDataType, "SN") == 0))
        {
            sprintf(szFieldName, "%s", "DZ");
            if((pNc = fnReadNcFile(g_option.m_szNC_FileName, szFieldName, g_site.m_fAzimuthCorrection)) == NULL)
                return -1;
        }
        else
            return -1;
    }

    //oskim 0318
    fnSetSiteValueToNC(pNc);

    g_pSiteData = fnGetMatrixFloat(g_site.m_nSiteYdim, g_site.m_nSiteXdim);

    if(strcmp(g_option.m_szProductType, "PPI") == 0)
    {
        if(fnCreatePPINC(pNc) < 0)
        {
            free(pNc);
            return -1;
        }
    }
    /*
    else if(strcmp(g_option.m_szProductType,"CAPPI") == 0)
    {
        if(fnCreateMohrCAPPI(pNc, nIs_Dbz) < 0)
        {
            free(pNc);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"BASE") == 0)
    {
        if(fnCreateBASE(pNc) < 0)
        {
            free(pNc);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"CMAX") == 0)
    {
        if(fnCreateCMAX(pNc) < 0)
        {
            free(pNc);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"ETOP") == 0)
    {
        if(fnCreateETOP(pNc) < 0)
        {
            free(pNc);
            return -1;
        }
    }
    else if(strcmp(g_option.m_szProductType,"VIL") == 0)
    {
        if(fnCreateVIL(pNc) < 0)
                    {
                        free(pNc);
                        return -1;
                    }
    }
    else
    {
        if(fnCreateMohrCAPPI(pNc, nIs_Dbz) < 0)
        {
            free(pNc);
            return -1;
        }
    }
    */
    fnSetSiteDataBound();

    free(pNc);
    return 0;
}

static int fnRead_DataFile()
{   
//////2017. 3. 31 추가//////////////////////////////////////////
//    if((strcmp(g_option.m_szQC_Type, "NOQC") != 0) &&
//       (strcmp(g_option.m_szQC_Type, "FUZZYQC") != 0) &&
//       (strcmp(g_option.m_szQC_Type, "ORPGQC") != 0))
    if(strcmp(g_option.m_szQC_Type, "") == 0)
       return -1;
////////////////////////////////////////////////////////////////

    if(fnSetInputFileName() < 0)
        return -1;

    if (g_option.m_nNC_Flag == 1)
    {
        rdr_stn_vol_chk_nc();
        rdr_stn_ppi_nc();
        fnGetSite_RDR_NC_Data();

        /*
        if (fnGetSite_NC_Data() < 0)
        {
            fprintf(stderr, "fail fnGetSite_NC_Data\n");
            return -1;
        }
        */
    }
    else
    {
        if (g_option.m_nUF_Flag == 1)
        {
            if(fnGetSite_UF_Data() < 0)     // UF 데이터 읽기
                return -1;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}
/* ================================================================================ */
// FUNCTION

/* ================================================================================ */
// MAIN

int main(int argc, char** argv)
{
#define MAIN_FREE() \
        if(g_pImgData != NULL) { fnFreeMatrixFloat(g_pImgData, g_option.m_nImgYdim); }\
        if(g_pSiteData != NULL) {fnFreeMatrixFloat(g_pSiteData, g_site.m_nSiteYdim); }\
        gdFreeFontCache();\
        fnFreeNcData();
    
    fnInitProc();   //g_option, g_site memset

    if(fnParamSet() < 0)    //파라미터 분리
    {
        fnDumpDisp();
        MAIN_FREE()
        return -1;
    }
    
    if(g_option.m_nUnitBar == 1)
    {
        if(fnCreateUnitBar() <0)
        {
            fnDumpDisp();
            MAIN_FREE()
            return -1;
        }
        MAIN_FREE()
        return 0;
    }
    
    if(fnRead_DataFile() < 0)
    {
        if((g_option.m_nPointRain == 1) || (g_option.m_nW_Flag == 1))
        {
            fprintf(stdout, "Content-type: text/html\r\n\r\n");
            fprintf(stdout, "%s\r\n\r\n", "-");
            MAIN_FREE()
            return -1;
        }
        else
        {
            fnNoDataDisp(g_option.m_nImgYdim, g_option.m_nImgXdim);
            MAIN_FREE()
            return -1;
        }
    }

    //oskim 0325
    //rdr_nc_head_dec(g_option.m_szNC_FileName,&rdr);
    
    if(g_option.m_nSmooth == 1 && ((strcmp(g_option.m_szProductType, "ETOP") != 0)
                               && (strcmp(g_option.m_szProductType, "VIL") != 0)))
    {
        if(fnKmaSmooth(g_pSiteData, g_site.m_nSiteYdim, g_site.m_nSiteXdim) < 0)
        {
            fnDumpDisp();
            MAIN_FREE()
            return -1;
        }
    }

    if(g_option.m_nPointRain == 1)
    {
        if(fnWritePointRain(g_pSiteData, g_option.m_szDataType, g_site.m_nSiteXdim, g_site.m_nSiteYdim, 
                            g_site.m_fSiteGridKm, g_site.m_fSiteLon, g_site.m_fSiteLat, g_site.m_nSiteXdim/2, 
                            g_site.m_nSiteYdim/2, g_option.m_fPoint_lon, g_option.m_fPoint_lat, 
                            g_option.m_fZr_A, g_option.m_fZr_B) < 0)
        {
            MAIN_FREE()
            return -1;
        }
        else
            MAIN_FREE()
            return 0;
    }

    if(fnMakeImgData() < 0)
    {
        fnDumpDisp();
        MAIN_FREE()
        return -1;
    }

    if(g_option.m_nW_Flag == 1)
    {
        if(fnRainPointFileWrite(g_pImgData, g_option.m_szDataType, g_option.m_lN_Date,
                                g_option.m_nImgYdim, g_option.m_nImgXdim,
                                g_option.m_fZr_A, g_option.m_fZr_B) < 0)
        { 
            fprintf(stdout, "Content-type: text/html\r\n\r\n");
            fprintf(stdout, "%s\r\n\r\n", "-");
            MAIN_FREE()
            return -1;
        }
        else
        {
            fprintf(stdout, "content-type: text/html\r\n\r\n");
            fprintf(stdout, "%s\r\n\r\n", "OK");
            MAIN_FREE()
            return 0;
        }
    }

    if(fnCreateImg(g_option.m_fZr_A, g_option.m_fZr_B) < 0)
    {   
        fnDumpDisp();
        MAIN_FREE()
        return -1;
    }

    MAIN_FREE()

    //oskim 0325
    free((float*)(rayf)); 

    return 0;
}
/* ================================================================================ */
