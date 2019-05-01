#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <zlib.h>
#include "gd.h"
#include "gdfonts.h"
#include "gdfontg.h"
#include "gdfontl.h"

#include "cgiutil.h"
#include "map_ini.h"
#include "parameter.h"
#include "seq_time.h"    
#include "color.h"  
#include "lgt.h"
#include "common_disp_image.h"
#include "lgt_disp_image.h"

char *strptime(const char *s, const char *format, struct tm *tm);

//oskim 20180605 20190110 , colorbar index modify (9th color cnt is Hard Code Value)
float dbzDay[9]     = {1,2,3,4,6,10,15,20,45};
float dbzMonth[9]   = {1,2,3,4,6,10,20,40,90};
float dbzYear[9]    = {1,2,3,4,6,10,20,40,135};
/*
float dbzDay[8] = {2,3,4,6,10,15,20,45};
float dbzMonth[8] = {2,3,4,6,10,20,40,90};
float dbzYear[8] = {2,3,4,6,10,20,40,135};
*/
/*******************************************************************************
 * Real Time Lgt data read
 *******************************************************************************/
int data_init(int **DATA)
{
	int i,j;
	//?珂?화
    for (j=0; j<MJ; j++) 
    {
        for (i=0; i<MI; i++) 
        {
      	    //DATA[j][i] = -12800;
      	    DATA[j][i] = 0;
        }
    }
	return 0;
}

/*******************************************************************************
 * Real Time Lgt data read
 *******************************************************************************/
int real_time_data_read(PARAM_INFO var, int lcount, LDATA *ldata, int **DATA)
{
	int i, _x, _y;
	float x, y;
	struct lamc_parameter map;

	MAP_INFO mapInfo = comp_map_info();
	//MAP_INFO mapInfo = comp_map_info_1km();
	map = mapInfo.map;
	
	data_init(DATA);

	for(i=0 ; i<lcount ; i++)
    {
		//??표 ??환 (DEGREE to PIXEL)
		x = -999.0;
		y = -999.0;
		lamcproj(&ldata[i].lon, &ldata[i].lat, &x, &y, 0, map);
		
	    // 	_x =   (int) ( ( x - var.nOrgX ) / var.nXscalef +0.5);
    	//	_y =   (int) ( 1999 - ( y - var.nOrgY ) / var.nYscalef +0.5);
	
	 	_x =   (int) x;          
		_y =   (int) y;
		if ((_x >= 0 && _x < var.nDispX) && (_y >= 0 && _y < var.nDispY))
        {	
			DATA[_y][_x]++;
		}
	}

	return 0;
}

/*******************************************************************************
 * Day, Month, Year Acc Lgt data read
 *******************************************************************************/
int lgt_data_read(PARAM_INFO var, int **DATA, char *FILE_NAME)
{
	
	FILE *fp;
	int buf[MI];
	int	i, j, k, n;
	char timeStr[BUFFER_SIZE];
	char fileName[BUFFER_SIZE];
	struct tm curTime;	
    int fchk=0;
	
	//?珂?화 
	data_init(DATA);
	
	sprintf(timeStr, "%04d%02d%02d%02d%02d", var.YY, var.MM, var.DD, var.HH, var.min);
	strptime(timeStr, "%Y%m%d%H%M%S", &curTime);	
	
	for(k=0; k<=var.intv; k++)
    {
		if(k > 0 && var.tmode == 'D') curTime = getIncDay(curTime, -1);
		if(k > 0 && var.tmode == 'M') curTime = getIncMonth(curTime, -1);
		if(k > 0 && var.tmode == 'Y') curTime = getIncYear(curTime, -1);
		
		strftime(fileName, BUFFER_SIZE, FILE_NAME, &curTime);
		//fprintf(stderr, "%d . fileName = %s\n",k, fileName);
		
		if((fp = gzopen(fileName, "rb")) != NULL)
        { 
			 //Data Read
            for (j=0; j<MJ; j++)
            {
                n = gzread(fp, buf, MI*sizeof(int));   
                
                for (i=0; i<MI; i++)
                {
                	DATA[j][i] += buf[i];
                }
            }	    			
			fchk = 1;  
			gzclose(fp);       
		} 
	}
	
  return fchk;	
}

