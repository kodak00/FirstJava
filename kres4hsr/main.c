/* ================================================================================ */
//
// 레이더 에코 영상 GIS 합성 일기현상(실시간)
//
// 2016.07.11 
//
// SnK 
//
/* ================================================================================ */
// INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gd.h>

#include "parameter.h"
#include "disp_img.h"
#include "main.h"

#include "cgi_comp_value.h"
#include "cgi_cmm_map_ini.h"
#include "cgi_cmm_color.h"
#include "cgi_cmm_util.h"
#include "cgi_comp_calc.h"
#include "cgi_comp_echo_disp.h"
#include "cgi_comp_extra_disp.h"
#include "cgi_comp_origin_data_process.h"
#include "cgi_cmm_wst_disp.h"
#include "cgi_comp_img_data_process.h"

#include "rdr_common.h" 
#include "rdr_util.h"  
#include "rdr_in_out.h"

/* ================================================================================ */
// GLOBAL

float**     g_pCompData     = NULL; 
float**     g_pImgData      = NULL; 
st_Option   g_option;           
st_Comp     g_comp;             
int         nFileTypeFlag   = 0;

/* ================================================================================ */
// LOCAL FUNTION

static void fnInitProc(void)
{
    memset(&g_option, 0x00, sizeof(g_option));
    memset(&g_comp,   0x00, sizeof(g_comp));
}

static void fnFileName(void)
{
    char szHdf5Format[1024] = "";
    char szBinFormat[1024]  = "";

    if((strcmp(g_option.m_szP_type, "PPI") == 0) && (strcmp(g_option.m_szQ_type, "NOQC") == 0))
    {
        sprintf(szHdf5Format, FILE_FORMAT_HDF5_PPI_NOQC, g_option.m_szComp_method);
        sprintf(szBinFormat,  FILE_FORMAT_PPI_NOQC,      g_option.m_szComp_method);
    }

    nFileTypeFlag = fnFileFormatName(szHdf5Format, szBinFormat);
}

static int fnGetHdf5CompData(void)
{
    STD_PRODUCT     *pStdProduct    = NULL;
    HDF_PRODUCT     *pHdfProduct    = NULL;

    if(access(g_option.m_szF_name, 0))
        return -1;

    if((pHdfProduct = fnLoadHdfProductByIdx(g_option.m_szF_name, g_option.m_szP_type, 0, NULL)) == NULL)
        return -1;

    if((pStdProduct = fnHdfProductSyncUpStd(pHdfProduct)) == NULL)
    {
        fnFreeHdfProduct(pHdfProduct);
        return -1;
    }
    fnFreeHdfProduct(pHdfProduct);

    g_comp.m_fCompLon    = COMP_ORIGIN_LON;
    g_comp.m_fCompLat    = COMP_ORIGIN_LAT;
    g_comp.m_fCompGridKm = (float)pStdProduct->m_product_hdr.m_dXscale;
    g_comp.m_nCompXdim   = (float)pStdProduct->m_product_hdr.m_lXsize;
    g_comp.m_nCompYdim   = (float)pStdProduct->m_product_hdr.m_lYsize;
    g_comp.m_nCompXo     = COMP_240KM_XO*(1.0/g_comp.m_fCompGridKm);
    g_comp.m_nCompYo     = COMP_240KM_YO*(1.0/g_comp.m_fCompGridKm);

    if(!strcmp(g_option.m_szP_type, "PPI"))
    {
        if((g_pCompData = fnGetStdProductDataByIdx(pStdProduct, g_option.m_szP_type, 0, "CZ")) == NULL)
        {   fnFreeStdProduct(pStdProduct); return -1; }
    }
    else
    {   fnFreeStdProduct(pStdProduct); return -1; }

    fnFreeStdProduct(pStdProduct);

    return 0;
}


static void fnBinaryCompDataSetting(void)
{
//oskim
    g_comp.m_fCompLon       = COMP_ORIGIN_LON;
    g_comp.m_fCompLat       = COMP_ORIGIN_LAT;
    g_comp.m_fCompGridKm    = 1.0;
    g_comp.m_nCompXdim      = 1152;//960;
    g_comp.m_nCompYdim      = 1440;//1200;
    g_comp.m_nCompXo        = 560;//400;
    g_comp.m_nCompYo        = 840;//800;
/*
    g_comp.m_fCompLon       = COMP_ORIGIN_LON;
    g_comp.m_fCompLat       = COMP_ORIGIN_LAT;
    g_comp.m_fCompGridKm    = 1.0;
    g_comp.m_nCompXdim      = COMP_240KM_XDIM;
    g_comp.m_nCompYdim      = COMP_240KM_YDIM;
    g_comp.m_nCompXo        = COMP_240KM_XO;
    g_comp.m_nCompYo        = COMP_240KM_YO;
*/
}

