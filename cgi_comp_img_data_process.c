/* ================================================================================ */
//
// 레이더 에코 영상 GIS - cgi_comp_img_data_process.c (이미지 데이터 생성)
//
// 2016.09.08
//
// SnK
//
/* ================================================================================ */
// INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <gd.h>
#include <math.h>

#include "cgi_comp_value.h"
#include "cgi_cmm_color.h"
#include "cgi_cmm_map_ini.h"
#include "cgi_cmm_util.h"
#include "cgi_comp_calc.h"
#include "cgi_comp_img_data_process.h"

/* ================================================================================ */
// LOCAL FUNTION


/* ================================================================================ */
// FUNCTION

void fnKmaPosToGisPos(float **pData, int nYdim, int nXdim, float fYGridM, float fXGridM, float fLU_lon, float fLU_lat, int nDiff_x, int nDiff_y, int nDiff_RL_x, int nDiff_RL_y)
{
    float           **pTempData     = NULL;
    int             nXIdx           = 0;
    int             nYIdx           = 0;
    float           fLon            = 0.0;
    float           fLat            = 0.0;
    float           fKmaLU_x        = 0.0;
    float           fKmaLU_y        = 0.0;
    float           fGisLU_x        = 0.0;
    float           fGisLU_y        = 0.0;
    float           fKma_x          = 0.0;
    float           fKma_y          = 0.0;
    float           fGis_x          = 0.0;
    float           fGis_y          = 0.0;
    int             nGis_x          = 0;
    int             nGis_y          = 0;

    st_LAMC_VAR       kmaVar;
    st_LAMC_PARAMETER kmaMap;
    st_LAMC_VAR       gisVar;
    st_LAMC_PARAMETER gisMap;

    if(pData == NULL)
        return;

    pTempData = fnGetMatrixFloat(nYdim, nXdim);
    if(pTempData == NULL)
        return;


    for(nYIdx = 0; nYIdx < nYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nXdim; nXIdx++)
        {
            if((nDiff_x == 0) && (nDiff_y == 0) && (nDiff_RL_x == 0) && (nDiff_RL_y == 0))
            {
                pTempData[nYIdx][nXIdx] = BAD_VALUE_F;
            }
            else
            {
                pTempData[nYIdx][nXIdx] = BOUND_VALUE_F;

                if((nDiff_x <= nXIdx) && (nXIdx <= nDiff_RL_x) 
                        && ((nYdim - nDiff_RL_y) <= nYIdx) && (nYIdx <= (nYdim - nDiff_y)))
                {
                    pTempData[nYIdx][nXIdx] = BAD_VALUE_F;
                }
            }
        }
    }

    kmaVar.m_nFirst = 0;
    kmaMap = fnCgiGetMapInfo(KMA_MAP_RE, 30, 60, 126, 38, 0.001, 0, 0);
    gisVar.m_nFirst = 0;
    gisMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126,  0, 0.001, 0, 0);

    fnCgiLamcproj(&fLU_lon, &fLU_lat, &fKmaLU_x, &fKmaLU_y, 0, &kmaMap, &kmaVar);
    fnCgiLamcproj(&fLU_lon, &fLU_lat, &fGisLU_x, &fGisLU_y, 0, &gisMap, &gisVar);

    for(nYIdx = 0; nYIdx < nYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nXdim; nXIdx++)
        {
            fKma_x = (nXIdx*fXGridM) + fKmaLU_x;
            fKma_y = fKmaLU_y - (nYIdx*fYGridM);
            fnCgiLamcproj(&fLon, &fLat, &fKma_x, &fKma_y, 1, &kmaMap, &kmaVar);

            fnCgiLamcproj(&fLon, &fLat, &fGis_x, &fGis_y, 0, &gisMap, &gisVar);
            nGis_x = (int)((fGis_x - fGisLU_x)/fXGridM + 0.5);
            nGis_y = (int)((fGisLU_y - fGis_y)/fYGridM + 0.5);

            if(nGis_x < 0 || nGis_x >= nXdim || nGis_y < 0 || nGis_y >= nYdim)
                continue;

            pTempData[nGis_y][nGis_x] = pData[nYIdx][nXIdx];
        }
    }
   
    fnFreeMatrixFloat(pTempData, nYdim);
}

