#ifndef MAIN_H
#define MAIN_H

/* ================================================================================ */
// DEFINE

#define COMP_ORIGIN_LON             126
#define COMP_ORIGIN_LAT             38
#define COMP_240KM_XO               400
#define COMP_240KM_YO               800
#define COMP_240KM_XDIM             960
#define COMP_240KM_YDIM             1200
#define COMP_LU_LON                 121.028351
#define COMP_LU_LAT                 41.590557
#define COMP_RL_LON                 131.863235
#define COMP_RL_LAT                 30.506027

/*----------------------------------------------------------------------------------*/
//realtime HDF5

#define FILE_FORMAT_HDF5_PPI_NOQC   "/DATA/OUTPUT/BIN/CSPH/%%Y%%m/%%d/RDR_CSQCZ_PP00%c_%%Y%%m%%d%%H%%M.h5"

/*----------------------------------------------------------------------------------*/
//realtime BIN

//oskim
//#define FILE_FORMAT_PPI_NOQC        "/DATA/OUTPUT/BIN/ZSQP/%%Y%%m/%%d/RDR_CSQCZ_PP00%c_%%Y%%m%%d%%H%%M.bin.gz"
#define FILE_FORMAT_PPI_NOQC        "/DATA/OUTPUT/BIN/ZSQP/%%Y%%m/%%d/RDR_CMP_HSR1_%%Y%%m%%d%%H%%M.bin.gz"

/* ================================================================================ */
// EXTERN

extern st_Option  g_option;           
extern st_Comp    g_comp;            
extern float**    g_pImgData;

/* ================================================================================ */
// FUNCTION PROTO


/* ================================================================================ */

#endif /* MAIN_H */
