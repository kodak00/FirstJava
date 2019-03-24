home = /srv/kres/rdr/
www  = $(home)www/
src  = $(home)src/
lib  = $(home)src/lib/
bin  = $(home)www/cgi-bin/rdr/

lib_dir = $(www)cgi-src/lib/
cgi_dir = $(www)cgi-src/lib/
aws_dir = $(www)cgi-src/lib/
map_dir = $(src)lib/map/

cgi_lib  = $(lib)cgiutil.c
nr_lib   = $(lib)nrutil.c
time_lib = $(lib)seq_time.c
stn_lib  = $(lib)stn_inf_read.c
aws_lib  = $(lib)aws_io.c
aws2_lib = $(lib)aws2_io.c
aws3_lib = $(lib)aws3_io.c
vis2_lib = $(lib)vis2_io.c
url_lib  = $(lib)url_io.c
zoom_lib = $(lib_dir)grid_zooming.c
map_lib  = $(map_dir)lamcproj.c
map1_lib = $(map_dir)albeproj.c $(map_dir)lamcproj.c $(map_dir)azedproj.c $(map_dir)sterproj.c
gd_lib   = $(cgi_dir)gd_map.c
cmn_lib  = $(cgi_dir)gd_util.c $(cgi_dir)disp_htm.c
rdr_acc  = $(src)rdr/rdr_cmp_acc.c
rdr_nc_lib  = $(lib)/rdr_nc_lib.c
gz_lib = $(www)/include/rdr_common/build/lib/libz.a

grid_inp      = $(cgi_dir)grid_inp_get.c
grid_disp_img = $(cgi_dir)grid_disp_img.c
grid_map_inf  = $(cgi_dir)grid_map_inf.c
grid_stn      = $(cgi_dir)grid_stn.c
grid_lib  = $(cgi_dir)grid_inp_get.c $(cgi_dir)grid_disp_html.c $(cgi_dir)grid_disp_img.c $(cgi_dir)grid_img.c $(cgi_dir)grid_making.c $(cgi_dir)grid_zooming.c $(cgi_dir)grid_stn.c $(cgi_dir)grid_map_img.c $(cgi_dir)grid_map_inf.c $(cgi_dir)grid_color.c

INCLUDEDIRS=-I. -I/usr/local/trmm/include -I$(src)include -I$(www)cgi-src/include -I$(src)lib/map -I/DATA/LIB/RSL-WRC/include -I$(www)/include/rdr_common/build/include
LIBDIRS=-L. -L/usr/lib64 -L/usr/local/trmm/lib -L$(lib)map -L/DATA/LIB/RSL-WRC/lib -L$(www)/include/rdr_common/build/lib
LIBS=-lm -lgd -lpng -lz -lnetcdf 
CFLAGS=-g

# 레이더기반 알람
rdr_alarm: rdr_alarm.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_alarm  rdr_alarm.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 지상기준 눈비분석
rdr_sfc_pty_img: rdr_sfc_pty_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_sfc_pty_img  rdr_sfc_pty_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 3차원 관측에 대한 지상기준 검증
rdr_sfc_obs_chk: rdr_sfc_obs_chk.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_sfc_obs_chk  rdr_sfc_obs_chk.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl	

# 레이더 WISSDOM 결과 이미지 표출
rdr_wis_ana_img: rdr_wis_ana_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_wis_ana_img  rdr_wis_ana_img.c $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(gz_lib) -lm -lgd -lpng -lcurl

