package kres.us.site.web;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.URL;
import java.net.URLConnection;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import kres.ad.menu.service.AdMenuMngVO;
import kres.ad.option.service.AdOptionVO;
import kres.cm.util.AboutString;
import kres.cm.util.CmUtilApp;
import kres.us.cmmn.service.UsCmmnService;
import kres.us.comp.service.UsCompRightVO;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.servlet.ModelAndView;

import egovframework.rte.fdl.property.EgovPropertyService;

@Controller
public class UsSiteController {
	
	/** DataPropertyService */
	@Resource(name = "DataPropertiesService")
	protected EgovPropertyService DataPropertiesService;
	
	/** GISPropertiesService */
	@Resource(name = "ImgesPropertiesService")
	protected EgovPropertyService	ImgesPropertiesService;
	
	/** UsMntrService */
	@Resource(name="usCmmnService")
	private UsCmmnService usCmmnService;
	
	@RequestMapping(value="/site/site.do")
	public String site(
			@RequestParam(value="dateTime", required=false) String dateTime,
			@RequestParam(value="site", required=false) String site,
			HttpServletRequest req, HttpServletResponse resq,
			Model model) throws Exception {

		String imgHead = ImgesPropertiesService.getString("CGI_BASE_PATH");
		String imgName = "site_noqc_zoom";
		String params = "";
		
		if(dateTime == null || dateTime.equals("")){
			Calendar cal = Calendar.getInstance();
			SimpleDateFormat format = new SimpleDateFormat("yyyyMMddHHmm");
			dateTime = format.format(cal.getTime()); 
		}
		if(site == null || site.equals("")) site = "GNG"; //default 강릉으로 임시 설정
		
		String path = "";
		// 자료시간
		path = DataPropertiesService.getString("DATA_ROOT_PATH");
		path += DataPropertiesService.getString("SITE_PPI_NOQC_DIR");
		path += DataPropertiesService.getString("SITE_PPI_NOQC_FILE").replace("%S", "GNG");
		dateTime = AboutString.searchFileNearestTime(path, dateTime);
		
		String context = req.getSession().getServletContext().getRealPath("/").replaceAll( "\\\\", "/");
		List<Map<String, Object>> aniImgList = new ArrayList<Map<String,Object>>();
		
		List<Date> aniImgListTmp =  CmUtilApp.createDateList(dateTime, context);
		SimpleDateFormat format = new SimpleDateFormat("yyyyMMddHHmm");
		
		for (int i = 0; i < aniImgListTmp.size(); i++) {
			Map<String, Object> map = new HashMap<String, Object>();
			String aniImgDate =format.format(aniImgListTmp.get(i));

			String snapPath = CmUtilApp.getAddStr(DataPropertiesService.getString("SNAPSHOT_DIR"), "", "", aniImgDate) +
					CmUtilApp.getAddStr(DataPropertiesService.getString("SNAPSHOT_FILE"),"", "", aniImgDate);
			snapPath = snapPath.substring(0, snapPath.length()-5)  + "0.png";
			File file = new File(snapPath); //스넵샷 이미지 있는지 조회
			map.put("fileExist", file.exists());
			
			aniImgDate = CmUtilApp.getAddStr(DataPropertiesService.getString("SNAPSHOT_FILE"),"", "", aniImgDate);
			map.put("aniImgDate", aniImgDate);
			aniImgList.add(map);
		}
		
		model.addAttribute("aniImgList", aniImgList);
		model.addAttribute("imgHead", imgHead);
		model.addAttribute("imgName", imgName);
		model.addAttribute("dateTime", dateTime);
		
		//사이트 타입 가져오기
		List<UsCompRightVO> usSiteSortList = usCmmnService.selectSiteSortList();
		model.addAttribute("usSiteSortList", usSiteSortList);
		
		//사이트 옵션 가져오기

		HashMap<String, Object> map = new HashMap<String, Object>();
		map.put("sort", 0);
		List<UsCompRightVO> obsStationList = usCmmnService.selectObsStation(map);
		model.addAttribute("obsStationList", obsStationList);

		HashMap<String, Object> param = new HashMap<String, Object>();
		param.put("depth", "1");
		List<AdMenuMngVO> depth1MenuList = usCmmnService.selectMenuList(param);
		model.addAttribute("depth1MenuList", depth1MenuList);
		
		List<String> depthList = new ArrayList<String>();
		depthList.add(depth1MenuList.get(1).getNo());

		param.put("depth", "2");
		param.put("pno", depthList);
		List<AdMenuMngVO> depth2MenuList = usCmmnService.selectMenuList(param);
		model.addAttribute("depth2MenuList", depth2MenuList);

		List<String> depth2List = new ArrayList<String>();
		for(int i=0;i<depth2MenuList.size();i++){
			depth2List.add(depth2MenuList.get(i).getNo());
		}
		param.put("depth", "3");
		param.put("pno", depth2List);
		List<AdMenuMngVO> depth3MenuList = usCmmnService.selectMenuList(param);
		model.addAttribute("depth3MenuList", depth3MenuList);
		
		List<AdOptionVO> optionList = usCmmnService.getAllOptionList();
		model.addAttribute("optionList", optionList);
		
		int optionMenuCount = usCmmnService.getOptionMenuCount();
		model.addAttribute("optionMenuCount", optionMenuCount);
		
		return "us/site/siteMain";
	}
	
