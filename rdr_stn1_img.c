/*******************************************************************************
**
**  ���̴� �������ڷ� ǥ��
**
**=============================================================================*
**
**     o �ۼ��� : ����ȯ (2018.7.9)
**
********************************************************************************/
#include "rdr_stn1_img.h"

// Volume ���
struct VOL_LIST vol_inf[47] = {
  { 0, "DZ"},{ 1, "VR"},{ 2, "SW"},{ 3, "CZ"},{ 4, "ZT"},
  { 5, "DR"},{ 6, "LR"},{ 7, "ZD"},{ 8, "DM"},{ 9, "RH"},
  {10, "PH"},{11, "XZ"},{12, "CD"},{13, "MZ"},{14, "MD"},
  {15, "ZE"},{16, "VE"},{17, "KD"},{18, "TI"},{19, "DX"},
  {20, "CH"},{21, "AH"},{22, "CV"},{23, "AV"},{24, "SQ"},
  {25, "VS"},{26, "VL"},{27, "VG"},{28, "VT"},{29, "NP"},
  {30, "HC"},{31, "VC"},{32, "V2"},{33, "S2"},{34, "V3"},
  {35, "S3"},{36, "CR"},{37, "CC"},{38, "PR"},{39, "SD"},
  {40, "ZZ"},{41, "RD"},{42, "ET"},{43, "EZ"},{44, "TV"},
  {45, "ZV"},{46, "SN"}
};

// ����ü �󼼺з���(NCAR) ����ǥ
#define  NUM_HCI_DETAIL_COLOR  16
struct RDR_HCI_DETAIL_COLOR {
  int  hci;
  char hci_ko[40];
  int  color;
  int  R;
  int  G;
  int  B;
} hci_detail_color[NUM_HCI_DETAIL_COLOR] = {
  { 0,"����",     0,255,255,255},
  { 1,"����",     0,204,255,204},
  { 2,"�̽���",   0,153,204,255},
  { 3,"���Ѻ�",   0,102,153,255},
  { 4,"�߰���",   0, 51,102,255},
  { 5,"���Ѻ�",   0, 51, 51,204},
  { 6,"���",     0,255, 51,  0},
  { 7,"���/ ��", 0,255,102,  0},
  { 8,"���S/�ζ���", 0,255,153,153},
  { 9,"�ζ��� /��",0,255,204,204},
  {10,"�Ǽ�",     0,255,102,255},
  {11,"����",     0,102,255,255},
  {14,"���ð�",   0, 51,204,204},
  {12,"������", 0,245,255,102},
  {13,"����",   0,255,204,102},
  {50,"�񰭼�",   0,210,210,210},
};

//------------------------------------------------------------------------------
// ������
struct INPUT_VAR  var;
struct STN_VAL  stn_data[MAX_STN];

struct RDR_VOL_HEAD rdr;
float  **rayf;
float  gate_size, elev, ddeg;
float  blank1s = BLANK1*0.01, blank2s = BLANK2*0.01, blank3s = BLANK3*0.01;
int    max_ray, max_gate;

/*******************************************************************************
 *
 *  MAIN
 *
 *******************************************************************************/
int main()
{
  float km_px;
  int   num_sm, i;
  int   err = 0, code;

  // 1. �ʱ�ȭ
  setvbuf(stdout, NULL, _IONBF, 0);
  //alarm(20);

  printf("HTTP/1.0 200 OK\n");
  //printf("Content-type: text/plain\n\n");

  // 2. ����� �Է� ���� �м�
  if ( user_input() < 0 ) {
    err = 1;
  }

  // 3. ���� �� ���� ���� ���� Ȯ��
  if (rdr_stn_file() < 0) {
    err = 2;
  }
  else {
    if (var.cdf)
      code = rdr_stn_vol_chk_nc();
    else
      code = rdr_stn_vol_chk_uf();
    if (code < 0) err = 3;
  }

  // 4. CAPPI/PPI �ڷ� ����
  if (err == 0) {
    if (!strcmp(var.cpi,"CPP") && strcmp(var.rdr,"HSR") && strcmp(var.rdr,"VER") &&
                                  strcmp(var.rdr,"LNG") && strcmp(var.rdr,"LQC")) {
      if (var.cdf)
        code = rdr_stn_cappi_nc();
      else
        code = rdr_stn_cappi_uf();
    }
    else {
      if (var.cdf)
        code = rdr_stn_ppi_nc();
      else
        code = rdr_stn_ppi_uf();
    }
    if (code < 0) err = 4;
  }

  // 5. ����ó��
  if (err > 0) {
    rdr_stn_err_img(err);
    return 0;
  }
  //time_print("--1--");

  // 6. �������� �ϰ�, Ȯ�� ó��
  /*
  if (var.aws > 0 && strcmp(var.rdr,"VER") != 0) {
    aws_info_get();
    aws_data_get();
    aws_zooming();
  }
  */

  // 7. ��Ȱȭ
  if (strcmp(var.rdr,"HCI") != 0) {
    for (i = 0; i < var.sms; i++)
      grid_filter();
  }

  // 8. ���̴� ������ �ڷ� ǥ��
  rdr_stn_img();

  //alarm(0);
  return 0;
}

/*******************************************************************************
 *
 *  �����ڷ� �� �̹��� ǥ��� ����� ��û �м� �κ�
 *
 *******************************************************************************/