/*******************************************************************************
 * Lgt data Echo display
 *******************************************************************************/
int lgt_echo_disp(PARAM_INFO var, int **DATA, gdImagePtr im, int color[], COLOR_INFO color_info)
{                                                                                                  
	int	c, i;
	int legend_cnt;
    int ii, jj;  
    double fval;
	float rain;
	short **topo_data;

	legend_cnt = color_info.legend_cnt;

	float dbz[legend_cnt];                                                                           
	float dbz_min = 0.0f;    

	//?????? ?丙??? 표??                                                                             
	for (i=0; i<legend_cnt; i++)
	{
		rain = color_info.data[i].rain;
        dbz[i] = color_info.data[i].dbz;
	}

	if (var.comp_flg=='L' || var.comp_flg=='S')
    {
		topo_data = get_topo_data(var, TOPO_MAP_PATH);
	}

	for (jj=0; jj<var.nDispY; jj++)
    {
	    for (ii=0; ii<var.nDispX; ii++)
        {
            /*
            if(var.nzoomdepth == 0)
                fval = lgt_calc_radar_value_add(var, DATA, jj-20 , ii+10 );
            else if(var.nzoomdepth == 1)
                fval = lgt_calc_radar_value_add(var, DATA, jj-40, ii+20);
            else
            */
            fval = lgt_calc_radar_value_add(var, DATA, jj, ii);

            //if (fval < 2)
            //    fval = 0;

            //oskim 20185031 , divided hit count by 4 cause of AreaSize Error!
            if( fval != BOUND && fval != BAD_VAL )
                fval = fval/25.0;   //oskim 20190423

            //c = 16;
            //dbz 즉, color index 범위는 kma_lgtn_comis.col 에서 읽은것 무시하고, lgt_level_color_disp 함수 내에서 재정의 (day, month, year) 한것을 사용한다!! ==> kma_lgtn_comis.col에서는 단지 color 값만 사용함!
            //그리고 color index 9 이후 것도, NO Area, map 등에서 사용하므로 39개 color index는 유지해야 함!
            //oskim 20190110 , "int fval" -> "float fval"
            c = lgt_level_color_disp(fval, legend_cnt, dbz, dbz_min, var.tmode);
            //c = lgt_level_color_disp((int)fval, legend_cnt, dbz, dbz_min, var.tmode);
            /*
               if(c >= 0 && c != 33) 
               gdImageSetPixel(im, ii, var.nDispY-jj, color[c]);
               */

            /*
               if(ii < 190)
               {
                   gdImageSetPixel(im, ii, jj, color[35]);
                   continue;
                   }
                   */

            if (c >=0)
            {
                if(var.comp_flg=='L')
                {
                    if(topo_data[jj][ii]>0)
                    {
                        gdImageSetPixel(im, ii  , jj  , color[c]);
                    }
                }
                else if(var.comp_flg=='S') 
                {
                    if(topo_data[jj][ii]<=0) 
                    {
                        gdImageSetPixel(im, ii, jj, color[c]);
                    }
                }
                else
                {
                    gdImageSetPixel(im, ii, jj, color[c]);
                }
            }
        }                                                                                             
    }

    if (var.comp_flg=='L' || var.comp_flg=='S')
    {
        for (jj=0; jj<var.nDispY; jj++)
        {
            free(topo_data[jj]);
        }
        free(topo_data);	 
    }
    return 0;                                                                                        
}

//oskim 20190110 , change "int v" -> "float v"
int lgt_level_color_disp(float v, int cnt, float *dbz, float dbz_min, char tmode)  
{	
    int c = -1, i;  
    int r = 1;


    dbz = dbzDay;

    if (tmode == 'M')
    {
        //oskim 20180605 , colorbar index modify
        //r = r * 2;
        dbz = dbzMonth;
    }
    else if (tmode == 'Y')
    {
        //oskim 20180605 , colorbar index modify
        //r = r * 3;
        dbz = dbzYear;
    }

    if (v <= dbz_min) 
    //if ((int)v == (int)dbz_min) 
    {
        c = 35;		// NO area			
    }
    else
    {	
        for (i=0; i<cnt; i++)
        {
            //oskim 20190110 ,
            //if (v < ((int)dbz[i]*r))
            if (v < (dbz[i]*r))
            {
                c = i;
                break;
            }
        }	
        //		if(i==cnt && r >= (int)dbz[i-1]) {
        if (i==cnt && v >= ((int)dbz[cnt-1]*r))
        {
            c = i-1;
        }    	
    }
    return c;
} 