float** fnMakeCompMapImgData(float** pImgData, float** pCompData, int nImgXdim, int nImgYdim, int nCompXdim, int nCompYdim, float fCompGridKm)
{
    int                     nXIdx           = 0;    
    int                     nYIdx           = 0;    
    float                   fGridScale      = 0.0;  
    float                   fGridKm         = 2.0;  
    
    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }

    fGridScale   = (fCompGridKm / fGridKm);

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData, 
                                                nCompXdim, 
                                                nCompYdim, 
                                                0,
                                                0,
                                                nXIdx,
                                                nYIdx,
                                                fGridScale,
                                                fGridScale); 
        }
    }

    return pImgData;
}

float** fnMakeCompImgData_HCI(float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nSmooth, int nCompXdim, int nCompYdim, float fCompGridKm, char szP_type[])
{
    int                     nXIdx           = 0;    
    int                     nYIdx           = 0;    
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;
    float                   fCompRL_lon     = 0.0;
    float                   fCompRL_lat     = 0.0;
    float                   fCompToImgRL_x  = 0.0;
    float                   fCompToImgRL_y  = 0.0;
    int                     nDiff_RL_x      = 0;
    int                     nDiff_RL_y      = 0;
    
    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                
    
    float**                 pImgData        = NULL;

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }

    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;
    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);

    fCompLU_lon = 118.820030;
    fCompLU_lat = 43.331890;

    fCompRL_lon = 132.164398;
    fCompRL_lat = 30.120716;

    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompRL_lon, &fCompRL_lat, &fCompToImgRL_x, &fCompToImgRL_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    nDiff_RL_x = (int)((fCompToImgRL_x - fImgLU_x)/fImgXGridM);
    nDiff_RL_y = (int)((fImgLU_y - fCompToImgRL_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);
    
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData, 
                                           nCompXdim, 
                                           nCompYdim, 
                                           nDiff_x,
                                           nDiff_y,
                                           nXIdx,
                                           nYIdx,
                                           fXGridScale,
                                           fYGridScale); 
        }
    }


    fnKmaPosToGisPos(pImgData, nImgYdim, nImgXdim, fImgYGridM, fImgXGridM, fLU_lon, fLU_lat, nDiff_x, nDiff_y, nDiff_RL_x, nDiff_RL_y);
        
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(pImgData[nYIdx][nXIdx] == BAD_VALUE_F)
            {
                if(nXIdx >= 1 && nXIdx < nImgXdim - 2)
                {
                    if(pImgData[nYIdx][nXIdx-1] != BAD_VALUE_F && pImgData[nYIdx][nXIdx+1] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx][nXIdx-1];
                }
                if(nYIdx >= 1 && nYIdx < nImgYdim - 2)
                {
                    if(pImgData[nYIdx-1][nXIdx] != BAD_VALUE_F && pImgData[nYIdx+1][nXIdx] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx-1][nXIdx];
                }
            }
        }
    }

    return pImgData;
}

float** fnMakeCompNewImgData(float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nSmooth, int nCompXdim, int nCompYdim, float fCompGridKm, char szP_type[])
{
    int                     nXIdx           = 0;    
    int                     nYIdx           = 0;    
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;
    float                   fCompRL_lon     = 0.0;
    float                   fCompRL_lat     = 0.0;
    float                   fCompToImgRL_x  = 0.0;
    float                   fCompToImgRL_y  = 0.0;
    int                     nDiff_RL_x      = 0;
    int                     nDiff_RL_y      = 0;
    
    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                
    
    float**                 pImgData        = NULL;

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }

    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;

    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);

    fCompLU_lon = 118.820030;
    fCompLU_lat = 43.331890;

    fCompRL_lon = 132.164398;
    fCompRL_lat = 30.120716;

    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompRL_lon, &fCompRL_lat, &fCompToImgRL_x, &fCompToImgRL_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    nDiff_RL_x = (int)((fCompToImgRL_x - fImgLU_x)/fImgXGridM);
    nDiff_RL_y = (int)((fImgLU_y - fCompToImgRL_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);
    
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
/*
            if(nSmooth == 1)
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValueSmooth(pCompData, 
                                                nCompXdim, 
                                                nCompYdim, 
                                                nDiff_x,
                                                nDiff_y,
                                                nXIdx,
                                                nYIdx,
                                                fXGridScale,
                                                fYGridScale); 
            }
            else
            {
*/
                pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData, 
                                                nCompXdim, 
                                                nCompYdim, 
                                                nDiff_x,
                                                nDiff_y,
                                                nXIdx,
                                                nYIdx,
                                                fXGridScale,
                                                fYGridScale); 

