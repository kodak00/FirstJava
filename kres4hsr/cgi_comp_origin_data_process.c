/* ================================================================================ */
//
// 레이더 에코 영상 GIS - origin_data_process.c (원본 데이터 처리)
//
// 2016.08.23
//
// SnK
//
/* ================================================================================ */
// INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <gd.h>
#include <zconf.h>
#include <sys/time.h>

#include "cgi_comp_value.h"
#include "cgi_cmm_color.h"
#include "cgi_cmm_util.h"
#include "cgi_comp_extra_disp.h"
#include "cgi_comp_origin_data_process.h"

//oskim
#include "rdr_cmp_cappi_img.h"

struct RDR_CMP_HEAD  rdr_cmp_head;
struct RDR_CMP_STN_LIST  rdr_cmp_stn_list[48];

/* ================================================================================ */
// FUNCTION

float** fnGetCompData(char szF_name[], int nDataXdim, int nDataYdim)
{
    float** pCompData               = NULL;
    gzFile  pFp                     = NULL;     
    char    szBuff[COMP_XDIM_240KM] = { 0, };   
    int     nXIdx                   = 0;        
    int     nYIdx                   = 0;        

    //oskim
    const unsigned short RDR_NY = 1440;
    const unsigned short RDR_NX = 1152;
    short dbz[RDR_NY+1][RDR_NX+1];
    short n = 0, i = 0, j = 0;
    /*
    n = gzread(pFp, dbz, (RDR_NY+1)*(RDR_NX+1)*2);

    for (j = 0; j <= RDR_NY; j++) {
      for (i = 0; i <= RDR_NX; i++) {
        if(dbz[j][i] != -30000 && dbz[j][i] != -25000)
                fprintf(stderr, "dbz[%d][%d]=%d\n", j,i,dbz[j][i]);
      }
    }
    */

    if((pFp = gzopen(szF_name, "rb")) == NULL)
    {
        return NULL;
    }
    //oskim, read header part
    n = gzread(pFp, &rdr_cmp_head, sizeof(rdr_cmp_head));
    n = gzread(pFp, rdr_cmp_stn_list, sizeof(rdr_cmp_stn_list));

    if((pCompData = fnGetMatrixFloat(nDataYdim, nDataXdim)) == NULL)
    {
        gzclose(pFp);
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nDataYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
        }
    }

    //oskim
    //for(nYIdx = nDataYdim-1-10; nYIdx >= 0; nYIdx--)
    for(nYIdx = nDataYdim-1; nYIdx >= 0; nYIdx--)
    {
        if(gzread(pFp, szBuff, nDataXdim) != nDataXdim)
        {
            break;
        }

        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            if(szBuff[nXIdx] == BOUND_VALUE_C)
            {
                pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
            }
            else if(szBuff[nXIdx] == BAD_VALUE_C)
            {
                pCompData[nYIdx][nXIdx] = BAD_VALUE_F;
            }
            else
            {
                pCompData[nYIdx][nXIdx] = (float)szBuff[nXIdx];
		//oskim
		//fprintf(stderr, "pCompData[%d][%d]=%d\n", nYIdx,nXIdx,szBuff[nXIdx]); 
            }
        }
    }

    gzclose(pFp);

    return pCompData;
}

float** fnGetCompVsrfData(char szF_name[], int nDataXdim, int nDataYdim, int nFcstTime)
{
    float** pCompData               = NULL;
    gzFile  pFp                     = NULL;     
    short   szBuff[COMP_XDIM_240KM] = { 0, };   
    int     nXIdx                   = 0;        
    int     nYIdx                   = 0;        
    int     nTime                   = 0;

    if((pFp = gzopen(szF_name, "rb")) == NULL)
    {
        return NULL;
    }

    if((pCompData = fnGetMatrixFloat(nDataYdim, nDataXdim)) == NULL)
    {
        gzclose(pFp);
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nDataYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
        }
    }

    nTime = nFcstTime / 60;
    gzseek(pFp, nDataYdim * nDataXdim * sizeof(short) * nTime, SEEK_CUR);

    for(nYIdx = nDataYdim-1; nYIdx >= 0; nYIdx--)
    {
        gzread(pFp, szBuff, nDataXdim*sizeof(short));
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            fnSwap2bytes(&szBuff[nXIdx]);

            if(szBuff[nXIdx] == BOUND_VALUE_S)
            {
                pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
            }
            else if(szBuff[nXIdx] == BAD_VALUE_S)
            {
                pCompData[nYIdx][nXIdx] = BAD_VALUE_F;
            }
            else
            {
                pCompData[nYIdx][nXIdx] = (float)szBuff[nXIdx];
            }
        }
    }

    gzclose(pFp);

    return pCompData;
}