/*============================================================================*
 * Title display
 *============================================================================*/
int title_disp_right(PARAM_INFO var, gdImagePtr im,int color[], char* strTitle)
{
    char title[36], text[36];
    int YY, MM, DD, HH, min;
    int brect[8];
    char *err;
    double sz = 13.;
    int sz_y = 0;	
    char f[37];
    char timeStr[BUFFER_SIZE];
    struct tm stime, etime;

    sprintf(f, "%s/times.ttf", FONT_PATH); // User supplied font

    if (var.nDispX < 400) sz = 9.;

    //?珂?화
    gdImageFilledRectangle(im, var.nDispX, 0, var.nDispX+var.title_width, var.nDispY, color[35]);
    gdImageRectangle(im, var.nDispX, 0, var.nDispX+var.title_width, var.nDispY, color[34]);

    seq2time(var.seq, &YY, &MM, &DD, &HH, &min, 'm', 'n');

    sprintf(timeStr, "%04d%02d%02d%02d%02d", var.YY, var.MM, var.DD, var.HH, var.min);
    stime = getConvStrToDateTime(timeStr);

    //strptime(timeStr, "%Y%m%d%H%M%S", &curTime);

    strcpy(title, strTitle);
    sprintf(text, "%s", title);
    err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 30, text);

    if (var.tmode == 'R')
    {
        strftime(text, sizeof(text), "%Y.%m.%d %H:%M", &stime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+15, 90-sz_y, text);

        sprintf(text,"~");
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+60, 75-sz_y, text);

        etime = getIncMin(stime, (-1)*var.intv);
        strftime(text, sizeof(text), "%Y.%m.%d %H:%M", &etime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+15, 60-sz_y, text);
    }
    else if (var.tmode == 'D')
    {
        strftime(text, sizeof(text), "%Y.%m.%d", &stime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+35, 90-sz_y, text);

        sprintf(text,"~");
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+60, 75-sz_y, text);

        etime = getIncDay(stime, (-1)*var.intv);
        strftime(text, sizeof(text), "%Y.%m.%d", &etime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+35, 60-sz_y, text);
    }
    else if (var.tmode == 'M')
    {
        strftime(text, sizeof(text), "%Y.%m", &stime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+45, 90-sz_y, text);

        sprintf(text,"~");
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+60, 75-sz_y, text);

        etime = getIncMonth(stime, (-1)*var.intv);
        strftime(text, sizeof(text), "%Y.%m", &etime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+45, 60-sz_y, text);
    }
    else if (var.tmode == 'Y')
    {
        strftime(text, sizeof(text), "%Y", &stime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+55, 90-sz_y, text);

        sprintf(text,"~");
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+60, 75-sz_y, text);

        etime = getIncYear(stime, (-1)*var.intv);
        strftime(text, sizeof(text), "%Y", &etime);
        err = gdImageStringFT(im, &brect[0], color[34], f, 10, 0.0, var.nDispX+55, 60-sz_y, text);
    }
    /*
       if (var.tmode == 'R')
       {
       if (var.intv >= 60)
       sprintf(text, "%d hours", var.intv/60);
       else
       sprintf(text, "%d min", var.intv);

       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 50-sz_y, text);
       sprintf(text, "%04d.%02d.%02d", YY, MM, DD );	
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 70-sz_y, text);
       sprintf(text, "%d:%02d(KST)", HH,min);	
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 90-sz_y, text);
       }
       else if (var.tmode == 'D')
       {
       sprintf(text, "%d days", var.intv);
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 50-sz_y, text);
       sprintf(text, "%04d.%02d.%02d", YY, MM, DD );	
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 70-sz_y, text);
       }
       else if (var.tmode == 'M')
       {
       sprintf(text, "%d months", var.intv);
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 50-sz_y, text);
       sprintf(text, "%04d.%02d", YY, MM);	
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 70-sz_y, text);
       }
       else if (var.tmode == 'Y')
       {
       sprintf(text, "%d years", var.intv);
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 50-sz_y, text);
       sprintf(text, "%04d", YY);	
       err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+15, 70-sz_y, text);
       }
       */	
    strcpy(title, "hits/km");
    sprintf(text, "%s", title);
    err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+50, 130-sz_y, text);

    strcpy(title, "2");
    sprintf(text, "%s", title);
    err = gdImageStringFT(im, &brect[0], color[34], f, sz-5, 0.0, var.nDispX+100, 125-sz_y, text);

    //data display area box
    gdImageRectangle(im, 0, 0, var.nDispX, var.nDispY, color[34]);

    return 0; 
}