	@RequestMapping(value="/site/selectCompCgiAjax.do")
	public ModelAndView getSiteCgiAjax(
			@RequestParam(value="dateTime", defaultValue="") String dateTime,
			Model model) throws Exception {
		
		ModelAndView mv = new ModelAndView("jsonView");
		
		String path = "";
		String dataType = "COMP_CAPPI_NOQC";
		
		// 자료시간
		path = DataPropertiesService.getString("DATA_ROOT_PATH");
		path += DataPropertiesService.getString(dataType+"_DIR");
		path += DataPropertiesService.getString(dataType+"_FILE");
		//dateTime = AboutString.searchFileNearestTime(path, dateTime);
		
		// 본사 테스트용
		//String url = "http://192.168.123.23:9090/cgi-bin/radar_comp_cgi_realtime?";
		String url =  ImgesPropertiesService.getString("CGI_BASE_PATH");
		       url+= "/radar_comp_cgi_240km?";
		       url+= "DATE=" + dateTime;
		       
		mv.addObject("compInfo", url);
		
		return mv;
	}

	/**
	 * @Method Name : selectElevationMenuListAjax
	 * @date,  2016. 10. 12.
	 * @author 강혜리
	 * @description 
	 * 	사이트 (PPI, CAPPI) 
	 *	: 파일에서 자료형태/고도각 읽어오기
	 *	: 파일 존재하지 않을경우 default파일에서 읽어온다
	 *
	 *	대기수상체분류 7, 14종
	 *	: cgi url에서 읽어오기 
	 *	: 파일이 존재하지 않을경우 하드코딩
	 * @param req
	 * @param resp
	 * @param site
	 * @param dateTime
	 * @param qtype
	 * @param ptype
	 * @return
	 * @throws Exception ModelAndView
	 */
	@RequestMapping(value="/site/selectElevationSuccessListAjax.do")
	public ModelAndView selectElevationSuccessListAjax(HttpServletRequest req, HttpServletResponse resp,
			@RequestParam(value="site", required=false) String site,
			@RequestParam(value="dateTime", required=false) String dateTime,
			@RequestParam(value="qtype", required=false, defaultValue="") String qtype,
			@RequestParam(value="ptype", required=false, defaultValue="") String ptype
			) throws Exception{
		
		ModelAndView mv = new ModelAndView("jsonView");
		
		List<UsCompRightVO> siteOptList = new ArrayList<UsCompRightVO>();
		
		String elevationPath = "";
		if(ptype.equals("PPI") || ptype.equals("CAPPI") || ptype.equals("480KM")){
			if(!qtype.equals("")){
				if(qtype.equals("NOQC")) elevationPath = "SITE_ELEVATION_NOQC_DIR"; 
				else if(qtype.equals("FUZZYQC")) elevationPath = "SITE_ELEVATION_FUZZYQC_DIR";
				else if(qtype.equals("ORPGQC")) elevationPath = "SITE_ELEVATION_ORPGQC_DIR";
			}else{
				elevationPath = "SITE_ELEVATION_LNG_DIR";
			}
			
			try {
				String path = DataPropertiesService.getString(elevationPath) +
						CmUtilApp.getAddStr(DataPropertiesService.getString("SITE_ELEVATION_FILE"),site, "", dateTime);
				//path = path.replace("%S", site);
				//System.out.println("**************** path : " + path);
				//해당파일존재(고도각 파일)
				File file = new File(path);
				
				if(file.isFile()){
					BufferedReader in =
							new BufferedReader(new FileReader(path)); 
					String s;
					List<String> dtList = new ArrayList<String>();
					UsCompRightVO vo = new UsCompRightVO();
					
					while ((s = in.readLine()) != null) {
						String[] dt = s.split(",");
						dtList.add(dt[0]);
						
						List<String> obj = new ArrayList<String>();
						
						for (int i = 1; i < dt.length; i++) {
							obj.add(dt[i]);
							vo.setSiteElev(obj);
						}
						siteOptList.add(vo);
					}
					List<String> dtListOrd = new ArrayList<String>();
					String[] dataOrd = {"RN","SN","CZ","DZ","VR","SW","DR","RH","PH","KD"};
					for (int j = 0; j < dataOrd.length; j++) {
							if(dtList.contains(dataOrd[j])){
								dtListOrd.add(dataOrd[j]);
							}
					}
					vo.setSiteDType(dtListOrd);
					siteOptList.add(vo);
					in.close();
				}
			} catch (IOException e) {
				System.err.println(e);
				System.exit(1);
			}
		}else if(ptype.equals("ATOM7") || ptype.equals("ATOM14")){
			//파라미터
			//http://~~~~/cgi-bin/write_altitude?SITE_NAME=BRI&DATE=201610190910
			String param = "/cgi-bin/write_altitude?SITE_NAME=" + site +"&DATE=" + dateTime;
			
			//cgi 주소 호출
			String localHost = InetAddress.getLocalHost().toString();
			String cgiUrl = "http://"+localHost.substring(6, localHost.length())+":"+req.getServerPort() + param;
			
			//읽어들이기
			String urlContents = "";
			StringBuilder sb = new StringBuilder();
			
			try {
				BufferedReader buffr = null;
				URL url = null;
				URLConnection urlCon = null;
				InputStreamReader reader = null;
				
				url = new URL(cgiUrl);
				HttpURLConnection conn = (HttpURLConnection)url.openConnection();
				if(conn.getResponseCode() == 200){
					urlCon = (URLConnection)url.openConnection();
					
					reader = new InputStreamReader(urlCon.getInputStream(), "utf-8");
					
					buffr = new BufferedReader(reader);
					
					while ((urlContents = buffr.readLine()) != null) {
						sb.append(urlContents);
					}
					
					buffr.close();
				}
					
			} catch (Exception e) {
				e.printStackTrace();
			}
			String recontents = sb.toString();
			
			// recontents가 빈값(해당 고도각데이터가 존재하지 않는 경우)인경우 기본값을 설정
			// 관리자 옵션쪽에서 default 값 등록되어 있음(0.19,0.48,0.82,1.23,1.75,2.38,3.20)
			
			if(recontents.intern() != "".intern()){
				String[] split = recontents.split(",");
				UsCompRightVO vo = new UsCompRightVO();
				
				List<String> obj = new ArrayList<String>();
				for (int i = 0; i < split.length; i++) {
					obj.add(split[i]);
					vo.setSiteElev(obj);
				}
				
				siteOptList.add(vo);
			}
			
		}
		
		mv.addObject("siteOptList", siteOptList);
		return mv;
	}
	
	
	