static void fnImgDataUnitSetting(void)
{
    if((strcmp(g_option.m_szP_type, "ETOP") != 0) && (strcmp(g_option.m_szP_type, "VIL") != 0))
    {
        if(strcmp(g_option.m_szUnit, "RN") == 0)
        {
            fnDbzToRainrate(g_pImgData, g_option.m_nImgYdim, g_option.m_nImgXdim,
                    BOUND_VALUE_F, BAD_VALUE_F, CGI_DF_DEFAULT_ZR_A, CGI_DF_DEFAULT_ZR_B);
        }
        else if(strcmp(g_option.m_szUnit, "SN") == 0)
        {
            fnDbzToRainrate(g_pImgData, g_option.m_nImgYdim, g_option.m_nImgXdim,
                    BOUND_VALUE_F, BAD_VALUE_F, CGI_DF_SNOW_ZR_A, CGI_DF_SNOW_ZR_B);
        }
    }
}
static int fnGetFileData(void)
{
    fnFileName();

    if(nFileTypeFlag == 1)
    {
        if(fnGetHdf5CompData() < 0)
        {
            return -1;       
        }
    }
    else if(nFileTypeFlag == 2)
    {
        fnBinaryCompDataSetting(); 

        if((g_pCompData = fnGetCompData(g_option.m_szF_name, g_comp.m_nCompXdim, g_comp.m_nCompYdim)) == NULL)
        {
            return -1;
        }
    }
    else if(nFileTypeFlag == 0)
    {
        return -1;
    }

    return 0;
}

/* ================================================================================ */
// MAIN

int main(int argc, char** argv)
{
#define MAIN_FREE() \
        fnFreeMatrixFloat(g_pImgData, g_option.m_nImgYdim); \
        fnFreeMatrixFloat(g_pCompData, g_comp.m_nCompYdim); 
        
    fnInitProc();

    if(fnParamSet() < 0)
    {
        fnDumpDisp();
        return -1;
    }

    if(g_option.m_nUnitBar == 1)
    {
        if(fnCreateLevelColorImg(g_option.m_szP_type, g_option.m_nImgYdim, g_option.m_szUnit) < 0)
        {
            gdFreeFontCache();
            fnDumpDisp();   
            return -1;
        }
        
        gdFreeFontCache();
        return 0;
    }
    

    if(fnGetFileData() < 0)
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
            if(fnNoCompDataDisp(g_option.m_nImgXdim, g_option.m_nImgYdim) < 0)
            {
                gdFreeFontCache();
                fnDumpDisp();
                return -1;
            }
            gdFreeFontCache();
            return -1;
        }
    }

    if(g_option.m_nW_Flag == 1)
    {
        if(fnRainPointFileWrite(g_pCompData, g_option.m_szUnit, g_option.m_lN_Date,
                        g_comp.m_nCompXdim, g_comp.m_nCompYdim, 0, g_option.m_nImgXdim, g_option.m_nImgYdim, 
                        g_option.m_fLU_lon, g_option.m_fLU_lat, g_option.m_fXDist, g_option.m_fYDist, g_comp.m_fCompGridKm,
                        COMP_LU_LON, COMP_LU_LAT, COMP_RL_LON, COMP_RL_LAT) < 0)
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

    if(g_option.m_nPointRain == 1)
    {
        if(fnWritePointRain(g_pCompData, g_option.m_szUnit, g_comp.m_nCompXdim, g_comp.m_nCompYdim, 
                        g_comp.m_fCompGridKm, g_comp.m_fCompLon, g_comp.m_fCompLat, g_comp.m_nCompXo, 
                        g_comp.m_nCompYo, g_option.m_fPointLon, g_option.m_fPointLat, 0) < 0)
        {
             fnDumpDisp();  
             MAIN_FREE()
             return -1;       
        }
    }
    else
    {
        if((g_pImgData = fnMakeCompImgData(g_pCompData, g_option.m_nImgXdim, g_option.m_nImgYdim, 
                        g_option.m_fLU_lon, g_option.m_fLU_lat, g_option.m_fXDist, g_option.m_fYDist, 
                        g_option.m_nSmooth, g_comp.m_nCompXdim, g_comp.m_nCompYdim, g_comp.m_fCompGridKm,
                        g_option.m_szP_type)) == NULL)
        {
            fnDumpDisp();  
            MAIN_FREE()
            return -1;
        }

        fnImgDataUnitSetting();

        if(fnCreateCompRealtimeImg() < 0)
        {
            fnDumpDisp();  
            gdFreeFontCache();
            MAIN_FREE()
            return -1;
        }
    }

    gdFreeFontCache();
    MAIN_FREE()

    return 0;
}

/* ================================================================================ */