/*============================================================================*
 * Level Color display
 *============================================================================*/
int level_disp_right(PARAM_INFO var, gdImagePtr im, int color[], COLOR_INFO color_info)
{
    char  text[126];
    float dy, rain;
    int   i, j, jc;
    int table_start_x,table_start_y;
    int table_finish_x,table_finish_y;
    int brect[8];
    char *err;
    double sz = 11.;
    char f[37];
    float r = 1;
    sprintf(f, "%s/times.ttf", FONT_PATH); // User supplied font

    if (var.nDispX < 400) sz = 8.;

    jc = color_info.legend_cnt;

    table_start_x  = var.nDispX+20;
    table_finish_x = var.nDispX+60;
    table_start_y  = var.nDispY*1/6+35;
    table_finish_y = var.nDispY;

    dy = (table_finish_y-table_start_y)/(jc+0.5);

    for (i=0; i<jc; i++)
    {
        gdImageFilledRectangle(im,table_start_x,table_start_y+dy*i,table_finish_x,table_start_y+dy*(i+1),color[jc-(i+1)]);
    }

    gdImageRectangle(im,table_start_x,table_start_y,table_finish_x,table_start_y+dy*(jc),color[36]);

    if (var.tmode == 'M')
    {
        r = r * 2;
    }
    else if (var.tmode == 'Y')
    {
        r = r * 3;
    }

    i=1;
    for (j=jc-2; j >= 0; j--)
    {
        rain = color_info.data[j].dbz*r;
        if (r == 100)
            sprintf(text, "%4.0f", rain);
        else
            sprintf(text, "%3.0f", rain);
        err = gdImageStringFT(NULL, &brect[0], 0, f, sz, 0., 0, 0, text);
        err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, table_finish_x+5, table_start_y+dy*(i)+5,text);
        i++;
    }	
    return 0;
}

int level_disp_right2(PARAM_INFO var, gdImagePtr im, int color[], COLOR_INFO color_info)
{
    char  text[126];
    float dy, rain;
    int   i, j, jc;
    int table_start_x,table_start_y;
    int table_finish_x,table_finish_y;
    int brect[8];
    char *err;
    double sz = 11.;
    char f[37];
    float r = 1;

    int x_pos;
    int y_pos;

    gdImageFilledRectangle(im, var.nDispX, 0,var.nDispX + IMAGE_INDEX_RIGHT_SIZE ,var.nDispY  ,color[35]);


    sprintf(f, "%s/gulim.ttc", FONT_PATH); // User supplied font

    if (var.nDispX < 400) sz = 8.;

    jc = color_info.legend_cnt;

    table_start_x  = var.nDispX;
    table_finish_x = var.nDispX+10;
    table_start_y  = 0;
    table_finish_y = var.nDispY+42;

    dy = (table_finish_y-table_start_y)/(jc+0.5);

    for (i=0; i<jc; i++)
    {
        gdImageFilledRectangle(im,table_start_x,table_start_y+dy*i,table_finish_x,table_start_y+dy*(i+1),color[jc-(i+1)]);
    }

    gdImageRectangle(im,table_start_x,table_start_y,table_finish_x,table_start_y+dy*(jc),color[36]);
    

    float *dbz = dbzDay;

    if (var.tmode == 'M')
    {
        //oskim 20180605 , colorbar index modify
        //r = r * 2;
        dbz = dbzMonth;
    }
    else if (var.tmode == 'Y')
    {
        //oskim 20180605 , colorbar index modify
        //r = r * 3;
        dbz = dbzYear;
    }

    i=0;
    for (j=jc-2; j >= 0; j--)
    {
        //oskim 20180605 , colorbar index modify
        //rain = color_info.data[j].dbz*r;
        rain = dbz[j]*r;
        if (r == 100)
            sprintf(text, "%4.0f", rain);
        else
            sprintf(text, "%3.0f", rain);
        x_pos = table_finish_x +5;
        y_pos = dy * i;
        y_pos += dy-gdFontGetSmall()->h/2;

        gdImageString(im, gdFontGetSmall(), x_pos, y_pos, text, color[34]);

        i++;
    }
    return 0;
}