	@RequestMapping(value="/site/selectElevationFailListAjax.do")
	public ModelAndView selectElevationFailListAjax(HttpServletRequest req, HttpServletResponse resp,
			@RequestParam(value="site", required=false) String site,
			@RequestParam(value="dateTime", required=false) String dateTime,
			@RequestParam(value="qtype", required=false, defaultValue="") String qtype,
			@RequestParam(value="ptype", required=false, defaultValue="") String ptype
			) throws Exception{
		
		ModelAndView mv = new ModelAndView("jsonView");
		
		List<UsCompRightVO> siteOptList = new ArrayList<UsCompRightVO>();
		
		String elevationPath = "";
		if(ptype.equals("PPI") || ptype.equals("CAPPI") || ptype.equals("480KM")){
			if(!qtype.equals("")){
				if(qtype.equals("NOQC")) elevationPath = "SITE_ELEVATION_NOQC_DIR"; 
				else if(qtype.equals("FUZZYQC")) elevationPath = "SITE_ELEVATION_FUZZYQC_DIR";
				else if(qtype.equals("ORPGQC")) elevationPath = "SITE_ELEVATION_ORPGQC_DIR";
			}else{
				elevationPath = "SITE_ELEVATION_LNG_DIR";
			}
			
			try {
				String path = DataPropertiesService.getString(elevationPath) +
						CmUtilApp.getAddStr(DataPropertiesService.getString("SITE_ELEVATION_FILE"),site, "", dateTime);
				//path = path.replace("%S", site);
				//System.out.println("**************** path : " + path);
				//해당파일존재(고도각 파일)
					String fileEmptyFullName = "";
					if(qtype.equals("NOQC") || qtype.equals("FUZZYQC")){
							fileEmptyFullName = DataPropertiesService.getString("SITE_DEFAULT_PPI_NOQC");
					}else if(qtype.equals("ORPGQC")){
						if(ptype.equals("PPI"))
							fileEmptyFullName = DataPropertiesService.getString("SITE_DEFAULT_PPI_ORPG");
						if(ptype.equals("CAPPI"))
							fileEmptyFullName = DataPropertiesService.getString("SITE_DEFAULT_CAPPI_ORPG");
					}else if(qtype.equals("")){
						fileEmptyFullName = DataPropertiesService.getString("SITE_DEFAULT_480KM_LNG");
					}
					
					
					File defaultFile = new File(fileEmptyFullName);
					
					if(defaultFile.isFile()){
						BufferedReader in =
								new BufferedReader(new FileReader(fileEmptyFullName)); //사이트별 고도각 default 경로
						
						String s;
						while ((s = in.readLine()) != null) {
							String[] split = s.split("\r\n");
							for (int i = 0; i < split.length; i++) {
								//System.out.println(split[i]);
								
								String[] data = split[i].split(":");
								if(data[0].equals(site)){
									UsCompRightVO vo = new UsCompRightVO();	
									List<String> obj = new ArrayList<String>();
									List<String> dtList = new ArrayList<String>();
									String[] dt = data[1].split(",");
									for (int j = 0; j < dt.length; j++) {
										dtList.add(dt[j]);
									}
									List<String> dtListOrd = new ArrayList<String>();
									String[] dataOrd = {"RN","SN","CZ","DZ","VR","SW","DR","RH","PH","KD"};
									for (int j = 0; j < dataOrd.length; j++) {
											if(dtList.contains(dataOrd[j])){
												dtListOrd.add(dataOrd[j]);
											}
									}
									vo.setSiteDType(dtListOrd);
									String[] siteElevData = data[2].split(",");
									for (int j = 0; j < siteElevData.length; j++) {
										obj.add(siteElevData[j]);
										vo.setSiteElev(obj);
									}
									siteOptList.add(vo);
								}
								
							}
							
						}
						in.close();
						
					}
			} catch (IOException e) {
				System.err.println(e);
				System.exit(1);
			}
		}else if(ptype.equals("ATOM7") || ptype.equals("ATOM14")){
			//파라미터
			//http://~~~~/cgi-bin/write_altitude?SITE_NAME=BRI&DATE=201610190910
			String param = "/cgi-bin/write_altitude?SITE_NAME=" + site +"&DATE=" + dateTime;
			
			//cgi 주소 호출
			String localHost = InetAddress.getLocalHost().toString();
			String cgiUrl = "http://"+localHost.substring(6, localHost.length())+":"+req.getServerPort() + param;
			
			//읽어들이기
			String urlContents = "";
			StringBuilder sb = new StringBuilder();
			
			try {
				BufferedReader buffr = null;
				URL url = null;
				URLConnection urlCon = null;
				InputStreamReader reader = null;
				
				url = new URL(cgiUrl);
				HttpURLConnection conn = (HttpURLConnection)url.openConnection();
				if(conn.getResponseCode() == 200){
					urlCon = (URLConnection)url.openConnection();
					
					reader = new InputStreamReader(urlCon.getInputStream(), "utf-8");
					
					buffr = new BufferedReader(reader);
					
					while ((urlContents = buffr.readLine()) != null) {
						sb.append(urlContents);
					}
					
					buffr.close();
				}
					
			} catch (Exception e) {
				e.printStackTrace();
			}
			String recontents = sb.toString();
			
			// recontents가 빈값(해당 고도각데이터가 존재하지 않는 경우)인경우 기본값을 설정
			// 관리자 옵션쪽에서 default 값 등록되어 있음(0.19,0.48,0.82,1.23,1.75,2.38,3.20)
			
			if(recontents.intern() != "".intern()){
				String[] split = recontents.split(",");
				UsCompRightVO vo = new UsCompRightVO();
				
				List<String> obj = new ArrayList<String>();
				for (int i = 0; i < split.length; i++) {
					obj.add(split[i]);
					vo.setSiteElev(obj);
				}
				
				siteOptList.add(vo);
			}
			
		}
		
		mv.addObject("siteOptList", siteOptList);
		return mv;
	}
	
	
	@RequestMapping(value="/site/selectSiteCgiChkAjax.do")
	public ModelAndView selectSiteCgiChkAjax(
			@RequestParam(value="selectSite", required=false) String selectSite,
			HttpServletRequest req,Model model) throws Exception {
		ModelAndView mv = new ModelAndView("jsonView");
		String fileChk = "http://"+req.getServerName()+":"+req.getServerPort() + selectSite.replace("radar_site_gis", "site_map_info")
				.replace("radar_site_480", "site_map_info"); //사이트 480km 추가
		URL file = new URL(fileChk);
		InputStream is = file.openStream();
		InputStreamReader isr =   new InputStreamReader(is);
		BufferedReader in = new BufferedReader(isr);	
		mv.addObject("result", in.readLine());
		return mv;
	}
	
	
	/**
	 * @Method Name : popVad
	 * @date,  2016. 9. 27.
	 * @author 강혜리
	 * @description 
	 *	연직시계열바람 팝업
	 * @param req
	 * @param resp
	 * @param siteName
	 * @param vadDate
	 * @param model
	 * @return
	 * @throws Exception String
	 */
	@RequestMapping(value="/site/popVad.do")
	public String popVad(
			HttpServletRequest req, HttpServletResponse resp,
			@RequestParam(value="vadSiteName", required=false) String vadSiteName,
			@RequestParam(value="vadDate", required=false) String vadDate,
			@RequestParam(value="timeSort", required=false) String timeSort,
			@RequestParam(value="eleResolution", required=false) String eleResolution,
			@RequestParam(value="dateForm", required=false) String dateForm,
			ModelMap model) throws Exception{

		String imgHead = CmUtilApp.getAddStr(DataPropertiesService.getString("SITE_VAD_DIR"),"", "", vadDate); 
		String imgName = "/" + vadSiteName + "_" + eleResolution + "_" + timeSort + "_" + dateForm + ".png";	
		//이미지 src
		String imgSrc = DataPropertiesService.getString("WEB_OUTPUTIMG_PATH") + imgHead + imgName;
		
		boolean fileExists = false;
		//실제 파일 경로
		File file = new File(DataPropertiesService.getString("DATA_ROOT_PATH") +"/OUTPUT/IMG"+ imgHead + imgName);
		
		
		if(file.isFile()) fileExists = true;
		else fileExists = false;
		
		model.addAttribute("imgHead", imgHead);
		model.addAttribute("imgName", imgName);
		model.addAttribute("imgSrc", imgSrc);
		model.addAttribute("fileExists", fileExists);
		model.addAttribute("vadSiteName", vadSiteName);
		model.addAttribute("vadDate", vadDate);
		
		HashMap<String, Object> map = new HashMap<String, Object>();
		map.put("sort", 1);
		List<UsCompRightVO> obsStationList1 = usCmmnService.selectObsStation(map);
		model.addAttribute("obsStationList1", obsStationList1);
		map.put("sort", 2);
		List<UsCompRightVO> obsStationList2 = usCmmnService.selectObsStation(map);
		model.addAttribute("obsStationList2", obsStationList2);
		map.put("sort", 3);
		List<UsCompRightVO> obsStationList3 = usCmmnService.selectObsStation(map);
		model.addAttribute("obsStationList3", obsStationList3);
		map.put("sort", 4);
		List<UsCompRightVO> obsStationList4 = usCmmnService.selectObsStation(map);
		model.addAttribute("obsStationList4", obsStationList4);
		map.put("sort", 5);
		List<UsCompRightVO> obsStationList5 = usCmmnService.selectObsStation(map);
		model.addAttribute("obsStationList5", obsStationList5);
		return "/us/site/popVad";
	}
	