int user_input() {
  char *qs;
  char tmp[256], item[32], value[32], tm[30];
  int  iYY, iMM, iDD, iHH, iMI, iSS;
  int  iseq, i, j;

  // 1. ���� �ʱⰪ : �ڷẰ ó�� ���α׷����� ���� ��Ȳ�� �°� ����
  strcpy(tm, "0");        // ����ð�
  strcpy(var.map, "HR");  // ���̴� ����
  var.cdf = 3;            // AUTO
  var.zoom_level = 0;     // ��ü����
  var.zoom_rate = 2;      // 2�� Ȯ�밡 �⺻
  var.ZRa = 200;
  var.ZRb = 1.6;
  var.num_gov = 0;
  var.aws = 0;
  var.sms = 1;
  var.legend = 1;
  var.dir_mode = 0;

  // 2. GET ������� ���޵� ����� �Էº������� �ص�
  qs = getenv ("QUERY_STRING");
  if (qs == NULL) return -1;

  for (i = 0; qs[0] != '\0'; i++) {
    getword (value, qs, '&');
    getword (item, value, '=');
    if (strlen(value) == 0) continue;

    if      ( !strcmp(item,"stn")) strcpy(var.stn, value);
    else if ( !strcmp(item,"rdr")) strcpy(var.rdr, value);
    else if ( !strcmp(item,"vol")) strcpy(var.vol, value);
    else if ( !strcmp(item,"cpi")) strcpy(var.cpi, value);
    else if ( !strcmp(item,"cdf")) var.cdf = atoi(value);
    else if ( !strcmp(item,"swpn")) var.swpn = atoi(value);
    else if ( !strcmp(item,"ht")) var.cappi_ht = atof(value)*1000;
    else if ( !strcmp(item,"area")) var.area = atoi(value);
    else if ( !strcmp(item,"ang")) var.ang = atof(value);
    else if ( !strcmp(item,"map")) strcpy(var.map, value);
    else if ( !strcmp(item,"color")) strcpy(var.color, value);
    else if ( !strcmp(item,"tm"))  strcpy(tm, value);
    else if ( !strcmp(item,"auto_man")) var.auto_man = value[0];
    else if ( !strcmp(item,"aws")) var.aws = atoi(value);
    else if ( !strcmp(item,"gov")) {
      for (var.num_gov = 0, j = 0; value[0] != '\0'; j++) {
        getword (tmp, value, ':');
        if (strlen(tmp) >= 3) {
          strcpy(var.gov_cd[var.num_gov], tmp);
          var.num_gov++;
        }
      }
    }
    else if ( !strcmp(item,"sms")) var.sms = atoi(value);
    else if ( !strcmp(item,"ZRa")) var.ZRa = atof(value);
    else if ( !strcmp(item,"ZRb")) var.ZRb = atof(value);
    else if ( !strcmp(item,"zoom_level")) var.zoom_level = atoi(value);
    else if ( !strcmp(item,"zoom_rate"))  var.zoom_rate = atoi(value);
    else if ( !strcmp(item,"zoom_x")) strcpy(var.zoom_x, value);
    else if ( !strcmp(item,"zoom_y")) strcpy(var.zoom_y, value);
    else if ( !strcmp(item,"size")) var.size = atoi(value);
    else if ( !strcmp(item,"legend")) var.legend = atoi(value);
    else if ( !strcmp(item,"dir")) var.dir_mode = atoi(value);
  }

  // 3. �⺻�� ����
  // 3.1. ������ �ϳ��� ���
  if (strcmp(var.rdr,"HSR") == 0 || strcmp(var.rdr,"VER") == 0 || strstr(var.rdr,"LNG")) {
    var.swpn = 0;
    strcpy(var.cpi,"PPI");
  }

  // 3.2. ����ϴ� ���������� ũ��(km)
  var.NX = var.NY = 1000;

  // 3.3. �ݰ�
  if (strcmp(var.rdr,"VER") == 0) var.area = 1;
  if (var.area == 2)
    var.range = 250;
  else if (var.area == 4)
    var.range = 480;
  else
    var.range = 250;
  var.range *= 1000;  // km->m

  // 3.4. ������
  if (var.num_gov == 0) {
    var.num_gov = 1;
    strcpy(var.gov_cd[0],"KMA");
  }

  // 3.5. ������ ���, ZR ����� ����
  if (strcmp(var.vol, "SN") == 0) {
    var.ZRa = 2000;
    var.ZRb = 2.0;
  }
  else {
    if (var.ZRa == 0) var.ZRa = 200;
    if (var.ZRb == 0) var.ZRa = 1.6;
  }

  // 4. ����ð� �� ���� �����ð� ����
  get_time(&iYY, &iMM, &iDD, &iHH, &iMI, &iSS);
  iseq = time2seq(iYY, iMM, iDD, iHH, iMI, 'm');
  var.seq_now = iseq;

  // 5. ��û�ð� ����
  if (strlen(tm) < 10 || var.auto_man == 'a')
    var.seq = iseq;
  else {
    strncpy(tmp, &tm[0], 4);  tmp[4] = '\0';  iYY = atoi(tmp);
    strncpy(tmp, &tm[4], 2);  tmp[2] = '\0';  iMM = atoi(tmp);
    strncpy(tmp, &tm[6], 2);  tmp[2] = '\0';  iDD = atoi(tmp);
    strncpy(tmp, &tm[8], 2);  tmp[2] = '\0';  iHH = atoi(tmp);
    strncpy(tmp, &tm[10],2);  tmp[2] = '\0';  iMI = atoi(tmp);
    var.seq = time2seq(iYY, iMM, iDD, iHH, iMI, 'm');
  }

  // 6. ���� ã��
  for (i = 0; i < 30; i++, var.seq--) {
    if (rdr_stn_file() >= 0) break;
  }
  return 0;
}

/*******************************************************************************
 *
 *  �ڷᰡ �̻��� �ִ� ���, ���� �̹��� ���
 *
 *******************************************************************************/