int level_disp_right_popup(PARAM_INFO var, gdImagePtr im, int color[], COLOR_INFO color_info)
{  
    char  text[126];
    float dy, rain;
    int   i, j, jc;
    int table_start_x,table_start_y;
    int table_finish_x,table_finish_y;
    int brect[8];
    char *err;
    double sz = 9.;
    char f[37];	
    float r = 1;

    sprintf(f, "%s/times.ttf", FONT_PATH); // User supplied font

    if(var.nDispX <= 300) sz = 6.;
    else if(var.nDispX > 300 && var.nDispX < 500) sz = 7.;
    else sz = 8.;		

    jc = color_info.legend_cnt;

    table_start_x  = var.nDispX+3;
    table_finish_x = var.nDispX+10;
    table_start_y  = 20;
    table_finish_y = var.nDispY;

    //?珂?화
    gdImageRectangle(im, 0, 0, var.nDispX, var.nDispY, color[34]);	
    gdImageFilledRectangle(im, var.nDispX+1, 0, var.nDispX+var.title_width, var.nDispY, color[35]);

    dy = (table_finish_y-table_start_y)/(jc+0.5);

    strcpy(text, "2");
    err = gdImageStringFT(im, &brect[0], color[34], f, sz-3, 0.0, var.nDispX+28, 11, text);

    strcpy(text, "hits/km");
    err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, var.nDispX+1, 13, text);


    for (i=0; i<jc; i++)
    {
        gdImageFilledRectangle(im,table_start_x,table_start_y+dy*i,table_finish_x,table_start_y+dy*(i+1),color[jc-i-1]);
    }

    gdImageRectangle(im,table_start_x,table_start_y,table_finish_x,table_start_y+dy*(jc),color[36]);

    if (var.tmode == 'M')
    {
        r = r * 2;
    }
    else if (var.tmode == 'Y')
    {
        r = r * 3;
    }

    i=1;
    for (j=(jc-2); j >= 0; j--)	
    {
        rain = color_info.data[j].dbz*r;

        if(r == 100)
            sprintf(text, "%4.0f", rain);
        else
            sprintf(text, "%3.0f", rain);

        err = gdImageStringFT(NULL, &brect[0], 0, f, sz, 0., 0, 0, text);
        err = gdImageStringFT(im, &brect[0], color[34], f, sz, 0.0, table_finish_x+5, table_start_y+dy*(i)+5,text);
        i++;
    }	
    return 0;   
}