float** fnGetCompMapleData(char szF_name[], int nDataXdim, int nDataYdim, int nFcstTime)
{
    gzFile              pFp                     = NULL;     
    int                 nXIdx                   = 0;        
    int                 nYIdx                   = 0;        
    int                 nSkip_indx              = 36;       
    int                 nX_index                = 0;        
    int                 nY_index                = 0;        
    float               rgfFca_buf[1024*1024];              
    int                 nSkip_size              = 0;        
    float               **pCompData             = NULL;     
    int                 nMinute                 = 0;        

    nMinute = nFcstTime / 10;

    if(nFcstTime>0) 
        nSkip_indx = nMinute-1;
    else
        nSkip_indx = 36;

    if((pFp = gzopen(szF_name, "rb")) == NULL)
    {
        return NULL;
    }

    if((pCompData = fnGetMatrixFloat(nDataYdim, nDataXdim)) == NULL)
    {
        gzclose(pFp);
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nDataYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            pCompData[nYIdx][nXIdx] = BAD_VALUE_F;
        }
    }

    if(gzbuffer(pFp, 1024*1024*16) != Z_OK)
    {
        gzclose(pFp); 
        return NULL;
    }

    nSkip_size = (1024*1024*sizeof(float)*nSkip_indx) 
                  + sizeof(stForecast_hed) + sizeof(stArrayhed);
    gzseek(pFp, nSkip_size,  SEEK_SET);
    gzread(pFp, rgfFca_buf, sizeof(rgfFca_buf));

    for(nYIdx = 0; nYIdx < 1024; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            nX_index = (1024-nYIdx)*1024+nXIdx+52/2;
            nY_index = nYIdx+28;

            if(nX_index > 1024*1024)
                continue;

            if(rgfFca_buf[nX_index]<0) 
            {
                pCompData[nDataYdim-nY_index][nXIdx] = BAD_VALUE_F;
            } 
            else if(rgfFca_buf[nX_index] < 0.1) 
            {
                pCompData[nDataYdim-nY_index][nXIdx] = BOUND_VALUE_F; 
            } 
            else 
            {
                pCompData[nDataYdim-nY_index][nXIdx] = (float)(rgfFca_buf[nX_index]*100);
            }
        }
    }

    gzclose(pFp);

    return pCompData;
}

float** fnGetComp480Data(char szF_name[], int nDataXdim, int nDataYdim, unsigned char szIn_bound[576][526])
{
    float**    pCompData      = NULL;
    gzFile     pFp            = NULL;       
    int        nXIdx          = 0;         
    int        nYIdx          = 0;          

    if((pFp = gzopen(szF_name, "rb")) == NULL)
    {   
        return NULL;
    }

    if((pCompData = fnGetMatrixFloat(nDataYdim, nDataXdim)) == NULL)
    {
        gzclose(pFp);
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nDataYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
        }
    }

    for(nYIdx = nDataYdim-1; nYIdx >= 0; nYIdx--)
    {
        gzread(pFp, pCompData[nYIdx], sizeof(float)*nDataXdim);
    }
    
    for(nYIdx = nDataYdim-1; nYIdx >= 0; nYIdx--)
    {
        gzread(pFp, szIn_bound[nYIdx], sizeof(unsigned char)*nDataXdim);
    } 

    for(nYIdx = 0; nYIdx < nDataYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
           if(szIn_bound[nYIdx][nXIdx] == MAP_VALUE_RANGE_OUT) 
           {
               pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
           }

        }
    }

    gzclose(pFp);

    return pCompData;
}

float** fnGetCompCj3Data(char szF_name[], int nDataXdim, int nDataYdim, int nFileFlag)
{
    float** pCompData               = NULL;
    gzFile  pFp                     = NULL;     
    short   szBuff[COMP_XDIM_CJ3]   = { 0, };   
    int     nXIdx                   = 0;        
    int     nYIdx                   = 0;        
    
    if((pFp = gzopen(szF_name, "rb")) == NULL)
    {
        return NULL;
    }

    if((pCompData = fnGetMatrixFloat(nDataYdim, nDataXdim)) == NULL)
    {
        gzclose(pFp);
        return NULL;
    }

    for(nYIdx = 0; nYIdx < nDataYdim; nYIdx++)
    {
        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
        }
    }

    for(nYIdx = nDataYdim-1; nYIdx >= 0; nYIdx--)
    {
        if(gzread(pFp, szBuff, nDataXdim*sizeof(short)) < 0)
        {
            return NULL;
        }

        for(nXIdx = 0; nXIdx < nDataXdim; nXIdx++)
        {
            fnSwap2bytes(&szBuff[nXIdx]);

            if(szBuff[nXIdx] == BOUND_VALUE_S)
            {
                pCompData[nYIdx][nXIdx] = BOUND_VALUE_F;
            }
            else if(szBuff[nXIdx] == BAD_VALUE_S)
            {
                pCompData[nYIdx][nXIdx] = BAD_VALUE_F;
            }
            else
            {
                if(nFileFlag == 1)
                {                 
                    pCompData[nYIdx][nXIdx] = (float)szBuff[nXIdx];
                }
                else if(nFileFlag == 2)
                {
                    pCompData[nYIdx][nXIdx] = (float)(szBuff[nXIdx] / 100.);
                }
            }
        }
    }

    gzclose(pFp);

    return pCompData;
}

/* ================================================================================ */