int rdr_stn_err_img(int err)
{
  gdImagePtr im;
  char  text[120], tmp[120];
  int   YY, MM, DD, HH, MI;
  int   color_lvl[16];
  int   x = 20, y = 30, i, j, k, n;

  // 1. �̹��� ���� ����
  var.NI = var.size;
  var.NJ = var.NI;
  var.GI = var.NI;
  if (var.legend == 1) var.GI += LEG_pixel;
  var.GJ = var.NJ + TITLE_pixel;

  // 2. �̹��� ����ü ���� �� ����ǥ �б�
  im = gdImageCreate(var.GI, var.GJ);
  color_lvl[0] = gdImageColorAllocate(im, 240, 240, 240);   // ����
  color_lvl[1] = gdImageColorAllocate(im, 0, 0, 0);         // ������
  gdImageFilledRectangle(im, 0, 0, var.GI, var.GJ, color_lvl[0]);

  // 3. ���� �޽���
  seq2time(var.seq, &YY, &MM, &DD, &HH, &MI, 'm', 'n');
  sprintf(text, "TIME = %04d.%02d.%02d.%02d:%02d / err = %d", YY, MM, DD, HH, MI, err);
  gdImageString(im, gdFontLarge, x, y, text, color_lvl[1]);

  sprintf(text, "SITE = %s / %s / %s", var.stn, var.rdr, var.vol);
  gdImageString(im, gdFontLarge, x, (y += 20), text, color_lvl[1]);

  sprintf(text, "MODE = %s / swpn = %d / cappi = %.2fkm", var.cpi, var.swpn, var.cappi_ht);
  gdImageString(im, gdFontLarge, x, (y += 20), text, color_lvl[1]);

  sprintf(text, "SIZE = %dpx / ZRa = %.1f / ZRb = %.2f", var.size, var.ZRa, var.ZRb);
  gdImageString(im, gdFontLarge, x, (y += 20), text, color_lvl[1]);

  if (err == 2)
    sprintf(text, "FILE = %s (file is not found)", var.fname);
  else
    sprintf(text, "FILE = %s", var.fname);
  gdImageString(im, gdFontLarge, x, (y += 20), text, color_lvl[1]);

  if (err == 3) {
    sprintf(text, "Vol.(%s:%d) is not exist", var.vol, var.voln);
    gdImageString(im, gdFontLarge, x, (y += 20), text, color_lvl[1]);
    /*
    gdImageString(im, gdFontLarge, x, (y += 20), "Vols : ", color_lvl[1]);
    for (n = 0, k = 0; k < radar->h.nvolumes; k++) {
      if ((volume = radar->v[k]) == NULL) continue;
      for (i = 0; i < 14; i++) {
        if (k == vol_inf[k].voln) {
          x = 110 + n*25;
          gdImageString(im, gdFontLarge, x, y, vol_inf[k].vol_cd, color_lvl[1]);
          n++;
          break;
        }
      }
    }
    */
  }
  else if (err == 4) {
    sprintf(text, "CAPPI/PPI data making error");
    gdImageString(im, gdFontLarge, x, (y += 20), text, color_lvl[1]);
  }

  // 4. �̹��� ����
  printf("Content-type: image/png\n\n");
  gdImagePng(im, stdout);
  gdImageDestroy(im);

  return 0;
}

/*******************************************************************************
 *
 *  ���̴� ������ �̹��� ���� �� ǥ��
 *
 *******************************************************************************/
int rdr_stn_img()
{
  gdImagePtr im;
  FILE  *fp;
  int   color_lvl[256];
  float data_lvl[256];

  // 1. �̹��� ���� ����
  var.NI = var.size;
  var.NJ = var.NI;
  var.GI = var.NI;
  if (var.legend == 1) var.GI += LEG_pixel;
  var.GJ = var.NJ + TITLE_pixel;

  // 2. �̹��� ����ü ���� �� ����ǥ �б�
  im = gdImageCreate(var.GI, var.GJ);
  color_table(im, color_lvl, data_lvl);
  gdImageFilledRectangle(im, 0, 0, var.GI, var.GJ, color_lvl[240]);

  // 3. �����ڷ� �̹��� ����
  rdr_stn_grid_disp(im, color_lvl, data_lvl);

  // 4. ���� �׸���
  if (strcmp(var.rdr,"VER") != 0) {
    if (var.zoom_level >= 1) map_disp(im, color_lvl[249], 1);
    map_disp(im, color_lvl[242], 4);
  }

  // 5. ���ɿ� �׸���
  ring_disp(im, color_lvl);

  // 6. ���� ǥ��
  if (var.aws > 0 && strcmp(var.rdr,"VER") != 0) aws_disp(im, color_lvl);

  // 7. ���� �׸�
  title_disp(im, color_lvl);

  // 8. ���� �׸���
  legend_disp(im, color_lvl, data_lvl);
  gdImageRectangle(im, 0, TITLE_pixel, var.NI-1, var.GJ-1, color_lvl[242]);

  // 9. �̹��� ����
  printf("Content-type: image/png\n\n");
  gdImagePng(im, stdout);
  gdImageDestroy(im);
  return 0;
}

/*=============================================================================*
 *  ����ǥ
 *=============================================================================*/
int color_table(gdImagePtr im, int color_lvl[], float data_lvl[])
{
  FILE  *fp;
  char  color_file[120];
  int   num_color;
  int   R, G, B;
  float v1;

  // 1. HCI ����ǥ ����
  if (strcmp(var.rdr,"HCI") == 0) {
    var.num_color = NUM_HCI_DETAIL_COLOR;
    for (num_color = 0; num_color < NUM_HCI_DETAIL_COLOR; num_color++) {
      R = hci_detail_color[num_color].R;
      G = hci_detail_color[num_color].G;
      B = hci_detail_color[num_color].B;
      hci_detail_color[num_color].color = gdImageColorAllocate(im, R, G, B);
    }
  }

  // 2. ��Ÿ �����鿡 ���� ����ǥ �ڷ� �б�
  else {
    strcpy(color_file,COLOR_SET_DIR);
    if      (!strcmp(var.vol,"ET")) strcat(color_file,"color_rdr_ebh.rgb");
    else if (!strcmp(var.vol,"EZ")) strcat(color_file,"color_rdr_ebh.rgb");
    else if (!strcmp(var.vol,"DZd")) strcat(color_file,"color_rdr_DZd.rgb");
    else if (!strcmp(var.vol,"CZd")) strcat(color_file,"color_rdr_CZd.rgb");
    else if (!strcmp(var.vol,"VRd")) strcat(color_file,"color_rdr_VRd.rgb");
    else if (!strcmp(var.vol,"SWd")) strcat(color_file,"color_rdr_SWd.rgb");
    else if (!strcmp(var.vol,"DCHd")) strcat(color_file,"color_rdr_DZ.rgb");
    else if (!strcmp(var.vol,"DCVd")) strcat(color_file,"color_rdr_DZ.rgb");
    else if (!strcmp(var.vol,"DZHs")) strcat(color_file,"color_rdr_DZ.rgb");
    else if (!strcmp(var.vol,"DZVs")) strcat(color_file,"color_rdr_DZ.rgb");
    else if (!strcmp(var.vol,"CZHs")) strcat(color_file,"color_rdr_DZ.rgb");
    else if (!strcmp(var.vol,"CZVs")) strcat(color_file,"color_rdr_DZ.rgb");
    else if (!strcmp(var.vol,"DZR")) strcat(color_file,"color_rdr_ZR.rgb");
    else if (!strcmp(var.vol,"CZR")) strcat(color_file,"color_rdr_ZR.rgb");
    else if (!strcmp(var.vol,"NCPd")) strcat(color_file,"color_rdr_NCPd.rgb");
    else if (!strcmp(var.vol,"SNRd")) strcat(color_file,"color_rdr_SNRd.rgb");
    else if (!strcmp(var.vol,"CCRd")) strcat(color_file,"color_rdr_CCRd.rgb");
    else if (!strcmp(var.vol,"DCHd")) strcat(color_file,"color_rdr_DCd.rgb");
    else if (!strcmp(var.vol,"DCVd")) strcat(color_file,"color_rdr_DCd.rgb");
    else if (!strncmp(var.vol,"NCP",3)) strcat(color_file,"color_rdr_NCP.rgb");
    else if (!strncmp(var.vol,"SNR",3)) strcat(color_file,"color_rdr_SNR.rgb");
    else if (!strncmp(var.vol,"CCR",3)) strcat(color_file,"color_rdr_CCR.rgb");
    else {
      strcat(color_file,"color_rdr_");
      strncat(color_file,var.vol,2);
      strcat(color_file,".rgb");
    }

    var.num_color = 0;
    if ((fp = fopen(color_file, "r")) != NULL) {
      while (fscanf(fp, "%d %d %d %f\n", &R, &G, &B, &v1) != EOF) {
        color_lvl[var.num_color] = gdImageColorAllocate(im, R, G, B);
        data_lvl[var.num_color] = v1;
        var.num_color++;
        if (var.num_color > 119) break;
      }
      fclose(fp);
    }
  }

  // 3. ��Ÿ ����ǥ ����
  color_lvl[240] = gdImageColorAllocate(im, 180, 180, 180);   // ����1
  color_lvl[241] = gdImageColorAllocate(im, 255, 255, 255);   // ����2
  color_lvl[242] = gdImageColorAllocate(im, 30, 30, 30);      // ������
  color_lvl[243] = gdImageColorAllocate(im, 12, 28, 236);     // ����
  color_lvl[244] = gdImageColorAllocate(im, 0, 0, 0);         // ������
  color_lvl[245] = gdImageColorAllocate(im, 240, 240, 240);
  color_lvl[246] = gdImageColorAllocate(im, 255, 0, 0);
  color_lvl[247] = gdImageColorAllocate(im, 0, 0, 255);
  color_lvl[248] = gdImageColorAllocate(im, 160, 160, 160);   // ����3
  color_lvl[249] = gdImageColorAllocate(im, 110, 110, 110);   // �ñ����
  return 0;
}