short** get_topo_data(PARAM_INFO var, char *topo_map_path)
{
    struct lamc_parameter  map;

    FILE *mapfile; //, *hillfile;	

    float topo_lon = 121.;
    float topo_lat = 42.;	

    char fname[120]; //, fname_hill[120];
    short readValue;
    int colorNo;

    int nrows=0, ncols=0;	
    float xllcorner, yllcorner, cellsize;	

    int int_x, int_y;

    double fval;

    float x, y, lat, lon;
    int	i, j, zoom_level=0;

    int dispX, dispY;
    short **high_data;
    short **topo_data;

    dispX = 1440;
    dispY = 1440;

    //	dispX = 670;
    //	dispY = 670;

    //zooming rate
    zoom_level  = var.nzoomdepth;    

    map.Re	  = 6371.00877f;
    map.slat1 = 30.0f;
    map.slat2 = 60.0f;
    map.olon  = 126.0f;
    map.olat  = 38.0f;
    //map.grid  = 1.f;      //oskim 20190423 , 육지해상 마스킹
    map.grid  = 4.f;//2.f	//작아지면, 면적이 넓어지면서 위로 올라감
    map.xo	  = 400;
    map.yo	  = 800;
    //	map.grid  = 2.f;
    //	map.xo	  = 200;
    //	map.yo	  = 400;
    //   map.xo    = 720;
    //	map.yo    = 920;
    map.first = 0;

    lon = topo_lon;
    lat = topo_lat;

    sprintf(fname, "%s/srtm_1km_sub.bin", topo_map_path);	
    //	sprintf(fname, "/DATA/CONFIG/MAP/F2R_map4.bln");

    //	ncols = 670;
    //	nrows = 670;
    ncols = 1440;
    nrows = 1440;

    //	xllcorner   = 117.000015;
    //	yllcorner   = 30.000004;
    /*  //oskim 20190423 , 육지해상 마스킹 cellsize 
    xllcorner   = 121.000015;
    yllcorner   = 30.000004;
    cellsize    = 0.008133333333;
    */
    xllcorner   = 115.2;
    yllcorner   = 26.2;
    cellsize    = 0.0160;//0.01422;//0.01720;  //이 값은 말그대로 셀사이즈 이고, 전체적 형태를 만든다. (매우 민감한 수치이니 조금씩 변경요) 
	// 작아지면, 면적이 좁아지면서 아래로 내려감

    mapfile = fopen(fname, "rb");

    //?珂?화.		
    high_data = malloc(dispY*sizeof(short *));
    for (i=0; i<dispY; i++) 
    {
        high_data[i] = malloc(dispX*sizeof(short));

        for (j=0; j<dispX; j++) 
            high_data[i][j] = BAD_VAL;
    }

    topo_data = malloc(var.nDispY*sizeof(short *));
    for (i=0; i<var.nDispY; i++)
    {
        topo_data[i] = malloc(var.nDispX*sizeof(short));

        for (j=0; j<var.nDispX; j++)
            topo_data[i][j] = BAD_VAL;
    }

    if(mapfile != NULL)
    {
        lon = xllcorner;
        lat = yllcorner + (cellsize * nrows);

        i = 0;

        while ((fread(&readValue, sizeof(short), 1, mapfile) > 0))
        {
            lamcproj(&lon, &lat, &x, &y, 0, map);

            int_x = (int)(x+.5) + 320;//280;    //커지면 우측이동 (하기 lon 와 같이 조절해서 사용)
            int_y = (int)(y+.5) - 580;//550;    //커지면 아래로 이동 (하기 lat 과 같이 조절해서 사용)

            if ((int_x >= 0 && int_x < dispX) && (int_y >= 0 && int_y < dispY))
                high_data[int_y][int_x] = readValue;

            //oskim 20190423
            //lon = xllcorner + (cellsize * (i % ncols)) * 1.075 ; //1.075
            //lat = yllcorner + (cellsize * nrows ) - (cellsize * (i/ncols))  /1.95 ; //2.00
            lon = xllcorner + (cellsize * (i % ncols)) * .98 ;  //커지면 넓적해지고, 우측으로 이동
            lat = yllcorner + (cellsize * nrows ) - (cellsize * (i/ncols))  / 2.3 ;  //커지면 위로 올라감

            i++;
        }
    }

    fclose(mapfile);

    for (j=0; j<var.nDispY; j++)
    {
        for(i=0; i<var.nDispX; i++)
        {
            //				if(high_data[i][j] > 0 )
            //					topo_data[i][j]  = 300;
            //			  fval = calc_topo_value(var, high_data, j, i);
            topo_data[j][i] = lgt_calc_radar_value_add(var, high_data, j, i);
        }
    }

    for (i=0; i<dispY; i++)
    {
        free(high_data[i]);
    }

    free(high_data);	 

    return topo_data;
}                                                                                     
