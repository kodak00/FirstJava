#include <netcdf.h>
#include "rdr_read_nc.h"
#include "cgi_site_nc.h"

#include "cgi_site_common.h"
#include "main.h"

extern st_Option   g_option;
extern struct RDR_VOL_HEAD rdr;

extern float  **rayf;
extern float  gate_size, eleva, ddeg;
extern float  blank1s, blank2s, blank3s;
extern int    max_ray, max_gate;

/*******************************************************************************
 *  레이더 지점별 netcDF파일에서 Vol 존재 여부 확인
 *******************************************************************************/
int rdr_stn_vol_chk_nc()
{
  char   vol[8], var_cd[8];
  int    code, ok, i, j, k;

  // 1. 볼륨 번호 확인 (강수,강설은 CZ 사용)
  if (strcmp(g_option.m_szDataType,"RN") == 0 ||strcmp(g_option.m_szDataType,"SN") == 0)
    strcpy(vol,"CZ");
  else
    strcpy(vol,g_option.m_szDataType);

  // 2. 이름명 변환
  if      (!strcmp(vol,"DZ" )) strcpy(var_cd,"UH");
  else if (!strcmp(vol,"DZv")) strcpy(var_cd,"UV");
  else if (!strcmp(vol,"DZd")) strcpy(var_cd,"UH");
  else if (!strcmp(vol,"DZHs")) strcpy(var_cd,"UH");
  else if (!strcmp(vol,"DZVs")) strcpy(var_cd,"UV");
  else if (!strcmp(vol,"DCHd")) strcpy(var_cd,"UH");
  else if (!strcmp(vol,"DCVd")) strcpy(var_cd,"UV");
  else if (!strcmp(vol,"CZ" )) strcpy(var_cd,"DBZH");
  else if (!strcmp(vol,"CZv")) strcpy(var_cd,"DBZV");
  else if (!strcmp(vol,"CZd")) strcpy(var_cd,"DBZH");
  else if (!strcmp(vol,"CZHs")) strcpy(var_cd,"DBZH");
  else if (!strcmp(vol,"CZVs")) strcpy(var_cd,"DBZV");
  else if (!strcmp(vol,"VR" )) strcpy(var_cd,"VELH");
  else if (!strcmp(vol,"VRv")) strcpy(var_cd,"VELV");
  else if (!strcmp(vol,"VRd")) strcpy(var_cd,"VELH");
  else if (!strcmp(vol,"SW" )) strcpy(var_cd,"WIDTHH");
  else if (!strcmp(vol,"SWv")) strcpy(var_cd,"WIDTHV");
  else if (!strcmp(vol,"SWd")) strcpy(var_cd,"WIDTHH");
  else if (!strcmp(vol,"DR" )) strcpy(var_cd,"ZDR");
  else if (!strcmp(vol,"RH" )) strcpy(var_cd,"RHOHV");
  else if (!strcmp(vol,"PH" )) strcpy(var_cd,"PHIDP");
  else if (!strcmp(vol,"DZR")) strcpy(var_cd,"UH");
  else if (!strcmp(vol,"CZR")) strcpy(var_cd,"DBZH");
  else if (!strcmp(vol,"NCP" )) strcpy(var_cd,"NCPH");
  else if (!strcmp(vol,"NCPv")) strcpy(var_cd,"NCPV");
  else if (!strcmp(vol,"NCPd")) strcpy(var_cd,"NCPH");
  else if (!strcmp(vol,"SNR" )) strcpy(var_cd,"SNRHC");
  else if (!strcmp(vol,"SNRv")) strcpy(var_cd,"SNRVC");
  else if (!strcmp(vol,"SNRd")) strcpy(var_cd,"SNRHC");
  else if (!strcmp(vol,"CCR" )) strcpy(var_cd,"CCORH");
  else if (!strcmp(vol,"CCRv")) strcpy(var_cd,"CCORV");
  else if (!strcmp(vol,"CCRd")) strcpy(var_cd,"CCORH");
  else strcpy(var_cd,vol);

  // 3. netCDF 파일의 헤더정보 읽기
  code = rdr_nc_head_dec(g_option.m_szNC_FileName, &rdr, 0);
  if (code < 0) return -1;
  g_option.m_fPoint_lon = (float)rdr.lon;
  g_option.m_fPoint_lat = (float)rdr.lat;
  g_option.stn_ht  = rdr.ht;

  // 4. 해당 볼륨이 있는지 확인
  g_option.voln = -1;
  for (k = 0; k < rdr.num_var; k++) {
    if (strcmp(rdr.var[k].var_cd, var_cd) == 0) {
      g_option.voln = k;
      break;
    }
  }
  if (g_option.voln < 0) return -1;

  // 4. 고도각 확인
  g_option.swp_deg = -99;
  if (g_option.m_nSweepNo >= 0 && g_option.m_nSweepNo < rdr.num_swp) {
    g_option.swp_deg = rdr.swp[g_option.m_nSweepNo].elev;
    g_option.pulse_width = rdr.swp[g_option.m_nSweepNo].pulse_width;
    ok = 1;
  }
  else
    ok = 0;

  // 5. 도달범위 확인
  if (ok == 1 && g_option.area == 1) {
    g_option.range = rdr.swp[g_option.m_nSweepNo].gate1 + rdr.swp[g_option.m_nSweepNo].gate_size*rdr.swp[g_option.m_nSweepNo].num_gate;
  }
  return ok;
}

