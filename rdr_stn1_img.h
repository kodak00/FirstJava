#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iconv.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <math.h>
#include <zlib.h>
#include <zconf.h>
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>
#include <gdfontl.h>
#include <gdfontg.h>
#include <netcdf.h>
#include <hdf5.h>

#include "cgiutil.h"
#include "nrutil.h"
#include "stn_inf.h"
#include "aws3_data.h"
#include "map_ini.h"
#include "url_io.h"
#include "rsl_wrc.h"
#include "rdr_file_header.h"

#define  DEGRAD   3.1415927/180.0
#define  RADDEG   180.0/3.1415927
#define  RE  6371008.77   // ��������(m)
#define  MAX_SWEEP  20    // ���̴� �ִ� Sweep��
#define  MAX_GOV  100     // AWS���� ������� �ִ��
#define  MAX_STN  5000    // AWS���� �ִ� ������
#define  NUM_AWS  1000    // ���ûAWS �ִ� ������
#define  RATE_RAY 2

//#define  RDR_RAW_DIR   "/DATA/RDR/RAW"        // ���̴� ������ RAW����
#define  RDR_RAW_DIR   "/DATA/INPUT/NCV"        // ���̴� ������ RAW����
#define  RDR_QCD_DIR   "/DATA/RDR/QCD"        // ���̴� ������ QCD����
#define  RDR_HSR_DIR   "/DATA/RDR/HSR"        // ���̴� ������ HSR����
#define  RDR_HCI_DIR   "/DATA/RDR/HCI"        // ���̴� ������ HCI����
#define  RDR_LNG_DIR   "/DATA/RDR/LNG"        // ���̴� ������ 480km����
#define  RDR_VER_DIR   "/DATA/RDR/VER"        // ���̴� ������ ������������
#define  RDR_CMP_DIR   "/DATA/RDR/CMP"        // ���̴��ռ����� ���������
#define  COLOR_SET_DIR "/rdr/REF/COLOR/"      // ����ǥ ���� �ӽ������
#define  MAP_DIR       "/DATA/GIS/MAP/dat"    // ���������� �ִ� �������

// �ռ��� �ڷῡ ����� �⺻��
#define  BLANK1  -30000     // No Area
#define  BLANK2  -25000     // No Data
#define  BLANK3  -20000     // Minimum Data

// �̹��� ������ �Ϻ� �⺻��
#define  LEG_pixel   35     // ����ǥ�ÿ���
#define  TITLE_pixel 20     // ����ǥ�ÿ���

// ����� �ѱ�TTF
#define  FONTTTF  "/usr/share/fonts/korean/TrueType/gulim.ttf"

// Volume ���
struct VOL_LIST {
  int  voln;
  char vol_cd[4];
};

//------------------------------------------------------------------------------
// ����� �Է� ����
struct INPUT_VAR {
  int   seq_now;      // ����ð� SEQ(��)
  int   seq;          // ���ؽð� SEQ(��)
  char  stn[8];       // ���̴� �����ڵ�
  char  rdr[8];       // HSR, HCI, 480 ��
  char  vol[8];       // UF�ڷ� ���� �ڵ�
  char  cpi[8];       // CPP(CAPPI), PPI(PPI)
  float stn_lon;      // ���̴�����Ʈ �浵(deg)
  float stn_lat;      // ���̴�����Ʈ ����(deg)
  float stn_ht;       // ���̴�����Ʈ �ع߰�(m)
  int   voln;         // obs�� �ش��ϴ� voln
  int   voln2;        // 2��° voln
  int   voln3;        // 3��° voln
  int   swpn;         // Sweep Number
  int   swpn2;        // �ι�° Sweep Number
  float swp_deg;      // Sweep ����(deg)
  float cappi_ht;     // CAPPI ��(m)
  float pulse_width;  // �޽���(us)
  int   area;         // ����
  float ang;          // RHI�ΰ�� ����(deg)
  float range;        // ǥ��ݰ�(m)
  char  map[8];       // ����� �����ڵ�
  char  color[8];     // ��û�� ����ǥ
  float ZRa;          // Z-R������� ���(�⺻:200)  Z=aR^b
  float ZRb;          // Z-R������� ����(�⺻:1.6)
  float grid;         // ����ũ��(km)
  int   zoom_level;   // Ȯ��Ƚ��
  int   zoom_rate;    // Ȯ�����
  char  zoom_x[16];   // X���� Ȯ��
  char  zoom_y[16];   // Y���� Ȯ��
  char  auto_man;     // �ڵ�����

  // ȭ��ǥ���
  int   size;         // �̹���ũ��(�ȼ�) : NI�� �ش�
  int   legend;       // ����ǥ��(1) ����
  int   aws;          // �������� ����(0:����ǥ�����, 1:������, 2:������ȣ, 3:������)
  int   sms;          // ��Ȱȭ ����(Ƚ��)

  // ����� �ڷ�
  int   num_gov;      // ������� �����
  char  gov_cd[MAX_GOV][8];  // ������ ������� TAG ���ڿ�
  int   num_aws_stn;  // AWS ������
  int   num_area_stn; // ǥ�⿵���� AWS ������
  int   NX, NY;       // ���ڼ�, ���� ���������� ������ 1�� ���ϸ� ��
  int   NI, NJ;       // �ڷ�ǥ���̹��� ����(�ȼ�)
  int   GI, GJ;       // ��ü �̹��� ����(�ȼ�)
  int   num_color;    // �����

  // ��������
  char  fname[120];   // ���ϸ�
  int   cdf;          // 1(nectCDF), 0(UF)
  int   dir_mode;     // 1 �̸� RDR_FTP_DIR ���
  int   HSR_mask;     // 1�̸� ����ŷ ����
};

// ���� �ڷ�
struct STN_VAL {
  int   stn_id;       // ������ȣ
  char  stn_ko[32];   // ������(�ѱ�)
  char  stn_sp[32];   // �����漺
  float x, y;         // ��ġ
  float ht;           // �ع߰�(m)
  float d;            // ��
  float v;            // ��(�ٶ����ͽ� Vǳ�� ���)
  float wd;           // ǳ��
  float ws;           // ǳ��
  int   re;           // ��������(�������� �Ǵ� 15�а��� ������)
  float s;            // �ӽ������
};