//            }
        }
    }

    clock_t s,e;
    s = clock();
    fnKmaPosToGisPos(pImgData, nImgYdim, nImgXdim, fImgYGridM, fImgXGridM, fLU_lon, fLU_lat, nDiff_x, nDiff_y, nDiff_RL_x, nDiff_RL_y);
    e = clock();
    fprintf(stderr, "fnKmaPosToGisPos %f\n", (float)(e-s)/1000000);
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(pImgData[nYIdx][nXIdx] == BAD_VALUE_F)
            {
                if(nXIdx >= 1 && nXIdx < nImgXdim - 2)
                {
                    if(pImgData[nYIdx][nXIdx-1] != BAD_VALUE_F && pImgData[nYIdx][nXIdx+1] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx][nXIdx-1];
                }
                if(nYIdx >= 1 && nYIdx < nImgYdim - 2)
                {
                    if(pImgData[nYIdx-1][nXIdx] != BAD_VALUE_F && pImgData[nYIdx+1][nXIdx] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx-1][nXIdx];
                }
            }
        }
    }

    return pImgData;
}

float** fnMakeCompNew480ImgData(float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nSmooth, int nCompXdim, int nCompYdim, float fCompGridKm, char szP_type[])
{
    int                     nXIdx           = 0;    
    int                     nYIdx           = 0;    
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;
    float                   fCompRL_lon     = 0.0;
    float                   fCompRL_lat     = 0.0;
    float                   fCompToImgRL_x  = 0.0;
    float                   fCompToImgRL_y  = 0.0;
    int                     nDiff_RL_x      = 0;
    int                     nDiff_RL_y      = 0;
    
    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                
    
    float**                 pImgData        = NULL;

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }

    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;

    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);

    fCompLU_lon = 115.767715;
    fCompLU_lat = 43.089016;
   
    fCompRL_lon = 134.147552;
    fCompRL_lat = 28.520864;

    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompRL_lon, &fCompRL_lat, &fCompToImgRL_x, &fCompToImgRL_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    nDiff_RL_x = (int)((fCompToImgRL_x - fImgLU_x)/fImgXGridM);
    nDiff_RL_y = (int)((fImgLU_y - fCompToImgRL_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);
    
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
/*
            if(nSmooth == 1)
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValueSmooth(pCompData, 
                                                nCompXdim, 
                                                nCompYdim, 
                                                nDiff_x,
                                                nDiff_y,
                                                nXIdx,
                                                nYIdx,
                                                fXGridScale,
                                                fYGridScale); 
            }
            else
            {
*/
                pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData, 
                                                nCompXdim, 
                                                nCompYdim, 
                                                nDiff_x,
                                                nDiff_y,
                                                nXIdx,
                                                nYIdx,
                                                fXGridScale,
                                                fYGridScale); 

//            }
        }
    }

    fnKmaPosToGisPos(pImgData, nImgYdim, nImgXdim, fImgYGridM, fImgXGridM, fLU_lon, fLU_lat, nDiff_x, nDiff_y, nDiff_RL_x, nDiff_RL_y);
        
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(pImgData[nYIdx][nXIdx] == BAD_VALUE_F)
            {
                if(nXIdx >= 1 && nXIdx < nImgXdim - 2)
                {
                    if(pImgData[nYIdx][nXIdx-1] != BAD_VALUE_F && pImgData[nYIdx][nXIdx+1] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx][nXIdx-1];
                }
                if(nYIdx >= 1 && nYIdx < nImgYdim - 2)
                {
                    if(pImgData[nYIdx-1][nXIdx] != BAD_VALUE_F && pImgData[nYIdx+1][nXIdx] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx-1][nXIdx];
                }
            }
        }
    }

    return pImgData;
}

