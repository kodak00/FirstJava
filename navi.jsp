<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ page trimDirectiveWhitespaces="true" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="fmt" uri="http://java.sun.com/jsp/jstl/fmt"%>
<style>	

@media only screen and (max-width : 1300px) {
	.topRight {
	  float: Left;
	}
}
/* oskim 20190127
@media only screen and (max-width : 1200px) {
	.leftArea, .rightArea, .nav-tabs {
		top: 70px !important;
	}
}
 */
</style>
<nav class="navbar navbar-default navbar-fixed-top">
	<div class="container-fluid bgGradient">
    	<!-- Brand and toggle get grouped for better mobile display -->
      	<div class="navbar-header">
        	<a class="navbar-brand" href="/comp/comp.do"><img src="/images/us/img/logo-kma.png" alt="레이더분석시스템"></a>
      		<!-- Collect the nav links, forms, and other content for toggling -->
      		<div class="topLeft">
            	<div class="fl margin-left-10" style="margin-top: 9px;">	<!--oskim 20171211 헤더, 풋터 높이 줄임-->	 <!--oskim 20171227 (원복)-->
		    	<fmt:parseDate value="${dateTime}" var="dateFmt" pattern="yyyyMMddHHmm"/>
		      		<div class="btn-group" role="group" >
		            	<label><input type="text" name="dateTimeReal" id="dateTimeReal" autocomplete="off" value="<fmt:formatDate value='${dateFmt}' pattern='yyyy.MM.dd HH:mm'/>" class="calendar" maxlength="16" style="color:black; margin-left:1px;"></label>
		            	<input type="hidden" name="dateTime" id="dateTime" value="<fmt:formatDate value='${dateFmt}' pattern='yyyy.MM.dd HH:mm'/>"/>
		        	</div>
		        	<!--oskim 20171211 헤더, 풋터 높이 줄임-->
		         	<button type="button" class="btn btn-gray btn-xs" style="" type="button" id="bSearch" title="검색"><i class="fa fa-search" aria-hidden="true"></i></button>
		      	</div>
          		<%-- <h1><i class="fa fa-map-o" aria-hidden="true"></i> <span class="color-orange fs12">2016.04.26. 13:00(KST)</span> </h1>--%>

                <div class="fl margin-right-20 margin-left-10"  style="margin-top: 10px;">	<!--oskim 20171211 헤더, 풋터 높이 줄임--> <!--oskim 20171227 (원복)-->
                    <button class="btn btn-primary btn-xs" type="button" id="setTimeNow" onclick="setTimeNow();" title="현재자료시각"><span class="fs11 padding2">Now</span></button>
                    <button class="btn btn-default btn-xs" type="button" id="setTimeB06" onclick="setTime(-60);" title="60분 뒤로"><span class="fs11 padding2">-60m</span></button>
                    <button class="btn btn-default btn-xs" type="button" id="setTimeB03" onclick="setTime(-30);" title="30분 뒤로"><span class="fs11 padding2">-30m</span></button>

										<!-- oskim 20180321 , 첫 로딩화면에서 5분 검색 버튼으로 변경 , 실제 각 주기 변경은 DB 관리자 페이지에서 변경한다. 실 적용 코드는 cmmnCtrl.js 내 checkTimepickerFormat() !!!! -->
										<!-- oskim 20181026 , +-10분 추가-->
										<button class="btn btn-default btn-xs" type="button" id="setTimeB01" onclick="setTime(-10);" title="10분 뒤로"><span class="fs11 padding2">-10m</span></button>
										<button class="btn btn-default btn-xs" type="button" id="setTimeB00" onclick="setTime(-5);" title="5분 뒤로"><span class="fs11 padding2">-5m</span></button>
					          <button class="btn btn-default btn-xs" type="button" id="setTimeA00" onclick="setTime(5);" title="5분 앞으로"><span class="fs11 padding2">+5m</span></button>
					          <button class="btn btn-default btn-xs" type="button" id="setTimeA01" onclick="setTime(10);" title="10분 앞으로"><span class="fs11 padding2">+10m</span></button>

                    <button class="btn btn-default btn-xs" type="button" id="setTimeA03" onclick="setTime(30);" title="30분 앞으로"><span class="fs11 padding2">+30m</span></button>
                    <button class="btn btn-default btn-xs" type="button" id="setTimeA06" onclick="setTime(60);" title="60분 앞으로"><span class="fs11 padding2">+60m</span></button>
                	<button class="btn btn-success btn-xs" type="button" id="autoBtn" title="자동갱신"><span class="fs11 padding2">자동갱신</span></button>
                	<input type="hidden" name="auto_reload" id="auto_reload" value="on" >
                	<input type="hidden" name="ani_play" id="ani_play" value="off" >
                </div>
        	</div>
        	<div class="topRight" style=""> <!--oskim 20171211 헤더, 풋터 높이 줄임, 7개 메뉴 항상 오른쪽 붙도록-->
            	<ul class="nav navbar-nav padding-right-10 pull-right">
                  	<li class="dropdown margin-top-5 margin-right-5 fl">
                  		<div class="cross-off" id="dbz_toggle"> <!--oskim 20171220 클릭시 버튼 올라가는 현상 수정-->
                        	<button class="btn btn-default dropdown-toggle" type="button" style="">강도표출</button> <!--oskim 20171211 헤더, 풋터 높이 줄임-->
                        	<input type="hidden" id="dbz_val" name="dbz_val" value="off">
                        	<input type="hidden" id="dbz_cgi" name="dbz_cgi" value="">
                    	</div>
                	</li>

                  	<li class="dropdown margin-top-5 margin-right-5 fl">
                  		<div class="cross-off" id="zone_toggle"> <!--oskim 20171220 클릭시 버튼 올라가는 현상 수정-->
                        	<button class="btn btn-default dropdown-toggle" type="button" style="">구역동기화</button> <!--oskim 20171211 헤더, 풋터 높이 줄임-->
                        	<input type="hidden" id="zone_val" name="zone_val" value="off">
                    	</div>
                	</li>
            		<li class="dropdown margin-top-5 margin-right-5 fl" style="left:195px;top:34px;background-color: #ffffff; border: 1px #666 solid; padding:7px; text-align:center;width:190px; height:41px;z-index:100;display:none" id="atomCrossText">
                    	<input class="noMenu" type="text" id="azimuthVal" name="rayVal" value="방위각" onFocus="focusOnOff('azimuth', 'on')" onBlur="focusOnOff('azimuth', 'off')" style="width:50px; height:24px; border:1px #ff9c00 solid; text-align:center; font-size:11px;">
                    	<input class="noMenu" type="text" id="startDistVal" name="sdisVal" value="거리" onFocus="focusOnOff('startDist', 'on')" onBlur="focusOnOff('startDist', 'off')" style="width:40px; height:24px; border:1px #ff9c00 solid; text-align:center; font-size:11px;">
                    	~<input class="noMenu" type="text" id="endDistVal" name="edisVal" value="거리" onFocus="focusOnOff('endDist', 'on')" onBlur="focusOnOff('endDist', 'off')" style="width:40px; height:24px; border:1px #ff9c00 solid; text-align:center; font-size:11px;margin-left:3px;">
                    	<i class="fa fa-search" id="crossSearch" style="margin-left:3px;cursor:pointer" aria-hidden="true"></i>
                    </li>
            		<li class="dropdown margin-top-5 margin-right-5 fl">
                  		<div class="cross-off" id="cross_toggle"> <!--oskim 20171220 클릭시 버튼 올라가는 현상 수정-->
                        	<button class="btn btn-default dropdown-toggle" type="button" style="">연직단면</button> <!--oskim 20171211 헤더, 풋터 높이 줄임-->
                    	</div>
                  	</li>

            		<li class="dropdown margin-top-5 margin-right-5 fl" id="setZone_cast">
                    	<div class="dropdown">
                        	<button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" data-submenu="" aria-expanded="true" style="">예보구역 <img src="/images/us/img/ui-icons-bottom.gif" style="margin-top:-3px;" /></button>
                        	<ul class="dropdown-menu area">
                        	<c:forEach items="${sigunList}" var="result" varStatus="status">
                        		<c:if test="${result.zoneSort eq 'CAST' && result.zoneLev1 eq 'Y'}">
	                            	<li class="dropdown-submenu">
	                            		<a tabindex="0" style="cursor:pointer" >${result.zoneNm1 }&nbsp;<img src="/images/us/img/ui-icons-right.gif" style="margin-top:-3px; display:inline-block; padding:0;" /></a>
	                            		<c:set value ="${result.zoneLev2 }" var="zoneVal"/>
		                                <ul class="dropdown-menu" style="position:absolute;left:100%;top:0;width:110%;">
		                                	<c:forEach items="${sigunList}" var="result2" varStatus="status">
		                                	<c:if test="${zoneVal eq result2.zoneLev2 }">
		                                	<li><a tabindex="0" style="cursor:pointer" onclick="distMoveZoom('${result2.zoneLat }','${result2.zoneLon }', 150); return false;">${result2.zoneNm2 }</a></li>
		                                    </c:if>
		                                    </c:forEach>
		                                </ul>
		                          	</li>
		                        </c:if>
                            </c:forEach>
							</ul>
						</div>
				  	</li>
                	<li class="dropdown margin-top-5 margin-right-5 fl" style="display:none;" id="setZone_adms">
                    	<div class="dropdown">
                        	<button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" data-submenu="" aria-expanded="true" style="width:92px;">행정구역 <img src="/images/us/img/ui-icons-bottom.gif" style="margin-top:-3px;" /></button>
                        	<ul class="dropdown-menu area">
                        	<c:forEach items="${sigunList}" var="result" varStatus="status">
                        		<c:if test="${result.zoneSort eq 'ADMS'}">
	                            	<c:if test="${result.zoneLev1 eq 'Y' }">
	                            	<li class="dropdown-submenu">
	                            		<a tabindex="0" style="cursor:pointer" >${result.zoneNm1 }&nbsp;<img src="/images/us/img/ui-icons-right.gif" style="margin-top:-3px; display:inline-block; padding:0;" /></a>
	                            		<c:set value ="${result.zoneLev2 }" var="zoneVal"/>
		                                <c:choose>
		                                <c:when test="${zoneVal == 2 }">
		                                <ul class="dropdown-menu" style="position:absolute;top:0;height:562px;overflow-y:auto;overflow-x:hidden; ">
		                                </c:when>
		                                <c:otherwise>
		                                <ul class="dropdown-menu" style="position:absolute;top:0;height:auto;overflow-y:auto;overflow-x:hidden; ">
		                                </c:otherwise>
		                                </c:choose>
		                                	<c:forEach items="${sigunList}" var="result2" varStatus="status">
		                                	<c:if test="${zoneVal eq result2.zoneLev2 }">
		                                	<li><a tabindex="0" style="cursor:pointer" onclick="distMoveZoom('${result2.zoneLat }','${result2.zoneLon }',150); return false;">${result2.zoneNm2 }</a></li>
		                                    </c:if>
		                                    </c:forEach>
		                                </ul>
		                          	</li>
		                          	</c:if>
	                          	</c:if>
                            </c:forEach>
							</ul>
						</div>
				  	</li>
				  	<li class="dropdown margin-top-5 margin-right-5 fl">
                  		<div class="dropdown" id="gisDropdown">
                        	<button id="gisDropdownBtn" class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" aria-expanded="true" style="" title="GIS옵션">GIS <img src="/images/us/img/ui-icons-bottom.gif" style="margin-top:-3px;" /></button> <!--oskim 20171211 헤더, 풋터 높이 줄임-->
                        	<ul class="dropdown-menu" role="menu">
                        		<li><a class="checkbox"><label><input type="checkbox" value="" id="stl" name="gisDropMenu">위성</label></a></li>
                        		<li><a class="checkbox"><label><input type="checkbox" value="" id="gid" name="gisDropMenu">위경도</label></a></li>
                        		<li><a class="checkbox"><label><input type="checkbox" value="" id="sri" name="gisDropMenu">음영기복도</label></a></li>
                          		<li><a class="checkbox"><label><input type="checkbox" value="" id="aws" name="gisDropMenu" checked>AWS지점</label></a></li>
	                          	<li><a class="checkbox"><label><input type="checkbox" value="" id="fir" name="gisDropMenu">비행정보구역</label></a></li>
	                          	<li><a class="checkbox"><label><input type="checkbox" value="" id="typ" name="gisDropMenu">태풍경계구역</label></a></li>
	                          	<li><a class="checkbox"><label><input type="checkbox" value="" id="air" name="gisDropMenu">주요공항지점</label></a></li>
	                          	<li><a class="checkbox"><label><input type="checkbox" value="" id="riv-chk" name="gisDropMenu">하천정보</label></a></li>
	                          	<li><a class="checkbox"><label><input type="checkbox" value="" id="nationRiv" name="gisDropMenu" style="margin-left: 0; position:relative;">국가하천</label></a></li>
                                <li><a class="checkbox"><label><input type="checkbox" value="" id="localRiv" name="gisDropMenu" style="margin-left: 0; position:relative;">지방하천</label></a></li>
	                          	<li><a class="checkbox"><label><input type="checkbox" value="" id="rod-chk" name="gisDropMenu">도로정보</label></a></li>
	                          	<li><a class="checkbox"><label><input type="checkbox" value="" id="express" name="gisDropMenu" style="margin-left: 0; position:relative;">고속국도</label></a></li>
                                <li><a class="checkbox"><label><input type="checkbox" value="" id="general" name="gisDropMenu" style="margin-left: 0; position:relative;">일반국도</label></a></li>
                                <li><a class="checkbox"><label><input type="checkbox" value="" id="local" name="gisDropMenu" style="margin-left: 0; position:relative;">지방도</label></a></li>
	                          	<!-- <li><a class="checkbox"><label><input type="checkbox" value="" id="rod" name="gisDropMenu">도로정보</label></a></li> -->
                                <!-- oskim 20180903 , 20181219 set to default SidoLay -->
                                <li><a class="checkbox"><label><input type="checkbox" value="" id="grp-chk" name="gisDropMenu" checked>행정구역</label></a></li>
                                <li><a class="checkbox"><label><input type="radio" value="" id="emd" name="gisDropMenu" >경계+명칭</label></a></li>
                                <li><a class="checkbox"><label><input type="radio" value="" id="txt" name="gisDropMenu" checked>경계</label></a></li>
                        	</ul>
                    	</div>
                  	</li>
                  	<li class="dropdown margin-top-5 margin-right-5 fl">
                    	<div class="dropdown menu-close" id="toolDropdown">
                        	<button id="toolDropdownBtn" class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" data-submenu="" aria-expanded="true" style="" title="GIS도구" >	<!--oskim 20171211 헤더, 풋터 높이 줄임-->
                            	<i class="fa fa-pencil" aria-hidden="true"></i> <img src="/images/us/img/ui-icons-bottom.gif" style="margin-top:-3px;" />
                          	</button>
                          	<ul id="toolDropdownMenu" class="dropdown-menu" style="width:50px;">
                            	<li class="dropdown-submenu">
                                	<a href="javascript:;" class="img" onclick="" id="measureBtn"><img src="/images/us/img/ui-icons-left.gif" style="margin-top:-1px; display:inline-block;" /><img src="/images/us/img/icn-ruler.gif" alt="거리재기" style="display:inline-block;" title="거리재기"></a>
                                  	<ul class="dropdown-menu cross">
                                    	<li style="position:absolute;left:-2px;top:0; background-color:#fff; border:1px #666 solid;width:39px;"><a href="#" tabindex="0" onclick="measure_remove_distance(); return false;"><img src="/images/us/img/icn-eraser.gif" alt="지우개" title="지우개"></a></li>
                                  	</ul>
                              	</li>
                              	<li><a href="#" class="img noMenu" onclick="save_evt_mClick('save'); return false;"><img src="/images/us/img/icn-imgdn.gif" alt="지도저장" title="지도저장"></a></li>
                      		  	<li><a href="#" class="img noMenu" onclick="save_evt_mClick('print'); return false;"><img src="/images/us/img/icn-print.gif" alt="지도인쇄" title="지도인쇄"></a></li>
                              	<li class="dropdown-submenu">
                                	<a href="#" class="img" id="drawBtn"><img src="/images/us/img/ui-icons-left.gif" style="margin-top:-1px; display:inline-block;" /><img src="/images/us/img/icn-pencel.gif" alt="그리기" title="그리기" style="display:inline-block;"></a>
                                  	<ul class="dropdown-menu cross">
                                    	<li style="position:absolute;left:-114px;top:0; background-color:#fff; border:1px #666 solid;width:39px;"><a tabindex="0" id="polygon" onclick="drawing('Polygon'); return false;"><img src="/images/us/img/icn-poly.gif" alt="다각형" title="다각형"></a></li>
                                    	<li style="position:absolute;left:-76px;top:0; background-color:#fff; border:1px #666 solid;width:39px;"><a tabindex="0" id="polygon" onclick="drawing('Square'); return false;"><img src="/images/us/img/icn-squre.gif" alt="사각형" title="사각형"></a></li>
                                    	<li style="position:absolute;left:-38px;top:0; background-color:#fff; border:1px #666 solid;width:39px;"><a tabindex="0" id="circle" onclick="drawing('Circle'); return false;"><img src="/images/us/img/icn-circle.gif" alt="원" title="원"></a></li>
                                    	<li style="position:absolute;left:0px;top:0; background-color:#fff; border:1px #666 solid;width:39px;"><a tabindex="0" id="lineString" onclick="drawing('LineString'); return false;"><img src="/images/us/img/icn-line.gif" alt="선" title="선"></a></li>
                                    	
                                    	<li style="position:absolute;left:-148px;top:0; background-color:#fff; border:1px #666 solid; width:39px; height:41px;"><a tabindex="0" onclick="drawing('Arrow'); return false;"><img src="/images/us/img/icn-arrow.gif" alt="화살표" title="화살표"></a></li>
                                    	<li style="position:absolute;left:-186px;top:0; background-color:#fff; border:1px #666 solid; width:39px; height:41px;"><a tabindex="0" onclick="drawing('None'); return false;"><img src="/images/us/img/icn-ctlline.gif" alt="곡선" title="곡선"></a></li>
                                    	<li style="position:absolute;left:-224px;top:0; background-color:#fff; border:1px #666 solid; width:39px; height:41px;"><a tabindex="0" onclick="drawing('None'); return false;"><img src="/images/us/img/icn-freeline.gif" alt="자유선" title="자유선"></a></li>
                                    	
                                    	<li style="position:absolute;left:-262px;top:0; background-color:#fff; border:1px #666 solid; width:39px; height:41px;"><a tabindex="0" onclick="drawing('None'); return false;"><img src="/images/us/img/icn-eraser.gif" alt="지우개" title="지우개"></a></li>
                                    	
                                    	<li class="dropdown-submenu" style="position:absolute;left:-308px;top:0; background-color:#fff; border:1px #666 solid; width:39px; height:41px;"><a tabindex="0" ><img src="/images/us/img/icn-line0.gif" alt="선모양" title="선모양"></a>
											<!-- <button class="btn btn-default btn-xs" type="button" title="메뉴"><img src="/images/us/img/smenu.gif" style="margin-top:-3px;" /></button> -->
											<ul class="dropdown-menu cross">
											<li style="position:absolute;left:40px;top:45px; background-color:#fff; border:1px #666 solid;width:39px;"><a tabindex="0" id="dashline0" ><img src="/images/us/img/icn-line1.gif" alt="직선" title="직선"></a></li>
											<li style="position:absolute;left:40px;top:85px; background-color:#fff; border:1px #666 solid;width:39px;"><a tabindex="0" id="dashline1" onclick="drawing('Polygon'); return false;"><img src="/images/us/img/icn-line2.gif" alt="점선1" title="점선1"></a></li>
											<li style="position:absolute;left:40px;top:125px; background-color:#fff; border:1px #666 solid;width:39px;"><a tabindex="0" id="dashline2" onclick="drawing('Polygon'); return false;"><img src="/images/us/img/icn-line3.gif" alt="점선2" title="점선2"></a></li>
											</ul>
										</li>
                                  	</ul>
                              	</li>
                           	</ul>
                      	</div>
				  	</li>
                    <!-- oskim 20180727 -->
                    <li class="dropdown margin-top-5 fl">
                        <button class="btn btn-default btn-xs" type="button" id="smenu" onclick="clickSMenu();" title="메뉴"><img src="/images/us/img/smenu.gif" style="margin-top:-3px;" /></button>
                    </li>
             	</ul>
         	</div>
      	</div>
      	<!-- /.navbar-collapse-->
    </div>
    <!-- /.container-fluid -->
 </nav>
 <form id="crossValue" name="crossValue" method="post" action="">
	 <input type="hidden" name="startPointX" id="startPointX">
	 <input type="hidden" name="startPointY" id="startPointY">
	 <input type="hidden" name="endPointX" id="endPointX">
	 <input type="hidden" name="endPointY" id="endPointY">
	 <input type="hidden" name="date" id="date">
	 <input type="hidden" name="dType" id="dType">
	 <input type="hidden" name="pType" id="pType">
	 <input type="hidden" name="qType" id="qType">
	 <input type="hidden" name="tType" id="tType">
	 <input type="hidden" name="siteName" id="siteName">
	 <input type="hidden" name="dTypeList" id="dTypeList">
	 <input type="hidden" name="azimuth" id="azimuth">
	 <input type="hidden" name="startDist" id="startDist">
	 <input type="hidden" name="endDist" id="endDist">
	 <input type="hidden" name="crossLat" id="crossLat">
	 <input type="hidden" name="crossLon" id="crossLon">
	 <input type="hidden" name="vWind" id="vWind">
	 <input type="hidden" name="mWind" id="mWind">
	 <input type="hidden" name="lNum" id="lNum" value="0" />
	 <input type="hidden" name="pNum" id="pNum" value="0" />
	 <input type="hidden" name="uiTime" id="uiTime" value="10" />
 </form>

 <input type="hidden" name="imgHeader" id="imgHeader" value="${imgHeader}">
 <input type="hidden" name="echoCgiName" id="echoCgiName" value="">
 <input type="hidden" name="siteElevationVal" id="siteElevationVal" value="0">
 <input type="hidden" name="urlCnt" id="urlCnt" value="0">