/*=============================================================================*
 *  CAPPI/PPI ǥ��
 *=============================================================================*/
int rdr_stn_grid_disp(gdImagePtr im, int color_lvl[], float data_lvl[])
{
  float lvl[256], dbz1, rain1;
  float rate, az1, s, x1, y1, x, y, dd;
  float zm = 1.0, xo = 0.0, yo = 0.0;
  float nx = 2.0*var.range*0.001, ny = 2.0*var.range*0.001, zx, zy;
  int   swpn, hci_ok, dn, ok;
  int   i, j, k, i1, j1, j2, ia, ja, color1;

  // 1. ����ó��
  if (var.num_color <= 0) {
    gdImageString(im, gdFontLarge, 100, 100, "No Color Level", color_lvl[244]);
    return -1;
  }

  // 2. Ȯ���, �߽���ġ�� Ȯ����� ���
  if (var.zoom_level > 0) {
    if (var.zoom_rate == 3) {
      for (i = 0; i < 2; i++) {
        zx = var.zoom_x[i]-'0';
        zy = var.zoom_y[i]-'0';
        if (zx == 0 || zy == 0) break;

        xo += (nx/9.0*(zx-1)/zm);
        yo += (ny/9.0*(zy-1)/zm);
        zm *= var.zoom_rate;
      }
    }
    else if (var.zoom_rate == 2) {
      for (i = 0; i < 7; i++) {
        zx = var.zoom_x[i]-'0';
        zy = var.zoom_y[i]-'0';
        if (zx == 0 || zy == 0) break;

        xo += (nx/8.0*(zx-1)/zm);
        yo += (ny/8.0*(zy-1)/zm);
        zm *= var.zoom_rate;
      }
    }
  }

  // 3. ���� ����
  if (strcmp(var.vol,"RN") == 0 || strcmp(var.vol,"SN") == 0) {
    for (k = 0; k < var.num_color; k++) {
      rain1 = data_lvl[k];
      dbz_rain_conv(&dbz1, &rain1, 1);
      lvl[k] = dbz1;
    }
  }
  else {
    for (k = 0; k < var.num_color; k++)
      lvl[k] = data_lvl[k];
  }

  // 4. �̹��� �ȼ����� ���
  rate = nx/(float)(var.NI);
  if (strcmp(var.rdr,"HCI") == 0)
    hci_ok = 1;
  else
    hci_ok = 0;

  for (j = 1; j < var.NJ; j++) {
  for (i = 1; i < var.NI; i++) {
    // 4.0. �ȼ��� �ش��ϴ� ��ǥ Ȯ��
    y1 = rate*j;
    x1 = rate*i;

    y = (y1/zm + yo - 0.5*ny)*1000;
    x = (x1/zm + xo - 0.5*nx)*1000;

    // 4.1. �ش� �������� ray ��ġ Ȯ��
    if (y > 0 || y < 0)
      az1 = atan2(x, y)*RADDEG;
    else
      az1 = 0;
    if (az1 < 0) az1 += 360;

    // 4.2. �������̴�/�ջ극�̴� ���󱸿� Ȯ��
    ok = 0;
    if (!strcmp(var.stn,"DJK")) {     // ������
      if (az1 >= 44 && az1 <= 46)
        ok = 1;
      else if (az1 >= 79 && az1 <= 81)
        ok = 1;
      else if (az1 >= 140 && az1 <= 190)
        ok = 1;
    }
    else if (!strcmp(var.stn,"SRI")) {    // ������
      if (az1 >= 50 && az1 <= 120)
        ok = 1;
      else if (az1 >= 220 && az1 <= 280)
        ok = 1;
    }
    else if (!strcmp(var.stn,"MIL")) {    // ���ϻ�
      if (az1 >= 25 && az1 <= 27)
        ok = 1;
      else if (az1 >= 110 && az1 <= 225)
        ok = 1;
    }
    else if (!strcmp(var.stn,"IIA")) {    // �ջ�
      if (elev <= 2.0) {
        if (az1 >= 324.2 && az1 <= 337.4) ok = 1;
      }
      else if (az1 <= 5.0) {
        if (az1 >= 334.4 && az1 <= 337.4) ok = 1;
      }
    }
    if (ok) continue;

    // 4.3. ����ǥ��� �̵��Ÿ� ����� �ش� bin�� ��ġ Ȯ��
    s = sqrt(x*x + y*y);
    ia = (int)(s/gate_size);
    if (ia < 0 || ia >= max_gate) continue;

    // 4.4. �ش���ġ�� �ڷ� �б�
    ja = (int)(az1/ddeg);
    if (ja < 0) ja += max_ray;
    if (ja >= max_ray) ja -= max_ray;

    dd = rayf[ja][ia];

    // 4.5. �ڷᰡ �������� ������ ǥ��
    if (dd > blank1s) {
      if (hci_ok) {
        dn = (int)(dd + 0.1);
        for (k = 0; k < NUM_HCI_DETAIL_COLOR; k++) {
          if (dn == hci_detail_color[k].hci) {
            gdImageSetPixel(im, i, var.GJ-j, hci_detail_color[k].color);
            break;
          }
        }
      }
      else {
        color1 = color_lvl[var.num_color-1];
        for (k = 0; k < var.num_color; k++) {
          if (dd <= lvl[k]) {
            color1 = color_lvl[k];
            break;
          }
        }
        gdImageSetPixel(im, i, var.GJ-j, color1);
      }
    }
  }
  }

  free((char *) (rayf));

  return 0;
}