float** fnMakeCompImgData(float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nSmooth, int nCompXdim, int nCompYdim, float fCompGridKm, char szP_type[])
{
    int                     nXIdx           = 0;    
    int                     nYIdx           = 0;    
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;
    float                   fCompRL_lon     = 0.0;
    float                   fCompRL_lat     = 0.0;
    float                   fCompToImgRL_x  = 0.0;
    float                   fCompToImgRL_y  = 0.0;
    int                     nDiff_RL_x      = 0;
    int                     nDiff_RL_y      = 0;
    
    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                
    
    float**                 pImgData        = NULL;

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }

    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;

    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);

    fCompLU_lon = 121.028351;
    fCompLU_lat = 41.590557;
    
    fCompRL_lon = 131.863235;
    fCompRL_lat = 30.506027;

    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompRL_lon, &fCompRL_lat, &fCompToImgRL_x, &fCompToImgRL_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    nDiff_RL_x = (int)((fCompToImgRL_x - fImgLU_x)/fImgXGridM);
    nDiff_RL_y = (int)((fImgLU_y - fCompToImgRL_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);
    
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if((nSmooth == 1) && ((strcmp(szP_type, "ETOP") != 0) && (strcmp(szP_type, "VIL") != 0)))
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValueSmooth(pCompData, 
                                                nCompXdim, 
                                                nCompYdim, 
                                                nDiff_x,
                                                nDiff_y,
                                                nXIdx,
                                                nYIdx,
                                                fXGridScale,
                                                fYGridScale); 
            }
            else
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData, 
                                                nCompXdim, 
                                                nCompYdim, 
                                                nDiff_x,
                                                nDiff_y,
                                                nXIdx,
                                                nYIdx,
                                                fXGridScale,
                                                fYGridScale); 

            }
        }
    }
    fnKmaPosToGisPos(pImgData, nImgYdim, nImgXdim, fImgYGridM, fImgXGridM, fLU_lon, fLU_lat, nDiff_x, nDiff_y, nDiff_RL_x, nDiff_RL_y);
        
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(pImgData[nYIdx][nXIdx] == BAD_VALUE_F)
            {
                if(nXIdx >= 1 && nXIdx < nImgXdim - 2)
                {
                    if(pImgData[nYIdx][nXIdx-1] != BAD_VALUE_F && pImgData[nYIdx][nXIdx+1] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx][nXIdx-1];
                }
                if(nYIdx >= 1 && nYIdx < nImgYdim - 2)
                {
                    if(pImgData[nYIdx-1][nXIdx] != BAD_VALUE_F && pImgData[nYIdx+1][nXIdx] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx-1][nXIdx];
                }
            }
        }
    }

    return pImgData;
}

float** fnMakeComp480ImgData(float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nSmooth, int nCompXdim, int nCompYdim, float fCompGridKm)
{
    int                     nXIdx           = 0;    
    int                     nYIdx           = 0;    
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;
    float                   fCompRL_lon     = 0.0;
    float                   fCompRL_lat     = 0.0;
    float                   fCompToImgRL_x  = 0.0;
    float                   fCompToImgRL_y  = 0.0;
    int                     nDiff_RL_x      = 0;
    int                     nDiff_RL_y      = 0;

    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                

    float**                 pImgData        = NULL;

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }
    
    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;

    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);
    
    fCompLU_lon = 116.147820;
    fCompLU_lat = 43.574066;
    
    fCompRL_lon = 134.196945;
    fCompRL_lat = 27.808210;

    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompRL_lon, &fCompRL_lat, &fCompToImgRL_x, &fCompToImgRL_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    nDiff_RL_x = (int)((fCompToImgRL_x - fImgLU_x)/fImgXGridM);
    nDiff_RL_y = (int)((fImgLU_y - fCompToImgRL_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);
    
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        { 
            if(nSmooth == 1)
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValueSmooth480(pCompData, 
                                                    nCompXdim, 
                                                    nCompYdim, 
                                                    nDiff_x,
                                                    nDiff_y,
                                                    nXIdx,
                                                    nYIdx,
                                                    fXGridScale,
                                                    fYGridScale); 
            }
            else
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData, 
                                                    nCompXdim, 
                                                    nCompYdim, 
                                                    nDiff_x,
                                                    nDiff_y,
                                                    nXIdx,
                                                    nYIdx,
                                                    fXGridScale,
                                                    fYGridScale); 
            }
        }
    }

    fnKmaPosToGisPos(pImgData, nImgYdim, nImgXdim, fImgYGridM, fImgXGridM, fLU_lon, fLU_lat, nDiff_x, nDiff_y, nDiff_RL_x, nDiff_RL_y);
    
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(pImgData[nYIdx][nXIdx] == BAD_VALUE_F)
            {
                if(nXIdx >= 1 && nXIdx < nImgXdim - 2)
                {
                    if(pImgData[nYIdx][nXIdx-1] != BAD_VALUE_F && pImgData[nYIdx][nXIdx+1] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx][nXIdx-1];
                }
                if(nYIdx >= 1 && nYIdx < nImgYdim - 2)
                {
                    if(pImgData[nYIdx-1][nXIdx] != BAD_VALUE_F && pImgData[nYIdx+1][nXIdx] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx-1][nXIdx];
                }
            }
        }
    }

    return pImgData;
}