# 레이더 3차원 관측 이미지 표출
rdr_r3d_obs_img: rdr_r3d_obs_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_r3d_obs_img  rdr_r3d_obs_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 3차원 기온 이미지 표출
rdr_r3d_ta_img: rdr_r3d_ta_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_r3d_ta_img  rdr_r3d_ta_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 3차원 합성자료 표출
rdr_r3d_img: rdr_r3d_img.c rdr_r3d_data.c rdr_r3d_aws.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_r3d_img  rdr_r3d_img.c rdr_r3d_data.c rdr_r3d_aws.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 3차원 합성 단면도 표출
rdr_r3d_rhi_img: rdr_r3d_rhi_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_r3d_rhi_img  rdr_r3d_rhi_img.c $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 대기 3차원 실황 단면도 표출
rdr_obs_rhi_img: rdr_obs_rhi_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_obs_rhi_img  rdr_obs_rhi_img.c $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 3차원에 사용된 기온,바람 관측자료 분포 표출
rdr_r3d_obs_map_img: rdr_r3d_obs_map_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_r3d_obs_map_img  rdr_r3d_obs_map_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 3차원 합성을 위한 검토자료 표출
rdr_cmp3_grid_map_img1: rdr_cmp3_grid_map_img1.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp3_grid_map_img1  rdr_cmp3_grid_map_img1.c $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 합성자료 QPF
rdr_qpf1_img: rdr_qpf1_img.c rdr_qpf1_data.c rdr_qpf1_aws.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_qpf1_img  rdr_qpf1_img.c rdr_qpf1_data.c rdr_qpf1_aws.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 지점별자료 표출
rdr_stn1_img: rdr_stn1_img.c rdr_stn1_data.c rdr_stn1_aws.c $(rdr_nc_lib)
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_stn1_img  $(CFLAGS) rdr_stn1_img.c rdr_stn1_data.c rdr_stn1_aws.c $(rdr_nc_lib) $(aws3_lib) $(map1_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(gz_lib) $(LIBDIRS) -lrsl_wrc -lm -lgd -lpng -lcurl -lnetcdf -lhdf5 -lhdf5_hl
# 레이더 지점별 RHI단면도 표출
rdr_stn1_rhi_img: rdr_stn1_rhi_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_stn1_rhi_img  rdr_stn1_rhi_img.c $(map1_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(LIBDIRS) -lrsl_wrc -lm -lgd -lpng -lz -lcurl

# 레이더 지점별 3차원대기의 RHI단면도 표출
rdr_stn1_rhi_obs_img: rdr_stn1_rhi_obs_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_stn1_rhi_obs_img  rdr_stn1_rhi_obs_img.c $(map1_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(LIBDIRS) -lrsl_wrc -lm -lgd -lpng -lz -lcurl

# 레이더 지점별로 합성영역에 표출
rdr_stn2_img: rdr_stn2_img.c rdr_stn2_data.c rdr_stn2_aws.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_stn2_img  rdr_stn2_img.c rdr_stn2_data.c rdr_stn2_aws.c $(aws3_lib) $(map1_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(LIBDIRS) -lrsl_wrc -lm -lgd -lpng -lz -lcurl

# 레이더 합성자료 제공용 URL-API
rdr_cmp1_api: rdr_cmp1_api.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp1_api  rdr_cmp1_api.c $(time_lib) $(cgi_lib) $(nr_lib) -lm -lz

rdr_cmp1_api_pub: rdr_cmp1_api_pub.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp1_api_pub  rdr_cmp1_api_pub.c $(time_lib) $(cgi_lib) $(nr_lib) -lm -lz

rdr_cmp1_test: rdr_cmp1_test.c 
	gcc $(INCLUDEDIRS) -o rdr_cmp1_test  rdr_cmp1_test.c $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 합성자료 종합표출
rdr_cmp1_img: rdr_cmp1_img.c rdr_cmp1_data.c rdr_cmp1_aws.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp1_img  rdr_cmp1_img.c rdr_cmp1_data.c rdr_cmp1_aws.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

rdr_cmp9_img: rdr_cmp9_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp9_img  rdr_cmp9_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 누적합성도
rdr_cmp_acc_img: rdr_cmp_acc_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_acc_img  rdr_cmp_acc_img.c $(rdr_acc) $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

rdr_cmp_pcp_img: 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_pcp_img  rdr_cmp_pcp_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 HCI 합성도
rdr_cmp_hci_img: rdr_cmp_hci_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_hci_img  rdr_cmp_hci_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 강수유무 합성도
rdr_cmp_num_img: rdr_cmp_num_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_num_img  rdr_cmp_num_img.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 강우유무 검증용
rdr_cmp_num_data: rdr_cmp_num_data.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_num_data  rdr_cmp_num_data.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 합성자료와 AWS 관측 비교
rdr_cmp_aws_chk: rdr_cmp_aws_chk.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_aws_chk  rdr_cmp_aws_chk.c $(rdr_acc) $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

rdr_pcp_aws_chk: rdr_pcp_aws_chk.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_pcp_aws_chk  rdr_pcp_aws_chk.c $(aws3_lib) $(map_lib) $(url_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

#  레이더 NC/UF 파일 정보 표출 (통합)
rdr_file_inf: rdr_file_inf.c $(rdr_nc_lib)
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_file_inf  rdr_file_inf.c $(rdr_nc_lib) $(time_lib) $(cgi_lib) $(gz_lib) $(LIBDIRS) -lrsl_wrc -lm -lcurl -lnetcdf -lhdf5 -lhdf5_hl

#  레이더 NC/UF 파일에서 자료 단계수 표출
rdr_file_inf_lvl: rdr_file_inf_lvl.c $(rdr_nc_lib)
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_file_inf_lvl  rdr_file_inf_lvl.c $(rdr_nc_lib) $(time_lib) $(cgi_lib) $(gz_lib) $(LIBDIRS) -lrsl_wrc -lm -lcurl -lnetcdf -lhdf5 -lhdf5_hl

#  레이더 NC 파일 정보 표출 (_inf에 통합)
rdr_file_nc: rdr_file_nc.c $(rdr_nc_lib)
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_file_nc  rdr_file_nc.c $(rdr_nc_lib) $(time_lib) $(cgi_lib) $(gz_lib) $(LIBDIRS) -lrsl_wrc -lm -lcurl -lnetcdf -lhdf5 -lhdf5_hl

#  레이더 UF 파일 정보 표출 (_inf에 통합)
rdr_file_uf: rdr_file_uf.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_file_uf  rdr_file_uf.c $(time_lib) $(cgi_lib) $(gz_lib) $(LIBDIRS) -lrsl_wrc -lm 

# 레이더 합성자료 헤더정보 표출
rdr_cmp_inf: rdr_cmp_inf.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_inf  rdr_cmp_inf.c $(time_lib) $(cgi_lib) $(nr_lib) -lm -lz -lcurl

# 레이더 COMIS 합성도
rdr_comis_img: rdr_comis_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_comis_img  rdr_comis_img.c $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 지형고도 이미지
rdr_topo_img: rdr_topo_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_topo_img  rdr_topo_img.c $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(url_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 합성도
rdr_cmp_img: rdr_cmp_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_img  rdr_cmp_img.c $(aws2_lib) $(stn_lib) $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 사이트자료 합성영역으로 변환한 이미지
rdr_cmp_stn_img: rdr_cmp_stn_img.c rdr_cmp_stn_data.c
#rdr_cmp_stn_img: rdr_cmp_stn_img.c 
#	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_stn_img  rdr_cmp_stn_img.c rdr_cmp_stn_data.c $(stn_lib) $(map1_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(LIBDIRS) -lrsl -lm -lgd -lpng -lz -lcurl
	cc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_stn_img  rdr_cmp_stn_img.c rdr_cmp_stn_data.c $(stn_lib) $(map1_lib) $(time_lib) $(cgi_lib) $(nr_lib) $(LIBDIRS) -lrsl_wrc -lm -lgd -lpng -lz -lcurl

# QPE 강우강도 표출
qpe_ram_int_img: qpe_ram_int_img.c qpe_ram_int_img_rdr.c qpe_ram_int_img_aws.c qpe_ram_int_img_obj.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-qpe_ram_int_img  qpe_ram_int_img.c qpe_ram_int_img_rdr.c qpe_ram_int_img_aws.c qpe_ram_int_img_obj.c $(aws2_lib) $(stn_lib) $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# QPE 1시간 강수 표출
qpe_ram_acc_img: qpe_ram_acc_img.c qpe_ram_acc_img_rdr.c qpe_ram_acc_img_aws.c qpe_ram_acc_img_obj.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-qpe_ram_acc_img  qpe_ram_acc_img.c qpe_ram_acc_img_rdr.c qpe_ram_acc_img_aws.c qpe_ram_acc_img_obj.c $(aws2_lib) $(stn_lib) $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# QPE 10분 누적
qpe_ram_10m_sum: qpe_ram_10m_sum.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-qpe_ram_10m_sum  qpe_ram_10m_sum.c $(stn_lib) $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# QPE 60분 누적
qpe_ram_60m_sum: qpe_ram_60m_sum.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-qpe_ram_60m_sum  qpe_ram_60m_sum.c $(stn_lib) $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl



# 레이더 합성 - 1시간 강수량
rdr_cmp_rn1h_img: rdr_cmp_rn1h_img.c 
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_cmp_rn1h_img  rdr_cmp_rn1h_img.c $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 레이더 MAPLE 예상도
rdr_maple: rdr_maple.c rdr_maple_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_maple  rdr_maple.c rdr_maple_img.c $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 초단기예보용 레이더 MAPLE 예상도
rdr_afs_maple_img: rdr_afs_maple_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_afs_maple_img  rdr_afs_maple_img.c $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 초단기예보용 레이더 MAPLE 1시간 강수량 예상도
rdr_afs_maple_rn1h_img: rdr_afs_maple_rn1h_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_afs_maple_rn1h_img  rdr_afs_maple_rn1h_img.c $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

# 초단기예보용 레이더 MAPLE 이동벡터
rdr_afs_maple_mvec_img: rdr_afs_maple_mvec_img.c
	gcc $(INCLUDEDIRS) -o $(bin)nph-rdr_afs_maple_mvec_img  rdr_afs_maple_mvec_img.c $(map_lib) $(time_lib) $(cgi_lib) $(nr_lib) -lm -lgd -lpng -lz -lcurl