/*=============================================================================*
 *  Z-R �����
 *     - mode : 0(dBZ->������), 1(������->dBZ)
 *=============================================================================*/
int dbz_rain_conv(float *dbz, float *rain, int mode)
{
  static int first = 0;
  static float za, zb;

  if (first == 0) {
    za = 0.1/var.ZRb;
    zb = log10(var.ZRa)/var.ZRb;
    first = 1;
  }

  if (mode == 0) {
    //*rain = (*dbz*0.1 - log10(var.ZRa) )/var.ZRb;
    *rain = *dbz*za - zb;
    *rain = pow(10.0, *rain);
  }
  else if (mode == 1) {
    *dbz = 10.0 * log10( var.ZRa * pow(*rain, var.ZRb) );
  }
  return 0;
}

/*=============================================================================*
 *  ���� ǥ��
 *=============================================================================*/
int map_disp(gdImagePtr im, int color_map, int kind)
{
  FILE  *fp;
  char  fname[120];
  float zm = 1.0, xo = 0.0, yo = 0.0, x1, y1, x2, y2;
  float dr = (var.NX)*0.5 - (var.range)*0.001;
  float nx, ny, zx, zy, rate, rg;
  int   depth, mode;
  int   i, j, k, n;

  // 0. �ʱ� ����
  xo = dr;
  yo = dr;
  nx = ny = 2.0*var.range*0.001;
  rate = nx/(float)(var.NI);

  if (var.NI > 600)
    depth = 1;
  else if (var.NI > 300 && kind == 4)
    depth = 1;
  else
    depth = 0;

  // 1. Ȯ���, �߽���ġ�� Ȯ����� ���
  if (var.zoom_level > 0) {
    if (var.zoom_rate == 3) {
      for (i = 0; i < 2; i++) {
        zx = var.zoom_x[i]-'0';
        zy = var.zoom_y[i]-'0';
        if (zx == 0 || zy == 0) break;

        xo += (nx/9.0*(zx-1)/zm);
        yo += (ny/9.0*(zy-1)/zm);
        zm *= var.zoom_rate;
      }
    }
    else if (var.zoom_rate == 2) {
      for (i = 0; i < 7; i++) {
        zx = var.zoom_x[i]-'0';
        zy = var.zoom_y[i]-'0';
        if (zx == 0 || zy == 0) break;

        xo += (nx/8.0*(zx-1)/zm);
        yo += (ny/8.0*(zy-1)/zm);
        zm *= var.zoom_rate;
      }
    }
  }

  // 2. �ؾȼ� ǥ��
  sprintf(fname, "%s/RDR_%s_map%d.dat", MAP_DIR, var.stn, kind);
  if ((fp = fopen(fname, "r")) != NULL) {
    while (fscanf(fp, "%d %d\n", &n, &mode) != EOF) {
      for (i = 0; i < n; i++) {
        fscanf(fp, "%f %f\n", &x2, &y2);

        if (var.zoom_level >= 0) {
          x2 = zm*(x2-xo);
          y2 = zm*(y2-yo);
        }
        x2 *= ((float)(var.NI)/(float)nx);
        y2 *= ((float)(var.NJ)/(float)ny);
        if (i > 0) {
          gdImageLine(im, (int)x1, var.GJ-(int)y1, (int)x2, var.GJ-(int)y2, color_map);
          if (depth) gdImageLine(im, (int)x1, var.GJ-(int)y1-1, (int)x2, var.GJ-(int)y2-1, color_map);
        }
        x1 = x2;
        y1 = y2;
      }
    }
    fclose(fp);
  }
  return 0;
}

/*=============================================================================*
 *  ���ɿ� ǥ��
 *=============================================================================*/