/*******************************************************************************
 *  PPI 자료 생산 (NC파일)
 *  - 지면거리 기준으로 재생산
 *******************************************************************************/
int rdr_stn_ppi_nc()
{
  int     ncid;   // NetCDF ID
  int     ndims;                      // number of dimensions
  int     var_id;                     // g_option.able ID
  int     var_dims[NC_MAX_VAR_DIMS];  // g_option.ables shape
  char    var_name[NC_MAX_NAME+1];    // g_option.able name
  char    dim_name[NC_MAX_NAME+1];    // dimension name
  size_t  rec1, recs;
  double  alpha, cs, ss, re1, s, r;
  float   az1, dd, dd_min, r1, r2, r3;
  float   **gate_ht;
  float   scale = rdr.var[g_option.voln].scale, scale2, scale3;
  float   blank = rdr.var[g_option.voln].blank*rdr.var[g_option.voln].scale;
  float   blank2, blank3;
  short   *ray_tot, *ray_tot2, *ray_tot3;
  int     *gate_num;
  int     status, var_mode = 0;
  int     ray_start_idx, nr;
  int     i, j, k, i1, i3, j1, j2, n;

  // 0. NC파일만 처리
  if (g_option.m_nNC_Flag != 1) return -1;

  // 1. PPI 자료 저장용 배열 생성
  //    방위각으로는 거의 등간격으로 저장되어 있고, 360도 다 채워져 있다고 가정함
  //    만일 물리적으로 저장되어 있지 않는 방위각들이 있으면 고쳐야 함
  // 1.1. 최대 Ray수, 최대 Gate수 확인
  max_ray = rdr.swp[g_option.m_nSweepNo].num_ray*2;
  max_gate = rdr.swp[g_option.m_nSweepNo].num_gate;

  gate_size = rdr.swp[g_option.m_nSweepNo].gate_size;
  gate_num = (int *)malloc((max_gate+1) * sizeof(int));
  if (!strcmp(g_option.m_szDataType,"RH")) blank = 0;

  // 1.3. CAPPI 저장할 배열 선언
  rayf = (float **)malloc(max_ray * sizeof(float *));
  for (j = 0; j < max_ray; j++) {
    rayf[j] = (float *)malloc((max_gate+1) * sizeof(float));
  }

  // 2. 해당 변수의 볼률자료 전체를 읽음 (속도향상 목적)
  status = nc_open(g_option.m_szNC_FileName, 0, &ncid);  // NC파일 open
  if (status < 0) return -1;
  if (!strcmp(g_option.m_szDataType,"DZd") || !strcmp(g_option.m_szDataType,"CZd") || !strcmp(g_option.m_szDataType,"VRd") || !strcmp(g_option.m_szDataType,"SWd") ||
      !strcmp(g_option.m_szDataType,"NCPd") || !strcmp(g_option.m_szDataType,"SNRd") || !strcmp(g_option.m_szDataType,"CCRd") ||
      !strcmp(g_option.m_szDataType,"DCHd") || !strcmp(g_option.m_szDataType,"DCVd") || !strcmp(g_option.m_szDataType,"DZR") || !strcmp(g_option.m_szDataType,"CZR")) {
    // H 성분 읽기
    if      (!strcmp(g_option.m_szDataType,"DZd")) strcpy(var_name,"UH");
    else if (!strcmp(g_option.m_szDataType,"CZd")) strcpy(var_name,"DBZH");
    else if (!strcmp(g_option.m_szDataType,"VRd")) strcpy(var_name,"VELH");
    else if (!strcmp(g_option.m_szDataType,"SWd")) strcpy(var_name,"WIDTHH");
    else if (!strcmp(g_option.m_szDataType,"NCPd")) strcpy(var_name,"NCPH");
    else if (!strcmp(g_option.m_szDataType,"SNRd")) strcpy(var_name,"SNRHC");
    else if (!strcmp(g_option.m_szDataType,"CCRd")) strcpy(var_name,"CCORH");
    else if (!strcmp(g_option.m_szDataType,"DCHd")) strcpy(var_name,"UH");
    else if (!strcmp(g_option.m_szDataType,"DCVd")) strcpy(var_name,"UV");
    else if (!strcmp(g_option.m_szDataType,"DZR")) strcpy(var_name,"UH");
    else if (!strcmp(g_option.m_szDataType,"CZR")) strcpy(var_name,"DBZH");

    status = nc_inq_varid(ncid, var_name, &var_id);
    status = nc_inq_varndims(ncid, var_id, &ndims);
    status = nc_inq_vardimid(ncid, var_id, var_dims);
    for (recs = 0, i = 0; i < ndims; i++) {
      status = nc_inq_dim(ncid, var_dims[i], dim_name, &rec1);
      recs += rec1;
    }
    ray_tot = (short *) malloc(recs * sizeof(short));
    status = nc_get_var_short(ncid, var_id, ray_tot);

    // V 성분 읽기
    if      (!strcmp(g_option.m_szDataType,"DZd")) strcpy(var_name,"UV");
    else if (!strcmp(g_option.m_szDataType,"CZd")) strcpy(var_name,"DBZV");
    else if (!strcmp(g_option.m_szDataType,"VRd")) strcpy(var_name,"VELV");
    else if (!strcmp(g_option.m_szDataType,"SWd")) strcpy(var_name,"WIDTHV");
    else if (!strcmp(g_option.m_szDataType,"NCPd")) strcpy(var_name,"NCPV");
    else if (!strcmp(g_option.m_szDataType,"SNRd")) strcpy(var_name,"SNRVC");
    else if (!strcmp(g_option.m_szDataType,"CCRd")) strcpy(var_name,"CCORV");
    else if (!strcmp(g_option.m_szDataType,"DCHd")) strcpy(var_name,"DBZH");
    else if (!strcmp(g_option.m_szDataType,"DCVd")) strcpy(var_name,"DBZV");
    else if (!strcmp(g_option.m_szDataType,"DZR")) strcpy(var_name,"RHOHV");
    else if (!strcmp(g_option.m_szDataType,"CZR")) strcpy(var_name,"RHOHV");

    status = nc_inq_varid(ncid, var_name, &var_id);
    status = nc_inq_varndims(ncid, var_id, &ndims);
    status = nc_inq_vardimid(ncid, var_id, var_dims);
    for (recs = 0, i = 0; i < ndims; i++) {
      status = nc_inq_dim(ncid, var_dims[i], dim_name, &rec1);
      recs += rec1;
    }
    ray_tot2 = (short *) malloc(recs * sizeof(short));
    status = nc_get_var_short(ncid, var_id, ray_tot2);

    // Zratio 1단계 계산 
    if (!strcmp(g_option.m_szDataType,"DZR") || !strcmp(g_option.m_szDataType,"CZR") ) {
      for (i = 0; i < rdr.num_var; i++) {
        if (var_id == rdr.var[i].var_id) {
          scale2 = rdr.var[i].scale;
          blank2 = 0;   // RhoHV는 0
          break;
        }
      }
    }
    // H-V 계산
    else {
      for (i = 0; i < recs; i++) {
        if (ray_tot[i] != rdr.var[g_option.voln].blank && ray_tot2[i] != rdr.var[g_option.voln].blank)
          ray_tot[i] -= ray_tot2[i];
        else
          ray_tot[i] = rdr.var[g_option.voln].blank;
      }
      var_mode = 1;
    }

    // Zratio 2단계 계산
    if (!strcmp(g_option.m_szDataType,"DZR") || !strcmp(g_option.m_szDataType,"CZR") ) {
      status = nc_inq_varid(ncid, "ZDR", &var_id);
      status = nc_inq_varndims(ncid, var_id, &ndims);
      status = nc_inq_vardimid(ncid, var_id, var_dims);
      for (recs = 0, i = 0; i < ndims; i++) {
        status = nc_inq_dim(ncid, var_dims[i], dim_name, &rec1);
        recs += rec1;
      }
      ray_tot3 = (short *) malloc(recs * sizeof(short));
      status = nc_get_var_short(ncid, var_id, ray_tot3);

      for (i = 0; i < rdr.num_var; i++) {
        if (var_id == rdr.var[i].var_id) {
          scale3 = rdr.var[i].scale;
          blank3 = rdr.var[i].blank;
          break;
        }
      }
      var_mode = 2;
    }
  }
  else {
    status = nc_inq_varndims(ncid, rdr.var[g_option.voln].var_id, &ndims);
    status = nc_inq_vardimid(ncid, rdr.var[g_option.voln].var_id, var_dims);
    for (recs = 0, i = 0; i < ndims; i++) {
      status = nc_inq_dim(ncid, var_dims[i], dim_name, &rec1);
      recs += rec1;
    }
    ray_tot = (short *) malloc(recs * sizeof(short));
    status = nc_get_var_short(ncid, rdr.var[g_option.voln].var_id, ray_tot);

    if (!strcmp(g_option.m_szDataType,"DZHs") || !strcmp(g_option.m_szDataType,"DZVs") || !strcmp(g_option.m_szDataType,"CZHs") || !strcmp(g_option.m_szDataType,"CZVs")) {
      nr = (int)(0.5*20000.0/(float)gate_size + 0.5);
      var_mode = 3;
    }
  }
  status = nc_close(ncid);

  // 3.1. 해당 층에서
  eleva = rdr.swp[g_option.m_nSweepNo].elev;
  //oskim 0325
  //if (!strcmp(var.rdr,"HSR")) eleva = 0;
  cs = cos(eleva*DEGRAD);
  ss = sin(eleva*DEGRAD);
  re1 = 4.0*(RE + rdr.ht)/3.0;    // m
  ddeg = 360.0/(float)max_ray;    // 방위각 간격(deg)

  // 3.2. 지면기준 거리에 해당하는 gate 위치와 높이를 사전 계산
  //oskim 0325
  if (0){
  //if (!strcmp(var.rdr,"VER")) {
    for (i = 0; i < max_gate; i++)
      gate_num[i] = i;
  }
  else {
    for (i = 0; i < max_gate; i++) {
      s = i*gate_size + 0.5*gate_size;  // 지표상 거리(m)
      alpha = 1 - pow((double)(cs/sin(s/re1)), (double)(2.0));
      r = -(re1/alpha)*(sqrt(ss*ss - alpha) + ss);  // 진행상 거리(m)
      i1 = (int)((r - rdr.swp[g_option.m_nSweepNo].gate1)/gate_size);   // 그 층에서 s위치에 해당하는 gate위치
      gate_num[i] = i1;
    }
  }

  // 3.3. 방위각 0->360도 순서로 계산
  for (j = 0; j < max_ray; j++) {
    // 방위각
    az1 = j*ddeg + 0.5*ddeg;

    // 해당 방위각에 가장 가까운 ray를 찾는다
    for (dd_min = 1000, j1 = 0, j2 = 0; j2 < rdr.swp[g_option.m_nSweepNo].num_ray; j2++) {
      dd = fabs(az1 - rdr.swp[g_option.m_nSweepNo].ray[j2].az);
      if (dd < dd_min) {
        dd_min = dd;
        j1 = j2;
      }
    }
    if (dd_min > 2.0) continue;   // 2도 이상 차이가 나면 처리하지 않음

    // 3.4. 지면기준 거리에 따라 계산
    ray_start_idx = rdr.swp[g_option.m_nSweepNo].ray[j1].ray1;
    for (i = 0; i < max_gate; i++) {
      i1 = gate_num[i];   // 실제 gate 배열 위치
      if (i1 < 0 || i1 >= rdr.swp[g_option.m_nSweepNo].ray[j1].num_gate) continue;

      if (var_mode == 2) {
        r1 = ray_tot[i1+ray_start_idx]*scale;
        r2 = ray_tot2[i1+ray_start_idx]*scale2;
        r3 = ray_tot3[i1+ray_start_idx]*scale3;
        if (r1 > blank && r2 > blank2 && r3 > blank3) {
          rayf[j][i] = r3 / (r1 * r2);
          if (rayf[j][i] > 9) rayf[j][i] = blank;
        }
        else
          rayf[j][i] = blank;
      }
      else if (var_mode == 3) {
        r1 = ray_tot[i1+ray_start_idx]*scale;
        if (r1 > blank) {
          for (r2 = 0, n = 0, i3 = i1-nr; i3 <= i1+nr; i3++) {
            if (i3 < 0 || i3 >= max_gate) continue;
            r3 = ray_tot[i3+ray_start_idx]*scale;
            if (r3 > blank && r3 <= r1) {
              r2 += ((r3-r1)*(r3-r1));
              n++;
            }
          }
          if (n > 1)
            rayf[j][i] = sqrt(r2/(float)(n-1));
          else
            rayf[j][i] = blank;
        }
        else {
          rayf[j][i] = blank;
        }
      }
      else {
        rayf[j][i] = ray_tot[i1+ray_start_idx]*scale;
      }
    }
  }

  // 배열 해제
  if (var_mode == 2) free((char*) (ray_tot3));
  if (var_mode == 1 || var_mode == 2) free((char*) (ray_tot2));
  free((char*) (ray_tot));
  free((char*) (gate_num));

  // BLANK 값 대치
  for (j = 0; j < max_ray; j++) {
    for (i = 0; i < max_gate; i++) {
      if (rayf[j][i] <= blank+0.01) rayf[j][i] = blank2s;
    }
  }
  return 0;
}