float** fnMakeCompVsrfEchoImgData(float** pImgData, float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nSmooth, int nCompXdim, int nCompYdim, float fCompGridKm)
{
    int                     nXIdx           = 0;
    int                     nYIdx           = 0;
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;

    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }
    
    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;

    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);

    fCompLU_lon = 121.028351;
    fCompLU_lat = 41.590557;
    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);
 
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(nSmooth == 1)
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValueSmooth(pCompData, 
                                                          nCompXdim, 
                                                          nCompYdim, 
                                                          nDiff_x,
                                                          nDiff_y,
                                                          nXIdx,
                                                          nYIdx,
                                                          fXGridScale,
                                                          fYGridScale); 
            }
            else
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData,
                                                          nCompXdim,
                                                          nCompYdim,
                                                          nDiff_x,
                                                          nDiff_y,
                                                          nXIdx,
                                                          nYIdx,
                                                          fXGridScale,
                                                          fYGridScale);
            }
     
            if((pImgData[nYIdx][nXIdx] != BOUND_VALUE_F) && (pImgData[nYIdx][nXIdx] != BAD_VALUE_F))
            {
                pImgData[nYIdx][nXIdx] = (float)(pImgData[nYIdx][nXIdx] / VSRF_DATA_SIZE);
            }   
        }
    }

    fnKmaPosToGisPos(pImgData, nImgYdim, nImgXdim, fImgYGridM, fImgXGridM, fLU_lon, fLU_lat, 0, 0, 0, 0);

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for (nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(pImgData[nYIdx][nXIdx] == BAD_VALUE_F)
            {
                if(nXIdx >= 1 && nXIdx < nImgXdim - 2)
                {
                    if(pImgData[nYIdx][nXIdx-1] != BAD_VALUE_F && pImgData[nYIdx][nXIdx+1] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx][nXIdx-1];
                }
                if(nYIdx >= 1 && nYIdx < nImgYdim - 2)
                {
                    if(pImgData[nYIdx-1][nXIdx] != BAD_VALUE_F && pImgData[nYIdx+1][nXIdx] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx-1][nXIdx];
                }
            }
        }
    }


    return pImgData;

}  
 
float** fnMakeCompVsrfConGuideImgData(float** pImgData, float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nCompXdim, int nCompYdim, float fCompGridKm)
{
    int                     nXIdx           = 0;
    int                     nYIdx           = 0;
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;

    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }
    
    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;

    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);

    fCompLU_lon = 121.028351;
    fCompLU_lat = 41.590557;
    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);
    
    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData,
                                                     nCompXdim,
                                                     nCompYdim,
                                                     nDiff_x,
                                                     nDiff_y,
                                                     nXIdx,
                                                     nYIdx,
                                                     fXGridScale,
                                                     fYGridScale);

        }
    }

    return pImgData;
} 