int ring_disp(gdImagePtr im, int color_lvl[])
{
  float zm = 1.0, xo = 0.0, yo = 0.0, x1, y1, x2, y2;
  float nx, ny, zx, zy, rate, rg;
  int   x, y;
  int   i, j, k;

  // 1. �ʱ� ����
  nx = ny = 2.0*var.range*0.001;
  rate = (float)(var.NI)/nx;

  // 2. Ȯ���, �߽���ġ�� Ȯ����� ���
  if (var.zoom_level > 0) {
    if (var.zoom_rate == 3) {
      for (i = 0; i < 2; i++) {
        zx = var.zoom_x[i]-'0';
        zy = var.zoom_y[i]-'0';
        if (zx == 0 || zy == 0) break;

        xo += (nx/9.0*(zx-1)/zm);
        yo += (ny/9.0*(zy-1)/zm);
        zm *= var.zoom_rate;
      }
    }
    else if (var.zoom_rate == 2) {
      for (i = 0; i < 7; i++) {
        zx = var.zoom_x[i]-'0';
        zy = var.zoom_y[i]-'0';
        if (zx == 0 || zy == 0) break;

        xo += (nx/8.0*(zx-1)/zm);
        yo += (ny/8.0*(zy-1)/zm);
        zm *= var.zoom_rate;
      }
    }
  }

  // 2. �밢�� ǥ��
  for (k = 0; k < 4; k++) {
    if      (k == 0) { x1 = 0;  y1 = ny/2; x2 = nx;  y2 = ny/2; }
    else if (k == 1) { x1 = nx/2; y1 = 0;  x2 = nx/2;  y2 = ny; }
    else if (k == 2) { x1 = 0,  y1 = 0;  x2 = nx;  y2 = ny; }
    else if (k == 3) { x1 = 0,  y1 = ny; x2 = nx;  y2 = 0;  }

    x1 = zm*(x1 - xo)*rate;  y1 = zm*(y1 - yo)*rate;
    x2 = zm*(x2 - xo)*rate;  y2 = zm*(y2 - yo)*rate;

    gdImageLine(im, (int)x1, var.GJ-(int)y1, (int)x2, var.GJ-(int)y2, color_lvl[249]);
  }

  // 3. ���ɿ� ǥ��
  if (strcmp(var.rdr,"VER") == 0) {
    for (k = 1; k <= var.range*0.001; k += 1) {
      rg = zm*2.0*k*rate;
      x1 = nx*0.5;  y1 = ny*0.5;
      x1 = zm*(x1 - xo)*rate;  y1 = zm*(y1 - yo)*rate;
      gdImageArc(im, (int)x1, var.GJ-(int)y1, (int)rg, (int)rg, 0, 360, color_lvl[249]);
    }
  }
  else {
    for (k = 50; k <= var.range*0.001; k += 50) {
      rg = zm*2.0*k*rate;
      x1 = nx*0.5;  y1 = ny*0.5;
      x1 = zm*(x1 - xo)*rate;  y1 = zm*(y1 - yo)*rate;
      gdImageArc(im, (int)x1, var.GJ-(int)y1, (int)rg, (int)rg, 0, 360, color_lvl[249]);
    }
  }

  // 4. RHI�� ���, ���õ� ������ ǥ��
  if (var.area == 9) {
    x = (int)(0.5*var.NI*(1.0 + sin(var.ang*DEGRAD)));
    y = TITLE_pixel + (int)(0.5*var.NI*(1.0 - cos(var.ang*DEGRAD)));
    gdImageLine(im, (int)(0.5*var.NI), TITLE_pixel+(int)(0.5*var.NI), x, y, color_lvl[246]);
  }
  return 0;
}
/*=============================================================================*
 *  EUC-KR���ڿ��� UTF-8�� ��ȯ
 *=============================================================================*/
int euckr2utf(char *str, char *out)
{
    iconv_t convp;
    size_t  ileft, oleft;
    int     err, len = strlen(str);

    ileft = len;
    oleft = len * 2;

    convp = iconv_open("UTF-8", "euc-kr");
    err = iconv(convp, &str, &ileft, &out, &oleft);
    iconv_close(convp);

    return err;
}

/*=============================================================================*
 *  ���� ǥ��(�ѱ�ó��)
 *=============================================================================*/