	/**
	 * @Method Name : getVadData
	 * @date,  2016. 9. 27.
	 * @author 강혜리
	 * @description 
	 *	연직시계열바람 gif 이미지 파일 읽어오기
	 * @param req
	 * @param resp
	 * @param siteName
	 * @param dateTime
	 * @param model
	 * @return
	 * @throws Exception ModelAndView
	 */
	@RequestMapping(value="/site/getVadData.do")
	public ModelAndView getVadData(
			HttpServletRequest req, HttpServletResponse resp,
			@RequestParam(value="vadSiteName", required=false) String vadSiteName,
			@RequestParam(value="dateTime", required=false) String dateTime,
			@RequestParam(value="timeSort", required=false) String timeSort,
			@RequestParam(value="eleResolution", required=false) String eleResolution,
			@RequestParam(value="dateForm", required=false) String dateForm,
			ModelMap model) throws Exception{

		String imgHead = CmUtilApp.getAddStr(DataPropertiesService.getString("SITE_VAD_DIR"),"", "", dateTime); 
		String imgName = "/" + vadSiteName + "_" + eleResolution + "_" + timeSort + "_" + dateForm + ".png";	
		//이미지 src
		String imgSrc = DataPropertiesService.getString("WEB_OUTPUTIMG_PATH") + imgHead + imgName;
		
		boolean fileExists = false;
		//실제 파일 경로
		File file = new File(DataPropertiesService.getString("DATA_ROOT_PATH") +"/OUTPUT/IMG"+ imgHead + imgName);

		if(file.isFile()) fileExists = true;
		else fileExists = false;
		
		
		ModelAndView mv = new ModelAndView("jsonView");
		mv.addObject("imgSrc", imgSrc);
		mv.addObject("fileExists", fileExists);
		
		return mv;
		
	}
	
}