float** fnMakeCompCj3ImgData(float** pCompData, int nImgXdim, int nImgYdim, float fLU_lon, float fLU_lat, float fXDist, float fYDist, int nSmooth, int nCompXdim, int nCompYdim, float fCompGridKm)
{
    int                     nXIdx           = 0;
    int                     nYIdx           = 0;
    float                   fImgXGridM      = 0.0;
    float                   fImgYGridM      = 0.0;
    float                   fImgLU_lon      = 0.0;
    float                   fImgLU_lat      = 0.0;
    float                   fImgLU_x        = 0.0;
    float                   fImgLU_y        = 0.0;
    float                   fCompLU_lon     = 0.0;
    float                   fCompLU_lat     = 0.0;
    float                   fCompToImgLU_x  = 0.0;
    float                   fCompToImgLU_y  = 0.0;
    int                     nDiff_x         = 0;
    int                     nDiff_y         = 0;
    float                   fXGridScale     = 0;
    float                   fYGridScale     = 0;
    float                   fCompRL_lon     = 0.0;
    float                   fCompRL_lat     = 0.0;
    float                   fCompToImgRL_x  = 0.0;
    float                   fCompToImgRL_y  = 0.0;
    int                     nDiff_RL_x      = 0;
    int                     nDiff_RL_y      = 0;

    st_LAMC_VAR             lamcVar;
    st_LAMC_PARAMETER       lamcMap;                

    float**                 pImgData        = NULL;

    if((pImgData = fnGetMatrixFloat(nImgYdim, nImgXdim)) == NULL)
    {
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            pImgData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }
    
    fImgXGridM = fXDist/nImgXdim;
    fImgYGridM = fYDist/nImgYdim;

    lamcVar.m_nFirst = 0;
    lamcMap = fnCgiGetMapInfo(GIS_MAP_RE, 30, 60, 126, 0, 0.001, 0, 0);

    if(nCompYdim == 801)
    {
        fCompLU_lon = 112.638809;
        fCompLU_lat = 42.748924;
    }
    else if(nCompYdim == 1151 || nCompYdim == 3453)
    {
        fCompLU_lon = 105.217768;
        fCompLU_lat = 47.190280;
    }
    else
    {
        fCompLU_lon = 111.489388;
        fCompLU_lat = 46.745865;
    }   
    
    fCompRL_lon = 136.499435;
    fCompRL_lat = 17.259367;

    //oskim 20190602
    if(nCompYdim == 3521)
    {
        fCompLU_lon = 98.92;
        fCompLU_lat = 45.19;

        fCompRL_lon = 145.0;
        fCompRL_lat = 16.9;
    }

    fImgLU_lon = fLU_lon;
    fImgLU_lat = fLU_lat;
    fnCgiLamcproj(&fImgLU_lon,  &fImgLU_lat,  &fImgLU_x,       &fImgLU_y,       0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompLU_lon, &fCompLU_lat, &fCompToImgLU_x, &fCompToImgLU_y, 0, &lamcMap, &lamcVar);
    fnCgiLamcproj(&fCompRL_lon, &fCompRL_lat, &fCompToImgRL_x, &fCompToImgRL_y, 0, &lamcMap, &lamcVar);

    nDiff_x = (int)((fCompToImgLU_x - fImgLU_x)/fImgXGridM);
    nDiff_y = (int)((fImgLU_y - fCompToImgLU_y)/fImgYGridM);

    nDiff_RL_x = (int)((fCompToImgRL_x - fImgLU_x)/fImgXGridM);
    nDiff_RL_y = (int)((fImgLU_y - fCompToImgRL_y)/fImgYGridM);

    fXGridScale = fCompGridKm/(fImgXGridM/1000);
    fYGridScale = fCompGridKm/(fImgYGridM/1000);

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(nSmooth == 1)
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValueSmooth480(pCompData, 
                                                          nCompXdim, 
                                                          nCompYdim, 
                                                          nDiff_x,
                                                          nDiff_y,
                                                          nXIdx,
                                                          nYIdx,
                                                          fXGridScale,
                                                          fYGridScale); 
            }
            else
            {
                pImgData[nYIdx][nXIdx] = fnCalcRadarValue(pCompData,
                                                          nCompXdim,
                                                          nCompYdim,
                                                          nDiff_x,
                                                          nDiff_y,
                                                          nXIdx,
                                                          nYIdx,
                                                          fXGridScale,
                                                          fYGridScale);
            }
            /* 
            if((pImgData[nYIdx][nXIdx] != BOUND_VALUE_F) && (pImgData[nYIdx][nXIdx] != BAD_VALUE_F))
            {
                if(nFileFlag == 1)
                {    
                    pImgData[nYIdx][nXIdx] = (float)(pImgData[nYIdx][nXIdx]);
                }
                else if(nFileFlag == 2)
                { 
                    pImgData[nYIdx][nXIdx] = (float)(pImgData[nYIdx][nXIdx] / CJ3_DATA_SIZE);
                }
            }
            */
        }
    }

    fnKmaPosToGisPos(pImgData, nImgYdim, nImgXdim, fImgYGridM, fImgXGridM, fLU_lon, fLU_lat, nDiff_x, nDiff_y, nDiff_RL_x, nDiff_RL_y);

    for(nYIdx = 0; nYIdx < nImgYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nImgXdim; nXIdx++)
        {
            if(pImgData[nYIdx][nXIdx] == BAD_VALUE_F)
            {
                if(nXIdx >= 1 && nXIdx < nImgXdim - 2)
                {
                    if(pImgData[nYIdx][nXIdx-1] != BAD_VALUE_F && pImgData[nYIdx][nXIdx+1] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx][nXIdx-1];
                }
                if(nYIdx >= 1 && nYIdx < nImgYdim - 2)
                {
                    if(pImgData[nYIdx-1][nXIdx] != BAD_VALUE_F && pImgData[nYIdx+1][nXIdx] != BAD_VALUE_F)
                        pImgData[nYIdx][nXIdx] = pImgData[nYIdx-1][nXIdx];
                }
            }
        }
    }

    return pImgData;
}  
  
/* ================================================================================ */