int title_disp(gdImagePtr im, int color_lvl[])
{
  char   title[80], tm_fc_str[100], num_stn_str[10], text[120], tmp[50];
  char   title_utf[100];
  double font_size = 11.5;
  int    brect[8];
  int    YY, MM, DD, HH, MI;
  int    x, y, i, k;

  // 1. ���񿵿��� ���� ó��
  gdImageFilledRectangle(im, 0, 0, var.GI, TITLE_pixel, color_lvl[241]);

  // 2. ���� ǥ��
  // 2.1 ������
  if      (strcmp(var.stn,"BRI") == 0) strcpy(title,"��ɵ�");
  else if (strcmp(var.stn,"KWK") == 0) strcpy(title,"���ǻ�");
  else if (strcmp(var.stn,"GDK") == 0) strcpy(title,"������");
  else if (strcmp(var.stn,"GNG") == 0) strcpy(title,"����");
  else if (strcmp(var.stn,"KSN") == 0) strcpy(title,"������");
  else if (strcmp(var.stn,"JNI") == 0) strcpy(title,"����");
  else if (strcmp(var.stn,"MYN") == 0) strcpy(title,"�����");
  else if (strcmp(var.stn,"PSN") == 0) strcpy(title,"������");
  else if (strcmp(var.stn,"GSN") == 0) strcpy(title,"���");
  else if (strcmp(var.stn,"SSP") == 0) strcpy(title,"����");
  else if (strcmp(var.stn,"IMJ") == 0) strcpy(title,"������");
  else if (strcmp(var.stn,"GRS") == 0) strcpy(title,"������");
  else if (strcmp(var.stn,"SBS") == 0) strcpy(title,"�ҹ��");
  else if (strcmp(var.stn,"SDS") == 0) strcpy(title,"�����");
  else if (strcmp(var.stn,"MHS") == 0) strcpy(title,"���Ļ�");
  else if (strcmp(var.stn,"BSL") == 0) strcpy(title,"�񽽻�");
  else if (strcmp(var.stn,"MIL") == 0) strcpy(title,"���ϻ�");
  else if (strcmp(var.stn,"SRI") == 0) strcpy(title,"������");
  else if (strcmp(var.stn,"DJK") == 0) strcpy(title,"������");
  else if (strcmp(var.stn,"K03") == 0) strcpy(title,"Ȳ����");
  else if (strcmp(var.stn,"K02") == 0) strcpy(title,"����");
  else if (strcmp(var.stn,"K01") == 0) strcpy(title,"����");
  else if (strcmp(var.stn,"RKSG") == 0) strcpy(title,"����(��)");
  else if (strcmp(var.stn,"RKJK") == 0) strcpy(title,"����(��)");
  else if (strcmp(var.stn,"KAN") == 0) strcpy(title,"����A");
  else if (strcmp(var.stn,"KWJ") == 0) strcpy(title,"����A");
  else if (strcmp(var.stn,"TAG") == 0) strcpy(title,"�뱸A");
  else if (strcmp(var.stn,"SCN") == 0) strcpy(title,"��õA");
  else if (strcmp(var.stn,"SAN") == 0) strcpy(title,"����A");
  else if (strcmp(var.stn,"SWN") == 0) strcpy(title,"����A");
  else if (strcmp(var.stn,"YCN") == 0) strcpy(title,"��õA");
  else if (strcmp(var.stn,"WNJ") == 0) strcpy(title,"����A");
  else if (strcmp(var.stn,"JWN") == 0) strcpy(title,"�߿�A");
  else  strcpy(title,var.stn);
  strcat(title," ");

  // 2.2. �ڷ�����
  strcat(title, var.rdr);

  // 2.3. ������
  if (strcmp(var.rdr,"HCI") != 0) {
    strcat(title, " ");
    if      (!strcmp(var.vol,"RN" )) strcat(title,"����");
    else if (!strcmp(var.vol,"SN" )) strcat(title,"����");
    else if (!strcmp(var.vol,"DZ" )) strcat(title,"DZ(H)");
    else if (!strcmp(var.vol,"DZv")) strcat(title,"DZ(V)");
    else if (!strcmp(var.vol,"DZd")) strcat(title,"DZ(H-V)");
    else if (!strcmp(var.vol,"DZHs")) strcat(title,"DZ.Std(H)");
    else if (!strcmp(var.vol,"DZVs")) strcat(title,"DZ.Std(V)");
    else if (!strcmp(var.vol,"DCHd")) strcat(title,"DZ-CZ(H)");
    else if (!strcmp(var.vol,"DCVd")) strcat(title,"DZ-CZ(V)");
    else if (!strcmp(var.vol,"CZ" )) strcat(title,"CZ(H)");
    else if (!strcmp(var.vol,"CZv")) strcat(title,"CZ(V)");
    else if (!strcmp(var.vol,"DZd")) strcat(title,"CZ(H-V)");
    else if (!strcmp(var.vol,"CZHs")) strcat(title,"CZ.Std(H)");
    else if (!strcmp(var.vol,"CZVs")) strcat(title,"CZ.Std(V)");
    else if (!strcmp(var.vol,"VR" )) strcat(title,"VR(H)");
    else if (!strcmp(var.vol,"VRv")) strcat(title,"VR(V)");
    else if (!strcmp(var.vol,"VRd")) strcat(title,"VR(H-V)");
    else if (!strcmp(var.vol,"SW" )) strcat(title,"SW(H)");
    else if (!strcmp(var.vol,"SWv")) strcat(title,"SW(V)");
    else if (!strcmp(var.vol,"SWd")) strcat(title,"SW(H-V)");
    else if (!strcmp(var.vol,"DR" )) strcat(title,"ZDR");
    else if (!strcmp(var.vol,"RH" )) strcat(title,"RhoHV");
    else if (!strcmp(var.vol,"PH" )) strcat(title,"PhiDP");
    else if (!strcmp(var.vol,"DZR" )) strcat(title,"DZratio");
    else if (!strcmp(var.vol,"CZR" )) strcat(title,"CZratio");
    else if (!strcmp(var.vol,"NCP" )) strcat(title,"NCP(H)");
    else if (!strcmp(var.vol,"NCPv")) strcat(title,"NCP(V)");
    else if (!strcmp(var.vol,"NCPd")) strcat(title,"NCP(H-V)");
    else if (!strcmp(var.vol,"SNR" )) strcat(title,"SNR(H)");
    else if (!strcmp(var.vol,"SNRv")) strcat(title,"SNR(V)");
    else if (!strcmp(var.vol,"SNRd")) strcat(title,"SNR(H-V)");
    else if (!strcmp(var.vol,"CCR" )) strcat(title,"CCR(H)");
    else if (!strcmp(var.vol,"CCRv")) strcat(title,"CCR(V)");
    else if (!strcmp(var.vol,"CCRd")) strcat(title,"CCR(H-V)");
    else strcat(title, var.vol);
  }

  // 2.4. ǥ����
  if (strcmp(var.rdr,"HSR") == 0 || strcmp(var.rdr,"VER") == 0) {
    sprintf(tmp, " %s", var.rdr);
  }
  else {
    if (strcmp(var.cpi,"CPP") == 0)
      sprintf(tmp, " CAPPI(%.0fm)", var.cappi_ht);
    else
      sprintf(tmp, " PPI(%.2f��)", var.swp_deg);
  }
  strcat(title,tmp);

  // 2.5. ���� ǥ��
  for (i = 0; i < 100; i++)
    title_utf[i] = 0;
  euckr2utf(title, title_utf);
  gdImageStringFT(im, &brect[0], color_lvl[247], FONTTTF, font_size, 0.0, 5, (int)(font_size+5), title_utf);
  gdImageStringFT(im, &brect[0], color_lvl[247], FONTTTF, font_size, 0.0, 6, (int)(font_size+5), title_utf);

  // 3. �ð� ���ڿ�
  seq2time(var.seq, &YY, &MM, &DD, &HH, &MI, 'm', 'n');
  sprintf(tm_fc_str, "%04d.%02d.%02d.%02d:%02d", YY, MM, DD, HH, MI);

  x = strlen(title)*8.5 + 10;
  gdImageString(im, gdFontLarge, x, 1, tm_fc_str, color_lvl[244]);
  gdImageString(im, gdFontLarge, x+1, 1, tm_fc_str, color_lvl[244]);

  // 4. �ڷ� ����
  if (var.cdf)
    strcpy(text,".nc");
  else
    strcpy(text,".uf");
  x = var.NI - strlen(text)*8.5 - 10;
  y = TITLE_pixel+3;
  gdImageString(im, gdFontLarge, x, y, text, color_lvl[244]);
  gdImageString(im, gdFontLarge, x+1, y, text, color_lvl[244]);

  // 5. pulse width
  sprintf(text, "%.0fus", var.pulse_width);
  x = var.NI - strlen(text)*8.5 - 10;
  y += 15;
  gdImageString(im, gdFontLarge, x, y, text, color_lvl[244]);
  gdImageString(im, gdFontLarge, x+1, y, text, color_lvl[244]);

  // 6. filter Ƚ��
  sprintf(text, "f%d", var.sms);
  x = var.NI - strlen(text)*8.5 - 10;
  y += 15;
  gdImageString(im, gdFontLarge, x, y, text, color_lvl[244]);
  gdImageString(im, gdFontLarge, x+1, y, text, color_lvl[244]);

  return 0;
}

/*=============================================================================*
 *  ���� ǥ��
 *=============================================================================*/
int legend_disp(gdImagePtr im, int color_lvl[], float data_lvl[])
{
  char   txt[20], txt_utf[30];
  double font_size = 9.0;
  int    brect[8];
  float  dy = (float)(var.NJ)/(float)(var.num_color);
  int    YY, MM, DD, HH, MI;
  int    x, y, i, j, k;

  if (var.legend != 1) return 0;
  if (var.num_color <= 0) return -1;

  // 1. ���� ���� ǥ��
  if (!strcmp(var.rdr,"HCI")) {
    for (k = 0; k <= var.num_color-1; k++) {
      y = TITLE_pixel + dy*k;
      gdImageFilledRectangle(im, var.NI, y, var.NI+8, y+dy, hci_detail_color[k].color);
    }
  }
  else {
    for (k = 0; k < var.num_color; k++) {
      y = var.GJ - dy*k;
      gdImageFilledRectangle(im, var.NI, y-dy, var.NI+8, y, color_lvl[k]);
    }
  }
  gdImageRectangle(im, var.NI-1, TITLE_pixel, var.NI+8, var.GJ-1, color_lvl[242]);

  // 2. ���ʰ� ǥ��
  gdImageFilledRectangle(im, var.NI+9, 0, var.GI-1, var.GJ, color_lvl[241]);
  if (!strcmp(var.rdr,"HCI")) {
    font_size = 9.0;
    for (k = 0; k <= var.num_color-1; k++) {
      for (j = 0; j < strlen(hci_detail_color[k].hci_ko)-1; j += 4) {
        y = TITLE_pixel + dy*k + dy/4 + j*4;
        strncpy(txt, &(hci_detail_color[k].hci_ko[j]), 4);
        txt[4] = '\0';
        for (i = 0; i < 30; i++) txt_utf[i] = 0;
        euckr2utf(txt, txt_utf);
        gdImageStringFT(im, &brect[0], color_lvl[244], FONTTTF, font_size, 0.0, var.NI+11, y, txt_utf);
      }
    }
  }
  else if (!strcmp(var.vol,"RH")) {
    for (k = 0; k < var.num_color-1; k++) {
      y = var.GJ - (k+1)*dy - 5;
      sprintf(txt, "%.2f", data_lvl[k]);
      gdImageString(im, gdFontSmall, var.NI+12, y, txt, color_lvl[244]);
    }
  }
  else if (!strcmp(var.vol,"RN") || !strcmp(var.vol,"SN")) {
    for (k = 0; k < var.num_color-1; k++) {
      y = var.GJ - (k+1)*dy - 5;
      if (data_lvl[k] >= 5)
        sprintf(txt, "%.0f", data_lvl[k]);
      else
        sprintf(txt, "%.1f", data_lvl[k]);
      gdImageString(im, gdFontSmall, var.NI+12, y, txt, color_lvl[244]);
    }
  }
  else if (!strcmp(var.vol,"DZR") || !strcmp(var.vol,"CZR")) {
    for (k = 0; k < var.num_color-1; k++) {
      y = var.GJ - (k+1)*dy - 5;
      sprintf(txt, "%.2f", data_lvl[k]);
      gdImageString(im, gdFontSmall, var.NI+12, y, txt, color_lvl[244]);
    }
  }
  else if (strstr(var.vol,"d")) {
    for (k = 0; k < var.num_color-1; k++) {
      y = var.GJ - (k+1)*dy - 5;
      sprintf(txt, "%.1f", data_lvl[k]);
      gdImageString(im, gdFontSmall, var.NI+12, y, txt, color_lvl[244]);
    }
  }
  else if (!strncmp(var.vol,"DZ",2) || !strncmp(var.vol,"CZ",2) ||
           !strncmp(var.vol,"VR",2) || !strcmp(var.vol,"PH")) {
    for (k = 0; k < var.num_color-1; k++) {
      y = var.GJ - (k+1)*dy - 5;
      sprintf(txt, "%.0f", data_lvl[k]);
      gdImageString(im, gdFontSmall, var.NI+12, y, txt, color_lvl[244]);
    }
  }
  else if (!strncmp(var.vol,"NCP",3)) {
    for (k = 0; k < var.num_color-1; k++) {
      y = var.GJ - (k+1)*dy - 5;
      sprintf(txt, "%.2f", data_lvl[k]);
      gdImageString(im, gdFontSmall, var.NI+12, y, txt, color_lvl[244]);
    }
  }
  else {
    for (k = 0; k < var.num_color-1; k++) {
      y = var.GJ - (k+1)*dy - 5;
      sprintf(txt, "%.1f", data_lvl[k]);
      gdImageString(im, gdFontSmall, var.NI+12, y, txt, color_lvl[244]);
    }
  }

  // 3. ���� ���� ǥ��
  if      (!strcmp(var.vol,"RN")) strcpy(txt,"mm/h");
  else if (!strcmp(var.vol,"SN")) strcpy(txt,"cm/h");
  else if (!strcmp(var.vol,"DZR")) strcpy(txt," ");
  else if (!strcmp(var.vol,"CZR")) strcpy(txt," ");
  else if (!strcmp(var.vol,"DZHs")) strcpy(txt,"dBz");
  else if (!strcmp(var.vol,"DZVs")) strcpy(txt,"dBz");
  else if (!strcmp(var.vol,"CZHs")) strcpy(txt,"dBz");
  else if (!strcmp(var.vol,"CZVs")) strcpy(txt,"dBz");
  else if (!strncmp(var.vol,"DZ", 2)) strcpy(txt,"dBZ");
  else if (!strncmp(var.vol,"VR", 2)) strcpy(txt,"m/s");
  else if (!strncmp(var.vol,"SW", 2)) strcpy(txt,"m/s");
  else if (!strncmp(var.vol,"CZ", 2)) strcpy(txt,"dBZ");
  else if (!strncmp(var.vol,"DC", 2)) strcpy(txt,"dBZ");
  else if (!strncmp(var.vol,"DR", 2)) strcpy(txt,"dB");
  else if (!strncmp(var.vol,"RH", 2)) strcpy(txt,"");
  else if (!strncmp(var.vol,"PH", 2)) strcpy(txt,"deg");
  else if (!strncmp(var.vol,"KD", 2)) strcpy(txt,"deg/km");
  else if (!strncmp(var.vol,"HC", 2)) strcpy(txt,"");
  else if (!strncmp(var.vol,"ET", 2)) strcpy(txt,"deg");
  else if (!strncmp(var.vol,"EZ", 2)) strcpy(txt,"km");
  else if (!strncmp(var.vol,"NCP",3)) strcpy(txt," ");
  else if (!strncmp(var.vol,"SNR",3)) strcpy(txt," ");
  else if (!strncmp(var.vol,"CCR",3)) strcpy(txt," ");

  x = var.NI - 3;
  gdImageString(im, gdFontLarge, x, 4, txt, color_lvl[244]);

  return 0;
}

/*=============================================================================*
 *  �ð� ǥ��
 *=============================================================================*/
int time_print(char *buf)
{
  int  YYg, MMg, DDg, HHg, MIg, SSg;

  get_time(&YYg, &MMg, &DDg, &HHg, &MIg, &SSg);
  printf("#%04d%02d%02d%02d%02d%02d:%s:\n", YYg, MMg, DDg, HHg, MIg, SSg, buf);
  return 0;
}
