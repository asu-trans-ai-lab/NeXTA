	// TLiteView.cpp : implementation of the CTLiteView class
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NeXTA Version 0.9.12022020 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "TLite.h"
#include "math.h"

#include "TLiteDoc.h"
#include "TLiteView.h"
#include "MainFrm.h"
#include "DlgMOE.h"
#include "Network.h"
#include "DlgFindALink.h"
#include "DlgLinkProperties.h"
#include "Dlg_NodeProperties.h"
#include "Dlg_VehicleClassification.h"
#include "Page_Node_Movement.h"
#include "Page_Node_Phase.h"
#include "CSVParser.h"

#include "Chart_Control\\NSChartCtrl.h"

#include "Dlg_DisplayConfiguration.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GDPoint g_Origin;
float g_Resolution;

_cursor_type g_current_cursor_type = _cursor_standard_arrow;


bool compare_IncomingLinkAngleData (IncomingLinkAngle first, IncomingLinkAngle second)
{
	if(first.Angle < second.Angle) return true;
	else 
		return false;
}

void g_SetCursor(_cursor_type cursor_type)
{
	//if(cursor_type == _cursor_movement_network && g_current_cursor_type!=_cursor_movement_network)
	//{	
	//	SetCursor(AfxGetApp()->LoadCursor(IDC_MOVENETWORK));
	//	return;
	//}

	if(cursor_type == _cursor_create_node && g_current_cursor_type!=_cursor_create_node)
	{	
		SetCursor(AfxGetApp()->LoadCursor(IDC_CREATE_NODE_CURSOR));
		return;
	}
	if(cursor_type == _cursor_create_link && g_current_cursor_type!=_cursor_create_link)
	{	
		SetCursor(AfxGetApp()->LoadCursor(IDC_CREATE_LINK_CURSOR));
		return;
	}
	if(cursor_type == _cursor_create_subarea && g_current_cursor_type!= _cursor_create_subarea)
	{	
		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_SUBAREA));
		return;
	}

	if(cursor_type == _cursor_create_zone && g_current_cursor_type!= _cursor_create_zone)
	{	
		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ZONE));
		return;
	}

	if(cursor_type == _cursor_standard_arrow && g_current_cursor_type!=_cursor_standard_arrow)
	{	
		SetCursor(AfxGetApp()->LoadCursor(IDC_ARROW));
		return;
	}
	if(cursor_type == _cursor_standard_arrow && g_current_cursor_type!=_cursor_standard_arrow)
	{	
		SetCursor(AfxGetApp()->LoadCursor(IDC_ARROW));
		return;
	}

	if(cursor_type == _cursor_wait && g_current_cursor_type!=_cursor_standard_arrow)
	{	
		SetCursor(AfxGetApp()->LoadCursor(IDC_WAIT));
		return;
	}

};

extern COLORREF g_MOEDisplayColor[MAX_MOE_DISPLAYCOLOR];
extern float g_Simulation_Time_Stamp;
extern bool g_LinkMOEDlgShowFlag;
int g_ViewID = 0;
IMPLEMENT_DYNCREATE(CTLiteView, CView)

BEGIN_MESSAGE_MAP(CTLiteView, CView)

	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()

	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_ZOOMIN, &CTLiteView::OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, &CTLiteView::OnViewZoomout)
	ON_COMMAND(ID_VIEW_SHOWNETWORK, &CTLiteView::OnViewShownetwork)
	ON_COMMAND(ID_VIEW_MOVE, &CTLiteView::OnViewMove)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVE, &CTLiteView::OnUpdateViewMove)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_VIEW_SELECT, &CTLiteView::OnViewSelect)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECT, &CTLiteView::OnUpdateViewSelect)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_NODE_ORIGIN, &CTLiteView::OnNodeOrigin)
	ON_COMMAND(ID_NODE_DESTINATION, &CTLiteView::OnNodeDestination)
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_SEARCH_FINDLINK, &CTLiteView::OnSearchFindlink)
	ON_COMMAND(ID_VIEW_IMAGE, &CTLiteView::OnEditChangebackgroupimagethroughmouse)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IMAGE, &CTLiteView::OnUpdateEditChangebackgroupimagethroughmouse)
	ON_COMMAND(ID_VIEW_BACKGROUNDIMAGE, &CTLiteView::OnViewBackgroundimage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACKGROUNDIMAGE, &CTLiteView::OnUpdateViewBackgroundimage)
	ON_COMMAND(ID_SHOW_SHOWALLPATHS, &CTLiteView::OnShowShowallpaths)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SHOWALLPATHS, &CTLiteView::OnUpdateShowShowallpaths)
	ON_COMMAND(ID_Edit_Create1WayLink, &CTLiteView::OnEditCreate1waylink)
	ON_COMMAND(ID_Edit_Create2WayLinks, &CTLiteView::OnEditCreate2waylinks)
	ON_UPDATE_COMMAND_UI(ID_Edit_Create1WayLink, &CTLiteView::OnUpdateEditCreate1waylink)
	ON_UPDATE_COMMAND_UI(ID_Edit_Create2WayLinks, &CTLiteView::OnUpdateEditCreate2waylinks)
	ON_COMMAND(ID_SHOW_GRID, &CTLiteView::OnShowGrid)
	ON_UPDATE_COMMAND_UI(ID_SHOW_GRID, &CTLiteView::OnUpdateShowGrid)
	ON_COMMAND(ID_SHOW_LINKARROW, &CTLiteView::OnShowLinkarrow)
	ON_UPDATE_COMMAND_UI(ID_SHOW_LINKARROW, &CTLiteView::OnUpdateShowLinkarrow)
	ON_COMMAND(ID_IMAGE_LOCKBACKGROUNDIMAGEPOSITION, &CTLiteView::OnImageLockbackgroundimageposition)
	ON_COMMAND(ID_VIEW_TEXTLABEL, &CTLiteView::OnViewTextlabel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTLABEL, &CTLiteView::OnUpdateViewTextlabel)
	ON_COMMAND(ID_LINK_DELETE, &CTLiteView::OnLinkDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETESELECTEDLINK, &CTLiteView::OnUpdateEditDeleteselectedlink)
	ON_COMMAND(ID_LINK_EDITLINK, &CTLiteView::OnLinkEditlink)
	ON_COMMAND(ID_EDIT_CREATENODE, &CTLiteView::OnEditCreatenode)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CREATENODE, &CTLiteView::OnUpdateEditCreatenode)
	ON_COMMAND(ID_EDIT_DELETESELECTEDNODE, &CTLiteView::OnEditDeleteselectednode)
	ON_COMMAND(ID_VIEW_SELECT_NODE, &CTLiteView::OnViewSelectNode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECT_NODE, &CTLiteView::OnUpdateViewSelectNode)
	ON_COMMAND(ID_EDIT_CREATESUBAREA, &CTLiteView::OnEditCreatesubarea)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CREATESUBAREA, &CTLiteView::OnUpdateEditCreatesubarea)
	ON_COMMAND(ID_TOOLS_REMOVENODESANDLINKSOUTSIDESUBAREA, &CTLiteView::OnToolsRemovenodesandlinksoutsidesubarea)
	ON_COMMAND(ID_EDIT_DELETELINKSOUTSIDESUBAREA, &CTLiteView::OnEditDeletelinksoutsidesubarea)
	ON_COMMAND(ID_EDIT_MOVENETWORKCOORDINATES, &CTLiteView::OnEditMovenetworkcoordinates)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVENETWORKCOORDINATES, &CTLiteView::OnUpdateEditMovenetworkcoordinates)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_VIEW_INCREASENODESIZE, &CTLiteView::OnViewIncreasenodesize)
	ON_COMMAND(ID_VIEW_DECREATENODESIZE, &CTLiteView::OnViewDecreatenodesize)
	ON_COMMAND(ID_NODE_CHECKCONNECTIVITYFROMHERE, &CTLiteView::OnNodeCheckconnectivityfromhere)

	ON_COMMAND(ID_LINK_INCREASEBANDWIDTH, &CTLiteView::OnLinkIncreasebandwidth)
	ON_COMMAND(ID_LINK_DECREASEBANDWIDTH, &CTLiteView::OnLinkDecreasebandwidth)
	ON_COMMAND(ID_NODE_MOVEMENTPROPERTIES, &CTLiteView::OnNodeMovementproperties)
	ON_COMMAND(ID_VIEW_SHOW_CONNECTOR, &CTLiteView::OnViewShowConnector)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_CONNECTOR, &CTLiteView::OnUpdateViewShowConnector)
	ON_COMMAND(ID_VIEW_HIGHLIGHTCENTROIDSANDACTIVITYLOCATIONS, &CTLiteView::OnViewHighlightcentroidsandactivitylocations)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIGHLIGHTCENTROIDSANDACTIVITYLOCATIONS, &CTLiteView::OnUpdateViewHighlightcentroidsandactivitylocations)
	ON_COMMAND(ID_VIEW_BACKGROUNDCOLOR, &CTLiteView::OnViewBackgroundcolor)
	ON_UPDATE_COMMAND_UI(ID_LINK_INCREASEBANDWIDTH, &CTLiteView::OnUpdateLinkIncreasebandwidth)
	ON_UPDATE_COMMAND_UI(ID_LINK_DECREASEBANDWIDTH, &CTLiteView::OnUpdateLinkDecreasebandwidth)
	ON_COMMAND(ID_EXPORT_CREATEVISSIMFILES, &CTLiteView::OnExportCreatevissimfiles)
	ON_COMMAND(ID_DEBUG_SHOWAgentSWITHINCOMPLETETRIPSONLY, &CTLiteView::OnDebugShowAgentswithincompletetripsonly)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_SHOWAgentSWITHINCOMPLETETRIPSONLY, &CTLiteView::OnUpdateDebugShowAgentswithincompletetripsonly)
	ON_COMMAND(ID_Agent_AgentNUMBER, &CTLiteView::OnAgentAgentnumber)
	ON_UPDATE_COMMAND_UI(ID_Agent_AgentNUMBER, &CTLiteView::OnUpdateAgentAgentnumber)
	ON_COMMAND(ID_Agent_SHOWSELECTEDAgentONLY, &CTLiteView::OnAgentShowselectedAgentonly)
	ON_UPDATE_COMMAND_UI(ID_Agent_SHOWSELECTEDAgentONLY, &CTLiteView::OnUpdateAgentShowselectedAgentonly)
	ON_COMMAND(ID_NODE_ADDINTERMEDIATEDESTINATIONHERE, &CTLiteView::OnNodeAddintermediatedestinationhere)
	ON_COMMAND(ID_NODE_REMOVEALLINTERMEDIATEDESTINATION, &CTLiteView::OnNodeRemoveallintermediatedestination)
	ON_COMMAND(ID_LINK_AVOIDUSINGTHISLINKINROUTING, &CTLiteView::OnLinkAvoidusingthislinkinrouting)
	ON_BN_CLICKED(IDC_BUTTON_Configuration, &CTLiteView::OnBnClickedButtonConfiguration)
	ON_COMMAND(ID_NODE_NODEPROPERTIES, &CTLiteView::OnNodeNodeproperties)
	ON_COMMAND(ID_NODE_AVOIDTHISNODE, &CTLiteView::OnNodeAvoidthisnode)
	ON_COMMAND(ID_NODE_REMOVENODEAVOIDANCECONSTRAINT, &CTLiteView::OnNodeRemovenodeavoidanceconstraint)
	ON_COMMAND(ID_EDIT_MOVENODE, &CTLiteView::OnEditMovenode)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVENODE, &CTLiteView::OnUpdateEditMovenode)
	ON_COMMAND(ID_BACKGROUNDIMAGE_MARKLONG_A, &CTLiteView::OnBackgroundimageMarklongA)
	ON_COMMAND(ID_BACKGROUNDIMAGE_MARKLONG_B, &CTLiteView::OnBackgroundimageMarklongB)
	ON_COMMAND(ID_BACKGROUNDIMAGE_ADDLAT, &CTLiteView::OnBackgroundimageAddlat)

	ON_COMMAND(ID_ZONE_CREATEZONE, &CTLiteView::OnZoneCreatezone)
	ON_COMMAND(ID_EDIT_CREATEZONE, &CTLiteView::OnEditCreatezone)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CREATEZONE, &CTLiteView::OnUpdateEditCreatezone)
	ON_COMMAND(ID_SUBAREA_HIGHLIGHTLINKSINSIDESUBAREA, &CTLiteView::OnSubareaHighlightlinksinsidesubarea)
	ON_UPDATE_COMMAND_UI(ID_SUBAREA_HIGHLIGHTLINKSINSIDESUBAREA, &CTLiteView::OnUpdateSubareaHighlightlinksinsidesubarea)
	ON_COMMAND(ID_SUBAREA_HIGHLIGHTLINKSACOSSSUBAREA, &CTLiteView::OnSubareaHighlightlinksacosssubarea)
	ON_UPDATE_COMMAND_UI(ID_SUBAREA_HIGHLIGHTLINKSACOSSSUBAREA, &CTLiteView::OnUpdateSubareaHighlightlinksacosssubarea)
	ON_COMMAND(ID_ODMATRIX_VIEWTOP50ODPAIRSONLY, &CTLiteView::OnOdmatrixViewtop50odpairsonly)
	ON_UPDATE_COMMAND_UI(ID_ODMATRIX_VIEWTOP50ODPAIRSONLY, &CTLiteView::OnUpdateOdmatrixViewtop50odpairsonly)
	ON_COMMAND(ID_MOVEMENT_HIGHLIGHTPROHIBITEDMOVEMENTS, &CTLiteView::OnMovementHighlightprohibitedmovements)
	ON_UPDATE_COMMAND_UI(ID_MOVEMENT_HIGHLIGHTPROHIBITEDMOVEMENTS, &CTLiteView::OnUpdateMovementHighlightprohibitedmovements)
	ON_COMMAND(ID_FILE_PRINT, &CTLiteView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CTLiteView::OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CTLiteView::OnFilePrintSetup)
	ON_COMMAND(ID_AgentDATA_SHOWCOMPLETETRAJECTORY, &CTLiteView::OnAgentdataShowcompletetrajectory)
	ON_UPDATE_COMMAND_UI(ID_AgentDATA_SHOWCOMPLETETRAJECTORY, &CTLiteView::OnUpdateAgentdataShowcompletetrajectory)
	ON_COMMAND(ID_AgentDATA_SHOW, &CTLiteView::OnAgentdataShow)
	ON_UPDATE_COMMAND_UI(ID_AgentDATA_SHOW, &CTLiteView::OnUpdateAgentdataShow)
	ON_COMMAND(ID_DETECTOR_UNLOCK, &CTLiteView::OnDetectorUnlock)
	ON_UPDATE_COMMAND_UI(ID_DETECTOR_UNLOCK, &CTLiteView::OnUpdateDetectorUnlock)
	ON_COMMAND(ID_DETECTOR_SHOWSENSORDESCRIPTION33769, &CTLiteView::OnDetectorShowsensordescription33769)
	ON_UPDATE_COMMAND_UI(ID_DETECTOR_SHOWSENSORDESCRIPTION33769, &CTLiteView::OnUpdateDetectorShowsensordescription33769)
	ON_COMMAND(ID_DETECTOR_SHOWMATCHEDLINKFORSENSOR, &CTLiteView::OnDetectorShowmatchedlinkforsensor)
	ON_UPDATE_COMMAND_UI(ID_DETECTOR_SHOWMATCHEDLINKFORSENSOR, &CTLiteView::OnUpdateDetectorShowmatchedlinkforsensor)
	ON_COMMAND(ID_GRID_UNLOCK, &CTLiteView::OnGridUnlock)
	ON_UPDATE_COMMAND_UI(ID_GRID_UNLOCK, &CTLiteView::OnUpdateGridUnlock)
	ON_COMMAND(ID_LINK_SHOWHIDEREFERENCENETWORK, &CTLiteView::OnLinkShowhidereferencenetwork)
	ON_UPDATE_COMMAND_UI(ID_LINK_SHOWHIDEREFERENCENETWORK, &CTLiteView::OnUpdateLinkShowhidereferencenetwork)
	ON_COMMAND(ID_EDIT_MOVEReferenceNETWORKCOORDINATES, &CTLiteView::OnEditMovereferencenetworkcoordinates)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVEReferenceNETWORKCOORDINATES, &CTLiteView::OnUpdateEditMovereferencenetworkcoordinates)
	ON_COMMAND(ID_DETECTOR_SHOWSPEEDSENSORLOCATION, &CTLiteView::OnDetectorShowspeedsensorlocation)
	ON_UPDATE_COMMAND_UI(ID_DETECTOR_SHOWFIXEDCOUNTDETECTORLOCATION, &CTLiteView::OnUpdateDetectorShowfixedcountdetectorlocation)
	ON_UPDATE_COMMAND_UI(ID_DETECTOR_SHOWSPEEDSENSORLOCATION, &CTLiteView::OnUpdateDetectorShowspeedsensorlocation)
	ON_COMMAND(ID_DETECTOR_SHOWFIXEDCOUNTDETECTORLOCATION, &CTLiteView::OnDetectorShowfixedcountdetectorlocation)
	ON_COMMAND(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE1, &CTLiteView::OnShowallowedlinksfordemandtypeDemandtype1)
	ON_COMMAND(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE2, &CTLiteView::OnShowallowedlinksfordemandtypeDemandtype2)
	ON_COMMAND(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE3, &CTLiteView::OnShowallowedlinksfordemandtypeDemandtype3)
	ON_COMMAND(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE4, &CTLiteView::OnShowallowedlinksfordemandtypeDemandtype4)
	ON_UPDATE_COMMAND_UI(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE1, &CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype1)
	ON_UPDATE_COMMAND_UI(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE2, &CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype2)
	ON_UPDATE_COMMAND_UI(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE3, &CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype3)
	ON_UPDATE_COMMAND_UI(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_DEMANDTYPE4, &CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype4)
	ON_COMMAND(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_SHOWALL, &CTLiteView::OnShowallowedlinksfordemandtypeShowall)
	ON_UPDATE_COMMAND_UI(ID_SHOWALLOWEDLINKSFORDEMANDTYPE_SHOWALL, &CTLiteView::OnUpdateShowallowedlinksfordemandtypeShowall)
	ON_COMMAND(ID_REFERENCELINE_SHOWREFERENCELINENO, &CTLiteView::OnReferencelineShowreferencelineno)
	ON_BN_CLICKED(IDC_BUTTON_Search, &CTLiteView::OnBnClickedButtonSearch)
	END_MESSAGE_MAP()

// CTLiteView construction/destruction
// CTLiteView construction/destruction

CBrush g_BlackBrush(RGB(10,10,10));

CBrush  g_WhiteBrush(RGB(255, 255, 255));
CBrush  g_GreenBrush(RGB(0, 255, 0));

CBrush g_ActivityLocationBrush(RGB(255,0,0));

CPen g_BlackPen(PS_SOLID,1,RGB(0,0,0));

CPen g_TransitPen(PS_SOLID,1,RGB(255,69,0));  // orange red
CBrush g_TransitBrush(RGB(184,134,11));  //DarkGoldenrod	

CPen g_TransitLightPen(PS_SOLID,1,RGB(216,191,216));  // orange red
CBrush g_TransitLightBrush(RGB(255,225,255));  //DarkGoldenrod	


CPen g_TransitAccessibilityPen(PS_SOLID,1,RGB(0,0,255)); //Blue 
CBrush g_TransitAccessibilityBrush(RGB(0,0,255));  //Blue 	

CPen g_LaneMarkingPen(PS_DASH,0,RGB(255,255,255));

CPen g_PenSelectColor(PS_SOLID,5,RGB(255,0,0));
CPen g_PenDetectorColor(PS_SOLID, 1, RGB(0, 0, 255));

CPen g_PenExternalDColor(PS_SOLID,2,RGB(173,255,047)); 
CPen g_PenExternalOColor(PS_SOLID,2,RGB(255,165,0));

CPen g_PenODColor(PS_SOLID,2,RGB(255,255,0));
CPen g_PenSelectPath(PS_SOLID,3,RGB(0,0,255));  // blue

CPen g_PenDisplayColor(PS_SOLID,2,RGB(255,255,0));

CPen g_PenStopSignNodeColor(PS_SOLID,1,RGB(205,200,177));
CPen g_PenSignalNodeColor(PS_SOLID,1,RGB(255,0,0));

CPen g_GPSTrajectoryColor(PS_SOLID, 2, RGB(0,255,0));
CPen g_GPSTrajectoryColor0(PS_SOLID, 2, RGB(255, 0, 0));

CPen g_PenNodeColor(PS_SOLID,1,RGB(0,0,0));

CPen g_PenConnectorColor(PS_DASH,1,RGB(255,255,0));
CPen g_PenCentroidColor(PS_SOLID,1,RGB(0,255,255));

CPen g_PenQueueColor(PS_SOLID,2,RGB(255,0,0));

CPen g_PenQueueBandColor(PS_SOLID,1,RGB(255,0,0));
CBrush g_BrushQueueBandColor(RGB(255,0,0));

CPen g_PenCrashColor(PS_SOLID,1,RGB(255,0,0));
CBrush  g_BrushCrash(HS_VERTICAL,RGB(255,0,255)); //green
CBrush  g_TransitCrash(HS_CROSS,RGB(255,0,255)); //green

CPen g_PenGreen(PS_SOLID,1,RGB(0,255,0));
CBrush  g_BrushGreen(RGB(0,255,0)); //green


CPen g_PenDashBlue(PS_DASH,1,RGB(0,0,255));
CPen g_PenBlue(PS_SOLID,2,RGB(0,0,255));
CPen g_PenReference(PS_SOLID, 2, RGB(255, 0, 255));
CBrush g_BrushReference(RGB(184, 134, 11));


CBrush  g_BrushBlue(RGB(0,0,255));

CPen g_PenSensorColor(PS_SOLID,0,RGB(0,255,0));
CPen g_PenSpeedSensorColor(PS_SOLID,0,RGB(255,0,255));

CBrush g_BrushSensor(RGB(0,255,0));
CBrush g_InvalidBrushSensor(RGB(255, 0, 0));
CBrush g_BrushSpeedSensor(RGB(255,0,255));
CPen g_PenNotMatchedSensorColor(PS_SOLID,1,RGB(255,0,0));

CPen g_PenSelectColor0(PS_SOLID,2,RGB(255,0,0));  // red
CPen g_PenSelectColor1(PS_SOLID,2,RGB(0,0,255));  // blue
CPen g_PenSelectColor2(PS_SOLID,2,RGB(255,0,255)); //magenta
CPen g_PenSelectColor3(PS_SOLID,2,RGB(0,255,255));   // cyan
CPen g_PenSelectColor4(PS_SOLID,2,RGB(0,255,0));  // green
CPen g_PenSelectColor5(PS_SOLID,2,RGB(255,255,0)); // yellow
CPen g_PenPaxSelectColor0(PS_DOT, 1, RGB(255, 0, 0));  // red

CPen g_Pen2SelectColor0(PS_DOT ,1,RGB(255,0,0));  // red
CPen g_Pen2SelectColor1(PS_DOT ,1,RGB(0,0,255));  // blue
CPen g_Pen2SelectColor2(PS_DOT ,1,RGB(255,0,255)); //magenta
CPen g_Pen2SelectColor3(PS_DOT ,1,RGB(0,255,255));   // cyan
CPen g_Pen2SelectColor4(PS_DOT ,1,RGB(0,255,255));  // green
CPen g_Pen2SelectColor5(PS_DOT ,1,RGB(255,255,0)); // yellow


CPen g_PenProhibitedMovement(PS_DASH,6,RGB(255,0,0));  // PS_DASH

CPen g_PenSelectColor0_obs(PS_SOLID,2,RGB(255,0,0));  // red
CPen g_PenSelectColor1_obs(PS_SOLID,2,RGB(0,255,0));  // green
CPen g_PenSelectColor2_obs(PS_SOLID,2,RGB(255,0,255)); //magenta
CPen g_PenSelectColor3_obs(PS_SOLID,2,RGB(0,255,255));   // cyan
CPen g_PenSelectColor4_obs(PS_SOLID,2,RGB(0,0,255));  // blue
CPen g_PenSelectColor5_obs(PS_SOLID,2,RGB(255,255,0)); // yellow

CBrush  g_BrushColor0(HS_BDIAGONAL, RGB(255,0,0));  // red
CBrush  g_BrushColor1(HS_FDIAGONAL,RGB(0,0,255));  // green
CBrush  g_BrushColor2(HS_VERTICAL,RGB(255,0,255)); //magenta
CBrush  g_BrushColor3(HS_HORIZONTAL,RGB(0,255,255));   // cyan
CBrush  g_BrushColor4(HS_CROSS,RGB(0,255,0));  // green
CBrush  g_BrushColor5(HS_DIAGCROSS,RGB(255,255,0)); // yellow

CBrush  g_BrushLinkBand(RGB(125,125,0)); // 
CBrush  g_BrushLinkBandVolume(RGB(0,0,255)); // 

CBrush  g_BrushLinkReference(HS_CROSS, RGB(255,0,255)); //magenta
CPen    g_PenLinkReference(PS_SOLID,2,RGB(255,0,255));  //magenta

CPen g_ThickPenSelectColor0(PS_SOLID,3,RGB(255,0,0));  // red
CPen g_ThickPenSelectColor1(PS_SOLID,3,RGB(0,0,255));  // blue
CPen g_ThickPenSelectColor2(PS_SOLID,3,RGB(255,0,255)); //magenta
CPen g_ThickPenSelectColor3(PS_SOLID,3,RGB(0,255,255));   // cyan
CPen g_ThickPenSelectColor4(PS_SOLID,3,RGB(0,255,0));  // green
CPen g_ThickPenSelectColor5(PS_SOLID,3,RGB(255,255,0)); // yellow

CPen g_SuperThickPenSelectColor0(PS_SOLID,5,RGB(255,0,0));  // red
CPen g_SuperThickPenSelectColor1(PS_SOLID,5,RGB(0,0,255));  // blue
CPen g_SuperThickPenSelectColor2(PS_SOLID,5,RGB(255,0,255)); //magenta
CPen g_SuperThickPenSelectColor3(PS_SOLID,5,RGB(0,255,255));   // cyan
CPen g_SuperThickPenSelectColor4(PS_SOLID,5,RGB(0,255,0));  // green
CPen g_SuperThickPenSelectColor5(PS_SOLID,5,RGB(255,255,0)); // yellow


CPen g_TempLinkPen(PS_DASH,0,RGB(255,255,255));
CPen g_AVILinkPen(PS_DASH,0,RGB(0,255,0));
CPen g_SubareaLinkPen(PS_SOLID,0,RGB(255,255,0));
CPen g_SubareaBoundaryLinkPen(PS_SOLID,2,RGB(255,0,255));
CPen g_SubareaPen(PS_DASH,2,RGB(255,0,0));

CPen g_GridPen(PS_SOLID,1,RGB(190,190,190));

CPen g_PenAgent(PS_SOLID,1,RGB(0,255,0));  // yellow
CPen g_BrushAgent(PS_SOLID,1,RGB(0,100,0)); //magenta

CPen g_PenSelectedAgent(PS_SOLID,2,RGB(255,0,0));  // red
CPen g_PenMatchedSensorLink(PS_DASH, 1, RGB(255, 0, 0));  // red

CPen g_BrushTransitUser(PS_SOLID,1,RGB(255,0,0)); //magenta



void g_SelectColorCode(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{

	case 0: pDC->SelectObject(&g_PenSelectColor0); break;
	case 1: pDC->SelectObject(&g_PenSelectColor1); break;
	case 2: pDC->SelectObject(&g_PenSelectColor2); break;
	case 3: pDC->SelectObject(&g_PenSelectColor3); break;
	case 4: pDC->SelectObject(&g_PenSelectColor4); break;
	case 5: pDC->SelectObject(&g_PenSelectColor5); break;
	default:
		pDC->SelectObject(&g_PenSelectColor5);
	}

}
void g_SelectColorCode2(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{

	case 0: pDC->SelectObject(&g_Pen2SelectColor0); break;
	case 1: pDC->SelectObject(&g_Pen2SelectColor1); break;
	case 2: pDC->SelectObject(&g_Pen2SelectColor2); break;
	case 3: pDC->SelectObject(&g_Pen2SelectColor3); break;
	case 4: pDC->SelectObject(&g_Pen2SelectColor4); break;
	case 5: pDC->SelectObject(&g_Pen2SelectColor5); break;
	default:
		pDC->SelectObject(&g_Pen2SelectColor5);
	}

}

void g_SelectColorCode3(CDC* pDC, int ColorCount)
{
	switch (ColorCount)
	{

	case 0: pDC->SelectObject(&g_Pen2SelectColor5); break;
	case 1: pDC->SelectObject(&g_Pen2SelectColor1); break;
	case 2: pDC->SelectObject(&g_Pen2SelectColor2); break;
	case 3: pDC->SelectObject(&g_Pen2SelectColor3); break;
	case 4: pDC->SelectObject(&g_Pen2SelectColor4); break;
	default:
		pDC->SelectObject(&g_Pen2SelectColor5);
	}

}

void g_SelectThickColorCode(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{

	case 0: pDC->SelectObject(&g_PenSelectColor0_obs); break;
	case 1: pDC->SelectObject(&g_PenSelectColor1_obs); break;
	case 2: pDC->SelectObject(&g_PenSelectColor2_obs); break;
	case 3: pDC->SelectObject(&g_PenSelectColor3_obs); break;
	case 4: pDC->SelectObject(&g_PenSelectColor4_obs); break;
	case 5: pDC->SelectObject(&g_PenSelectColor5_obs); break;
	default:
		pDC->SelectObject(&g_PenSelectColor5_obs);
	}

}


void g_SelectThickPenColor(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{
	case 0: pDC->SelectObject(&g_ThickPenSelectColor0); break;
	case 1: pDC->SelectObject(&g_ThickPenSelectColor1); break;
	case 2: pDC->SelectObject(&g_ThickPenSelectColor2); break;
	case 3: pDC->SelectObject(&g_ThickPenSelectColor3); break;
	case 4: pDC->SelectObject(&g_ThickPenSelectColor4); break;
	case 5: pDC->SelectObject(&g_ThickPenSelectColor5); break;
	default:
		pDC->SelectObject(&g_PenSelectColor5);
	}
}

void g_SelectBrushColor(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{
	case 0: pDC->SelectObject(&g_BrushColor0); break;
	case 1: pDC->SelectObject(&g_BrushColor1); break;
	case 2: pDC->SelectObject(&g_BrushColor2); break;
	case 3: pDC->SelectObject(&g_BrushColor3); break;
	case 4: pDC->SelectObject(&g_BrushColor4); break;
	case 5: pDC->SelectObject(&g_BrushColor5); break;
	default:
		pDC->SelectObject(&g_BrushColor0);
	}
}


void g_SelectSuperThickPenColor(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{

	case 0: pDC->SelectObject(&g_SuperThickPenSelectColor0); break;
	case 1: pDC->SelectObject(&g_SuperThickPenSelectColor1); break;
	case 2: pDC->SelectObject(&g_SuperThickPenSelectColor2); break;
	case 3: pDC->SelectObject(&g_SuperThickPenSelectColor3); break;
	case 4: pDC->SelectObject(&g_SuperThickPenSelectColor4); break;
	case 5: pDC->SelectObject(&g_SuperThickPenSelectColor5); break;
	default:
		pDC->SelectObject(&g_SuperThickPenSelectColor5);
	}

}



CTLiteView::CTLiteView()
{
	m_DislayedAgentType = 0;
	m_bShowFixedDetectorLocation = true;
	m_bShowSpeedSensorLocation = true;
	m_DislayReferenceLineNo = false;

	m_bShowWalkLinksOnly = false;
	m_MoveLayerNo = 0;
	m_bShowCompleteTrajectory = true;
	m_bShowAllCompleteTrajectory = false;
	m_bShowTransitLinksOnly = false;
	m_bShowWalkLinksOnly = false;
	m_bShowProhibitedMovements = false;
	m_bShowTransitAccessibility  = false;
	m_bShowTop10ODOnly = true;

	m_bUpdateLinkAttributeBasedOnType  = false;
	m_LinkTextFontSize = 12;
	m_NodeDisplayBoundarySize = 200;

	m_bNetworkCooridinateHints = false;
	bShowAgentsWithIncompleteTrips = false;

	m_msStatus = 0;

	m_link_display_mode = link_display_mode_band; // 

	m_NodeTypeFaceName      = "Arial";

	m_bShowAVISensor = true;
	m_bShowSensorDecription = false;
	m_bShowSensorMatchedLink = false;
	m_bShowODDemandVolume = false;
	m_bHighlightSubareaLinks = true;
	m_bHighlightSubareaBoundaryLinks = false;
	m_bShowConnector = true;
	m_bHighlightActivityLocation = false;
	isCreatingSubarea = false;
	isFinishSubarea = false;	
	m_ViewID = g_ViewID++;
	m_Resolution = 1;

	m_ShowLinkTextMode  = link_display_none;
	m_ShowMovementTextMode  = movement_display_none;
	m_MovmentTextSize = 1.0;
	m_LinkTextSize = 1.0;

	m_bMouseDownFlag = false;
	m_ShowAllPaths = true;

	m_OriginOnBottomFlag = -1;

	//	RGB(102,204,204);

	m_ToolMode = select_feature_tool;
	m_bMoveDisplay = false;
	m_bMoveImage = false;
	m_bMoveSensorMap = false;
	m_bMoveNetwork = false;
	m_bShowImage = true;
	m_bShowGrid  = false;
	m_bShowLinkArrow = false;
	m_bShowNode = true;
	m_ShowNodeTextMode = node_display_node_number;
	m_ShowGPSTextMode = GPS_display_none;

	m_ShowLinkTextMode = link_display_none;
	m_ShowMovementTextMode  = movement_display_none;

	m_bShowAgentNumber = false;
	m_bShowSelectedAgentOnly = true;
	m_bShowLinkType  = true;

	m_Origin.x = 0;
	m_Origin.y = 0;

	m_AgentSize = 1;
	m_bShowTransit = false;

	m_SelectFromNodeID = 0;
	m_SelectToNodeID = 0;
	g_ViewList.push_back(this);

}
CTLiteView::~CTLiteView()
{
	std::list<CTLiteView*>::iterator iView = g_ViewList.begin ();
	while (iView != g_ViewList.end())
	{
		if((*iView) == this)
		{
			g_ViewList.erase (iView);  // remove the Viewument to be deleted
			break;

		}
		iView++;
	}

}

BOOL CTLiteView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CTLiteView drawing
BOOL CTLiteView::OnPreparePrinting(CPrintInfo* pInfo)
{
   return CView::OnPreparePrinting(pInfo);
}

////////////////////////////////////////////////////////////////////////////////
// OnPrint
////////////////////////////////////////////////////////////////////////////////
void CTLiteView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{

   CView::OnPrint(pDC, pInfo);

   // Reset the resolution and background color
}

void CTLiteView::DrawBitmap(CDC *pDC, CPoint point,UINT nIDResource )
{

	CBitmap bitmapImage;
	//load bitmap from resource
	bitmapImage.LoadBitmap(nIDResource);

	BITMAP bmpInfo;
	CSize bitSize;
	//get size of bitmap
	bitmapImage.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);

	CDC* pwholeWndDC;
	pwholeWndDC=pDC;

	CImageList imageList;
	//create ImageList
	imageList.Create(bitSize.cx, bitSize.cy, ILC_COLOR|ILC_MASK, 1, 1);
	//select color yellow is transparent color
	imageList.Add(&bitmapImage, RGB(255,255,0));// Add bitmap to CImageList.

	point.x-=bitSize.cx /2;
	point.y-=bitSize.cy /2;


	//drawing the first bitmap of imagelist Transparently
	//imageList.SetBkColor(RGB(0,0,0));
	imageList.Draw(pwholeWndDC,0, point, ILD_TRANSPARENT );

}
void CTLiteView::OnDraw(CDC* pDC)
{
	//	FollowGlobalViewParameters(this->m_ViewID);

	// Custom draw on top of memDC
	CTLiteDoc* pDoc = GetDocument();

   
CRect rectClient(0,0,0,0);
	// Get window bounds
   if(pDC->IsPrinting())
   {
      CSize size = pDC->GetWindowExt();
      rectClient.bottom = -size.cy / 2;
      rectClient.left = -size.cx / 2;
      rectClient.top = size.cy / 2;
      rectClient.right = size.cx / 2;

	CBrush brush;

	if (!brush.CreateSolidBrush(theApp.m_BackgroundColor))
		return;

	brush.UnrealizeObject();
	pDC->FillRect(rectClient, &brush);
	DrawObjects(pDC);

   }
   else
   {
   	GetClientRect(&rectClient);
     
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectClient.Width(),
		rectClient.Height());
	memDC.SelectObject(&bmp);


	CBrush brush;

	if (!brush.CreateSolidBrush(theApp.m_BackgroundColor))
		return;

	brush.UnrealizeObject();
	memDC.FillRect(rectClient, &brush);

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	if(pDoc->m_BackgroundBitmapLoaded && pMainFrame->m_bShowLayerMap[layer_background_image])
	{
		//pDoc->m_ImageX2  = pDoc->m_ImageX1+ pDoc->m_ImageWidth * pDoc->m_ImageXResolution;
		//pDoc->m_ImageY2  = pDoc->m_ImageY1+ pDoc->m_ImageHeight * pDoc->m_ImageYResolution ;

		GDPoint IMPoint1;
		IMPoint1.x = pDoc->m_ImageX1;
		IMPoint1.y = pDoc->m_ImageY1;

		GDPoint IMPoint2;
		IMPoint2.x = pDoc->m_ImageX2;
		IMPoint2.y = pDoc->m_ImageY2;


		CPoint point1 = NPtoSP(IMPoint1);
		CPoint point2 = NPtoSP(IMPoint2);

		pDoc->m_BackgroundBitmap.StretchBlt(memDC,point1.x,point1.y,point2.x-point1.x,abs(point2.y-point1.y),SRCCOPY);


		//if(pDoc->m_bPointA_Initialized )
		//{
		//	GDPoint pt;
		//	pt.x = pDoc->m_PointA_x;
		//	pt.y = pDoc->m_PointA_y;

		//	CPoint point = NPtoSP(pt);
		//	int size  = 5;
		//	memDC.Ellipse (point.x - size, point.y + size,
		//		point.x + size, point.y - size);

		//	point.y -= size / 2;
		//	memDC.TextOut(point.x , point.y , _T("A"));
		//}

		//if(pDoc->m_bPointB_Initialized )
		//{
		//	GDPoint pt;
		//	pt.x = pDoc->m_PointB_x;
		//	pt.y = pDoc->m_PointB_y;

		//	CPoint point = NPtoSP(pt);
		//	int size  = 5;
		//	memDC.Ellipse (point.x - size, point.y + size,
		//		point.x + size, point.y - size);

		//	point.y -= size / 2;
		//	memDC.TextOut(point.x , point.y , _T("B"));
		//}

	}


	DrawObjects(&memDC);

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &memDC, 0,
		0, SRCCOPY);

	ReleaseDC(pDC);

   }	// TODO: add draw code for native data here
}


// CTLiteView diagnostics

#ifdef _DEBUG
void CTLiteView::AssertValid() const
{
	CView::AssertValid();
}

void CTLiteView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTLiteDoc* CTLiteView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTLiteDoc)));
	return (CTLiteDoc*)m_pDocument;
}
#endif //_DEBUG


// CTLiteView message handlers
void CTLiteView::DrawObjects(CDC* pDC)
{

	CTLiteDoc* pDoc = GetDocument();
		CPen pen_moe[MAX_LOS_SIZE];
		CBrush brush_moe[MAX_LOS_SIZE];

		for(int los = 0; los < MAX_LOS_SIZE-1; los++)
		{
			pen_moe[los].CreatePen (PS_SOLID, 1, pDoc->m_colorLOS[los]);
			brush_moe[los].CreateSolidBrush (pDoc->m_colorLOS[los]);
		}
	// step 1: set the objects in the network to the screen 

	std::list<DTANode*>::iterator iNode;

	if(!pDoc->m_bFitNetworkInitialized  )
	{
		FitNetworkToScreen();
		pDoc->m_bFitNetworkInitialized = true;

	}

	pDC->SetBkMode(TRANSPARENT);

	CRect ScreenRect;
	GetClientRect(ScreenRect);

	// step 2: draw grids

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	if(pMainFrame->m_bShowLayerMap[layer_grid] == true)
	{
		pDC->SelectObject(&g_GridPen);
		pDC->SetTextColor(RGB(255,228,181));
		
		// get the closest power 10 number
		m_GridResolutionX = g_FindClosestXResolution(ScreenRect.Width ()/m_Resolution/10.0f, SPtoNP(ScreenRect.CenterPoint()).x, SPtoNP(ScreenRect.CenterPoint()).y);

		if(pDoc->m_bUseMileVsKMFlag==false && pDoc->m_LongLatFlag==false ) //use km
		{
			m_GridResolutionX = 1;
		}

		int LeftX  = int(SPtoNP(ScreenRect.TopLeft()).x)-1;

		if(m_GridResolutionX>1)
			LeftX = LeftX- LeftX%int(m_GridResolutionX);

		for(double x = LeftX; x<= int(SPtoNP(ScreenRect.BottomRight ()).x)+1; x+=m_GridResolutionX)
		{
			fromp.x = x;
			fromp.y = SPtoNP(ScreenRect.TopLeft()).y;

			top.x = x;
			top.y = SPtoNP(ScreenRect.BottomRight()).y;

			FromPoint = NPtoSP(fromp);
			ToPoint = NPtoSP(top);

			pDC->SelectObject(&g_GridPen);

			pDC->MoveTo(FromPoint);
			pDC->LineTo(ToPoint);

			CString str;

			if(m_GridResolutionX<1)
				str.Format ("%.4f",x);
			else
				str.Format ("%.0f",x);

			pDC->TextOut(FromPoint.x,ScreenRect.TopLeft().y+10,str);
		}

		int BottomY  = int(SPtoNP(ScreenRect.BottomRight()).y)-1;

		m_GridResolutionY = g_FindClosestYResolution(ScreenRect.Width() / m_Resolution / 10.0f, SPtoNP(ScreenRect.CenterPoint()).x, SPtoNP(ScreenRect.CenterPoint()).y);

		pDoc->m_Doc_Resolution = m_GridResolutionX;

		if(m_GridResolutionX>1)
			BottomY = BottomY- BottomY%int(m_GridResolutionX);

		for(double y = BottomY; y<= int(SPtoNP(ScreenRect.TopLeft ()).y)+1; y+=m_GridResolutionY)
		{

			fromp.x =  SPtoNP(ScreenRect.TopLeft()).x ;
			fromp.y = y;

			top.x = SPtoNP(ScreenRect.BottomRight()).x ;
			top.y = y ;

			FromPoint = NPtoSP(fromp);
			ToPoint = NPtoSP(top);

			pDC->MoveTo(FromPoint);
			pDC->LineTo(ToPoint);

			CString str;
			if(m_GridResolutionX<1)
				str.Format ("%.4f",y);
			else
				str.Format ("%.0f",y);

			pDC->TextOut(ScreenRect.TopLeft().x+10,FromPoint.y,str);

		}

			//0 111.320 km
			//15 107.551 km
			//30 96.486 km
			//45 78.847 km
			//60 55.800 km
			//75 28.902 km


		//CString str;
		//if (pDoc->m_bUseMileVsKMFlag)
		//	str.Format("grid width: %.4f mi (%.4f km); height: %.4f mi (%.4f km)", m_GridResolutionX * 96.49/1.6093,
		//		m_GridResolutionX * 96.49 , m_GridResolutionX * 110.852 / 1.6093, m_GridResolutionX * 110.852);
		//else
		//	str.Format("grid width: %.4f; height: %.4f", m_GridResolutionX, m_GridResolutionX);

		//pDoc->SendTexttoStatusBar(str, 2);

	}

	if (pMainFrame->m_bShowLayerMap[layer_reference_line])
	{

		// draw generic link layer
		std::list<DTALine*>::iterator iLine;

		if (pDoc->m_DTALineSet.size() > 0)
		{
			pDC->SelectObject(&g_BlackBrush);
			pDC->SetTextColor(RGB(255, 255, 0));
			pDC->SelectObject(&g_PenReference);
			pDC->SetBkColor(RGB(0, 0, 0));

			for (iLine = pDoc->m_DTALineSet.begin(); iLine != pDoc->m_DTALineSet.end(); iLine++)
			{

				for (unsigned int i = 0; i < (*iLine)->m_ShapePoints.size(); i++)
				{
					CPoint point = NPtoSP((*iLine)->m_ShapePoints[i]);
					if (i == 0)
						pDC->MoveTo(point);
					else
						pDC->LineTo(point);

				}

				


			}

		}

	}  // reference line layer

	// step 2: select font and color for node drawing, and compute the bandwidth for links
	CFont node_font;  // local font for nodes. dynamically created. it is effective only inside this function. if you want to pass this font to the other function, we need to pass the corresponding font pointer (which has a lot of communication overheads)
	int node_size = max(2,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution));

	node_size = min(50,node_size);

	int NodeTypeSize = pDoc->m_NodeTextDisplayRatio;
	int nFontSize =  max(node_size * NodeTypeSize, 10);


	node_font.CreatePointFont(nFontSize, m_NodeTypeFaceName);

	pDC->SelectObject(&node_font);

	COLORREF oldColor = pDC->SetTextColor(RGB(255,255,0));

	UINT oldAlign = pDC->SetTextAlign(TA_CENTER);

	// Text height
	TEXTMETRIC tm;
	memset(&tm, 0, sizeof TEXTMETRIC);
	pDC->GetOutputTextMetrics(&tm);

	//test the display width of a lane, if greather than 1, then band display mode

	int dispaly_link_flag = 0;
	//	for (unsigned int l = 0; l < pDoc->m_LinkSet.size(); l++)
	//{
	//	DTALink* pLink = pDoc->FindLinkWithLinkNo (pDoc->m_LinkSet[l]);
	//	if(pLink!=NULL)
	//	{
	//		dispaly_link_flag += pLink->m_DisplayLinkID ;
	//	}

	//}
	if (pMainFrame->m_bShowLayerMap[layer_link] == true || (dispaly_link_flag && pMainFrame->m_bShowLayerMap[layer_path] == true) )  // show link layer either one of 2 conditions are met
	{

		if( pDoc->m_LinkMOEMode == MOE_Agent || pDoc->m_LinkMOEMode== MOE_QueueLengthRatio  || pDoc->m_LinkMOEMode == MOE_bottleneck)
				m_link_display_mode = link_display_mode_line;
		else if( pDoc->m_LinkMOEMode == MOE_volume ||pDoc->m_LinkMOEMode ==  MOE_density ||  pDoc->m_LinkMOEMode ==  MOE_speed )
				m_link_display_mode = link_display_mode_band;
		else if(pDoc->m_LinkMOEMode == MOE_none)
		{

			if(pDoc->m_LaneWidthInKM * pDoc->m_UnitDistance*m_Resolution * 5 > 0.1f) // 5 lanes
				m_link_display_mode = link_display_mode_line;
			else
				m_link_display_mode = link_display_mode_band;
	
		}
			

		// draw links
		std::list<DTALink*>::iterator iLink;
		std::list<DTALink*>::iterator iLinkDisplay;

		CPoint FromPoint,ToPoint;
		CRect link_rect;

		CPen pen_freeway, pen_ramp, pen_arterial, pen_connector, pen_transit, pen_walking, pen_pickup, pen_dropoff;
		CBrush brush_freeway, brush_ramp, brush_arterial, brush_connector, brush_walking, brush_pickup, brush_dropoff;

		pen_freeway.CreatePen (PS_SOLID, 1, theApp.m_FreewayColor);
		brush_freeway.CreateSolidBrush (theApp.m_FreewayColor);

		pen_ramp.CreatePen (PS_SOLID, 1, theApp.m_RampColor);
		brush_ramp.CreateSolidBrush (theApp.m_RampColor);

		pen_arterial.CreatePen (PS_SOLID, 1, theApp.m_ArterialColor);
		brush_arterial.CreateSolidBrush (theApp.m_ArterialColor);

		pen_connector.CreatePen (PS_SOLID, 1, theApp.m_ConnectorColor);
		brush_connector.CreateSolidBrush (theApp.m_ConnectorColor);

		pen_transit.CreatePen (PS_SOLID, 3, theApp.m_TransitColor);

		pen_walking.CreatePen (PS_SOLID, 4, theApp.m_WalkingColor);
		pen_pickup.CreatePen(PS_SOLID, 3, theApp.m_PickupColor);
		pen_dropoff.CreatePen(PS_SOLID, 3, theApp.m_DropoffColor);

		brush_pickup.CreateSolidBrush(theApp.m_PickupColor);
		brush_dropoff.CreateSolidBrush(theApp.m_DropoffColor);

		brush_walking.CreateSolidBrush (theApp.m_WalkingColor);

		// recongenerate the lind band width offset only when chaning display mode or on volume mode
		if(	pDoc -> m_PrevLinkMOEMode != pDoc -> m_LinkMOEMode || 
			pDoc->m_LinkMOEMode == MOE_density || 
			pDoc->m_LinkMOEMode == MOE_volume || 
			pDoc->m_LinkMOEMode == MOE_speed )
		{
			pDoc->GenerateOffsetLinkBand();
		}

		if(pDoc->m_LinkSet.size() > 0)  // update for a network with links
		{
			pDoc -> m_PrevLinkMOEMode = pDoc -> m_LinkMOEMode;
		}

		// step 3: draw all links
		int min_x, min_y, max_x, max_y;

		pDC->SelectObject(&g_BrushLinkBand);   //default brush  , then MOE, then apply speical conditions for volume and Agent mode

		//	TRACE("connector layer: %d\n", pMainFrame->m_bShowLayerMap[layer_connector]);


		for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
		{


			if( pMainFrame->m_bShowLayerMap[layer_transit] == true && m_bShowTransitLinksOnly )
			{
				if (pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].IsTransit  ()== false)  //show transit only
					continue; 
			}


			if( pMainFrame->m_bShowLayerMap[layer_transit] == true && m_bShowWalkLinksOnly )
			{
				if (pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].IsWalking  ()== false)  //show walking only
					continue; 
			}

			if( !pMainFrame->m_bShowLayerMap[layer_connector] )
			{
				if (pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].IsConnector ())  //hide connectors
					continue; 
			}

			if( !pMainFrame->m_bShowLayerMap[layer_transit] )
			{
				if (pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].IsTransit  () || pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].IsWalking   ())  //hide transit
					continue; 
			}


			// step 1: decide if a link is included in the screen region
			FromPoint = NPtoSP((*iLink)->m_FromPoint);
			ToPoint = NPtoSP((*iLink)->m_ToPoint);

			min_x = min(FromPoint.x,ToPoint.x);
			max_x = max(FromPoint.x,ToPoint.x);

			min_y = min(FromPoint.y,ToPoint.y);
			max_y = max(FromPoint.y,ToPoint.y);

			int size = 1000;
			link_rect.SetRect(min_x-size,min_y-size,max_x+size,max_y+size);

			if(RectIsInsideScreen(link_rect,ScreenRect) == false)  // not inside the screen boundary
				continue;


			// step 4: select color and brush for MOE mode

			//			continue;
			CPen LinkTypePen;


			// step 5: select color and brush according to link MOE
			float value = -1.0f ;

			if ((*iLink)->m_bActive = false)  // for deleted links
				continue; 

			if( pDoc->m_LinkMOEMode != MOE_none) 
			{

				if ( (*iLink)->m_FromNodeID == 78424 && (*iLink)->m_ToNodeID == 61158 && g_Simulation_Time_Stamp>=1320) 
				{

					TRACE("trace");

				}

				// dynamically create LOS pen and brush  
				float power =0 ; int LOS = 1;

				power = pDoc->GetLinkMOE((*iLink), pDoc->m_LinkMOEMode, (int)g_Simulation_Time_Stamp, g_MOEAggregationIntervalInMin, value);

				LOS = pDoc->GetLOSCode(power);

				//draw link as lines
				if(m_link_display_mode == link_display_mode_line )
				{
					if(pDoc->m_LinkMOEMode != MOE_volume) // not volume display
					{
						pDC->SelectObject(&pen_moe[LOS]);
						pDC->SelectObject(&g_BrushLinkBand);   //default brush  , then MOE, then condition with volume
					}else
					{
						pDC->SelectObject(&g_BlackPen);
						pDC->SelectObject(&g_BlackBrush);

					}
				}
				else 
				{// display link as a band so use black pen
					pDC->SelectObject(&g_BlackPen);
					pDC->SelectObject(&brush_moe[LOS]);
				}

			}
			else if(m_bShowLinkType)  // show link type
			{
				//		COLORREF link_color = pDoc->GetLinkTypeColor((*iLink)->m_link_type );
				//		LinkTypePen.CreatePen(PS_SOLID, 1, link_color);
				//		pDC->SelectObject(&LinkTypePen);

				if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsFreeway ())
				{
					pDC->SelectObject(&pen_freeway);
					pDC->SelectObject(&brush_freeway);
				}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsArterial())
				{
					pDC->SelectObject(&pen_arterial);
					pDC->SelectObject(&brush_arterial);
				}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsRamp())
				{
					pDC->SelectObject(&pen_ramp);
					pDC->SelectObject(&brush_ramp);
				}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type] .IsConnector())
				{
					pDC->SelectObject(&pen_connector);
					pDC->SelectObject(&brush_connector);
				}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsTransit())
				{
					pDC->SelectObject(&pen_transit);
					pDC->SelectObject(&g_TransitCrash);
				}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsWalking())
				{
					pDC->SelectObject(&pen_walking);
					pDC->SelectObject(&brush_walking);
				}
				else if (pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsPickup())
				{
					pDC->SelectObject(&pen_pickup);
					pDC->SelectObject(&brush_pickup);
				}
				else if (pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsDropoff())
				{
					pDC->SelectObject(&pen_dropoff);
					pDC->SelectObject(&brush_dropoff);
				}
				else
				{
					pDC->SelectObject(&pen_arterial);
					pDC->SelectObject(&g_BrushLinkBand);   //default brush
				}

				// special condition for subarea link
				if(m_bHighlightSubareaLinks &&(*iLink)->m_bIncludedinSubarea && pDoc->m_SubareaShapePoints.size()>=3)
					pDC->SelectObject(&g_SubareaLinkPen);

				if(m_bHighlightSubareaBoundaryLinks &&(*iLink)->m_bIncludedinBoundaryOfSubarea  && pDoc->m_SubareaShapePoints.size()>=3)
					pDC->SelectObject(&g_SubareaBoundaryLinkPen);

			}else  // default arterial model
			{
				pDC->SelectObject(&pen_arterial);
				pDC->SelectObject(&g_BrushLinkBand);   //default brush
			}

			//special condition 1: Agent mode

			if( pDoc->m_LinkMOEMode == MOE_Agent)  // when showing Agents, use black
				pDC->SelectObject(&g_BlackPen);

			//special condition 2:  volume mode
			if (pDoc->m_LinkMOEMode == MOE_volume)   // under volume mode, dynamically generate volume band
				pDC->SelectObject(&g_BrushLinkBandVolume);   //default brush

			// special condition 3: when a link is selected

			//outside subarea
			if((*iLink)->m_DisplayLinkID>=0 && pMainFrame->m_bShowLayerMap[layer_path]==true)
			{
				g_SelectThickPenColor(pDC,(*iLink)->m_DisplayLinkID);
				pDC->SetTextColor(RGB(255,0,0));
			}
			else if(m_bHighlightSubareaLinks &&(*iLink)->m_bIncludedinSubarea)
			{
				g_SelectThickPenColor(pDC,1);
				pDC->SetTextColor(RGB(255,0,0));
			}else if(m_bHighlightSubareaBoundaryLinks &&(*iLink)->m_bIncludedinBoundaryOfSubarea  && pDoc->m_SubareaShapePoints.size()>=3)
			{
				pDC->SelectObject(&g_SubareaBoundaryLinkPen);
			}else if  ((*iLink)->m_LinkNo == pDoc->m_SelectedLinkNo)
			{
				g_SelectThickPenColor(pDC,0);
				pDC->SetTextColor(RGB(255,0,0));
			}else 
				pDC->SetTextColor(RGB(255,228,181));


			// special condition 4: when a link is a reference link
			if (m_bShowReferenceNetwork)
			{
				if((*iLink)->m_LayerNo == 1)
				  pDC->SelectObject(&g_PenReference);
				if ((*iLink)->m_LayerNo == 2)
					pDC->SelectObject(&g_PenDashBlue);

			}


			if (m_bShowReferenceNetwork == true && pMainFrame->m_bShowLayerMap[layer_link] == false && (*iLink)->m_LayerNo ==0)
			{  //skip the baseline link layer under special conditions 
				continue;
			}


			
			if (pMainFrame->m_bShowLayerMap[layer_reference_line] == false)
			{
				if ((*iLink)->m_LayerNo == 1)  //skip reference nodes
					return;
			}


			//step 6: draw link as line or band/bar
			if(m_link_display_mode == link_display_mode_line )  
			{
				// calibration mode, do not show volume
				DrawLinkAsLine((*iLink),pDC);
			}
			else 
			{
				
				if(DrawLinkAsBand((*iLink),pDC,false)==false)
					return;
			}

			CPoint ScenarioPoint;

			if (pMainFrame->m_bShowLayerMap[layer_workzone] == true &&
				((*iLink)->GetImpactedFlag(g_Simulation_Time_Stamp, true) >= 0.01 || (g_Simulation_Time_Stamp <= 0.1 && (*iLink)->GetImpactedFlag(-1, true) >= 0.01)))
			{
				ScenarioPoint = NPtoSP((*iLink)->GetRelativePosition(0.6));  // get relative position of a link 
				DrawBitmap(pDC, ScenarioPoint, IDB_WORKZONE);
			}




			if ((*iLink)->m_AdditionalCost >= 1 && pMainFrame->m_bShowLayerMap[layer_path] == true)
			{
				ScenarioPoint = NPtoSP((*iLink)->GetRelativePosition(0.6));  // get relative position of a link 
				DrawBitmap(pDC, ScenarioPoint, IDB_LINK_CLOSURE);
			}

		

			//step 7: show link labels
			if( m_ShowLinkTextMode != link_display_none )
			{

				int node_size = max(2,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution));

				node_size = min(50,node_size);

				int LinkTextFontSize =   10;
				LinkTextFontSize = min(50,m_LinkTextFontSize)*m_LinkTextSize;


				float sqr_value = (FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x) + (FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y);
				float screen_distance = sqrt( sqr_value) ;

				// create rotation font
				CFont link_text_font;

				LOGFONT lf;
				memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
				lf.lfHeight = LinkTextFontSize;        // request a 12-pixel-height font

				int last_shape_point_id = (*iLink) ->m_ShapePoints .size() -1;
				float DeltaX = (*iLink)->m_ShapePoints[last_shape_point_id].x - (*iLink)->m_ShapePoints[0].x;
				float DeltaY = (*iLink)->m_ShapePoints[last_shape_point_id].y - (*iLink)->m_ShapePoints[0].y;

				float theta = atan2(DeltaY, DeltaX);

				float theta_deg = fmod(theta/PI*180,360);

				if(theta_deg < -90)
					theta_deg += 180;

				if(theta_deg > 90)
					theta_deg -= 180;

				lf.lfEscapement = theta_deg*10;
				strcpy(lf.lfFaceName, "Arial");       

				link_text_font.CreateFontIndirect(&lf);

				pDC->SelectObject(&link_text_font);

				// select text string to be displayed

				bool with_text = true;
				CString str_text, str_reference_text;
				str_text.Format (""); 

				// show text condition 1: street name
				// show text condition 2: crash rates
			

				if (screen_distance < 5)
				{
					with_text = false;
				}

				switch (m_ShowLinkTextMode)
				{
				case link_display_from_id_to_id: 
					str_text.Format ("%d->%d", (*iLink)->m_FromNodeID , (*iLink)->m_ToNodeID ); break;
				case link_display_street_name:
					str_text = (*iLink)->m_Name.c_str(); break;

				case link_display_link_id: 
					str_text.Format ("%s", (*iLink)->m_LinkID.c_str()  ); break;

				case  link_display_speed_limit_:
					str_text.Format ("%.1f",(*iLink)->m_FreeSpeed ); break;
				case link_display_length_:
					str_text.Format ("%.3f",(*iLink)->m_Length  ); break;
	
					break;
				case  link_display_free_flow_travel_time:
					str_text.Format ("%.2f",(*iLink)->m_FreeFlowTravelTime   ); break;

				case link_display_number_of_lanes:
					str_text.Format ("%d",(*iLink)->m_NumberOfLanes ); break;						

				case link_display_lane_capacity_per_hour:

					str_text.Format ("%.0f",(*iLink)->m_LaneCapacity); break;

				case link_display_link_capacity_per_hour:
					str_text.Format ("%.0f",(*iLink)->m_LaneCapacity*(*iLink)->m_NumberOfLanes  ); break;



				case link_display_link_type_in_text:
					str_text.Format ("%s",pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].link_type_name.c_str ()  ); break;

				case link_display_link_type_in_number:
					str_text.Format ("%d", (*iLink)->m_link_type  ); break;

				case link_display_main_node_id:

					str_text.Format("%s", (*iLink)->main_node_id.c_str()); 
					
					break;

				case link_display_NEMA_phase_number:
					str_text.Format("%s", (*iLink)->NEMA_phase_number.c_str()); 
					break;

				case link_display_movement_str:
					str_text.Format("%s", (*iLink)->movement_str.c_str()); break;


				case link_display_LevelOfService:
					str_text.Format ("%s",(*iLink)->m_LevelOfService    ); break;


				case link_display_avg_waiting_time_on_loading_buffer:
					if( (*iLink)->m_avg_waiting_time_on_loading_buffer > 0.1)
						str_text.Format ("%.1f",(*iLink)->m_avg_waiting_time_on_loading_buffer   ); break;


				case link_display_total_link_volume:
					str_text.Format("%.1f", (*iLink)->m_total_link_volume);

					break;

				case link_display_hourly_link_volume:
					str_text.Format("%.1f", (*iLink)->m_hourly_link_volume);
					break;

				case link_display_hourly_lane_volume:
					str_text.Format("%.1f", (*iLink)->m_total_link_volume / max(1, (*iLink)->m_NumberOfLanes));
					break;

				case link_display_avg_speed:
					str_text.Format("%.1f", (*iLink)->m_MeanSpeed);

					break;
					
				case link_display_avg_travel_time:  // min 
					str_text.Format("%.1f", (*iLink)->m_Length / max( 0.0001, (*iLink)->m_MeanSpeed)*60);

					break;

				case link_display_VOC_ratio:
					str_text.Format("%.2f", (*iLink)->m_VoCRatio);

					break;

				case link_display_total_delay:
					if((*iLink)->m_total_delay>=1 && pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsConnector () == false)
						str_text.Format ("%.1f",(*iLink)->m_total_delay/60.0   );

					break;


				break;

				case link_display_time_dependent_link_volume:

					pDoc->GetLinkMOE((*iLink), MOE_volume,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value);

					value = max(0,value);

					str_text.Format ("%.1f",value);
					break;

				case link_display_time_dependent_lane_volume:

					pDoc->GetLinkMOE((*iLink), MOE_volume,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value);
					str_text.Format ("%.0f",value/max(1,(*iLink)->m_NumberOfLanes) );
					break;



				case link_display_time_dependent_speed_mph:
					pDoc->GetLinkMOE((*iLink), MOE_speed,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value);
					str_text.Format ("%.1f",value);
					break;

				case link_display_time_dependent_density:
					pDoc->GetLinkMOE((*iLink), MOE_density,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value);

					if(value > 0.1)
					{
						str_text.Format ("%.0f",value);
					}
					break;

				case link_display_time_dependent_queue_length:
					pDoc->GetLinkMOE((*iLink), MOE_queue_length,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value);

					if(value>0.01)
					{
						str_text.Format ("%.1f%%",value*100);
					}
					break;


				case link_display_time_dependent_congestion_duration:
					value = (*iLink)->GetImpactDuration ((int)g_Simulation_Time_Stamp);

					if(value>0.01)
					{
						str_text.Format ("%.0f min",value);
					}
					break;

				case link_display_time_dependent_congestion_start_time:
					value = (*iLink)->GetImpactRelativeStartTime  ((int)g_Simulation_Time_Stamp );

					if(value <9999)
					{
						str_text.Format ("%s", pDoc->GetTimeStampString24HourFormat(g_Simulation_Time_Stamp+value));
					}
					break;

					with_text = true;
				}


				// after all the above 3 conditions, show test now. 
				if(with_text)
				{
					CPoint TextPoint = NPtoSP((*iLink)->GetRelativePosition(0.3));
					pDC->SetBkColor(RGB(0,0, 0));

					if((*iLink)->m_LinkNo == pDoc->m_SelectedLinkNo)
					{
						pDC->SetTextColor(RGB(255,0,0));
					}else
					{
						pDC->SetTextColor(RGB(0,0,0));
					}


					pDC->TextOut(TextPoint.x,TextPoint.y - m_LinkTextFontSize*1.5, str_text);

					if(str_reference_text.GetLength () > 0 )  // reference text with different color4
					{
						pDC->SetTextColor(RGB(128, 255, 255));
						pDC->TextOut(TextPoint.x,TextPoint.y+m_LinkTextFontSize, str_reference_text);
					}

				}

				// step 8:  show location of scenario/incident /work zone/ toll: link-based objects
				pDC->SetBkColor(RGB(0, 0, 0));

				//************************************

				// step 9: draw sensor (based on a link)
			}
		}
	}

	if(pMainFrame->m_bShowLayerMap[layer_link_MOE] == true && (theApp.m_VisulizationTemplate == e_train_scheduling))
	{	std::list<DTALink*>::iterator iLink;

	}


	pDC->SelectObject(&node_font);

	// step 10: draw shortest path
	int i;
	/*
	unsigned int iPath;

	if(m_ShowAllPaths)
	{
	for (iPath = 0; iPath < pDoc->m_PathDisplayList.size(); iPath++)
	{
	g_SelectThickPenColor(pDC,0);

	for (i=0 ; i<pDoc->m_PathDisplayList[iPath].m_LinkSize; i++)
	{
	DTALink* pLink = pDoc->m_LinkNoMap[pDoc->m_PathDisplayList[iPath].m_LinkVector[i]];
	if(pLink!=NULL)
	{
	CPoint FromPoint = NPtoSP(pLink->m_FromPoint);
	CPoint ToPoint = NPtoSP(pLink->m_ToPoint);

	if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
	continue; 

	pDC->MoveTo(FromPoint);
	pDC->LineTo(ToPoint);

	}
	}
	}
	}

	*/
	//draw select path
	if (pMainFrame->m_bShowLayerMap[layer_path] && m_PathNodeSequence.GetLength() >0)
	{

		std::vector<int> path_node_sequence;
		std::string str_line = pDoc->CString2StdString(m_PathNodeSequence);
		path_node_sequence = pDoc->ParseLineToIntegers(str_line);

		pDC->SelectObject(&g_WhiteBrush);


		//with Agent location data at this time stamp
		for (int i = 0; i < path_node_sequence.size() - 1; i++)
		{


			DTANode* pFromNode = pDoc->FindNodeWithNodeID(path_node_sequence[i]);
			DTANode* pToNode = pDoc->FindNodeWithNodeID(path_node_sequence[i + 1]);
			if (pFromNode != NULL&& pToNode != NULL)
			{

				CPoint from_point = NPtoSP(pFromNode->pt);
				CPoint to_point = NPtoSP(pToNode->pt);

				DTALink* pLink = pDoc->FindLinkWithNodeIDs(path_node_sequence[i], path_node_sequence[i + 1]);

				if (pLink!=NULL)
					g_SelectColorCode(pDC, 1);
				else
					g_SelectColorCode2(pDC, 1);




				node_size = 5;
				/// starting drawing nodes in normal mode
				pDC->Ellipse(from_point.x - node_size, from_point.y + node_size,
					from_point.x + node_size, from_point.y - node_size);

				pDC->MoveTo(from_point);

				pDC->LineTo(to_point);

				CString str_text;

				GDPoint text_pt;
				text_pt.x = (from_point.x + to_point.x) / 2;
				text_pt.y = (from_point.y + to_point.y) / 2;

				str_text.Format("pn=%d", i+1);

				CPoint screen_point = NPtoSP(text_pt);
				pDC->TextOut(screen_point.x, screen_point.y, str_text);

				CPoint FromPoint = from_point;
				CPoint ToPoint = to_point;

				double slopy = atan2((double)(FromPoint.y - ToPoint.y), (double)(FromPoint.x - ToPoint.x));
				double cosy = cos(slopy);
				double siny = sin(slopy);
				double display_length = sqrt((double)(FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y) + (double)(FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x));
				double arrow_size = min(7, display_length / 5.0);

				if (arrow_size > 0.2)
				{

					m_arrow_pts[0] = ToPoint;
					m_arrow_pts[1].x = ToPoint.x + (int)(arrow_size * cosy - (arrow_size / 2.0 * siny) + 0.5);
					m_arrow_pts[1].y = ToPoint.y + (int)(arrow_size * siny + (arrow_size / 2.0 * cosy) + 0.5);
					m_arrow_pts[2].x = ToPoint.x + (int)(arrow_size * cosy + arrow_size / 2.0 * siny + 0.5);
					m_arrow_pts[2].y = ToPoint.y - (int)(arrow_size / 2.0 * cosy - arrow_size * siny + 0.5);

					pDC->Polygon(m_arrow_pts, 3);
				}

			}



		}
	}



	if(pMainFrame->m_bShowLayerMap[layer_path] && pDoc->m_PathDisplayList.size() > pDoc->m_SelectPathNo && pDoc->m_SelectPathNo!=-1)
	{


		for (i=0 ; i<pDoc->m_PathDisplayList[pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{
			DTALink* pLink = pDoc->m_LinkNoMap[pDoc->m_PathDisplayList[pDoc->m_SelectPathNo].m_LinkVector[i]];
			if(pLink!=NULL)
			{
				if(i==0) //
				{
					pDoc->m_ONodeNo = pLink->m_FromNodeNo ;
				}
				if(i== pDoc->m_PathDisplayList[pDoc->m_SelectPathNo].m_LinkVector.size()-1)
				{
					pDoc->m_DNodeNo =  pLink->m_ToNodeNo ;
				}

				// select link color 
				if(pLink->m_LinkNo == pDoc->m_SelectedLinkNo)
				{
					g_SelectThickPenColor(pDC,0);
					pDC->SetTextColor(RGB(255,0,0));
				}
				else
				{
					pDC->SelectObject(&g_PenSelectPath);
				}

				DrawLinkAsLine(pLink,pDC);

			}
		}
	}

	// step 11: select font for origin and destination  of the shortest path

	// step 11: all all nodes

	if(pMainFrame->m_bShowLayerMap[layer_node] == true)
	{
		CPoint point;
		float feet_size;

		CFont od_font;
		int nODNodeSize = max(node_size,10);


		CPen pen_node;
		CBrush brush_node;

		pen_node.CreatePen (PS_SOLID, 1, theApp.m_NodeColor);
		brush_node.CreateSolidBrush (theApp.m_NodeBrushColor);


		int nODFontSize =  max(nODNodeSize * NodeTypeSize, 10);

		m_NodeTextFontSize = nODFontSize; 

		od_font.CreatePointFont(nODFontSize, m_NodeTypeFaceName);

		for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
		{

			point = NPtoSP((*iNode)->pt);

			CRect node_rect;
			node_rect.SetRect(point.x-m_NodeDisplayBoundarySize,point.y-m_NodeDisplayBoundarySize,point.x+50,point.y+m_NodeDisplayBoundarySize);

			if(RectIsInsideScreen(node_rect,ScreenRect) == false)  // not inside the screen boundary
				continue;

			pDC->SelectObject(&brush_node);
			pDC->SetTextColor(RGB(255,255,0));
			pDC->SelectObject(&pen_node);
			pDC->SetBkColor(RGB(0,0,0));

			if((*iNode)->m_NodeNo == pDoc->m_SelectedNodeNo)
			{
				pDC->SelectObject(&g_PenSelectColor);

			}else if((*iNode)->m_ZoneID ==  pDoc->m_SelectedZoneID  && m_bHighlightActivityLocation)
			{
				pDC->SelectObject(&g_PenCentroidColor);
				pDC->SelectObject(&g_ActivityLocationBrush);

				if((*iNode)->m_External_OD_flag == 1)  // external origin 
				{

					pDC->SelectObject(&g_PenExternalOColor);

					pDC->SetTextColor(RGB(255,0,0));
					pDC->SetBkColor(RGB(0,0,0));

				}

				if((*iNode)->m_External_OD_flag == -1)  // external destination
				{
					pDC->SelectObject(&g_PenExternalDColor);
					pDC->SetTextColor(RGB(255,0,0));
					pDC->SetBkColor(RGB(0,0,0));


				}

			}else
			{

				//default;
				pDC->SelectObject(&pen_node);

				if((*iNode)->m_ControlType == pDoc->m_ControlType_YieldSign || 
					(*iNode)->m_ControlType == pDoc->m_ControlType_2wayStopSign ||
					(*iNode)->m_ControlType == pDoc->m_ControlType_4wayStopSign)
				{			
					pDC->SelectObject(&g_PenStopSignNodeColor);
				}
				if((*iNode)->m_ControlType == pDoc->m_ControlType_PretimedSignal || 
					(*iNode)->m_ControlType == pDoc->m_ControlType_ActuatedSignal)
				{
					pDC->SelectObject(&g_PenSignalNodeColor);
				}

			}

			if((*iNode)->m_NodeNo == pDoc->m_ONodeNo && pMainFrame->m_bShowLayerMap[layer_path] ==true)
			{

				CFont* oldFont = pDC->SelectObject(&od_font);
				pDC->SelectObject(&g_PenSelectColor);
				pDC->SelectObject(&g_BlackBrush);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->SetBkColor(RGB(0,0,0));

				TEXTMETRIC tmOD;
				memset(&tmOD, 0, sizeof TEXTMETRIC);
				pDC->GetOutputTextMetrics(&tmOD);

				pDC->Rectangle (point.x - nODNodeSize, point.y + nODNodeSize,
					point.x + nODNodeSize, point.y - nODNodeSize);

				point.y -= tmOD.tmHeight / 2;
				pDC->TextOut(point.x , point.y , _T("A"));

				pDC->SelectObject(oldFont);  // restore font



			}else if((*iNode)->m_NodeNo == pDoc->m_DNodeNo && pMainFrame->m_bShowLayerMap[layer_path] ==true)
			{
				CFont* oldFont = pDC->SelectObject(&od_font);// these are local font, created inside the function, we do not want to create them in another sub functions to speed up the display efficiency.

				TEXTMETRIC tmOD;
				memset(&tmOD, 0, sizeof TEXTMETRIC);
				pDC->GetOutputTextMetrics(&tmOD);

				pDC->SelectObject(&g_PenSelectColor);
				pDC->SelectObject(&g_BlackBrush);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->SetBkColor(RGB(0,0,0));

				pDC->Rectangle (point.x - nODNodeSize, point.y + nODNodeSize,
					point.x + nODNodeSize, point.y - nODNodeSize);

				point.y -= tmOD.tmHeight / 2;
				pDC->TextOut(point.x , point.y , _T("B"));

				pDC->SelectObject(oldFont);  // restore font


			}else if ((*iNode)->m_IntermediateDestinationNo>=1)
			{
				CFont* oldFont = pDC->SelectObject(&od_font);// these are local font, created inside the function, we do not want to create them in another sub functions to speed up the display efficiency.

				TEXTMETRIC tmOD;
				memset(&tmOD, 0, sizeof TEXTMETRIC);
				pDC->GetOutputTextMetrics(&tmOD);

				pDC->SelectObject(&g_PenSelectColor);
				pDC->SelectObject(&g_BlackBrush);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->SetBkColor(RGB(0,0,0));

				pDC->Rectangle (point.x - nODNodeSize, point.y + nODNodeSize,
					point.x + nODNodeSize, point.y - nODNodeSize);

				point.y -= tmOD.tmHeight / 2;
				CString str;
				str.Format ("I%d",(*iNode)->m_IntermediateDestinationNo);
				pDC->TextOut(point.x , point.y , str);

				pDC->SelectObject(oldFont);  // restore font

			}else
			{
				if(m_bShowNode)
				{

					if(m_ShowNodeTextMode == node_display_cycle_length_in_second_for_signal_only)
					{
						if( (*iNode)->m_ControlType != pDoc->m_ControlType_PretimedSignal &&  (*iNode)->m_ControlType != pDoc-> m_ControlType_ActuatedSignal)
						{

							continue;  // do not draw nodes;

						}

					}

					feet_size = pDoc->m_UnitDistance*m_Resolution;

					if((*iNode)->m_NodeNo == pDoc->m_SelectedNodeNo)
					{
						feet_size = max(50,min(100,feet_size*3));  // 

					}


					/// starting drawing nodes in normal mode

					if((*iNode)->m_DistanceToRoot > MAX_SPLABEL-1)
					{
						pDC->SelectObject(&g_PenSelectColor2_obs);

						feet_size = max(2,feet_size);  // show node anyway
					}


					if ((*iNode)->m_LayerNo == 1)
					{
						pDC->SelectObject(&g_PenReference);
						pDC->SelectObject(&g_BrushReference);
					}
					if(pDoc->m_LinkMOEMode == MOE_bottleneck)
					{

					int LOS = 0;
					float avg_delay = pDoc->GetNodeTotalDelay( (*iNode)->m_NodeID ,g_Simulation_Time_Stamp,LOS);

					if(avg_delay>1)
					{
					float size = avg_delay*pDoc->m_BottleneckDisplaySize;

					//if(size > 200)
					//	size = 200;

					pDC->SelectObject(&g_BlackPen);
					pDC->SelectObject(&brush_moe[LOS]);



					float avg_delay_ratio = avg_delay/max(1,size);

					if(g_bShowRadarChart==true)
						DrawNodeRadarChart(pDC, (*iNode),point, size, avg_delay_ratio, LOS);
					else
						DrawNodePieChart(pDC, (*iNode),point, size, LOS);

					}
					}
					else
					{
						DrawNode(pDC, (*iNode),point, node_size,tm);
					}

				}
			}

		}


	}

	if(pMainFrame->m_bShowLayerMap[layer_movement] == true || m_bShowProhibitedMovements)
	{
		CRect PlotRect;
		for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
		{
			CPoint point = NPtoSP((*iNode)->pt);

			CRect node_rect;
			node_rect.SetRect(point.x- m_NodeDisplayBoundarySize,point.y-m_NodeDisplayBoundarySize,point.x+ m_NodeDisplayBoundarySize,point.y+ m_NodeDisplayBoundarySize);

			if(RectIsInsideScreen(node_rect,ScreenRect) == false)  // not inside the screen boundary
				continue;



			DrawNodeMovements(pDC, (*iNode), PlotRect);
		}

	}

	//step 12: draw generic point layer , e.g. crashes

	if(pMainFrame->m_bShowLayerMap[layer_Trajectory])
	{
		pDC->SelectObject(&g_BlackBrush);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SelectObject(&g_GPSTrajectoryColor);
		pDC->SetBkColor(RGB(0,0,0));

		CFont GPS_font;  // local font for nodes. dynamically created. it is effective only inside this function. if you want to pass this font to the other function, we need to pass the corresponding font pointer (which has a lot of communication overheads)
		int node_size = min(200,max(2,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution)));

		int NodeTypeSize = pDoc->m_NodeTextDisplayRatio;
		int nFontSize =  max(node_size * NodeTypeSize, 10);

		GPS_font.CreatePointFont(nFontSize, m_NodeTypeFaceName);

		CFont* pOldFont = pDC->SelectObject(&node_font);

		
		long timestamp_in_second = (long)(g_Simulation_Time_Stamp*60 + 0.5); // *60 convert min to second, +0.5, round to nearest integer

			pDC->SelectObject(&g_WhiteBrush);


				//with Agent location data at this time stamp

				std::map<string,AgentLocationTimeIndexedMap>::iterator itr2;

				for(itr2 = pDoc->m_AgentWithLocationVectorMap.begin();
					itr2 != pDoc->m_AgentWithLocationVectorMap .end(); itr2++)
				{		//scan all Agent records at this timestamp

					string agent_id = (*itr2).first;

					if (m_bShowSelectedAgentOnly && agent_id != pDoc->m_SelectedAgentID)
						continue;

						
				for (int i = 0; i<(*itr2).second.AgentLocationRecordVector.size()-1; i++)
				{
					std::string agent_type = (*itr2).second.AgentLocationRecordVector[i].agent_type;


					CPoint from_point, to_point;

					GDPoint from_pt;
					from_pt.x = (*itr2).second.AgentLocationRecordVector[i].x;
					from_pt.y = (*itr2).second.AgentLocationRecordVector[i].y;

					int current_second = (*itr2).second.AgentLocationRecordVector[i].time_stamp_in_second;



					GDPoint to_pt;
					to_pt.x = (*itr2).second.AgentLocationRecordVector[i+1].x;
					to_pt.y = (*itr2).second.AgentLocationRecordVector[i+1].y;

					from_point = NPtoSP(from_pt);
					 to_point = NPtoSP(to_pt);

				 /// starting drawing nodes in normal mode
					 pDC->Ellipse(from_point.x - node_size, from_point.y + node_size,
						 from_point.x + node_size, from_point.y - node_size);
					pDC->MoveTo(from_point);
					pDC->LineTo (to_point);

				
				}

				}
	
		
		
	



		pDC->SelectObject(&g_BlackBrush);
		pDC->SelectObject(&g_GPSTrajectoryColor);



		pDC->SelectObject(pOldFont);
	}


	// step 13: draw zone layer

	CPen ZonePen;

	// Create a solid red pen of width 2.
	ZonePen.CreatePen(PS_DASH, 1, theApp.m_ZoneColor);

	if(pMainFrame->m_bShowLayerMap[layer_zone])
	{	

		CFont zone_font;  // local font for nodes. dynamically created. it is effective only inside this function. if you want to pass this font to the other function, we need to pass the corresponding font pointer (which has a lot of communication overheads)
		zone_font.CreatePointFont(nFontSize*5, m_NodeTypeFaceName);

		pDC->SelectObject(&zone_font);

		std::map<int, DTAZone>	:: const_iterator itr;


		pDC->SetTextColor(pDoc->m_ZoneTextColor);


		for(int draw_flag = 0; draw_flag<=1; draw_flag++)
		{
			for(itr = pDoc->m_ZoneMap.begin(); itr != pDoc->m_ZoneMap.end(); itr++)
			{
				if(draw_flag==1 && itr->first != pDoc->m_SelectedZoneID) 
				{  // draw the selected zone only when draw_flag = 1
					continue;
				}



				if(itr->first == pDoc->m_SelectedZoneID )
				{
					pDC->SelectObject(&g_PenSelectColor0);
				}
				else
				{
					pDC->SelectObject(&ZonePen);
				}

				int center_x = 0;
				int center_y = 0;

				if(itr->second.m_ShapePoints .size() > 0)
				{

					int min_x, max_x, min_y, max_y;

					for (int i = 0; i < itr->second.m_ShapePoints.size(); i++)
					{
						CPoint point = NPtoSP(itr->second.m_ShapePoints[i]);

						if (i == 0)
						{
							min_x = max_x = point.x;
							min_y = max_y = point.y;

						}

						min_x = min(point.x, min_x);
						max_x = max(point.x, max_x);

						min_y = min(point.y, min_y);
						max_y = max(point.y, max_y);


					}

					int size = 100;
					CRect zone_rect;
					zone_rect.SetRect(min_x - size, min_y - size, max_x + size, max_y + size);


					if (RectIsInsideScreen(zone_rect, ScreenRect) == false)  // zone not inside the screen boundary: do not draw
						continue;


					for(int i = 0; i< itr->second.m_ShapePoints .size(); i++)
					{

						CPoint point =  NPtoSP(itr->second.m_ShapePoints[i]);

						center_x += point.x;
						center_y += point.y;

						if(i == 0)
							pDC->MoveTo(point);
						else
							pDC->LineTo(point);

					}



					CPoint point_0 =  NPtoSP(itr->second.m_ShapePoints[0]);  // back to the starting point

					pDC->LineTo(point_0);

					center_x  = center_x/max(1,itr->second.m_ShapePoints .size());
					center_y  = center_y/max(1,itr->second.m_ShapePoints .size()) - tm.tmHeight;

					if (zone_rect.Height() >= 10 || zone_rect.Width()>=10)
					{
						CString zone_id_str;
						zone_id_str.Format("%s", itr->second.name.c_str());

						pDC->TextOut(center_x, center_y, zone_id_str);
					}
				}

			}
		}

	}	


	// step 13: draw subarea layer
	if(GetDocument()->m_SubareaShapePoints.size() > 0 && (pMainFrame->m_bShowLayerMap[layer_subarea]||pMainFrame->m_bShowLayerMap[layer_zone]))
	{
		CPoint point_0  = NPtoSP(GetDocument()->m_SubareaShapePoints[0]);


		pDC->SelectObject(&g_PenDashBlue);


		pDC->MoveTo(point_0);

		for (int sub_i= 0; sub_i < GetDocument()->m_SubareaShapePoints.size(); sub_i++)
		{
			CPoint point =  NPtoSP(GetDocument()->m_SubareaShapePoints[sub_i]);
			pDC->LineTo(point);

		}
		if(isFinishSubarea)
			pDC->LineTo(point_0);

	}
	// step 14: show not-matched sensors
	// step 15: show Agents

	if( pDoc->m_LinkMOEMode == MOE_Agent)
	{
		CBrush brush_agent[5];
		brush_agent[1].CreateSolidBrush(theApp.m_AgentColor[1]);
		brush_agent[2].CreateSolidBrush(theApp.m_AgentColor[2]);
		brush_agent[3].CreateSolidBrush(theApp.m_AgentColor[3]);
		brush_agent[4].CreateSolidBrush(theApp.m_AgentColor[4]);


		CFont Agent_font;  // local font for nodes. dynamically created. it is effective only inside this function. if you want to pass this font to the other function, we need to pass the corresponding font pointer (which has a lot of communication overheads)

		int Agent_size = min(30,max(5,int(pDoc->m_AgentDisplaySize *pDoc->m_UnitDistance*m_Resolution)));

		int NodeTypeSize = pDoc->m_NodeTextDisplayRatio;
		int nFontSize =  max(Agent_size * NodeTypeSize*0.8, 10);

		Agent_font.CreatePointFont(nFontSize, m_NodeTypeFaceName);

		pDC->SelectObject(&Agent_font);


		TEXTMETRIC tm_Agent;
		memset(&tm_Agent, 0, sizeof TEXTMETRIC);
		pDC->GetOutputTextMetrics(&tm_Agent);

		pDC->SelectObject(&g_PenAgent);  //green
		pDC->SelectObject(&brush_agent[1]); //green

		


		int Agent_set_size  = pDoc->m_AgentSet.size();

		bool bSamplingDisplay = false;
		int sample_size  = 1;
		if(Agent_set_size>500000)
		{ // more than 0.5 millon Agents
			bSamplingDisplay = true;
			sample_size = Agent_set_size/500000;
		}

		std::list<DTAAgent*>::iterator iAgent;
		for (iAgent = pDoc->m_AgentSet.begin(); iAgent != pDoc->m_AgentSet.end(); iAgent++)
		{

			if((*iAgent)->m_DepartureTime <=g_Simulation_Time_Stamp &&
				g_Simulation_Time_Stamp <=(*iAgent)->m_ArrivalTime && (*iAgent)->m_NodeSize>=2)
			{

				if(bShowAgentsWithIncompleteTrips && (*iAgent)->m_bComplete)  // show incomplete Agents only
					continue;

				if(m_bShowSelectedAgentOnly && (*iAgent)->m_AgentID != pDoc->m_SelectedAgentID)  // show selected Agent only
					continue;


				float ratio = 0;

					int link_sequence_no;
					int LinkID = pDoc->GetVehilePosition((*iAgent), g_Simulation_Time_Stamp, link_sequence_no, ratio);

					DTALink* pLink = pDoc->m_LinkNoMap[LinkID];
					if(pLink!=NULL)
					{

						CPoint VehPoint= NPtoSP(pLink->GetRelativePosition(ratio));
						if((*iAgent)->m_AgentID == pDoc->m_SelectedAgentID)
						{
							pDC->SelectObject(&g_PenSelectedAgent); //red transit users
							pDC->Ellipse (VehPoint.x - Agent_size*3, VehPoint.y - Agent_size*3,
								VehPoint.x + Agent_size*3, VehPoint.y + Agent_size*3);
							pDC->SelectObject(&g_PenAgent);  //green

						}else
						{ 

							if (!m_bShowLinkArrow)
							{
								pDC->Ellipse(VehPoint.x - Agent_size, VehPoint.y - Agent_size,
									VehPoint.x + Agent_size, VehPoint.y + Agent_size);
							}
							else
							{

								FromPoint = NPtoSP(pLink->m_FromPoint);
								ToPoint = NPtoSP(pLink->m_ToPoint);

								double slopy = atan2((double)(FromPoint.y - ToPoint.y), (double)(FromPoint.x - ToPoint.x));
								double cosy = cos(slopy);
								double siny = sin(slopy);
								double display_length = sqrt((double)(FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y) + (double)(FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x));
								double arrow_size = Agent_size;

								ToPoint = VehPoint;  // overwrite the position
								if (arrow_size > 0.2)
								{

									m_arrow_pts[0] = ToPoint;
									m_arrow_pts[1].x = ToPoint.x + (int)(arrow_size * cosy - (arrow_size / 3.0 * siny) + 0.5);
									m_arrow_pts[1].y = ToPoint.y + (int)(arrow_size * siny + (arrow_size / 3.0 * cosy) + 0.5);
									m_arrow_pts[2].x = ToPoint.x + (int)(arrow_size * cosy + arrow_size / 3.0 * siny + 0.5);
									m_arrow_pts[2].y = ToPoint.y - (int)(arrow_size / 3.0 * cosy - arrow_size * siny + 0.5);

									pDC->Polygon(m_arrow_pts, 3);
								}
							}
						}
						if(m_bShowAgentNumber)
						{

							CString str_number;
							str_number.Format ("%s",(*iAgent)->m_AgentID.c_str()  );

							VehPoint.y -= tm_Agent.tmHeight / 2;

							pDC->TextOut(VehPoint.x , VehPoint.y,str_number);
						}
					}


					//GDPoint pt;
					//if(pDoc->GetAgentPosition((*iAgent), g_Simulation_Time_Stamp, pt) == false)
					//	continue;

					//CPoint VehPoint= NPtoSP(pt);
					//if((*iAgent)->m_AgentID == pDoc->m_SelectedAgentID)
					//{
					//	pDC->SelectObject(&g_PenSelectedAgent); //red transit users
					//	pDC->Ellipse (VehPoint.x - Agent_size*3, VehPoint.y - Agent_size*3,
					//		VehPoint.x + Agent_size*3, VehPoint.y + Agent_size*3);
					//	pDC->SelectObject(&g_PenAgent);  //green

					//}else
					//{ 

					//	pDC->Ellipse (VehPoint.x - Agent_size, VehPoint.y - Agent_size,
					//		VehPoint.x + Agent_size, VehPoint.y + Agent_size);

					//}

					//if(m_bShowAgentNumber)
					//{

					//	CString str_number;
					//	str_number.Format ("%d",(*iAgent)->m_AgentID  );

					//	VehPoint.y -= tm_Agent.tmHeight / 2;

					//	pDC->TextOut(VehPoint.x , VehPoint.y,str_number);
					//}


		

			}

		}
	}

		if( pDoc->m_LinkMOEMode == MOE_Agent)
	{

			CFont pt_font;
			int nODNodeSize = max(node_size*0.8, 8);
			int nODFontSize = max(nODNodeSize * NodeTypeSize, 8);

			pt_font.CreatePointFont(nODFontSize, m_NodeTypeFaceName);

			CFont* oldFont = pDC->SelectObject(&pt_font);

		pDC->SelectObject(&g_PenAgent);  //green
		pDC->SelectObject(&g_BrushAgent); //green
		std::map<string, AgentLocationTimeIndexedMap>::iterator itr2;

		for (itr2 = pDoc->m_AgentWithLocationVectorMap.begin();
			itr2 != pDoc->m_AgentWithLocationVectorMap.end(); itr2++)
		{		//scan all Agent records at this timestamp

			std::vector<AgentLocationRecord>::iterator itr;


			//if (bSamplingDisplay)
			//{
			//	if((*iAgent)->m_AgentID %sample_size !=0)
			//		continue;
			//}

			//if((*iAgent)->m_DayNo != g_SimulatedLastDayNo)
			//	continue;


				//if(bShowAgentsWithIncompleteTrips && (*iAgent)->m_bComplete)  // show incomplete Agents only
				//	continue;

				//if(m_bShowSelectedAgentOnly && (*iAgent)->m_AgentID != pDoc->m_SelectedAgentID)  // show selected Agent only
				//	continue;


				//if((*iAgent)->m_PricingType ==4)  // transit user
				//	pDC->SelectObject(&g_BrushTransitUser); //red transit users

			GDPoint pt;


			if (pDoc->GetAgentPosition((*itr2).first, g_Simulation_Time_Stamp, pt))
			{
						CPoint AgentPoint = NPtoSP(pt);

						//if((*iAgent)->m_AgentID == pDoc->m_SelectedAgentID)
						//{
						//	pDC->SelectObject(&g_PenSelectedAgent); //red transit users
						//	pDC->Ellipse (VehPoint.x - Agent_size*3, VehPoint.y - Agent_size*3,
						//		VehPoint.x + Agent_size*3, VehPoint.y + Agent_size*3);
						//	pDC->SelectObject(&g_PenAgent);  //green

						//}else
						//{ 

						DrawBitmap(pDC, AgentPoint, IDB_BUSMOVING);


							//pDC->Ellipse (VehPoint.x - Agent_size, VehPoint.y - Agent_size,
							//	VehPoint.x + Agent_size, VehPoint.y + Agent_size);

						//}
							CString str_number;
							str_number.Format("%s", (*itr2).first.c_str());
							AgentPoint.y -=15;
							pDC->TextOut(AgentPoint.x, AgentPoint.y, str_number);
			
				}



		}
	}
	//////////////////////////////////////
	// step 16: draw OD demand

	if(pMainFrame->m_bShowLayerMap[layer_ODMatrix])
	{
		float MaxODDemand = 10;

		int  p = 0; 
		int i,j;


		bool bODDataReady = false;
		if(pDoc->m_OD_data_vector.size()>0)
		{
			bODDataReady = true;
		}

		std::map<int, DTAZone>	::iterator itr_o;
		std::map<int, DTAZone>	::iterator itr_d;



		for (itr_o = pDoc->m_ZoneMap.begin(); itr_o != pDoc->m_ZoneMap.end(); itr_o++)  // for each origin
		{

			for (itr_d = pDoc->m_ZoneMap.begin(); itr_d != pDoc->m_ZoneMap.end(); itr_d++)
			{

				CString label;
				label.Format("%d,%d", itr_o->first, itr_d->first);

				if (pDoc->m_ODMatrixMap.find(label) != pDoc->m_ODMatrixMap.end())
				{

					if (bODDataReady == false && pDoc->m_ODMatrixMap[label].demand_volume > 0.01 && itr_o->first != itr_d->first)
					{
						pDoc->m_OD_data_vector.push_back(pDoc->m_ODMatrixMap[label].demand_volume);

						if (pDoc->m_ODMatrixMap[label].demand_volume > MaxODDemand)
							MaxODDemand = pDoc->m_ODMatrixMap[label].demand_volume;

					}

				}
			}
		}

			int threashold_200 = 100;   // width: 1
			int threashold_100 = 100;  //width: 2
			int threashold_50 = 100;  //width: 3
			int threashold_20 = 100;  //width: 4
			int threashold_10 = 100;  //width: 5

			if(bODDataReady == false)
			{
				sort(pDoc->m_OD_data_vector.begin(), pDoc->m_OD_data_vector.end());
			}

			if(pDoc->m_OD_data_vector.size()>200)
			{
				int size  = pDoc->m_OD_data_vector.size();
				threashold_200 = pDoc->m_OD_data_vector[size-200];

				threashold_100 = pDoc->m_OD_data_vector[size- 100];
				threashold_50 = pDoc->m_OD_data_vector[size- 50];
				threashold_20 = pDoc->m_OD_data_vector[size- 20];
				threashold_10 = pDoc->m_OD_data_vector[size- 10];
			}

			// threashold is 100th largest number

			for (itr_o = pDoc->m_ZoneMap.begin(); itr_o != pDoc->m_ZoneMap.end(); itr_o++)  // for each origin
			{

				for (itr_d = pDoc->m_ZoneMap.begin(); itr_d != pDoc->m_ZoneMap.end(); itr_d++)
				{

						CString label;
						label.Format("%d,%d", itr_o->first, itr_d->first);

						if (pDoc->m_ODMatrixMap.find(label) != pDoc->m_ODMatrixMap.end())
						{
							CPoint FromPoint = NPtoSP(itr_o->second.GetCenter ());
							CPoint ToPoint = NPtoSP(itr_d->second.GetCenter());

							CPen penmoe;
							double value = pDoc->m_ODMatrixMap[label].demand_volume;
							float Width = value / max(0.00001,MaxODDemand) * 10;

							if (m_bShowTop10ODOnly == false)
							{
								if (value >= threashold_200)
									Width = max(1.1, Width);  // show the line for 200th largest value

								if (value >= threashold_100)
									Width = max(Width, 2);

								if (value >= threashold_50)
									Width = max(Width, 3);  // show the line for 50th largest value

								if (value >= threashold_20)
									Width = max(Width, 4);  // show the line for 50th largest value
							}
							if (value >= threashold_10)
								Width = max(Width, 5);  // show the line for 50th largest value

							if(Width>50)
								Width = 50;  // show the line for 50th largest value

							if (Width >= 1)  //draw critical OD demand only
							{

								penmoe.CreatePen(PS_SOLID, (int)(Width), RGB(0, 255, 255));
								pDC->SelectObject(&penmoe);
								pDC->MoveTo(FromPoint);
								pDC->LineTo(ToPoint);
							}
						}

					}
				}

			
			CString label;
			label.Format("%d,%d", pDoc->m_SelectedOZoneID, pDoc->m_SelectedDZoneID);

			if (pDoc->m_ODMatrixMap.find(label) != pDoc->m_ODMatrixMap.end())
			{
				if (pDoc->m_ZoneMap.find(pDoc->m_SelectedOZoneID) != pDoc->m_ZoneMap.end() &&
					pDoc->m_ZoneMap.find(pDoc->m_SelectedDZoneID) != pDoc->m_ZoneMap.end())
				{
				
				CPoint FromPoint = NPtoSP(pDoc->m_ZoneMap[pDoc->m_SelectedOZoneID].GetCenter());
				CPoint ToPoint = NPtoSP(pDoc->m_ZoneMap[pDoc->m_SelectedDZoneID].GetCenter());

				CPen penmoe;
				double value = pDoc->m_ODMatrixMap[label].demand_volume;
				float Width = value / max(0.00001, MaxODDemand) * 10;

				if (m_bShowTop10ODOnly == false)
				{
					if (value >= threashold_200)
						Width = max(1.1, Width);  // show the line for 200th largest value

					if (value >= threashold_100)
						Width = max(Width, 2);

					if (value >= threashold_50)
						Width = max(Width, 3);  // show the line for 50th largest value

					if (value >= threashold_20)
						Width = max(Width, 4);  // show the line for 50th largest value
				}
				if (value >= threashold_10)
					Width = max(Width, 5);  // show the line for 50th largest value

				if (Width > 50)
					Width = 50;  // show the line for 50th largest value

				penmoe.CreatePen(PS_SOLID, (int)(Width), RGB(255, 0, 0));
				pDC->SelectObject(&penmoe);
				pDC->MoveTo(FromPoint);
				pDC->LineTo(ToPoint);
				}
			}


	}


	//if(pDoc->m_ZoneMap.find(pDoc->m_CriticalOriginZone)!= pDoc->m_ZoneMap.end() 
	//	&& pDoc->m_ZoneMap.find(pDoc->m_CriticalDestinationZone)!= pDoc->m_ZoneMap.end())
	//{
	//	CPoint FromPoint = NPtoSP(pDoc->m_ZoneMap[pDoc->m_CriticalOriginZone].GetCenter());
	//	CPoint ToPoint = NPtoSP(pDoc->m_ZoneMap[pDoc->m_CriticalDestinationZone].GetCenter());

	//	CPen penmoe;
	//	float Width = 5;

	//	if(Width>=0.2)  //draw critical OD demand only
	//	{
	//		penmoe.CreatePen (PS_SOLID, (int)(Width), RGB(0,255,255));
	//		pDC->SelectObject(&penmoe);
	//		pDC->MoveTo(FromPoint);
	//		pDC->LineTo(ToPoint);
	//	}

	//}


	// step 17: draw Public Transit Layer

	if(pMainFrame->m_bShowLayerMap[layer_transit])
	{
		CFont pt_font;
		int nODNodeSize = max(node_size*0.8,8);
		int nODFontSize =  max(nODNodeSize * NodeTypeSize, 8);

		pt_font.CreatePointFont(nODFontSize, m_NodeTypeFaceName);

		CFont* oldFont = pDC->SelectObject(&pt_font);

		pDC->SetTextColor(RGB(255,0,0));
		pDC->SetBkColor(RGB(0,0,0));

		if(m_bShowTransitAccessibility)
		{
			pDC->SelectObject(&g_TransitLightBrush);
			pDC->SelectObject(&g_TransitLightPen);

		}
		else
		{
			pDC->SelectObject(&g_TransitBrush);
			pDC->SelectObject(&g_TransitPen);
		}
		DrawPublicTransitLayer(pDC);
		pDC->SelectObject(oldFont);  // restore font

	}

	if(m_bShowTransitAccessibility)
	{
		CFont pt_font;
		int nODNodeSize = max(node_size*0.8,8);
		int nODFontSize =  max(nODNodeSize * NodeTypeSize, 8);

		pt_font.CreatePointFont(nODFontSize, m_NodeTypeFaceName);

		CFont* oldFont = pDC->SelectObject(&pt_font);

		pDC->SetTextColor(RGB(255,0,0));
		pDC->SetBkColor(RGB(0,0,0));

		pDC->SelectObject(&g_TransitAccessibilityPen);
		pDC->SelectObject(&g_TransitAccessibilityBrush);

		DrawPublicTransitAccessibilityLayer(pDC);
		pDC->SelectObject(oldFont);  // restore font

	}


}

void CTLiteView::FitNetworkToScreen()
{
	CRect ScreenRect;
	GetClientRect(ScreenRect);
	m_ScreenOrigin = ScreenRect.CenterPoint ();

	CTLiteDoc* pDoc = GetDocument();

	if(pDoc==NULL)
		return;

	double res_wid = ScreenRect.Width()/(pDoc->m_NetworkRect.Width()+0.000001);
	double res_height = ScreenRect.Height()/(pDoc->m_NetworkRect.Height()+0.000001);

	m_Resolution = min(res_wid, res_height);  

	//+1 to avide devided by zero error;

	m_Resolution*=0.8f;

	m_Origin.x = pDoc->m_NetworkRect.Center ().x ;
	m_Origin.y = pDoc->m_NetworkRect.Center ().y ;

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();

	//	TRACE("Wid: %d, %f, m_Resolution=%f,m_Origin: x = %f\n",ScreenRect.Width(),pDoc->m_NetworkRect.Width(), m_Resolution, m_Origin.x);

}


// CTLiteView message handlers

void CTLiteView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	OnViewShownetwork();
}


BOOL CTLiteView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// change image size
	CTLiteDoc* pDoc = GetDocument();

	if(m_ToolMode != backgroundimage_tool && m_ToolMode !=  network_coordinate_tool)  //select, move
	{
		if(zDelta > 0)
		{
			if(m_Resolution*1.6 * max(pDoc->m_NetworkRect.Width () , pDoc->m_NetworkRect.Height  () ) <  INT_MAX) 
			{
				m_Resolution*=1.5f;
			}
		}
		else
		{
			m_Resolution/=1.5f;
		}
	}else
	{

		if(m_ToolMode == network_coordinate_tool)
		{

			int LayerNo = 1;

			// change image size
			CTLiteDoc* pDoc = GetDocument();
			float increase_ratio = 1.005f;
			float decrease_ratio = 1.0f / increase_ratio;

			if(zDelta > 0)
			{

				// control -> Y only
				// shift -> X only



				if(nFlags != MK_CONTROL)  // shift or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,increase_ratio,1.0f,0.0f,0.0f);
				}

				if(nFlags != MK_SHIFT)  // control or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,1.0f,increase_ratio,0.0f,0.0f);
				}
			}
			else
			{
				if(nFlags != MK_CONTROL)  // shift or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,decrease_ratio,1.0f,0.0f,0.0f);
				}

				if(nFlags != MK_SHIFT)  // control or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,1.0f,decrease_ratio,0.0f,0.0f);
				}
			}
		}


		if(m_ToolMode == backgroundimage_tool)
		{

			if(zDelta > 0)
			{

				// control -> Y only
				// shift -> X only

				if(nFlags != MK_CONTROL)  // shift or nothing
					pDoc->m_ImageXResolution*=1.02f;

				if(nFlags != MK_SHIFT)  // control or nothing
					pDoc->m_ImageYResolution*=1.02f;
			}
			else
			{
				if(nFlags != MK_CONTROL)  // shift or nothing
					pDoc->m_ImageXResolution/=1.02f;

				if(nFlags != MK_SHIFT)  // control or nothing
					pDoc->m_ImageYResolution/=1.02f;
			}
		}
	}
	SetGlobalViewParameters();
	CRect ScreenRect;
	GetClientRect(ScreenRect);

	CPoint LeftTop, RightBottom;
	LeftTop.x = ScreenRect.left ;
	LeftTop.y = ScreenRect.top  ;

	RightBottom.x = ScreenRect.right ;
	RightBottom.y = ScreenRect.bottom ;

	GDPoint  gdpt_lt  = SPtoNP(LeftTop);
	GDPoint  gdpt_rb  = SPtoNP(RightBottom);

	double width = fabs(gdpt_lt.x - gdpt_rb.x )/ pDoc->m_UnitDistance;
	double height = fabs(gdpt_lt.y - gdpt_rb.y )/ pDoc->m_UnitDistance;

	CString str;

	GDPoint  gdpt = SPtoNP(pt);
	str.Format("%.5f,%.5f", gdpt.x, gdpt.y);
	pDoc->SendTexttoStatusBar(str);

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();
	Invalidate();

	return TRUE;
}

BOOL CTLiteView::OnEraseBkgnd(CDC* pDC)
{

	return TRUE;
}

void CTLiteView::OnViewZoomin()
{

	CTLiteDoc* pDoc = GetDocument();

	if(m_Resolution*1.2 * max(pDoc->m_NetworkRect.Width () , pDoc->m_NetworkRect.Height  () ) <  INT_MAX) 
	{
		m_Resolution*=1.1f;

	}

	SetGlobalViewParameters();

	Invalidate();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();

}

void CTLiteView::OnViewZoomout()
{
	m_Resolution/=1.1f;

	SetGlobalViewParameters();
	Invalidate();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();
}

void CTLiteView::OnViewShownetwork()
{
	FitNetworkToScreen();

	Invalidate();
}

void CTLiteView::OnViewMove()
{
	m_ToolMode = move_tool;
}

int CTLiteView::FindClosestNode(CPoint point, float Min_screen_distance)
{
	int SelectedNodeNo = -1;
	CTLiteDoc* pDoc = GetDocument();

	std::list<DTANode*>::iterator iNode;

	for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
	{
		CPoint NodePoint = NPtoSP((*iNode)->pt);

		CSize size = NodePoint - point;
		double distance = pow((size.cx*size.cx + size.cy*size.cy),0.5);
		if( distance < Min_screen_distance)
		{
			SelectedNodeNo = (*iNode)->m_NodeNo ;
			Min_screen_distance = distance;
		}

	}

	return SelectedNodeNo;
}

int CTLiteView::FindClosestZone(CPoint point, float Min_distance)
{
	CTLiteDoc* pDoc = GetDocument();
	if(pDoc !=NULL)
	{
		GDPoint pt = SPtoNP (point);
		int ZoneID = pDoc->GetZoneID(pt);

		if(ZoneID > 0)
			return ZoneID;

	}
	return -1;
}
void CTLiteView::OnLButtonDown(UINT nFlags, CPoint point)
{

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	//speical condition first

	CTLiteDoc* pDoc = GetDocument();

	if (m_ToolMode == move_tool || (m_ToolMode == move_node_tool && pDoc->m_SelectedNodeNo <= -1))
	{
		m_last_cpoint = point;
		m_last_left_down_point  = point;
		g_SetCursor(_cursor_movement_network);
		m_bMoveDisplay = true;
	}



	if(m_ToolMode == select_feature_tool || m_ToolMode == select_link_tool  )
	{
		m_last_cpoint = point;
		m_last_left_down_point  = point;
		m_bMoveDisplay = true;
	}

	if(m_ToolMode == backgroundimage_tool)
	{
		m_last_cpoint = point;
		g_SetCursor(_cursor_movement_network);
		m_bMoveImage = true;
	}

	if (m_ToolMode == sensor_location_move_tool)
	{
		m_last_cpoint = point;
		g_SetCursor(_cursor_movement_network);
		m_bMoveSensorMap = true;
	}
	if(m_ToolMode == network_coordinate_tool)
	{
		m_last_cpoint = point;
		g_SetCursor(_cursor_movement_network);
		m_bMoveNetwork = true;
	}


	if(m_ToolMode == create_1waylink_tool || m_ToolMode == create_2waylinks_tool)
	{
		m_TempLinkStartPoint = point;
		m_TempLinkEndPoint = point;
		m_bMouseDownFlag = true;


		g_SetCursor(_cursor_create_link);


	}

	if(m_ToolMode == move_node_tool)
	{
		pDoc->m_SelectedNodeNo = FindClosestNode(point, pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution*2.0f);
		m_bMouseDownFlag = true;

	}


	if(m_ToolMode == subarea_tool || m_ToolMode == add_zone_tool)
	{
		if(m_ToolMode == subarea_tool)
			g_SetCursor(_cursor_create_subarea);

		if(m_ToolMode == add_zone_tool)
			g_SetCursor(_cursor_create_zone);

		m_TempZoneStartPoint = point;
		m_TempZoneEndPoint = point;

		m_bMouseDownFlag = true;

		if(!isCreatingSubarea && GetDocument()->m_SubareaShapePoints.size()==0)
		{
			// Record the start location and find the closest feature point
			m_FirstSubareaPoints = point;
			GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));

			isCreatingSubarea = true;
			SetCapture();
			m_last_left_down_point = point;
		}
		else
		{
			if(bFindCloseSubareaPoint(point) && GetDocument()->m_SubareaShapePoints.size()>= 3)
		 {
			 GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
			 GetDocument()->m_SubareaShapePoints.push_back(GetDocument()->m_SubareaShapePoints[0]);
			 isCreatingSubarea = false;
			 CopyLinkSetInSubarea();
			 isFinishSubarea = true;
			 if(m_ToolMode == add_zone_tool)
			 {
				 GetDocument()-> OnSubareaCreatezonefromsubarea();

			 }else
			 {

				 GetDocument()->PushBackNetworkState ();
			 }
			 m_ToolMode = move_tool;
			 ReleaseCapture();
			 m_last_left_down_point = point;

			 m_last_cpoint = point;


			 Invalidate();



			 //FitNetworkToScreen();


		 }
			else
		 {
			 GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
		 }
		}
	}

	CView::OnLButtonDown(nFlags, point);
}


void CTLiteView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CTLiteDoc* pDoc = GetDocument();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	if (m_ToolMode == move_tool || (m_ToolMode == move_node_tool && pDoc->m_SelectedNodeNo <= -1))
	{
		CSize OffSet = point - m_last_cpoint;
		m_Origin.x -= OffSet.cx/m_Resolution;
		m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

		g_SetCursor(_cursor_standard_arrow);
		SetGlobalViewParameters();

		m_bMoveDisplay = false;
	}

	if(m_ToolMode == move_node_tool && pDoc->m_SelectedNodeNo>=0)  //  a node has been selected
	{

		MoveNode(pDoc->m_SelectedNodeNo,point);
		pDoc->m_bLinkToBeShifted = true;
		pDoc->OffsetLink();
		pDoc->GenerateOffsetLinkBand();
		m_bMouseDownFlag = false;

	}


	if(m_ToolMode == select_link_tool)
	{
		CSize OffSet_ButtonDownUp = point - m_last_left_down_point;
		CSize OffSet = point - m_last_cpoint;

		// use move tool mode when offset is positive

		if((abs(OffSet_ButtonDownUp.cx) + abs(OffSet_ButtonDownUp.cx)) >=2) 
		{
			m_Origin.x -= OffSet.cx/m_Resolution;
			m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

			g_SetCursor(_cursor_standard_arrow);

			SetGlobalViewParameters();

			m_bMoveDisplay = false;
		}else
		{	// same point for LButton down and up messages

		}
	}

	if(m_ToolMode == select_feature_tool)
	{
		CSize OffSet_ButtonDownUp = point - m_last_left_down_point;
		CSize OffSet = point - m_last_cpoint;
		// use move tool mode when offset is positive
		if((abs(OffSet_ButtonDownUp.cx) + abs(OffSet_ButtonDownUp.cx)) >=2) 
		{
			m_Origin.x -= OffSet.cx/m_Resolution;
			m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

			g_SetCursor(_cursor_standard_arrow);

			SetGlobalViewParameters();

			m_bMoveDisplay = false;
		}else
		{ // same point for LButton down and up messages

			switch (pMainFrame->m_iSelectedLayer )
			{
			case layer_node:

				pDoc->m_SelectedLinkNo = -1;
				for (std::list<DTALink*>::iterator iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
				{
					(*iLink)->m_DisplayLinkID = -1;
				}

				pDoc->m_SelectedNodeNo = FindClosestNode(point, pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution*2.0f);

				if(pDoc->m_SelectedNodeNo >= 0)
				{

					pMainFrame->m_FeatureInfoVector.clear();

					DTANode* pNode = pDoc->m_NodeNoMap [pDoc->m_SelectedNodeNo ];
					CFeatureInfo element;
					element.Attribute = "Node ID";
					element.Data.Format ("%d",pNode->m_NodeID  );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "Control Type";

					if(pNode->m_ControlType == pDoc->m_ControlType_UnknownControl) element.Data.Format ("Unknown Control" );
					if(pNode->m_ControlType == pDoc->m_ControlType_ExternalNode) element.Data.Format ("External Node" );
					if(pNode->m_ControlType ==  pDoc->m_ControlType_NoControl) element.Data.Format ("No Control" );
					if(pNode->m_ControlType ==  pDoc->m_ControlType_YieldSign) element.Data.Format ("Yield Sign" );
					if(pNode->m_ControlType ==  pDoc->m_ControlType_2wayStopSign) element.Data.Format ("2 Way Stop" );
					if(pNode->m_ControlType ==  pDoc->m_ControlType_4wayStopSign) element.Data.Format ("4 Way Stop" );
					if(pNode->m_ControlType ==  pDoc->m_ControlType_PretimedSignal) element.Data.Format ("Pretimed Signal" );
					if(pNode->m_ControlType ==  pDoc->m_ControlType_ActuatedSignal) element.Data.Format ("actuated Signal" );
					if(pNode->m_ControlType ==  pDoc->m_ControlType_Roundabout) element.Data.Format ("Roundabout" );

					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "x";
					element.Data.Format ("%f",pNode->pt.x   );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "y";
					element.Data.Format ("%f",pNode->pt.y   );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "Associated Zone ID";
					element.Data.Format ("%d",pNode->m_ZoneID   );
					pMainFrame->m_FeatureInfoVector.push_back (element);

	
					if(pNode->m_Name .size()>0 && pNode->m_Name!="(null)")
					{
						element.Attribute = "Street Name";
						element.Data.Format ("%s",pNode->m_Name.c_str ()   );

						pMainFrame->m_FeatureInfoVector.push_back (element);
					}


					pMainFrame->FillFeatureInfo ();

				}else
				{
					pMainFrame->m_FeatureInfoVector.clear();
					pMainFrame->FillFeatureInfo ();
				}


				break;

			case layer_zone:

				pDoc->m_SelectedZoneID = FindClosestZone(point,0);

				pMainFrame->m_FeatureInfoVector.clear();

				if(pDoc->m_SelectedZoneID>=0)
				{
					CFeatureInfo element;
					element.Attribute = "Zone ID";
					element.Data.Format ("%d", pDoc->m_SelectedZoneID );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					pDoc->m_SelectedLinkNo = -1;
					for (std::list<DTALink*>::iterator iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
					{
						(*iLink)->m_DisplayLinkID = -1;
					}

					pDoc->m_SelectedNodeNo = -1;


					pMainFrame->FillFeatureInfo ();
				}

				break;
			case layer_link:
			case layer_connector:
			case layer_link_MOE:
			case layer_workzone:
				case layer_path:


				pDoc->m_SelectedNodeNo = -1;
				OnClickLink(nFlags, point);

				if(pDoc->m_SelectedLinkNo >= 0)
				{

					pMainFrame->m_FeatureInfoVector.clear();

					DTALink* pLink = pDoc->m_LinkNoMap [pDoc->m_SelectedLinkNo ];
					CFeatureInfo element;
					element.Attribute = "Link ID";
					element.Data.Format ("%s",pLink->m_LinkID.c_str()  );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "name";
					element.Data.Format ("%s",pLink->m_Name.c_str ()  );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "From Node ID";
					element.Data.Format ("%d",pLink->m_FromNodeID  );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "To Node ID";
					element.Data.Format ("%d",pLink->m_ToNodeID   );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					if(pDoc->m_LinkTypeMap.find(pLink->m_link_type) != pDoc->m_LinkTypeMap.end())
					{
						element.Attribute = "Type";
						element.Data.Format ("%s",pDoc->m_LinkTypeMap  [ pLink->m_link_type ].link_type_name.c_str () );
						pMainFrame->m_FeatureInfoVector.push_back (element);
					}

					element.Attribute = "Free Speed";

					element.Data.Format ("%3.0f",pLink->m_FreeSpeed );
					pMainFrame->m_FeatureInfoVector.push_back (element);

						element.Attribute = "Length ";

					element.Data.Format ("%4.3f",pLink->m_Length   );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "FFTT";
					element.Data.Format ("%4.3f",pLink->m_FreeFlowTravelTime    );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "# of Lanes";
					element.Data.Format ("%d",pLink->m_NumberOfLanes );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "Lane Capacity";
					element.Data.Format ("%4.0f",pLink->m_MaximumServiceFlowRatePHPL  );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					element.Attribute = "Link Capacity";
					element.Data.Format ("%4.0f",pLink->m_MaximumServiceFlowRatePHPL*pLink->m_NumberOfLanes   );
					pMainFrame->m_FeatureInfoVector.push_back (element);

					//element.Attribute = "Mode Code";
					//element.Data.Format ("%s",pLink->m_Mode_code   );
					//pMainFrame->m_FeatureInfoVector.push_back (element);


					pMainFrame->FillFeatureInfo ();


				}else
				{
					pMainFrame->m_FeatureInfoVector.clear();	
					pMainFrame->FillFeatureInfo ();
				}

				if(pMainFrame->m_iSelectedLayer == layer_link_MOE)
				{
					g_LinkMOEDlgShowFlag = false;
					pDoc->OnViewShowmoe();
				}

				break;

			}
			m_bMoveDisplay = false;


		}
	}

	if(m_ToolMode == backgroundimage_tool)
	{

		CSize OffSet = point - m_last_cpoint;
		CTLiteDoc* pDoc = GetDocument();
		pDoc->m_ImageX1  += OffSet.cx*pDoc->m_ImageMoveSize;
		pDoc->m_ImageY1  += OffSet.cy*m_OriginOnBottomFlag*pDoc->m_ImageMoveSize;

		g_SetCursor(_cursor_standard_arrow);

		m_bMoveImage = false;
	}

	if (m_ToolMode == sensor_location_move_tool)
	{

		CSize OffSet = point - m_last_cpoint;
		CTLiteDoc* pDoc = GetDocument();
		pDoc->m_SensorMapX += OffSet.cx*m_Resolution;
		pDoc->m_SensorMapY += OffSet.cy*m_OriginOnBottomFlag*m_Resolution;

		g_SetCursor(_cursor_standard_arrow);

		m_bMoveSensorMap = false;
	}
	if(m_ToolMode == network_coordinate_tool)
	{
		g_SetCursor(_cursor_standard_arrow);
		m_bMoveNetwork = false;
	}



	if(m_ToolMode == create_1waylink_tool || m_ToolMode == create_2waylinks_tool)
	{
		m_bMouseDownFlag = false;
		CTLiteDoc* pDoc = GetDocument();

		m_TempLinkEndPoint = point;

		CSize OffSet = m_TempLinkStartPoint - m_TempLinkEndPoint;

		if(abs(OffSet.cx) +  abs(OffSet.cy) < 3)  // clicking on the same point, do not create links
			return;

		pDoc->Modify();
		pDoc->PushBackNetworkState();


		DTANode* pFromNode = 0;// create from node if there is no overlapping node
		float min_selection_distance = max(10,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution));

		min_selection_distance = min(50,min_selection_distance);  // 50 PiX FOR SELECTION 

		bool flag_split_link = true;

		// if the control key is not pressed 
		if ((nFlags & MK_CONTROL) == false)
		{
			flag_split_link = false;
		}

		int FromNodeNo = FindClosestNode(m_TempLinkStartPoint, min_selection_distance);
		if(FromNodeNo ==-1)
		{
			pFromNode = pDoc->AddNewNode(SPtoNP(m_TempLinkStartPoint),0,0,false, flag_split_link);
		}else
		{
			pFromNode = pDoc-> m_NodeNoMap[FromNodeNo];
		}

		DTANode* pToNode = 0;// create from node if there is no overlapping node
		int ToNodeNo = FindClosestNode(m_TempLinkEndPoint, min_selection_distance);
		if(ToNodeNo ==-1)
		{
			pToNode = pDoc->AddNewNode(SPtoNP(m_TempLinkEndPoint),0,0,false, flag_split_link);
		}else
		{
			pToNode = pDoc-> m_NodeNoMap[ToNodeNo];
		}

		// create one way link

		if(m_ToolMode == create_1waylink_tool)
		{
			pDoc->AddNewLink(pFromNode->m_NodeID, pToNode->m_NodeID, true);
		}

		// create 2 way links with opposite direction
		if(m_ToolMode == create_2waylinks_tool)
		{
			pDoc->AddNewLink(pFromNode->m_NodeID, pToNode->m_NodeID, true);
			pDoc->AddNewLink(pToNode->m_NodeID,pFromNode->m_NodeID,true);

		}

		pDoc->m_bFitNetworkInitialized = true; // do not reflesh after adding links

	}

	if(m_ToolMode == create_node_tool)
	{
		CTLiteDoc* pDoc = GetDocument();

		DTANode* pFromNode = 0;// create from node if there is no overlapping node

		float min_selection_distance = max(20,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution));
		int FromNodeNo = FindClosestNode(point, min_selection_distance);
		if(FromNodeNo ==-1)
		{
			pFromNode = pDoc->AddNewNode(SPtoNP(point),0,0,false,true);
		}

		pDoc->m_bFitNetworkInitialized = true; // do not reflesh after adding links

	}

	Invalidate();
	CView::OnLButtonUp(nFlags, point);
}

void CTLiteView::OnUpdateViewMove(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == move_tool ? 1 : 0);
}

void CTLiteView::OnMouseMove(UINT nFlags, CPoint point)
{
	CTLiteDoc* pDoc = GetDocument();

	if ((m_ToolMode == move_tool || (m_ToolMode == move_node_tool && pDoc->m_SelectedNodeNo <= -1) || m_ToolMode == select_link_tool || m_ToolMode == select_feature_tool) && m_bMoveDisplay)
	{
		CSize OffSet = point - m_last_cpoint;
		//		if(!(OffSet.cx !=0 && OffSet.cy !=0))
		{
			m_Origin.x -= OffSet.cx/m_Resolution;
			m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;
			m_last_cpoint = point;

		}

		if(m_bMoveDisplay)
			g_SetCursor(_cursor_movement_network);

		SetGlobalViewParameters();

		Invalidate();

	}
	if(m_ToolMode == move_node_tool && pDoc->m_SelectedNodeNo>=0 && m_bMouseDownFlag)  //  a node has been selected
	{

		MoveNode(pDoc->m_SelectedNodeNo,point);

		pDoc->m_bLinkToBeShifted = true;
		pDoc->OffsetLink();
		pDoc->GenerateOffsetLinkBand();

		Invalidate();

	}

	if(m_ToolMode == backgroundimage_tool && m_bMoveImage)
	{
		CSize OffSet = point - m_last_cpoint;
		//		if(!(OffSet.cx !=0 && OffSet.cy !=0))
		{
			pDoc->m_ImageX1  += OffSet.cx*pDoc->m_ImageMoveSize;
			pDoc->m_ImageY1  += OffSet.cy*m_OriginOnBottomFlag*pDoc->m_ImageMoveSize;

			m_last_cpoint = point;

		}
		if(m_bMoveDisplay)
			g_SetCursor(_cursor_movement_network);

		Invalidate();

	}

	if (m_ToolMode == sensor_location_move_tool && m_bMoveSensorMap)
	{
		CSize OffSet = point - m_last_cpoint;
		//		if(!(OffSet.cx !=0 && OffSet.cy !=0))
		{
			pDoc->m_SensorMapX += OffSet.cx*pDoc->m_SensorMapMoveSize;
			pDoc->m_SensorMapY += OffSet.cy*m_OriginOnBottomFlag*pDoc->m_SensorMapMoveSize;

			m_last_cpoint = point;

		}
		if (m_bMoveSensorMap)
			g_SetCursor(_cursor_movement_network);

		Invalidate();

	}


	if(m_ToolMode == network_coordinate_tool && m_bMoveNetwork)
	{
		CSize OffSet = point - m_last_cpoint;
		//		if(!(OffSet.cx !=0 && OffSet.cy !=0))
		{
			CTLiteDoc* pDoc = GetDocument();
			float delta_x = OffSet.cx/m_Resolution;
			float delta_y = OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

			int LayerNo = m_MoveLayerNo; // for imported network
			pDoc->ChangeNetworkCoordinates(LayerNo,1.0f,1.0f,delta_x,delta_y);

			m_last_cpoint = point;

		}
		Invalidate();

	}

	if(m_ToolMode == create_node_tool )
	{
		g_SetCursor(_cursor_create_node);
	}

	if(m_ToolMode == create_1waylink_tool || m_ToolMode == create_2waylinks_tool)
	{
		g_SetCursor(_cursor_create_link);

		if(m_bMouseDownFlag)
		{
			// if it is the first moving operation, erase the previous temporal link
			if(m_TempLinkStartPoint!=m_TempLinkEndPoint)
				DrawTemporalLink(m_TempLinkStartPoint,m_TempLinkEndPoint);

			// update m_TempLinkEndPoint from the current mouse point
			m_TempLinkEndPoint = point;

			// draw a new temporal link
			DrawTemporalLink(m_TempLinkStartPoint,m_TempLinkEndPoint);
		}



	}
	if(m_ToolMode == subarea_tool  || m_ToolMode == add_zone_tool)
	{
		if(m_ToolMode == subarea_tool )
			g_SetCursor(_cursor_create_subarea);

		if(m_ToolMode == add_zone_tool )
			g_SetCursor(_cursor_create_zone);

		if(isCreatingSubarea)
		{
			// if it is the first moving operation, erase the previous temporal link
			if(m_TempZoneStartPoint!=m_TempZoneEndPoint)
				DrawTemporalLink(m_TempZoneStartPoint,m_TempZoneEndPoint);

			// update m_TempLinkEndPoint from the current mouse point
			m_TempZoneEndPoint = point;

			// draw a new temporal link
			DrawTemporalLink(m_TempZoneStartPoint,m_TempZoneEndPoint);
		}

	}


	CString str;
	GDPoint  gdpt  = SPtoNP(point);
	str.Format("%.5f,%.5f", gdpt.x, gdpt.y);
	GetDocument()->SendTexttoStatusBar(str);

	CRect ScreenRect;
	GetClientRect(ScreenRect);

	CPoint LeftTop, RightBottom;
	LeftTop.x = ScreenRect.left ;
	LeftTop.y = ScreenRect.top  ;

	RightBottom.x = ScreenRect.right ;
	RightBottom.y = ScreenRect.bottom ;

	GDPoint  gdpt_lt  = SPtoNP(LeftTop);
	GDPoint  gdpt_rb  = SPtoNP(RightBottom);

	double width = fabs(gdpt_lt.x - gdpt_rb.x )/ pDoc->m_UnitDistance;
	double height = fabs(gdpt_lt.y - gdpt_rb.y )/ pDoc->m_UnitDistance;

	str.Format("%.5f,%.5f", gdpt.x, gdpt.y);
	GetDocument()->SendTexttoStatusBar(str);

	pDoc->m_ScreenWidth_InMile = width;
	pDoc->SendTexttoStatusBar(str,1);
	CView::OnMouseMove(nFlags, point);
}

void CTLiteView::OnViewSelect()
{
	m_ToolMode = select_link_tool;
}

void CTLiteView::OnUpdateViewSelect(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == select_link_tool ? 1 : 0);
}

void CTLiteView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnRButtonDown(nFlags, point);

	if((m_ToolMode == subarea_tool || m_ToolMode == add_zone_tool)&& GetDocument()->m_SubareaShapePoints.size()>= 3)
	{
		CWaitCursor wait;
		GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
		GetDocument()->m_SubareaShapePoints.push_back(GetDocument()->m_SubareaShapePoints[0]);
		isCreatingSubarea = false;
		CopyLinkSetInSubarea();
		isFinishSubarea = true;
		m_ToolMode = select_link_tool;
		ReleaseCapture();

		m_last_left_down_point = point;

		Invalidate();
	}
}

void CTLiteView::OnNodeOrigin()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	pDoc->m_ONodeNo = pDoc->m_SelectedNodeNo;
	pDoc->Routing(false);

	m_ShowAllPaths = true;
	Invalidate();

}



void CTLiteView::OnNodeDestination()
{
	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	pDoc->m_DNodeNo = pDoc->m_SelectedNodeNo;

	m_ShowAllPaths = true;
	if(pDoc->Routing(false)==0)
	{
		AfxMessageBox("The selected OD pair does not have a connected path. Please checklink.csv for details.");

	}

	Invalidate();

}



void CTLiteView::OnContextMenu(CWnd* pWnd, CPoint point)
{

	Invalidate();

	CPoint MenuPoint = point;
	CTLiteDoc* pDoc = GetDocument();
	ScreenToClient(&point);

	m_CurrentMousePoint = point;



	CClientDC dc(this);


	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	//speical condition first


	//if(pMainFrame-> m_iSelectedLayer == layer_zone)
	//{
	//	CMenu cm;
	//	cm.LoadMenu(IDR_MENU1);

	//	if(pDoc->m_SubareaShapePoints .size()>=3)
	//	{
	//		cm.GetSubMenu(layer_subarea)->TrackPopupMenu(
	//		TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
	//		MenuPoint.x, MenuPoint.y, AfxGetMainWnd());
	//	}else
	//	{
	//		cm.GetSubMenu(layer_zone)->TrackPopupMenu(
	//		TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
	//		MenuPoint.x, MenuPoint.y, AfxGetMainWnd());
	//	
	//	}
	//	

	//}else
	{
		CMenu cm;
			cm.LoadMenu(IDR_MENU1);

		int layer_no = (int)(pMainFrame-> m_iSelectedLayer);
		cm.GetSubMenu(layer_no)->TrackPopupMenu(
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			MenuPoint.x, MenuPoint.y, AfxGetMainWnd());

	}


}

void CTLiteView::OnClickLink(UINT nFlags, CPoint point)
{

	CTLiteDoc* pDoc = GetDocument();
	double Min_distance = 100;  // in screen points


	for (std::list<DTALink*>::iterator iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		if((*iLink) ->m_ShapePoints.size() <2)
			continue;

		for(int si = 0; si < (*iLink) ->m_ShapePoints .size()-1; si++)
		{

			FromPoint = NPtoSP((*iLink)->m_ShapePoints[si]);
			ToPoint = NPtoSP((*iLink)->m_ShapePoints[si+1]);

			GDPoint p0, pfrom, pto;
			p0.x  = point.x; p0.y  = point.y;
			pfrom.x  = FromPoint.x; pfrom.y  = FromPoint.y;
			pto.x  = ToPoint.x; pto.y  = ToPoint.y;

			float distance = g_GetPoint2LineDistance(p0, pfrom, pto, 1);

			if(distance >0 && distance < Min_distance)
			{
				pDoc->m_SelectedLinkNo = (*iLink)->m_LinkNo ;

				Min_distance = distance;
			}
		}
	}

	//if(Min_distance > pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution*20)
	//{
	//	pDoc->m_SelectedLinkNo = -1;
	//	g_ClearLinkSelectionList();
	//}else
	{
		// if the control key is not pressed 
		if ( (nFlags & MK_CONTROL) ==false)
		{
			g_ClearLinkSelectionList();
		}

		int bFoundFlag = false;

		for (std::list<s_link_selection>::iterator iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++)
		{
			if( (*iLink).link_no == pDoc->m_SelectedLinkNo &&  (*iLink).document_no == pDoc->m_DocumentNo )
			{ 
				bFoundFlag = true; 
				break;
			}

		}
		if(!bFoundFlag)  //has not been selected;
		{
			GDPoint pt  = SPtoNP(point);

			DTALink* pLink = pDoc->m_LinkNoMap [pDoc->m_SelectedLinkNo];

			g_AddLinkIntoSelectionList(pLink,pDoc->m_SelectedLinkNo, pDoc->m_DocumentNo, true, pt.x, pt.y);
		}
	}



	// reset
	for (std::list<DTALink*>::iterator iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_DisplayLinkID = -1;
	}

	int LinkCount=0;
	for (std::list<s_link_selection>::iterator iLinkDisplay = g_LinkDisplayList.begin(); iLinkDisplay != g_LinkDisplayList.end(); iLinkDisplay++, LinkCount++)
	{

		if((*iLinkDisplay).pLink  !=NULL && pDoc->m_DocumentNo == (*iLinkDisplay).document_no )
		{
			pDoc->m_LinkNoMap[(*iLinkDisplay).link_no ]->m_DisplayLinkID = LinkCount;
		}

	}

}
void CTLiteView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CTLiteDoc* pDoc = GetDocument();

	/*	if( pDoc->m_TrafficFlowModelFlag > 0 || m_ToolMode == select_link_tool)
	{
	// create MOE Dlg when double clicking

	OnClickLink(nFlags, point);
	g_LinkMOEDlgShowFlag = false;
	pDoc->OnViewShowmoe();
	}
	*/
	/*		
	std::list<DTANode*>::iterator iNode;


	for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
	{
	CPoint NodePoint = NPtoSP((*iNode)->pt);

	CSize size = NodePoint - point;
	double distance = pow((size.cx*size.cx + size.cy*size.cy),0.5);
	if( distance < Min_distance)
	{
	m_SelectedNodeNo = (*iNode)->m_NodeNo ;
	Min_distance = distance;
	return;
	}

	}

	if(Min_distance > m_NodeSize*2)
	{
	m_SelectedNodeNo = -1;
	}

	*/

	if((m_ToolMode == subarea_tool || m_ToolMode == add_zone_tool) && GetDocument()->m_SubareaShapePoints.size()>= 3)
	{
		CWaitCursor wait;
		GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
		GetDocument()->m_SubareaShapePoints.push_back(GetDocument()->m_SubareaShapePoints[0]);
		isCreatingSubarea = false;
		CopyLinkSetInSubarea();
		isFinishSubarea = true;


		//add a zone
		if(m_ToolMode == add_zone_tool)
		{
			GetDocument()-> OnSubareaCreatezonefromsubarea();

		}
		m_ToolMode = select_link_tool;
		ReleaseCapture();
		m_last_left_down_point = point;

	}
	m_bMoveDisplay = false;

	CView::OnLButtonDblClk(nFlags, point);
}

void CTLiteView::OnSearchFindlink()
{
	CDlgFindALink dlg;

	CTLiteDoc* pDoc = GetDocument();

	dlg.m_SearchHistoryVector = pDoc->m_SearchHistoryVector;
	dlg.m_SearchMode = pDoc->m_SearchMode;

	if(dlg.DoModal () == IDOK)
	{

		m_PathNodeSequence = "";

		if(dlg.m_SearchMode == efind_link)
		{

			DTALink* pLink = pDoc->FindLinkWithNodeIDs(dlg.m_FromNodeID ,dlg.m_ToNodeID );

			if(pLink !=NULL)
			{
				pDoc->m_SelectedLinkNo = pLink->m_LinkNo ;
				pDoc->m_SelectedNodeNo = -1;

				pDoc->ZoomToSelectedLink(pLink->m_LinkNo);

				m_SelectFromNodeID = dlg.m_FromNodeID;
				m_SelectToNodeID = dlg.m_ToNodeID;
				Invalidate();

			}
		}
		if (dlg.m_SearchMode == efind_link_id)
		{
			std::string str_link_id = pDoc->CString2StdString(dlg.m_LinkID);

			DTALink* pLink = pDoc->FindLinkWithLinkID (str_link_id);

			if (pLink != NULL)
			{
				pDoc->m_SelectedLinkNo = pLink->m_LinkNo;
				pDoc->m_SelectedNodeNo = -1;

				pDoc->ZoomToSelectedLink(pLink->m_LinkNo);

				m_SelectFromNodeID = dlg.m_FromNodeID;
				m_SelectToNodeID = dlg.m_ToNodeID;
				Invalidate();

			}
		}

		if(dlg.m_SearchMode == efind_node)
		{
			DTANode* pNode = pDoc->FindNodeWithNodeID (dlg.m_NodeID);
			if(pNode !=NULL)
			{
				pDoc->m_SelectedLinkNo = -1;
				pDoc->m_SelectedNodeNo = pNode->m_NodeNo ;

				m_Origin = pNode->pt ;

				m_bShowNode = true;
				Invalidate();
			}else
			{
				CString str;

				if(dlg.m_NodeID != 0)  // 0 is the default node number in this dialog
				{
					str.Format ("Node %d cannot be found.",dlg.m_NodeID);
					AfxMessageBox(str);
				}
			}
		}

		if(dlg.m_SearchMode == efind_path)
		{
			m_SelectFromNodeID = dlg.m_FromNodeID;
			m_SelectToNodeID = dlg.m_ToNodeID;

			m_ShowAllPaths = true;

			DTANode* pFromNode = pDoc->FindNodeWithNodeID (dlg.m_FromNodeID);
			if(pFromNode ==NULL)
			{
				CString str;
				str.Format ("From Node %d cannot be found.",dlg.m_FromNodeID);
				AfxMessageBox(str);
				return;
			}else
			{
				pDoc->m_ONodeNo = pFromNode->m_NodeNo;

			}
			DTANode* pToNode = pDoc->FindNodeWithNodeID (dlg.m_ToNodeID);
			if(pToNode ==NULL)
			{
				CString str;
				str.Format ("To Node %d cannot be found.",dlg.m_ToNodeID);
				AfxMessageBox(str);
				return;
			}else
			{
				pDoc->m_DNodeNo = pToNode->m_NodeNo;
			}

			pDoc->Routing(false);

		}

		

		if (dlg.m_SearchMode == efind_path_node_sequence)
		{

			m_PathNodeSequence = dlg.m_PathNodeSequence;
			Invalidate();

			return;
		}

		if(dlg.m_SearchMode == efind_Agent)
		{
			string SelectedAgentID = dlg.m_AgentNumber; // internal Agent index starts from zero
			std::vector<int> LinkVector;

			if(pDoc->m_AgentIDMap.find(SelectedAgentID) == pDoc->m_AgentIDMap.end())
			{
				if(SelectedAgentID.size()>0)
				{
					CString str_message;
					str_message.Format ("Agent Id %s cannot be found.", SelectedAgentID.c_str());
					AfxMessageBox(str_message);
				}

				pDoc->m_SelectedAgentID = -1;
				pDoc->HighlightPath(LinkVector,1);

				return;
			}


			



			pDoc->m_SelectedAgentID = SelectedAgentID;

			DTAAgent* pAgent = pDoc->m_AgentIDMap[SelectedAgentID];

			CPlayerSeekBar m_wndPlayerSeekBar;


			// set departure time to the current time of display
			g_Simulation_Time_Stamp = pAgent->m_DepartureTime +1;

			CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

			pMainFrame->m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

			pDoc->m_LinkMOEMode = MOE_Agent;

			for(int link= 1; link<pAgent->m_NodeSize; link++)
			{
				LinkVector.push_back (pAgent->m_NodeAry[link].LinkNo);
			}

			pDoc->HighlightPath(LinkVector,1);

			pDoc->m_HighlightGDPointVector.clear();

			pDoc->m_HighlightGDPointVector = pAgent->m_GPSLocationVector ; // assign the GPS points to be highlighted

		}

		pDoc->m_SearchHistoryVector = dlg.m_SearchHistoryVector;
		pDoc->m_SearchMode = dlg.m_SearchMode;


	}

	Invalidate();
}

void CTLiteView::OnEditChangebackgroupimagethroughmouse()
{
	if(m_ToolMode!= backgroundimage_tool)
		m_ToolMode = backgroundimage_tool;
	else
		m_ToolMode = move_tool;

}

void CTLiteView::OnUpdateEditChangebackgroupimagethroughmouse(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == backgroundimage_tool ? 1 : 0);
}

void CTLiteView::OnViewBackgroundimage()
{
	m_bShowImage = !m_bShowImage;
	Invalidate();
}

void CTLiteView::OnUpdateViewBackgroundimage(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowImage);
}

void CTLiteView::OnViewShowlinktype()
{
	m_bShowLinkType = !m_bShowLinkType;
	Invalidate();
}

void CTLiteView::OnUpdateViewShowlinktype(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowLinkType);

}


void CTLiteView::OnShowShownode()
{
	m_bShowNode = !m_bShowNode;
	Invalidate();
}

void CTLiteView::OnUpdateShowShownode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowNode);
}

void CTLiteView::OnShowShowallpaths()
{
	//	m_ShowAllPaths = !m_ShowAllPaths;

	Invalidate();
}

void CTLiteView::OnUpdateShowShowallpaths(CCmdUI *pCmdUI)
{
}



void CTLiteView::OnShowShowNodeID()
{
	if(m_ShowNodeTextMode != node_display_none)
		m_ShowNodeTextMode = node_display_none;
	else 
		m_ShowNodeTextMode = node_display_node_number;

	if(m_ShowNodeTextMode != node_display_none)
		m_bShowNode = true;

	Invalidate();

}

void CTLiteView::OnUpdateShowShowNodeID(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ShowNodeTextMode != node_display_none);

}

void  CTLiteView::DrawTemporalLink(CPoint start_point, CPoint end_point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc);

	dc.SelectObject(&g_TempLinkPen);
	// Select drawing mode

	int oldROP2 = dc.SetROP2(R2_XORPEN);

	// Draw the line
	dc.MoveTo(start_point);
	dc.LineTo(end_point);

	// Reset drawing mode
	dc.SetROP2(oldROP2);

}

void CTLiteView::OnEditCreate1waylink()
{
	if(m_ToolMode != create_1waylink_tool)
	{

		CTLiteDoc* pDoc = GetDocument();

		m_link_display_mode = link_display_mode_line;
		m_bShowLinkArrow = true;

		m_ToolMode = create_1waylink_tool;
		Invalidate();

	}
	else   //reset to default selection tool
		m_ToolMode = select_link_tool;

}

void CTLiteView::OnEditCreate2waylinks()
{
	if(m_ToolMode != create_2waylinks_tool)
	{
			CTLiteDoc* pDoc = GetDocument();


		m_link_display_mode = link_display_mode_line;
		m_bShowLinkArrow = true;


		if (pDoc->m_bLinkToBeShifted == false || pDoc->m_OffsetInDistance <0.004)
		{
			pDoc->OnLinkIncreaseoffsetfortwo();
			pDoc->OnLinkIncreaseoffsetfortwo();
		}

		m_ToolMode = create_2waylinks_tool;
			Invalidate();

	}
	else   //reset to default selection tool
		m_ToolMode = select_link_tool;

}

void CTLiteView::OnUpdateEditCreate1waylink(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == create_1waylink_tool);

}

void CTLiteView::OnUpdateEditCreate2waylinks(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == create_2waylinks_tool);
}

void CTLiteView::OnShowGrid()
{
	m_bShowGrid = !m_bShowGrid;
	Invalidate();
}

void CTLiteView::OnUpdateShowGrid(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowGrid);
}

void CTLiteView::OnShowLinkarrow()
{
	m_bShowLinkArrow = !m_bShowLinkArrow;

	if(m_bShowLinkArrow)
	{
		m_link_display_mode = link_display_mode_line;
	}
	Invalidate();
}

void CTLiteView::OnUpdateShowLinkarrow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowLinkArrow);
}


void CTLiteView::OnImageLockbackgroundimageposition()
{
	m_ToolMode = move_tool;  // swtich m_ToolMode from backgroundimage_tool to other tool
}

void CTLiteView::OnViewTextlabel()
{

}

void CTLiteView::OnUpdateViewTextlabel(CCmdUI *pCmdUI)
{
}


void CTLiteView::OnLinkDelete()
{

	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_SelectedLinkNo == -1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}
	pDoc->Modify ();
	pDoc->PushBackNetworkState();

	pDoc->DeleteLink(pDoc->m_SelectedLinkNo);
	pDoc->m_SelectedLinkNo = -1;
	Invalidate();
}

void CTLiteView::OnUpdateEditDeleteselectedlink(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (GetDocument()->m_SelectedLinkNo>=0);
}

void CTLiteView::OnLinkEditlink()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= pDoc->m_LinkNoMap [pDoc->m_SelectedLinkNo];
	if(pLink!=NULL)
	{
		CDlgLinkProperties dlg;


		GDPoint pt = SPtoNP(m_CurrentMousePoint);
		dlg.m_LinkLocationX = pt.x;
		dlg.m_LinkLocationY = pt.y;

		dlg.m_pDoc = pDoc;

		dlg.m_LinkID = pLink->m_LinkID.c_str() ;
		dlg.StreetName  = pLink->m_Name.c_str () ;
		dlg.FromNode = pLink->m_FromNodeID ;
		dlg.ToNode = pLink->m_ToNodeID ;
		dlg.m_NumLeftTurnLanes  = pLink-> m_NumberOfLeftTurnLanes;
		dlg.m_NumRightTurnLanes  = pLink-> m_NumberOfRightTurnLanes;

		dlg.main_node_id = pLink->main_node_id.c_str();
		dlg.NEMA_phase_number = pLink->NEMA_phase_number.c_str();
		dlg.movement_str = pLink->movement_str.c_str();


			dlg.LinkLength = pLink->m_Length ;
			dlg.FreeSpeed = pLink->m_FreeSpeed ;

		dlg.FreeFlowTravelTime = pLink->m_FreeFlowTravelTime ;
		dlg.LaneCapacity  = pLink->m_LaneCapacity ;
		dlg.nLane = pLink->m_NumberOfLanes ;
		dlg.LinkType = pLink->m_link_type;

		if(dlg.DoModal() == IDOK)
		{

				pLink->m_Length = dlg.LinkLength;
				pLink->m_FreeSpeed = dlg.FreeSpeed;

				pLink->main_node_id = pDoc->CString2StdString(dlg.main_node_id);
				pLink->NEMA_phase_number = pDoc->CString2StdString(dlg.NEMA_phase_number);
				pLink->movement_str = pDoc->CString2StdString(dlg.movement_str);


			pLink->m_Name  = pDoc->CString2StdString(dlg.StreetName);
			pLink->m_LinkID = pDoc->CString2StdString(dlg.m_LinkID);
			pLink->m_LaneCapacity  = dlg.LaneCapacity;

			if(pLink->m_NumberOfLanes  != dlg.nLane)
			{
				pLink->m_NumberOfLanes  = dlg.nLane;
				pDoc->GenerateOffsetLinkBand();  // update width of band
			}

			std::list<DTANode*>::iterator iNode;


			if(pLink->m_link_type != dlg.LinkType)
			{
				dlg.m_bEditChange = true;
				pLink->m_link_type = dlg.LinkType ;
			}


			if( pDoc->m_LinkTypeMap[pLink->m_link_type].IsFreeway () ||  pDoc->m_LinkTypeMap[pLink->m_link_type].IsRamp  ())
			{
				pDoc->m_NodeNoMap[pLink->m_FromNodeNo ]->m_bConnectedToFreewayORRamp = true;
				pDoc->m_NodeNoMap[pLink->m_ToNodeNo ]->m_bConnectedToFreewayORRamp = true;
			}


			pDoc->m_DefaultSpeedLimit = dlg.DefaultSpeedLimit;
			pDoc->m_DefaultCapacity = dlg.DefaultCapacity;
			pDoc->m_DefaultNumLanes = dlg.DefaultnLane;

			// error checking here

			if(dlg.m_bEditChange)
			{
				pDoc->Modify();
				pDoc->PushBackNetworkState();
			}

		}
		Invalidate();
	}

}


void CTLiteView::OnEditCreatenode()
{
	if(m_ToolMode != create_node_tool)
	{
		m_ToolMode = create_node_tool;
		m_bShowNode = true;
		m_ShowNodeTextMode = node_display_node_number;

	}
	else   //reset to default selection tool
		m_ToolMode = select_link_tool;
}

void CTLiteView::OnUpdateEditCreatenode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == create_node_tool ? 1 : 0);
}

void CTLiteView::OnEditDeleteselectednode()
{
	CTLiteDoc* pDoc = GetDocument();
	pDoc->Modify ();
	pDoc->PushBackNetworkState();

	if(pDoc->m_SelectedNodeNo == -1)
	{
		AfxMessageBox("Please select a node first.");
		return;
	}
	if(pDoc->DeleteNode(pDoc->m_SelectedNodeNo) ==false)
	{
		AfxMessageBox("Only an isolated node can be deleted. Please delete adjacent links first.");
		return;
	}
	pDoc->m_SelectedNodeNo = -1;
	Invalidate();

}

void CTLiteView::OnEditSelectnode()
{
	// TODO: Add your command handler code here
}

void CTLiteView::OnViewSelectNode()
{
	m_ToolMode = select_feature_tool;
}

void CTLiteView::OnUpdateViewSelectNode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == select_feature_tool ? 1 : 0);
}



void CTLiteView::OnEditCreatesubarea()
{
	m_ToolMode = subarea_tool;
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	pMainFrame->m_bShowLayerMap[layer_subarea] = true;
	pMainFrame-> m_iSelectedLayer = layer_subarea;
	GetDocument()->m_SubareaShapePoints.clear();

	CListCtrl * pGISLayerList = (CListCtrl *)(pMainFrame->m_GISLayerBar).GetDlgItem(IDC_LIST_GISLAYER);
	pGISLayerList->Invalidate(1);  // update display of selected layer

}

void CTLiteView::OnUpdateEditCreatesubarea(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == subarea_tool ? 1 : 0);
}


void CTLiteView::CopyLinkSetInSubarea()
{
	CTLiteDoc* pDoc = GetDocument();

	if( pDoc->m_SubareaShapePoints.size() < 3)
		return;

	LPPOINT m_subarea_points = new POINT[pDoc->m_SubareaShapePoints.size()];
	for (int sub_i= 0; sub_i < pDoc->m_SubareaShapePoints.size(); sub_i++)
	{
		CPoint point =  NPtoSP(pDoc->m_SubareaShapePoints[sub_i]);
		m_subarea_points[sub_i].x = point.x;
		m_subarea_points[sub_i].y = point.y;
	}

	// Create a polygonal region
	m_polygonal_region = CreatePolygonRgn(m_subarea_points, pDoc->m_SubareaShapePoints.size(), WINDING);

	pDoc->m_SubareaNodeSet.clear();
	pDoc->m_SubareaLinkSet.clear();
	pDoc->m_SubareaNodeNoMap.clear();

	std::list<DTANode*>::iterator iNode = pDoc->m_NodeSet.begin ();
	while (iNode != pDoc->m_NodeSet.end())
	{
		CPoint point = NPtoSP((*iNode)->pt);
		if(PtInRegion(m_polygonal_region, point.x, point.y))  //outside subarea
		{
			pDoc->m_SubareaNodeSet .push_back ((*iNode));
			pDoc->m_SubareaNodeNoMap[(*iNode)->m_NodeID ]=  (*iNode);
			//inside subarea
		}
		iNode++;
	}

	std::list<DTALink*>::iterator iLink;

	iLink = pDoc->m_LinkSet.begin(); 

	while (iLink != pDoc->m_LinkSet.end())
	{
		(*iLink)->m_bIncludedinSubarea = false;
		if(pDoc->m_SubareaNodeNoMap.find((*iLink)->m_FromNodeNo ) != pDoc->m_SubareaNodeNoMap.end() || pDoc->m_SubareaNodeNoMap.find((*iLink)->m_ToNodeNo ) != pDoc->m_SubareaNodeNoMap.end()) 
		{
			pDoc->m_SubareaLinkSet.push_back (*iLink);

			(*iLink)->m_bIncludedinSubarea = true;


		}

		(*iLink)->m_bIncludedinBoundaryOfSubarea = false;
		if( (pDoc->m_SubareaNodeNoMap.find((*iLink)->m_FromNodeNo ) != pDoc->m_SubareaNodeNoMap.end() && pDoc->m_SubareaNodeNoMap.find((*iLink)->m_ToNodeNo ) == pDoc->m_SubareaNodeNoMap.end())
			|| (pDoc->m_SubareaNodeNoMap.find((*iLink)->m_ToNodeNo ) != pDoc->m_SubareaNodeNoMap.end() && pDoc->m_SubareaNodeNoMap.find((*iLink)->m_FromNodeNo ) == pDoc->m_SubareaNodeNoMap.end()))
		{
			(*iLink)->m_bIncludedinBoundaryOfSubarea = true;
		}

		iLink++;
	}


	DeleteObject(m_polygonal_region);
	delete [] m_subarea_points;

}

void CTLiteView::OnToolsRemovenodesandlinksoutsidesubarea()
{
	CWaitCursor wait;

	CTLiteDoc* pDoc = GetDocument();

	pDoc->Modify ();
	pDoc->PushBackNetworkState();


	LPPOINT m_subarea_points = new POINT[pDoc->m_SubareaShapePoints.size()];
	for (unsigned int sub_i= 0; sub_i < pDoc->m_SubareaShapePoints.size(); sub_i++)
	{
		CPoint point =  NPtoSP(pDoc->m_SubareaShapePoints[sub_i]);
		m_subarea_points[sub_i].x = point.x;
		m_subarea_points[sub_i].y = point.y;
	}

	// Create a polygonal region
	m_polygonal_region = CreatePolygonRgn(m_subarea_points, pDoc->m_SubareaShapePoints.size(), WINDING);

	// step 01
	std::list<DTANode*>::iterator iNode;
	iNode = pDoc->m_NodeSet.begin ();


	while (iNode != pDoc->m_NodeSet.end())
	{
		CPoint point = NPtoSP((*iNode)->pt);
		if(PtInRegion(m_polygonal_region, point.x, point.y) == false)  //outside subarea
		{
			(*iNode)->m_bSubareaFlag = 0;
		}
		else
		{
			(*iNode)->m_bSubareaFlag = 1;	//inside subarea
		}

		(*iNode)->m_tobeRemoved = true;  // mark all to be removed first

		++iNode;
	}



	//step 02
	//remove links
	pDoc->m_LinkNoMap.clear();

	std::list<DTALink*>::iterator iLink;

	iLink = pDoc->m_LinkSet.begin(); 

	while (iLink != pDoc->m_LinkSet.end())
	{
		DTANode* pFromNode = pDoc->m_NodeNoMap[(*iLink)->m_FromNodeNo];
		DTANode* pToNode = pDoc->m_NodeNoMap[(*iLink)->m_ToNodeNo];

		if(pFromNode->m_bSubareaFlag == 0 &&  pToNode ->m_bSubareaFlag == 0 ) 
		{
			iLink = pDoc->m_LinkSet.erase(iLink);  // remove when one of end points are covered by the subarea

		}else
		{
			pDoc->m_LinkNoMap[(*iLink)->m_LinkNo] = (*iLink);

			pFromNode->m_tobeRemoved = false;  // no need to remove as long as one adjacent link is kept
			pToNode->m_tobeRemoved = false; 

			++iLink;
		}
	}


	// step 03: remove nodes
	pDoc->m_NodeNoMap.clear();
	pDoc->m_NodeNoMap.clear();

	iNode = pDoc->m_NodeSet.begin ();
	while (iNode != pDoc->m_NodeSet.end())
	{
		if((*iNode)->m_NodeID ==511)
		{
			TRACE("..");
		}

		if((*iNode)->m_tobeRemoved == true )  //outside subarea
		{
			iNode = pDoc->m_NodeSet.erase (iNode);
		}
		else
		{
			pDoc->m_NodeIDMap[(*iNode)->m_NodeID  ] = (*iNode);
			pDoc->m_NodeIDMap[(*iNode)->m_NodeID  ] = (*iNode);

			++iNode;
			//inside subarea
		}
	}




	// step 05: add new zones if the boundary has zone --

	iNode = pDoc->m_NodeSet.begin ();
	int TAZ = 1000;  // mark for subarea

	int boundary_zone_count = 0;
	while (iNode != pDoc->m_NodeSet.end())
	{

		if((*iNode)->m_bSubareaFlag ==0 && (*iNode)->m_ZoneID == 0)  // remaining boundary-and-not-activity-location nodes after out of subarea links are removed. 
		{
			// create new zone number and add this node as activity center
			// try to get TAZ as the node number

			TAZ = (*iNode)->m_NodeID ;

			while(pDoc->m_ZoneMap.find (TAZ) != pDoc->m_ZoneMap.end ())  // find unused TAZ as new TAZ
			{
				TAZ ++;
			}

			pDoc->m_ZoneMap [TAZ].m_ZoneID = TAZ;
			(*iNode)->m_ZoneID = TAZ;



		}
		++iNode;
	}

	// to do: generate input_subarea_agent.csv file 
	// with updated departure time, path_node sequence, new origin zone id, new destination zone id
	// keep demand type, Agent type, 

	// step 06: generate route file
	std::list<DTAAgent*>::iterator iAgent;
	pDoc->m_PathMap.clear();
	pDoc->m_ODMatrixMap.clear();


	for (iAgent = pDoc->m_AgentSet.begin(); iAgent != pDoc->m_AgentSet.end(); iAgent++)
	{
		DTAAgent* pAgent = (*iAgent);

		if(pAgent->m_NodeSize >= 2 && pAgent->m_bComplete)  // with physical path in the network
		{

			// update node sequence
			std::vector<int> LinkVector;
			std::vector<int> NodeVector;

			int StartFlag = 0; // 0: not start yet, 1: start node chain, 2: end node chain
			for(int link= 1; link<pAgent->m_NodeSize; link++)
			{
				int OrgLinkNO = pAgent->m_NodeAry[link].LinkNo;


				if( pDoc->m_LinkNoMap.find(OrgLinkNO)!= pDoc->m_LinkNoMap.end()) // the link exists in subarea 
				{

					if(StartFlag <=1)
						LinkVector.push_back(OrgLinkNO);

					if(StartFlag ==0)
						StartFlag = 1; // start node chain
				}else  // link has been removed
				{
					if(StartFlag==1) // node chain has been started 
					{
						StartFlag = 2;  // terminate
						break;
					} else 
					{
						//continue // StartFlag ==0
					}
				}

			}
			// LinkVector for remaining links in subarea 

			if(LinkVector.size() >=2)
			{

				DTALink* pLinkOrg = pDoc->m_LinkNoMap[LinkVector[0]];
				DTALink* pLinkDes = pDoc->m_LinkNoMap[LinkVector[LinkVector.size()-1]];

				int NewOriginZoneID = pDoc->m_NodeNoMap[pLinkOrg->m_FromNodeNo ]->m_ZoneID  ;
				int NewDestinationZoneID = pDoc->m_NodeNoMap[pLinkDes->m_ToNodeNo ]->m_ZoneID  ;;
				int NewNodeIDSum = 0;

				for(unsigned li = 0; li < LinkVector.size(); li++)
				{
					DTALink* pLink = pDoc->m_LinkNoMap[LinkVector[li]];
					if(li==0) // first link
					{
						NewNodeIDSum += pLink->m_FromNodeID;
					}

					NewNodeIDSum += pLink->m_ToNodeID;
				}

				int Newm_NodeSize = LinkVector.size() +1;
				CString label;
				label.Format("%d,%d,%d,%d", NewOriginZoneID  , NewDestinationZoneID , NewNodeIDSum , Newm_NodeSize );

				//existing path
				pDoc->m_PathMap[label].Origin = NewOriginZoneID;
				pDoc->m_PathMap[label].Destination  = NewDestinationZoneID;
				pDoc->m_PathMap[label].TotalAgentSize+=1;

				if(pDoc->m_PathMap[label].TotalAgentSize == 1)  // new path
				{

					for(unsigned li = 0; li < LinkVector.size(); li++)
					{
						DTALink* pLink = pDoc->m_LinkNoMap[LinkVector[li]];
						if(li==0) // first link
						{
							pDoc->m_PathMap[label].m_NodeVector.push_back(pLink->m_FromNodeID );
						}

						pDoc->m_PathMap[label].m_NodeVector.push_back(pLink->m_ToNodeID);

					}
				}

				//
				// reuse label as OD label
				label.Format("%d,%d", pAgent->m_o_ZoneID  , pAgent->m_d_ZoneID);
				pDoc->m_ODMatrixMap[label].Origin = NewOriginZoneID;
				pDoc->m_ODMatrixMap[label].Destination  = NewDestinationZoneID;
				pDoc->m_ODMatrixMap[label].TotalAgentSize+=1;


			}

		}
	}


	// beginning of outputting subarea_agent file
	std::string fname_agent =  "subarea_agent.csv";
	
		FILE* st_agent = NULL;

		int number_of_subarea_Agents = 0;

		fopen_s(&st_agent, pDoc->m_ProjectDirectory + fname_agent.c_str(), "w");

		if (st_agent != NULL)
		{
			float first_entrance_time = 99999;
			float last_entrance_time = -99999;

			std::list<DTAAgent*>::iterator iAgent;

			fprintf(st_agent, "agent_id,o_zone_id,d_zone_id,o_node_id,d_node_id,time_period,agent_type,node_sequence\n");

			for (iAgent = pDoc->m_AgentSet.begin(); iAgent != pDoc->m_AgentSet.end(); iAgent++)
			{
				DTAAgent* pAgent = (*iAgent);
				float entrance_time = 0;
				if (pAgent->m_NodeSize >= 2 && pAgent->m_bComplete)  // with physical path in the network
				{
					number_of_subarea_Agents++;
					// update node sequence
					std::vector<int> LinkVector;
					std::vector<int> NodeVector;

					int StartFlag = 0; // 0: not start yet, 1: start node chain, 2: end node chain
					for (int link = 1; link < pAgent->m_NodeSize; link++)
					{
						int OrgLinkNO = pAgent->m_NodeAry[link].LinkNo;


						if (pDoc->m_LinkNoMap.find(OrgLinkNO) != pDoc->m_LinkNoMap.end()) // the link exists in subarea 
						{

							if (StartFlag <= 1)
								LinkVector.push_back(OrgLinkNO);

							if (StartFlag == 0)
							{ 
								StartFlag = 1; // start node chain
								entrance_time = pAgent->m_NodeAry[link].ArrivalTimeOnDSN;
								if (entrance_time < first_entrance_time)
									first_entrance_time = entrance_time;
								if (entrance_time > last_entrance_time)
									last_entrance_time = entrance_time;

							}
						}
						else  // link has been removed
						{
							if (StartFlag == 1) // node chain has been started 
							{
								StartFlag = 2;  // terminate
								break;
							}
							else
							{
								//continue // StartFlag ==0
							}
						}

					}
					// LinkVector for remaining links in subarea 

					if (LinkVector.size() >= 2)
					{

						DTALink* pLinkOrg = pDoc->m_LinkNoMap[LinkVector[0]];
						DTALink* pLinkDes = pDoc->m_LinkNoMap[LinkVector[LinkVector.size() - 1]];

						int NewOriginZoneID = pDoc->m_NodeNoMap[pLinkOrg->m_FromNodeNo]->m_ZoneID;
						int NewDestinationZoneID = pDoc->m_NodeNoMap[pLinkDes->m_ToNodeNo]->m_ZoneID;;
						int NewNodeIDSum = 0;

						for (unsigned li = 0; li < LinkVector.size(); li++)
						{
							DTALink* pLink = pDoc->m_LinkNoMap[LinkVector[li]];
							if (li == 0) // first link
							{
								NewNodeIDSum += pLink->m_FromNodeID;
							}

							NewNodeIDSum += pLink->m_ToNodeID;
						}

						int Newm_NodeSize = LinkVector.size() + 1;

						std::vector<int > NodeVector;
						for (unsigned li = 0; li < LinkVector.size(); li++)
						{
							DTALink* pLink = pDoc->m_LinkNoMap[LinkVector[li]];
							if (li == 0) // first link
							{
								NodeVector.push_back(pLink->m_FromNodeID);
							}

							NodeVector.push_back(pLink->m_ToNodeID);

						}


						fprintf(st_agent, "%d,%d,%d,%d,%d,%4.3f,%d,%d,",
							pAgent->m_AgentID,
							NewOriginZoneID,
							NewDestinationZoneID,
							NodeVector[0],
							NodeVector[NodeVector.size() - 1],
							entrance_time,
							pAgent->m_AgentType,
							
							Newm_NodeSize);




						for (int j = 0; j < NodeVector.size(); j++)  // for all nodes
						{
							fprintf(st_agent, "%d;", NodeVector[j]);
						}
						fprintf(st_agent, ",");

						fprintf(st_agent, "\n");

					}
				}
			}
			fclose(st_agent);

			pDoc->m_DemandLoadingEndTimeInMin = first_entrance_time;
			pDoc->m_DemandLoadingStartTimeInMin = last_entrance_time;
		}
		else
		{
			AfxMessageBox("File subarea_output.csv cannot be opened.");
		}

				
			  // not loaded in simulation


// end of generating agent csv file
	DeleteObject(m_polygonal_region);
	delete [] m_subarea_points;

	int zone_count = 0;

	std::map<int, DTAZone>	:: const_iterator itr_zone;

	

	pDoc->m_bSaveProjectFromSubareaCut =true;  // mark subarea cut

	CString SubareaCutMessage;
	SubareaCutMessage.Format ("The subarea includes %d nodes, %d links;\n%d zones, %d newly generated boundary zones;\n%d subarea OD pairs, %d subarea path records, %d subarea Agents.",
		pDoc->m_NodeSet.size(), pDoc->m_LinkSet.size(), zone_count,boundary_zone_count,pDoc->m_ODMatrixMap.size(),
		pDoc->m_PathMap.size(), number_of_subarea_Agents);

	AfxMessageBox(SubareaCutMessage, MB_ICONINFORMATION);

	Invalidate();
}

void CTLiteView::OnViewShowAVISensor()
{
	m_bShowAVISensor = !m_bShowAVISensor;
	Invalidate();
}

void CTLiteView::OnUpdateViewShowAVISensor(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAVISensor ? 1 : 0);
}

void CTLiteView::OnFileDataexchangewithgooglefusiontables()
{
}


void CTLiteView::OnEditDeletelinksoutsidesubarea()
{

	OnToolsRemovenodesandlinksoutsidesubarea();
}

void CTLiteView::OnEditMovenetworkcoordinates()
{
	m_MoveLayerNo = 0; //baseline layer
	m_ToolMode = network_coordinate_tool;
	m_bShowGrid = true;
	if(m_bNetworkCooridinateHints==false)
	{
		AfxMessageBox("Hints:\n1. Please use mouse to move the reference network and shift its coordinates;\n2. Use mouse wheel to adjust the size of the reference network (zoom);\n3. Click shift or control key together to change the horizonal or vertical size, respectively;\n4. Click on the other toolbar button to turn on disable this moving-reference-network mode.", MB_ICONINFORMATION);
		m_bNetworkCooridinateHints = true;
	}

	Invalidate();

}

void CTLiteView::OnUpdateEditMovenetworkcoordinates(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == network_coordinate_tool ? 1 : 0);
}


void CTLiteView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	switch (nChar)
	{
	case 27:  //ESCAPE

		m_ToolMode= select_feature_tool;

		break;
	case 90 :   //'Z'

		if( GetKeyState(VK_CONTROL) < 0)
		{
			CTLiteDoc* pDoc =GetDocument();
			pDoc->Undo();
		}

	}

	Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTLiteView::OnViewIncreasenodesize()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_LinkMOEMode != MOE_bottleneck && pDoc->m_LinkMOEMode != MOE_Agent)
	{
		pDoc->m_NodeDisplaySize = max(pDoc->m_NodeDisplaySize *1.2, pDoc->m_NodeDisplaySize+0.01);

	}
	else if(pDoc->m_LinkMOEMode == MOE_bottleneck)
	{

		pDoc->m_BottleneckDisplaySize = max(pDoc->m_BottleneckDisplaySize *1.2, pDoc->m_BottleneckDisplaySize+1);
	}
	else if (pDoc->m_LinkMOEMode == MOE_Agent)
	{

		pDoc->m_AgentDisplaySize = max(pDoc->m_AgentDisplaySize *1.2, pDoc->m_AgentDisplaySize + 1);
		
	}


	Invalidate();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();

}

void CTLiteView::OnViewDecreatenodesize()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_LinkMOEMode != MOE_bottleneck &&  pDoc->m_LinkMOEMode != MOE_Agent)
	{
		pDoc->m_NodeDisplaySize /=1.2;
		pDoc->m_NodeDisplaySize = max(0.00001,pDoc->m_NodeDisplaySize);
	}
	else if(pDoc->m_LinkMOEMode == MOE_bottleneck)
	{
		pDoc->m_BottleneckDisplaySize/=1.2;
		pDoc->m_BottleneckDisplaySize = max(0.00001,pDoc->m_BottleneckDisplaySize);
	}
	else if (pDoc->m_LinkMOEMode == MOE_Agent)
	{
		pDoc->m_AgentDisplaySize /= 1.2;
		pDoc->m_AgentDisplaySize = max(0.00001, pDoc->m_AgentDisplaySize);
	}


	Invalidate();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();


}


void CTLiteView::DrawLinkAsLine(DTALink* pLink, CDC* pDC)
{
	// 

	if(pLink->m_ShapePoints.size() == 0)
		return;

	// normal line
	CTLiteDoc* pDoc = GetDocument();

	if (pLink->m_FromNodeID == 7720 && pLink->m_ToNodeID == 7721)
		TRACE("");

	FromPoint = NPtoSP(pLink->m_ShapePoints[0]);
	ToPoint = NPtoSP(pLink->m_ShapePoints[pLink->m_ShapePoints.size() - 1]);

	if (FromPoint.x == ToPoint.x && FromPoint.y == ToPoint.y)  // same node
		return;

	if (abs(FromPoint.x - ToPoint.x < 10) && abs(FromPoint.y - ToPoint.y) < 10)  // almost same locations 
	{
		DrawLinkAsStraightLine(pLink, pDC);
		return;
	}

	for(int si = 0; si < pLink->m_ShapePoints.size()-1; si++)
	{
		FromPoint = NPtoSP(pLink->m_ShapePoints[si]);
		ToPoint = NPtoSP(pLink->m_ShapePoints[si+1]);

		if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
			continue; 


		if(pDoc->m_LinkMOEMode == MOE_queue_length)  // green color as background
		{

			if(pLink->m_LinkNo == pDoc->m_SelectedLinkNo)
				pDC->SelectObject(&g_PenBlue); 
			else
				pDC->SelectObject(&g_PenGreen); 
		}



		pDC->MoveTo(FromPoint);
		pDC->LineTo(ToPoint);

		if(m_bShowLinkArrow && si ==0)
		{
			double slopy = atan2((double)(FromPoint.y - ToPoint.y), (double)(FromPoint.x - ToPoint.x));
			double cosy = cos(slopy);
			double siny = sin(slopy);   
			double display_length  = sqrt((double)(FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y)+(double)(FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x));
			double arrow_size = min(7,display_length/5.0);

			if(arrow_size>0.2)
			{

				m_arrow_pts[0] = ToPoint;
				m_arrow_pts[1].x = ToPoint.x + (int)(arrow_size * cosy - (arrow_size / 2.0 * siny) + 0.5);
				m_arrow_pts[1].y = ToPoint.y + (int)(arrow_size * siny + (arrow_size / 2.0 * cosy) + 0.5);
				m_arrow_pts[2].x = ToPoint.x + (int)(arrow_size * cosy + arrow_size / 2.0 * siny + 0.5);
				m_arrow_pts[2].y = ToPoint.y - (int)(arrow_size / 2.0 * cosy - arrow_size * siny + 0.5);

				pDC->Polygon(m_arrow_pts, 3);
			}

		}

	}

	if(pDoc->m_LinkMOEMode == MOE_queue_length)   // queue length mode
	{
		CPen * pOldPen  = pDC->SelectObject(&g_PenQueueColor);
		float value;
		float queue_ratio = pDoc->GetLinkMOE(pLink, pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value);

		if(queue_ratio> 1)
			queue_ratio = 1;

		if(queue_ratio<0)
			queue_ratio = 0;


		if(pLink->m_FromNodeID == 11 && pLink->m_ToNodeID == 12)
		{
			TRACE("");

		}
		for(int si = 0; si < pLink->m_ShapePoints.size()-1; si++)
		{
			bool bDrawQueueCell = false;

			FromPoint = NPtoSP(pLink->m_ShapePoints[si]);
			ToPoint = NPtoSP(pLink->m_ShapePoints[si+1]);

			if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
				continue; 


			if(queue_ratio < 0.005f)
				break;


			GDPoint pt;

			if(pLink->m_ShapePoints.size() == 2)
			{  // simple straight line



				pt.x =  pLink->m_ShapePoints[0].x + (1-queue_ratio) * (pLink->m_ShapePoints[1].x - pLink->m_ShapePoints[0].x);
				pt.y =  pLink->m_ShapePoints[0].y + (1-queue_ratio) * (pLink->m_ShapePoints[1].y - pLink->m_ShapePoints[0].y);

				FromPoint = NPtoSP(pt);  // new to point as the end of queue line
				bDrawQueueCell = true;

			}else  // more than 2 feature points
			{
				if( pLink->m_ShapePointRatios[si] > 1-queue_ratio) //  1- queue_ratio) is the starting point to draw queue: e.g. queue_ratio = 0.5, then we should draw queeus from 0.5 to 1.0
				{
					bDrawQueueCell = true;

					if (si >=1 && pLink->m_ShapePointRatios[si-1] < (1-queue_ratio)) // first segment to draw the link
					{
						float ratio  = ((pLink->m_ShapePointRatios[si] - (1-queue_ratio))/max(0.00001,pLink->m_ShapePointRatios[si] -pLink-> m_ShapePointRatios[si-1] ));

						if(ratio >=1.1  || ratio <-0.1)
							TRACE("");

						pt.x =  pLink->m_ShapePoints[si-1].x + (1-ratio) * (pLink->m_ShapePoints[si].x - pLink->m_ShapePoints[si-1].x);
						pt.y =  pLink->m_ShapePoints[si-1].y + (1-ratio) * (pLink->m_ShapePoints[si].y - pLink->m_ShapePoints[si-1].y);

						FromPoint = NPtoSP(pt);  // new from point as the end of queue line
						ToPoint = NPtoSP(pLink->m_ShapePoints[si]);  // to point as the beginning of queue line

					}


				}


			}
			if(bDrawQueueCell)
			{
				pDC->MoveTo(FromPoint);
				pDC->LineTo(ToPoint);
			}

		}



		pDC->SelectObject(pOldPen);
	}

}

void CTLiteView::DrawLinkAsStraightLine(DTALink* pLink, CDC* pDC)
{
	// 

	if (pLink->m_ShapePoints.size() == 0)
		return;

	// normal line
	CTLiteDoc* pDoc = GetDocument();

		FromPoint = NPtoSP(pLink->m_ShapePoints[0]);
		ToPoint = NPtoSP(pLink->m_ShapePoints[pLink->m_ShapePoints.size() - 1]);


		if (pDoc->m_LinkMOEMode == MOE_queue_length)  // green color as background
		{

			if (pLink->m_LinkNo == pDoc->m_SelectedLinkNo)
				pDC->SelectObject(&g_PenBlue);
			else
				pDC->SelectObject(&g_PenGreen);
		}

		pDC->MoveTo(FromPoint);
		pDC->LineTo(ToPoint);

		if (m_bShowLinkArrow)
		{
			double slopy = atan2((double)(FromPoint.y - ToPoint.y), (double)(FromPoint.x - ToPoint.x));
			double cosy = cos(slopy);
			double siny = sin(slopy);
			double display_length = sqrt((double)(FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y) + (double)(FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x));
			double arrow_size = min(7, display_length / 5.0);

			if (arrow_size>0.2)
			{

				m_arrow_pts[0] = ToPoint;
				m_arrow_pts[1].x = ToPoint.x + (int)(arrow_size * cosy - (arrow_size / 2.0 * siny) + 0.5);
				m_arrow_pts[1].y = ToPoint.y + (int)(arrow_size * siny + (arrow_size / 2.0 * cosy) + 0.5);
				m_arrow_pts[2].x = ToPoint.x + (int)(arrow_size * cosy + arrow_size / 2.0 * siny + 0.5);
				m_arrow_pts[2].y = ToPoint.y - (int)(arrow_size / 2.0 * cosy - arrow_size * siny + 0.5);

				pDC->Polygon(m_arrow_pts, 3);
			}

		}

	

	if (pDoc->m_LinkMOEMode == MOE_queue_length)   // queue length mode
	{
		CPen * pOldPen = pDC->SelectObject(&g_PenQueueColor);
		float value;
		float queue_ratio = pDoc->GetLinkMOE(pLink, pDoc->m_LinkMOEMode, (int)g_Simulation_Time_Stamp, g_MOEAggregationIntervalInMin, value);

		if (queue_ratio> 1)
			queue_ratio = 1;

		if (queue_ratio<0)
			queue_ratio = 0;


			bool bDrawQueueCell = false;


			if (queue_ratio < 0.005f)
				return;


			GDPoint pt;

			pt.x = pLink->m_ShapePoints[0].x + (1 - queue_ratio) * (pLink->m_ShapePoints[pLink->m_ShapePoints.size()-1].x - pLink->m_ShapePoints[0].x);
			pt.y = pLink->m_ShapePoints[0].y + (1 - queue_ratio) * (pLink->m_ShapePoints[pLink->m_ShapePoints.size() - 1].y - pLink->m_ShapePoints[0].y);

				FromPoint = NPtoSP(pt);  // new to point as the end of queue line
				bDrawQueueCell = true;

			if (bDrawQueueCell)
			{
				pDC->MoveTo(FromPoint);
				pDC->LineTo(ToPoint);
			}


		pDC->SelectObject(pOldPen);
	}


}

bool CTLiteView::DrawLinkAsBand(DTALink* pLink, CDC* pDC, bool bObservationFlag =false)
{
	// draw queue length
	CTLiteDoc* pDoc = GetDocument();


	if(pLink->m_BandLeftShapePoints.size() == 0)
		return false;

	int band_point_index = 0;  

	if (pLink->m_FromNodeID == 7720 && pLink->m_ToNodeID == 7721)
		TRACE("");

	if(pLink ->m_ShapePoints.size() > 900)
	{
//		AfxMessageBox("Too many shape points...");
		return false;
	}
	int si; // we should not use unsigned integer here as si-- 
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();


	FromPoint = NPtoSP(pLink->m_ShapePoints[0]);
	ToPoint = NPtoSP(pLink->m_ShapePoints[pLink->m_ShapePoints.size() - 1]);

	if (FromPoint.x == ToPoint.x && FromPoint.y == ToPoint.y)  // same node
		return true;

	if (abs(FromPoint.x - ToPoint.x < 10) && abs(FromPoint.y - ToPoint.y) < 10)  // almost same locations 
	{
		DrawLinkAsStraightBand(pLink, pDC, bObservationFlag);
		return true;
	}

	if(bObservationFlag == false)
	{  // simulated data

		if( pLink ->m_BandLeftShapePoints.size() > 0)
		{
			for(si = 0; si < pLink ->m_BandLeftShapePoints .size(); si++)
			{
				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[si]);
			}

			for(si = pLink ->m_BandRightShapePoints .size()-1; si >=0 ; si--)
			{
				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandRightShapePoints[si]);
			}

			m_BandPoint[band_point_index++]= NPtoSP(pLink->m_BandLeftShapePoints[0]);
		}

	}

	if(pDoc->m_LinkMOEMode == MOE_queue_length)  // green color as background
	{

		if(pLink->m_LinkNo == pDoc->m_SelectedLinkNo)
		{
			pDC->SelectObject(&g_PenBlue); 
			pDC->SelectObject(&g_BrushBlue); 


		}else
		{
			pDC->SelectObject(&g_PenGreen); 
			pDC->SelectObject(&g_BrushGreen); 
		}

	}

	pDC->Polygon(m_BandPoint, band_point_index);

	// ****************************************/

	if(pDoc->m_LinkMOEMode == MOE_queue_length)   // queue length mode
	{
		CPen * pOldPen  = pDC->SelectObject(&g_PenQueueColor);
		float value;

		if(pLink->m_FromNodeID == 11 && pLink->m_ToNodeID == 12)
		{
			TRACE("");

		}
		float queue_ratio = pDoc->GetLinkMOE(pLink, pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value)/100.0;



		if(queue_ratio> 1)
			queue_ratio = 1;

		if(queue_ratio<0)
			queue_ratio = 0;



		for(int si = 0; si < pLink->m_ShapePoints.size()-1; si++)
		{
			bool bDrawQueueCell = false;

			FromPoint = NPtoSP(pLink->m_ShapePoints[si]);
			ToPoint = NPtoSP(pLink->m_ShapePoints[si+1]);

			if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
				continue; 


			GDPoint pt;

			if(pLink->m_ShapePoints.size() == 2)
			{  // simple straight line


				if(queue_ratio < 0.01f)
					break;

				pt.x =  pLink->m_ShapePoints[0].x + (1-queue_ratio) * (pLink->m_ShapePoints[1].x - pLink->m_ShapePoints[0].x);
				pt.y =  pLink->m_ShapePoints[0].y + (1-queue_ratio) * (pLink->m_ShapePoints[1].y - pLink->m_ShapePoints[0].y);

				FromPoint = NPtoSP(pt);  // new to point as the end of queue line
				bDrawQueueCell = true;

			}else  // more than 2 feature points
			{
				if( pLink->m_ShapePointRatios[si] > 1-queue_ratio) //  1- queue_ratio) is the starting point to draw queue: e.g. queue_ratio = 0.5, then we should draw queeus from 0.5 to 1.0
				{
					bDrawQueueCell = true;

					if (si >=1 && pLink->m_ShapePointRatios[si-1] < (1-queue_ratio)) // first segment to draw the link
					{
						float ratio  = ((pLink->m_ShapePointRatios[si] - (1-queue_ratio))/max(0.00001,pLink->m_ShapePointRatios[si] -pLink-> m_ShapePointRatios[si-1] ));

						if(ratio >=1.1  || ratio <-0.1)
							TRACE("");

						pt.x =  pLink->m_ShapePoints[si-1].x + (1-ratio) * (pLink->m_ShapePoints[si].x - pLink->m_ShapePoints[si-1].x);
						pt.y =  pLink->m_ShapePoints[si-1].y + (1-ratio) * (pLink->m_ShapePoints[si].y - pLink->m_ShapePoints[si-1].y);

						FromPoint = NPtoSP(pt);  // new to point as the end of queue line

						ToPoint = NPtoSP(pLink->m_ShapePoints[si]);  // end of queue line

					}


				}


			}
			if(bDrawQueueCell)
			{
				pDC->MoveTo(FromPoint);
				pDC->LineTo(ToPoint);
			}

		}



		pDC->SelectObject(pOldPen);
	}


	return true;
}


bool CTLiteView::DrawLinkAsStraightBand(DTALink* pLink, CDC* pDC, bool bObservationFlag = false)
{
	// draw queue length
	CTLiteDoc* pDoc = GetDocument();


	if (pLink->m_BandLeftShapePoints.size() == 0)
		return false;

	int band_point_index = 0;

	if (pLink->m_ShapePoints.size() > 900)
	{
//		AfxMessageBox("Too many shape points...");
		return false;
	}
	int si; // we should not use unsigned integer here as si-- 
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();


	FromPoint = NPtoSP(pLink->m_ShapePoints[0]);
	ToPoint = NPtoSP(pLink->m_ShapePoints[pLink->m_ShapePoints.size() - 1]);

	if (FromPoint.x == ToPoint.x && FromPoint.y == ToPoint.y)  // same node
		return true;

	if (bObservationFlag == false)
	{  // simulated data

		if (pLink->m_BandLeftShapePoints.size() > 0)
		{

				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[0]);
				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[pLink->m_BandRightShapePoints.size() - 1]);

				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandRightShapePoints[pLink->m_BandRightShapePoints.size() - 1]);
				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandRightShapePoints[0]);

				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[0]);
		}

	}
	else

	if (pDoc->m_LinkMOEMode == MOE_queue_length)  // green color as background
	{

		if (pLink->m_LinkNo == pDoc->m_SelectedLinkNo)
		{
			pDC->SelectObject(&g_PenBlue);
			pDC->SelectObject(&g_BrushBlue);


		}
		else
		{
			pDC->SelectObject(&g_PenGreen);
			pDC->SelectObject(&g_BrushGreen);
		}

	}

	pDC->Polygon(m_BandPoint, band_point_index);

	// ****************************************/

	if (pDoc->m_LinkMOEMode == MOE_queue_length)   // queue length mode
	{
		CPen * pOldPen = pDC->SelectObject(&g_PenQueueColor);
		float value;

		if (pLink->m_FromNodeID == 11 && pLink->m_ToNodeID == 12)
		{
			TRACE("");

		}
		float queue_ratio = pDoc->GetLinkMOE(pLink, pDoc->m_LinkMOEMode, (int)g_Simulation_Time_Stamp, g_MOEAggregationIntervalInMin, value) / 100.0;



		if (queue_ratio> 1)
			queue_ratio = 1;

		if (queue_ratio<0)
			queue_ratio = 0;



			bool bDrawQueueCell = false;

			if (FromPoint.x == ToPoint.x && FromPoint.y == ToPoint.y)  // same node
				return true;


			GDPoint pt;

				if (queue_ratio < 0.01f)
					return true;

				pt.x = pLink->m_ShapePoints[0].x + (1 - queue_ratio) * (pLink->m_ShapePoints[pLink->m_ShapePoints.size()-1].x - pLink->m_ShapePoints[0].x);
				pt.y = pLink->m_ShapePoints[0].y + (1 - queue_ratio) * (pLink->m_ShapePoints[pLink->m_ShapePoints.size() - 1].y - pLink->m_ShapePoints[0].y);

				FromPoint = NPtoSP(pt);  // new to point as the end of queue line
				bDrawQueueCell = true;

			if (bDrawQueueCell)
			{
				pDC->MoveTo(FromPoint);
				pDC->LineTo(ToPoint);
			}

				pDC->SelectObject(pOldPen);
	}


	return true;
}



void CTLiteView::OnViewDisplaylanewidth()
{
	if(m_link_display_mode ==link_display_mode_line )
		m_link_display_mode = link_display_mode_band;
	else
		m_link_display_mode = link_display_mode_line;

	Invalidate();
}


void CTLiteView::OnNodeCheckconnectivityfromhere()
{
	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit
	pDoc->m_ONodeNo = pDoc->m_SelectedNodeNo;
	pDoc->Routing(true);
	m_ShowAllPaths = true;
	Invalidate();
}



extern void g_RandomCapacity(float* ptr, int num, float mean, float COV,int seed);
#define MAX_SAMPLE_SIZE 200

void CTLiteView::OnNodeDirectiontohereandreliabilityanalysis()
{   
	OnNodeDestination();


}
void CTLiteView::OnLinkIncreasebandwidth()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_MaxLinkWidthAsLinkVolume  /=1.2;

	pDoc->GenerateOffsetLinkBand();

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();

	Invalidate();
}

void CTLiteView::OnLinkDecreasebandwidth()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_MaxLinkWidthAsLinkVolume  *=1.2;

	pDoc->GenerateOffsetLinkBand();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();
	Invalidate();

}

void CTLiteView::OnLinkSwichtolineBandwidthMode()
{

	if(m_link_display_mode == link_display_mode_band)
		m_link_display_mode = link_display_mode_line;
	else
		m_link_display_mode = link_display_mode_band;

	CTLiteDoc* pDoc = GetDocument();
	pDoc->GenerateOffsetLinkBand();


	Invalidate();
}

void CTLiteView::OnViewTransitlayer()
{
	m_bShowTransit = !m_bShowTransit;
	Invalidate();
}

void CTLiteView::OnUpdateViewTransitlayer(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowTransit);
}

void CTLiteView::DrawPublicTransitLayer(CDC *pDC)
{

	CTLiteDoc* pDoc = GetDocument();
	std::map<long, long> RouteMap;

	int stop_size = min(20,max(2,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution)));

	TEXTMETRIC tmOD;
	memset(&tmOD, 0, sizeof TEXTMETRIC);
	pDC->GetOutputTextMetrics(&tmOD);

	CRect ScreenRect;
	GetClientRect(ScreenRect);


}

void CTLiteView::DrawPublicTransitAccessibilityLayer(CDC *pDC)
{

	//CWaitCursor wait;
	CTLiteDoc* pDoc = GetDocument();
	std::map<long, long> RouteMap;

	int stop_size = min(20,max(2,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitDistance*m_Resolution)));

	TEXTMETRIC tmOD;
	memset(&tmOD, 0, sizeof TEXTMETRIC);
	pDC->GetOutputTextMetrics(&tmOD);

	CRect ScreenRect;
	GetClientRect(ScreenRect);


}



void CTLiteView::DrawNodeRadarChart(CDC *pDC, DTANode* pNode, CPoint point, int chart_size, float delay_ratio, int LOS)
{

	CTLiteDoc* pDoc = GetDocument();

 	CRect ChartRect(point.x - chart_size, point.y + chart_size,
			point.x + chart_size, point.y - chart_size);


	int theSliceCount = 0;

    CPen pen(PS_SOLID, 1, RGB(0,0,255));
    pDC->SelectObject(&pen);

	if(pNode->m_IncomingLinkVector.size()==1)
	{
    pDC->Ellipse(ChartRect);
	
		return;
	}


   
	
	 std::vector<IncomingLinkAngle> IncomingLinkVector;


		for(unsigned int i = 0; i< pNode->m_IncomingLinkVector.size(); i++)
		{
	
			DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_IncomingLinkVector[i]];
			int Angle = pDoc->Find_P2P_Angle(  pDoc->m_NodeNoMap[pLink->m_FromNodeNo]->pt, pDoc->m_NodeNoMap[pLink->m_ToNodeNo]->pt);
		
		
			IncomingLinkAngle element;
			element.pLink = pLink;
			element.Angle = Angle;

		
			IncomingLinkVector.push_back(element);
		}


		std::sort(IncomingLinkVector.begin(), IncomingLinkVector.end(), compare_IncomingLinkAngleData);


		CPoint PtVector[10];
		int vector_size = min(10, IncomingLinkVector.size());
		for(unsigned int i = 0; i< vector_size ; i++)
		{
			
			DTALink* pLink = IncomingLinkVector[i].pLink ;

			float link_delay = pLink->GetDynamicNodeDelay(g_Simulation_Time_Stamp );
			float radius_onscreen = link_delay *pDoc->m_BottleneckDisplaySize;

			radius_onscreen = max(2,radius_onscreen);
			

			int point_size = pLink->m_ShapePoints.size();


			CPoint pt0 =  NPtoSP(pLink->m_ShapePoints[0]); 
			CPoint pt1 =  NPtoSP(pLink->m_ShapePoints[point_size - 1]); 

			double delta_x = pt0.x- pt1.x;
			double delta_y = pt0.y- pt1.y;

			double link_screen_size = sqrt(pow(delta_x,2) +  pow(delta_y,2) );

			double ratio = 1.0f - radius_onscreen/max(1,link_screen_size);
			CPoint pt;

			pt.x =  (1-ratio)*pt0.x + ratio * pt1.x;
			pt.y =  (1-ratio)*pt0.y + ratio * pt1.y;

				PtVector[i]= pt;
			
		}

		   pDC->Polygon(PtVector, vector_size);


//		for(unsigned int i = 0; i< theSliceCount; i++)
//		{
//			
//			DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_IncomingLinkVector[i]];
//			
//			SlicePercentage[i] = link_delay[i]/ max(1,total_delay)*100;
//
//				// method 1: identify movement approach from scratch 
//			DTA_Direction direction =  pDoc->Find_Closest_Angle_to_Approach( pDoc->Find_P2P_Angle(pLink->m_FromPoint,pLink->m_ToPoint));
//
//			color_vector[i] = pDoc->m_ColorDirectionVector[direction];
//
//		}
///*
//		for(unsigned int i = 0; i< theSliceCount; i++)
//		{
//			
//			DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_IncomingLinkVector[i]];
//			SlicePercentage[i] = link_delay[i]/ max(1,total_delay)*100;
//
//			thePieInfo[i].color = 
//			thePieInfo[i].desc.Format("  ");
//			thePieInfo[i].percentage = SlicePercentage[i];
//
//		}
//		*/
//	thePieInfo[0] = PieInfo("Stocks", SlicePercentage[0], color_vector[0]);
//    thePieInfo[1] = PieInfo("Bonds", SlicePercentage[1], color_vector[1]);
//    thePieInfo[2] = PieInfo("Bonds", SlicePercentage[2], color_vector[2]);
//    thePieInfo[3] = PieInfo("Bonds", SlicePercentage[3], color_vector[3]);
//    thePieInfo[4] = PieInfo("Bonds", SlicePercentage[4], color_vector[4]);
// 
//
//
//    double percent = 0;
//
//    for(int i = 0; i < theSliceCount; i++)
//    {
//        percent += thePieInfo[i].percentage;
//        if(percent > 100)
//            percent = 100;
//        double angle = percent / 100 * 360;
//        double x = radius * cos(angle * PI / 180);
//        double y = radius * sin(angle * PI / 180);
//
//		SlicePt[i].x = (int)(xCenter + x + 0.5);
//		SlicePt[i].y = (int) (yCenter + y + 0.5);
//    }
// 
//
//    for(int i = 0; i < theSliceCount; i++)
//    {
//        CBrush br;
//        br.CreateSolidBrush(thePieInfo[i].color);
//        pDC->SelectObject(&br);
//
//		int index = i+1;
//
//		if(index == theSliceCount)
//			index =0;
//
//		pDC->Pie(rect,SlicePt[i], SlicePt[index]);
//		
//	}

}
void CTLiteView::DrawNodePieChart(CDC *pDC, DTANode* pNode, CPoint point, int chart_size, int LOS)
{
	PieInfo thePieInfo[32];
	CPoint SlicePt[32];
	float SlicePercentage[32];
	float link_delay[32];
	COLORREF color_vector[32];

	CTLiteDoc* pDoc = GetDocument();
	CRect ChartRect(point.x - chart_size, point.y + chart_size,
			point.x + chart_size, point.y - chart_size);


	int theSliceCount = 0;

	CRect rect = ChartRect;
  
    CPen pen(PS_SOLID, 1, RGB(0,0,255));
    pDC->SelectObject(&pen);
    pDC->Ellipse(rect);
 
  //  double xCenter = (rect.left + rect.right) / 2;
  //  double yCenter = (rect.top + rect.bottom) / 2;
 
  //  double radius = rect.Height() / 2;

		//theSliceCount = min(2,pNode->m_IncomingLinkVector.size());
 
		//float total_delay = 0;
		//
		//for(unsigned int i = 0; i< theSliceCount; i++)
		//{
		//	
		//	DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_IncomingLinkVector[i]];

		//	link_delay[i]= pLink->GetSimulatedNodeDelay(g_Simulation_Time_Stamp);
		//	total_delay+= link_delay[i];
		//}


//		for(unsigned int i = 0; i< theSliceCount; i++)
//		{
//			
//			DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_IncomingLinkVector[i]];
//			
//			SlicePercentage[i] = link_delay[i]/ max(1,total_delay)*100;
//
//				// method 1: identify movement approach from scratch 
//			DTA_Direction direction =  pDoc->Find_Closest_Angle_to_Approach( pDoc->Find_P2P_Angle(pLink->m_FromPoint,pLink->m_ToPoint));
//
//			color_vector[i] = pDoc->m_ColorDirectionVector[direction];
//
//		}
///*
//		for(unsigned int i = 0; i< theSliceCount; i++)
//		{
//			
//			DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_IncomingLinkVector[i]];
//			SlicePercentage[i] = link_delay[i]/ max(1,total_delay)*100;
//
//			thePieInfo[i].color = 
//			thePieInfo[i].desc.Format("  ");
//			thePieInfo[i].percentage = SlicePercentage[i];
//
//		}
//		*/
//	thePieInfo[0] = PieInfo("Stocks", SlicePercentage[0], color_vector[0]);
//    thePieInfo[1] = PieInfo("Bonds", SlicePercentage[1], color_vector[1]);
//    thePieInfo[2] = PieInfo("Bonds", SlicePercentage[2], color_vector[2]);
//    thePieInfo[3] = PieInfo("Bonds", SlicePercentage[3], color_vector[3]);
//    thePieInfo[4] = PieInfo("Bonds", SlicePercentage[4], color_vector[4]);
// 
//
//
//    double percent = 0;
//
//    for(int i = 0; i < theSliceCount; i++)
//    {
//        percent += thePieInfo[i].percentage;
//        if(percent > 100)
//            percent = 100;
//        double angle = percent / 100 * 360;
//        double x = radius * cos(angle * PI / 180);
//        double y = radius * sin(angle * PI / 180);
//
//		SlicePt[i].x = (int)(xCenter + x + 0.5);
//		SlicePt[i].y = (int) (yCenter + y + 0.5);
//    }
// 
//
//    for(int i = 0; i < theSliceCount; i++)
//    {
//        CBrush br;
//        br.CreateSolidBrush(thePieInfo[i].color);
//        pDC->SelectObject(&br);
//
//		int index = i+1;
//
//		if(index == theSliceCount)
//			index =0;
//
//		pDC->Pie(rect,SlicePt[i], SlicePt[index]);
//		
//	}

}

void CTLiteView::DrawNode(CDC *pDC, DTANode* pNode, CPoint point, int node_size,TEXTMETRIC tm)
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();

	if (pMainFrame->m_bShowLayerMap[layer_reference_line]==false)
	{
		if (pNode->m_LayerNo == 1)  //skip reference nodes
			return; 
	}

	//if(pNode->m_ZoneID >0)  // if destination node associated with zones
	//{
	//	float zone_multipler = 1.2;
	//	pDC->Ellipse(point.x - node_size*zone_multipler, point.y + node_size*zone_multipler,
	//		point.x + node_size*zone_multipler, point.y - node_size*zone_multipler);
	//}
	CTLiteDoc* pDoc = GetDocument();

	if (node_size < 2)  // node display size is too small to  show node labels 
	{
	
		pDC->MoveTo(point.x, point.y);
		pDC->LineTo(point.x, point.y);

		return;
	}
	else
	{

		if (m_ShowNodeTextMode == node_display_zone_number && pNode->m_ZoneID <= 0)
		{
			//donothing
		}
		else
		{


			if (pNode->m_ControlType == pDoc->m_ControlType_PretimedSignal ||
				pNode->m_ControlType == pDoc->m_ControlType_ActuatedSignal)  // traffic signal control
			{
				pDC->Rectangle(point.x - node_size, point.y + node_size,
					point.x + node_size, point.y - node_size);
			}
			else if (pNode->m_ControlType == pDoc->m_ControlType_YieldSign ||
				pNode->m_ControlType == pDoc->m_ControlType_2wayStopSign ||
				pNode->m_ControlType == pDoc->m_ControlType_4wayStopSign)
			{
				int  width_of_ellipse = node_size * 4 / 5;
				pDC->RoundRect(point.x - node_size, point.y + node_size,
					point.x + node_size, point.y - node_size, width_of_ellipse, width_of_ellipse);

			}
			else
			{
				pDC->Ellipse(point.x - node_size, point.y + node_size,
					point.x + node_size, point.y - node_size);
			}

		}
	}
	// 
	if (node_size < 4)  // node display size is too small to  show node labels 
		return; 

	if(m_ShowNodeTextMode != node_display_none)
	{

		CString str_node_label;
		str_node_label.Format ("%d",pNode->m_NodeID );

		if(m_ShowNodeTextMode == node_display_sequential_node_number)
		{
			str_node_label.Format ("%d",pNode->m_NodeID +1 );

		}



		if(m_ShowNodeTextMode == node_display_zone_number)
		{

			if(pNode->m_ZoneID > 0)  // external origin
				str_node_label.Format ("%d",pNode->m_ZoneID );
			else
				str_node_label.Format ("");
		}

	




		if(m_ShowNodeTextMode == node_display_intersection_name)
		{
			if(pNode->m_Name .size() >0 && pNode->m_Name!="(null)")
				str_node_label.Format ("%s",pNode->m_Name.c_str()  );
			else
				str_node_label.Format ("");

		}

		if(m_ShowNodeTextMode == node_display_control_type)
		{

			if( pDoc->m_NodeTypeMap.find(pNode->m_ControlType)!= pDoc->m_NodeTypeMap.end())
			{
				str_node_label = pDoc->m_NodeTypeMap[pNode->m_ControlType].c_str() ;
			}

		}




		if(m_ShowNodeTextMode == node_display_travel_time_from_origin && pNode->m_DistanceToRoot > 0.00001 && pNode->m_DistanceToRoot < MAX_SPLABEL-1)  // check connectivity, overwrite with distance to the root
			str_node_label.Format ("%4.1f",pNode->m_DistanceToRoot );


		point.y -= tm.tmHeight / 2;

		pDC->TextOut(point.x , point.y,str_node_label);
	}

	if(pNode->m_DistanceToRoot > MAX_SPLABEL-1)  //restore pen
		pDC->SelectObject(&g_PenNodeColor);





}



void CTLiteView::OnNodeMovementproperties()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc == NULL)
		return;

	if(pDoc->m_ProjectDirectory.GetLength () ==0)
	{

		AfxMessageBox("Please first save and then reload the data set to show movement data.");
		return;
	}

	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	if(pDoc->m_SelectedNodeNo >=0)
	{
		DTANode* pNode = pDoc->m_NodeNoMap [pDoc->m_SelectedNodeNo ];

		if(pNode==NULL)
			return;

		CPage_Node_Movement pMovementPage;

		pMovementPage.m_pDoc = pDoc;
		pMovementPage.m_CurrentNodeNo = pNode->m_NodeNo;
		pMovementPage.DoModal();

	}
}



void CTLiteView::OnViewShowConnector()
{
	m_bShowConnector = ! m_bShowConnector;
	Invalidate();
}

void CTLiteView::OnUpdateViewShowConnector(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowConnector);
}


void CTLiteView::OnViewHighlightcentroidsandactivitylocations()
{
	m_bHighlightActivityLocation = !m_bHighlightActivityLocation;
	Invalidate();
}

void CTLiteView::OnUpdateViewHighlightcentroidsandactivitylocations(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHighlightActivityLocation);
}

void CTLiteView::OnViewBackgroundcolor()
{
	CTLiteDoc* pDoc = GetDocument();

	CColorDialog dlg(theApp.m_BackgroundColor, CC_FULLOPEN);
	if (dlg.DoModal() == IDOK)
	{
		theApp.m_BackgroundColor= dlg.GetColor();
		pDoc->UpdateAllViews(0);
		pDoc->FileSaveColorScheme();

	}
	Invalidate();
}

void CTLiteView::OnMoeOddemand()
{
	m_bShowODDemandVolume = !m_bShowODDemandVolume;
	Invalidate();

}

void CTLiteView::OnUpdateMoeOddemand(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowODDemandVolume);
}

void CTLiteView::SetGlobalViewParameters()
{

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	if(pMainFrame->m_bSynchronizedDisplay)
	{
		std::list<CTLiteView*>::iterator iView = g_ViewList.begin ();
		while (iView != g_ViewList.end())
		{
			if((*iView) != this)  // not this window
			{
				(*iView)->m_Resolution  = m_Resolution;
				(*iView)->m_ScreenOrigin = m_ScreenOrigin;
				(*iView)->m_Origin  = m_Origin;

				CTLiteDoc * pDoc = (*iView)->GetDocument();
				pDoc->m_MaxLinkWidthAsLinkVolume = GetDocument()->m_MaxLinkWidthAsLinkVolume;
				(*iView)->Invalidate ();

			}
			iView++;
		}
	}
}
void CTLiteView::OnUpdateLinkIncreasebandwidth(CCmdUI *pCmdUI)
{
	CTLiteDoc* pDoc = GetDocument();
	pCmdUI->Enable ((pDoc->m_LinkMOEMode == MOE_volume || pDoc->m_LinkMOEMode == MOE_bottleneck || pDoc->m_LinkMOEMode == MOE_speed || pDoc->m_LinkMOEMode == MOE_density ||pDoc->m_LinkMOEMode == MOE_none) );

}

void CTLiteView::OnUpdateLinkDecreasebandwidth(CCmdUI *pCmdUI)
{
	CTLiteDoc* pDoc = GetDocument();

	pCmdUI->Enable ((pDoc->m_LinkMOEMode == MOE_volume || pDoc->m_LinkMOEMode == MOE_bottleneck || pDoc->m_LinkMOEMode == MOE_speed || pDoc->m_LinkMOEMode == MOE_density || pDoc->m_LinkMOEMode == MOE_none));
}


void CTLiteView::CreateDefaultJunction()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc == NULL)
		return;


}


void CTLiteView::OnExportCreatevissimfiles()
{

}


void CTLiteView::OnDebugShowAgentswithincompletetripsonly()
{
	bShowAgentsWithIncompleteTrips = !bShowAgentsWithIncompleteTrips;
	Invalidate();
}

void CTLiteView::OnUpdateDebugShowAgentswithincompletetripsonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(bShowAgentsWithIncompleteTrips);
}

void CTLiteView::OnAgentAgentnumber()
{
	m_bShowAgentNumber = !m_bShowAgentNumber;
	Invalidate();

}

void CTLiteView::OnUpdateAgentAgentnumber(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAgentNumber);
}

void CTLiteView::OnAgentShowselectedAgentonly()
{
	m_bShowSelectedAgentOnly = ! m_bShowSelectedAgentOnly;
	Invalidate();

}

void CTLiteView::OnUpdateAgentShowselectedAgentonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowSelectedAgentOnly);
}

void CTLiteView::OnNodeAddintermediatedestinationhere()
{
	CTLiteDoc* pDoc = GetDocument();

	int SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	if(SelectedNodeNo>=0)
	{

		pDoc->m_IntermediateDestinationVector.push_back(SelectedNodeNo);

		pDoc->m_NodeNoMap[SelectedNodeNo]->m_IntermediateDestinationNo = pDoc->m_IntermediateDestinationVector.size();

	}



	pDoc->Routing(false);


	m_ShowAllPaths = true;
	Invalidate();
}

void CTLiteView::OnNodeRemoveallintermediatedestination()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	pDoc->m_IntermediateDestinationVector.clear();
	std::list<DTANode*>::iterator iNode;

	for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
	{
		(*iNode)->m_IntermediateDestinationNo = 0;

	}

	std::list<DTALink*>::iterator iLink;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_AdditionalCost = 0;
	}

	pDoc->Routing(false);


	m_ShowAllPaths = true;
	Invalidate();
}

void CTLiteView::OnLinkAvoidusingthislinkinrouting()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink = pDoc->m_LinkNoMap [pDoc->m_SelectedLinkNo ];
	pLink -> m_AdditionalCost = 10000;
	pDoc->Routing(false,true);


	m_ShowAllPaths = true;
	Invalidate();

}


void CTLiteView::OnBnClickedButtonConfiguration()
{

	CDlg_DisplayConfiguration* m_pDlg = new CDlg_DisplayConfiguration;
	m_pDlg->pView = this;

	CTLiteDoc* pDoc = GetDocument();

	m_pDlg->m_bShowSignalNodeMovementOnly = 	pDoc->m_bShowSignalNodeMovementOnly ;

	m_pDlg->m_ShowNodeTextMode = this ->m_ShowNodeTextMode;
	m_pDlg->SetModelessFlag(true); // voila! this is all it takes to make your dlg modeless!
	m_pDlg->Create(IDD_DIALOG_DISPLAY_CONFIG); 
	m_pDlg->ShowWindow(SW_SHOW); 

}

void CTLiteView::OnNodeNodeproperties()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc == NULL)
		return;

	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	if (pDoc->m_SelectedNodeNo >= 0)
	{
		DTANode* pNode = pDoc->m_NodeNoMap[pDoc->m_SelectedNodeNo];

		if (pNode == NULL)
			return;

		CDlg_NodeProperties dlg;

		dlg.m_pDoc = pDoc;

		dlg.NodeNo = pNode->m_NodeID;
		dlg.NodeName = pNode->m_Name.c_str();
		dlg.ZoneID = pNode->m_ZoneID;
		dlg.m_ControlType = pNode->m_ctrl_type.c_str();
		dlg.m_NodeType = pNode->m_node_type.c_str();

		if (dlg.DoModal() == IDOK)
		{

			CT2CA pszConvertedAnsiString(dlg.NodeName);
			// construct a std::string using the LPCSTR input
			std::string strStd(pszConvertedAnsiString);

			if (pNode->m_ctrl_type .compare (dlg.m_ControlType)!=0
				|| pNode->m_node_type.compare(dlg.m_NodeType)!=0
				|| pNode->m_Name.compare (strStd)!=0
				|| pNode->m_ZoneID != dlg.ZoneID
				)
			{
				pDoc->Modify();
				pDoc->PushBackNetworkState();
				pNode->m_Name  = strStd;
				pNode->m_ZoneID = dlg.ZoneID;
				pNode->m_ctrl_type = dlg.m_ControlType;
				pNode->m_node_type = dlg.m_NodeType;

			}

			// if node id is changed by the users
			
			if (dlg.NodeNo != pNode->m_NodeID)
			{
				if (pDoc->m_NodeNoMap.find(dlg.NodeNo) != pDoc->m_NodeNoMap.end())
				{
					CString msg;
					msg.Format("Node ID: %d that you just input has been used. Please select a new node ID.", dlg.NodeNo);
					AfxMessageBox(msg);
				}
				else
				{
					pDoc->Modify();
					pDoc->PushBackNetworkState();


					int old_node_id = pNode->m_NodeID;
					int new_node_id = dlg.NodeNo;
					
					pNode->m_NodeID = dlg.NodeNo;
					
					//search related links,

					DTANode* pNode = pDoc->m_NodeIDMap[old_node_id];

					for (int i = 0; i < pNode->m_OutgoingLinkVector.size(); i++)
					{
						DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_OutgoingLinkVector[i]];
						pLink->m_FromNodeID = new_node_id;
					}
					for (int i = 0; i < pNode->m_IncomingLinkVector.size(); i++)
					{
						DTALink* pLink = pDoc->m_LinkNoMap[pNode->m_IncomingLinkVector[i]];
						pLink->m_ToNodeID = new_node_id;

					}

					// search activity locations for associated zone id
				
						
				}

			}
			


			// dlg.ZoneID // handing here
		}
		Invalidate();
	}
}

void CTLiteView::DrawMovementLink(CDC* pDC,GDPoint pt_from, GDPoint pt_to,int NumberOfLanes, double theta, int lane_width)
{

}

void CTLiteView::DrawNodeMovements(CDC* pDC, DTANode* pNode, CRect PlotRect)
{
	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));
	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_SOLID,0,RGB(0,0,0));
	CPen SelectedPen(PS_SOLID,4,RGB(255,0,0));
	CPen SelectedPhasePen(PS_SOLID,4,RGB(0,0,255));

	CBrush  WhiteBrush(RGB(255,255,255)); 

	pDC->SetBkMode(TRANSPARENT);

	pDC->SelectObject(&DataPen);
	pDC->SelectObject(&WhiteBrush);

	CBrush  BrushLinkBand(RGB(152,245,255)); 
	pDC->SelectObject(&BrushLinkBand);

	CTLiteDoc*  pDoc = GetDocument();

	//pDoc->m_MovementTextBoxSizeInDistance 
	double size_ratio= 0.001;

	double node_set_back = 0.001 * pDoc->m_UnitDistance;

	double link_length = size_ratio* pDoc->m_UnitDistance;
	double movement_length = size_ratio* pDoc->m_UnitDistance;
	double lane_width = size_ratio* pDoc->m_UnitDistance;


	double text_length =  pDoc->m_MovementTextBoxSizeInDistance * 0.15 *  pDoc->m_UnitDistance ;

	std::map<CString, double> Turn_Degree_map;

		for (unsigned int i = 0; i< pNode->m_MovementVector.size(); i++)
	{	
		DTANodeMovement movement = pNode->m_MovementVector[i];

		if( pDoc->m_hide_non_specified_movement_on_freeway_and_ramp && movement.bNonspecifiedTurnDirectionOnFreewayAndRamps )
			continue;


		if(pDoc->m_LinkNoMap.find(movement.IncomingLinkNo) ==  pDoc->m_LinkNoMap.end())
			return;

		if(pDoc->m_LinkNoMap.find(movement.OutgoingLinkNo) ==  pDoc->m_LinkNoMap.end())
			return;

		DTALink* pInLink  = pDoc->m_LinkNoMap [movement.IncomingLinkNo];
		DTALink* pOutLink  = pDoc->m_LinkNoMap [movement.OutgoingLinkNo ];

		int destination_node  = pOutLink->m_ToNodeID; 

		if(movement.turning_prohibition_flag ==1 && m_bShowProhibitedMovements)
		{
			// select thick red pen: 
			pDC->SelectObject(&g_PenProhibitedMovement);

		}

		if(m_bShowProhibitedMovements && movement.turning_prohibition_flag == 0)
		{
			continue;// no drawing

		}


		CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

		if( !pMainFrame->m_bShowLayerMap[layer_connector] &&  pDoc->m_LinkTypeMap[pInLink->m_link_type  ].IsConnector ())
			continue;

		//if( pDoc->m_LinkTypeMap[pInLink->m_link_type  ].IsRamp  ())  //always one way
		//	continue;

		GDPoint p1, p2, p3, p_text;
		// 1: fetch all data
		p1  = pDoc->m_NodeIDMap[movement.in_link_from_node_id ]->pt;
		p2  = pDoc->m_NodeIDMap[movement.in_link_to_node_id ]->pt;
		p3  = pDoc->m_NodeIDMap[movement.out_link_to_node_id]->pt;

		// reserved direction
		double DeltaX = p2.x - p1.x ;
		double DeltaY = p2.y - p1.y ;
		double theta = atan2(DeltaY, DeltaX);

		double movement_approach_turnection_theta = atan2(p3.y - p1.y, p3.x - p1.x);


		GDPoint p1_new, p2_new, p3_new;
		GDPoint p1_text, p3_text;
		GDPoint pt_movement[3];


		// 2. set new origin
		p2_new.x = pNode->pt.x +(-1)*node_set_back*cos(theta);  
		p2_new.y = pNode->pt.y + (-1)*node_set_back*sin(theta);

		p1_new.x = p1.x + DeltaX/2;
		p1_new.y = p1.y + DeltaY / 2;

		p_text.x = p1_new.x - text_length*cos(theta);
		p_text.y = p1_new.y - text_length*sin(theta);


		// 3 determine the control point for  PolyBezier
		double lane_width =  size_ratio*3*pDoc->m_LaneWidthInKM * pDoc->m_UnitDistance;
		double movement_offset = 0;
		double text_offset = 0.7*lane_width;
		int mid_lane_number = 4+ pInLink->m_NumberOfLanes / 2;
		float control_point_ratio = 0;

		float MovementTextFontSize = 8;

		CString movement_approach_turnection_label;
		movement_approach_turnection_label.Format ("%d,%s", movement.in_link_from_node_id , pDoc->GetTurnString(movement.movement_turn));

		if(movement.movement_turn == DTA_Through ) 
		{

			if(Turn_Degree_map.find (movement_approach_turnection_label ) != Turn_Degree_map.end())
			{  // this direction has been used/defined.

				if( Turn_Degree_map[movement_approach_turnection_label ] <  movement_approach_turnection_theta)
					movement_offset = lane_width * (mid_lane_number - 0.7);
				else
					movement_offset = lane_width * (mid_lane_number + 0.7);

			}else
			{
				movement_offset = lane_width * mid_lane_number;

			}

		}else if(movement.movement_turn == DTA_LeftTurn || movement.movement_turn == DTA_LeftTurn2 ) 
		{
			movement_offset = lane_width * (mid_lane_number-1);
		}else
		{
			movement_offset = lane_width * (mid_lane_number+1);
		}

		// keep a record
		Turn_Degree_map[movement_approach_turnection_label] = movement_approach_turnection_theta;

		int movement_size = max(2,int(text_length*m_Resolution));

		movement_size = min(50,movement_size);

		int PointSize =  max(movement_size, 10)*m_MovmentTextSize;

		GDPoint pt_from, pt_to, pt_text;
		pt_from.x = p1_new.x + movement_offset* cos(theta-PI/2.0f);
		pt_from.y = p1_new.y + movement_offset* sin(theta-PI/2.0f);


		double alpha  = 0.3;

		pt_to.x  = p2_new.x + movement_offset* cos(theta-PI/2.0f);
		pt_to.y  =  p2_new.y + movement_offset* sin(theta-PI/2.0f);

		// direction

		DeltaX = p2.x - p3.x ;
		DeltaY = p2.y - p3.y ;
		theta = atan2(DeltaY, DeltaX);


		if(movement.movement_turn == DTA_Through ) 
		{
			p3_new.x = pt_to.x - movement_length * cos(theta);
			p3_new.y = pt_to.y - movement_length * sin(theta);

		}else if(movement.movement_turn == DTA_LeftTurn ) 
		{
			p3_new.x = pt_to.x - movement_length * cos(theta -PI/4.0f);
			p3_new.y = pt_to.y - movement_length * sin(theta -PI/4.0f);
		}else
		{
			p3_new.x = pt_to.x - movement_length * cos(theta +PI/4.0f);
			p3_new.y = pt_to.y - movement_length * sin(theta +PI/4.0f);
		}

		CPoint DrawPoint[3];
		DrawPoint[0] = NPtoSP(pt_from);
		DrawPoint[1] = NPtoSP(pt_to);

		pDC->MoveTo(DrawPoint[0]);
		pDC->LineTo(DrawPoint[1]);
		DrawPoint[2] = NPtoSP(p3_new);


		pDC->LineTo(DrawPoint[2]);

		//restore pen
		pDC->SelectObject(&DataPen);


		//text 

		// create rotation font
		CFont movement_text_font;



		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
		lf.lfHeight = PointSize;

		DeltaX = p2.x - p1.x ;
		DeltaY = p2.y - p1.y ;
		theta = atan2(DeltaY, DeltaX);

		float theta_deg = fmod(theta/PI*180,360);

		if(theta_deg < -90)
			theta_deg += 180;

		if(theta_deg > 90)
			theta_deg -= 180;

		lf.lfEscapement = theta_deg*10;


		strcpy(lf.lfFaceName, "Arial");       

		movement_text_font.CreateFontIndirect(&lf);

		CFont* oldFont = pDC->SelectObject(&movement_text_font);

		pDC->SelectObject(&movement_text_font);

		// select text string to be displayed

		CString str_text;


		float number_of_hours = max(0.01, (pDoc->m_DemandLoadingEndTimeInMin - pDoc->m_DemandLoadingStartTimeInMin) / 60.0);
		float sim_turn_hourly_count = movement.sim_turn_count / number_of_hours;

		//empty label
		str_text.Format("");

		switch (m_ShowMovementTextMode)
		{
		case movement_display_turn_up_node_number: str_text.Format("%d", pInLink->m_FromNodeID); break;
		case movement_display_turn_dest_node_number: str_text.Format("%d", pOutLink->m_ToNodeID); break;
		case movement_display_turn_three_node_numbers: str_text.Format("%d,%d,%d", pInLink->m_FromNodeID,pInLink->m_ToNodeID,pOutLink->m_ToNodeID); break;

		case movement_display_turn_type: str_text.Format("%s", pDoc->GetTurnString(movement.movement_turn)); break;
		case movement_display_turn_direction: str_text.Format("%s", pDoc->GetTurnDirectionString(movement.movement_approach_turn)); break;
		case movement_display_turn_prohibited: str_text.Format("%d", movement.turning_prohibition_flag); break;


		case movement_display_sim_turn_count: 
			//					if(movement.sim_turn_count>=1)
			str_text.Format("%.0f", movement.sim_turn_count );

			break;


		case movement_display_sim_turn_percentage: 

			//					if(movement.sim_turn_count>=1)
			str_text.Format("%.1f%%", movement.turning_percentage  ); 

			break;

		case movement_display_sim_turn_delay: 

			//					if(movement.sim_turn_count>=1)
			str_text.Format("%.1f", movement.sim_turn_delay*60  ); // min to second

			break;


		case movement_display_sim_turn_delay_in_min: 

			str_text.Format("%.1f", movement.sim_turn_delay  ); 

			break;



		case movement_display_obs_turn_hourly_count: 
			//					if(movement.sim_turn_count>=1)
			str_text.Format("%d", movement.obs_turn_hourly_count); 

			break;



		case movement_display_obs_turn_delay: 

			//					if(movement.sim_turn_count>=1)
			str_text.Format("%d", movement.obs_turn_delay  ); 

			break;


		default: str_text.Format("");
		}

		CPoint TextPoint;

		double alpah = 1.5;


		TextPoint.x = alpah * DrawPoint[0].x + (1-alpah)* DrawPoint[1].x  ;
		TextPoint.y =  alpah * DrawPoint[0].y + (1-alpah)* DrawPoint[1].y ;
		CRect rt(TextPoint.x -1,TextPoint.y -1, TextPoint.x +1, TextPoint.y+1);

		pDC->SetBkColor(RGB(0,0, 0));

		pDC->SetTextColor(RGB(0,1,0));
		pDC->TextOut(TextPoint.x,TextPoint.y - PointSize*0.5, str_text);	


		pDC->SelectObject(oldFont);



	}


}

void CTLiteView::OnNodeAvoidthisnode()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	std::list<DTALink*>::iterator iLink;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_ToNodeNo == pDoc->m_SelectedNodeNo)
		{
			(*iLink)->m_AdditionalCost = 10000;
		}
	}

	pDoc->Routing(false, true);

	m_ShowAllPaths = true;
	Invalidate();
}

void CTLiteView::OnNodeRemovenodeavoidanceconstraint()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_SelectedNodeNo = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	std::list<DTALink*>::iterator iLink;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_AdditionalCost = 0;
	}

	pDoc->Routing(false, true);

	m_ShowAllPaths = true;
	Invalidate();

}

void CTLiteView::OnEditMovenode()
{
	m_ToolMode = move_node_tool;

	
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->m_iSelectedLayer = layer_node;

	CListCtrl * pGISLayerList = (CListCtrl *)(pMainFrame->m_GISLayerBar).GetDlgItem(IDC_LIST_GISLAYER);
	pGISLayerList->Invalidate(1);  // update display of selected layer

	m_bMouseDownFlag = false;
}

void CTLiteView::OnUpdateEditMovenode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == move_node_tool ? 1 : 0);
}



void CTLiteView::OnBackgroundimageMarklongA()
{
	CTLiteDoc* pDoc = GetDocument();

	GDPoint pt = SPtoNP(m_CurrentMousePoint);

	pDoc->m_PointA_x = pt.x;
	pDoc->m_PointA_y = pt.y;


	pDoc->m_bPointA_Initialized = true;

	Invalidate();
}

void CTLiteView::OnBackgroundimageMarklongB()
{

	CTLiteDoc* pDoc = GetDocument();
	GDPoint pt = SPtoNP(m_CurrentMousePoint);

	pDoc->m_PointB_x = pt.x;
	pDoc->m_PointB_y = pt.y;

	pDoc->m_bPointB_Initialized = true;

	Invalidate();

}

void CTLiteView::OnBackgroundimageAddlat()
{
}

void CTLiteView::OnZoneHighlightassociatedacititylocations()
{
	m_bHighlightActivityLocation = !m_bHighlightActivityLocation;
	Invalidate();

}

void CTLiteView::OnUpdateZoneHighlightassociatedacititylocations(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHighlightActivityLocation);
}

void CTLiteView::OnZoneCreatezone()
{
	OnEditCreatesubarea();
	Invalidate();

}

void CTLiteView::OnEditCreatezone()
{
	m_ToolMode = add_zone_tool;
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	pMainFrame->m_bShowLayerMap[layer_zone] = true;
	pMainFrame-> m_iSelectedLayer = layer_zone;
	GetDocument()->m_SubareaShapePoints.clear();

	CListCtrl * pGISLayerList = (CListCtrl *)(pMainFrame->m_GISLayerBar).GetDlgItem(IDC_LIST_GISLAYER);
	pGISLayerList->Invalidate(1);  // update display of selected layer
}

void CTLiteView::OnUpdateEditCreatezone(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == add_zone_tool ? 1 : 0);
	Invalidate();

}

void CTLiteView::OnSubareaHighlightlinksinsidesubarea()
{
	m_bHighlightSubareaLinks = !m_bHighlightSubareaLinks;

	if(m_bHighlightSubareaLinks)
		m_bHighlightSubareaBoundaryLinks = false;

	Invalidate();
}

void CTLiteView::OnUpdateSubareaHighlightlinksinsidesubarea(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHighlightSubareaLinks);
}

void CTLiteView::OnSubareaHighlightlinksacosssubarea()
{
	m_bHighlightSubareaBoundaryLinks = !m_bHighlightSubareaBoundaryLinks;

	if(m_bHighlightSubareaBoundaryLinks)
		m_bHighlightSubareaLinks = false;

	Invalidate();
}

void CTLiteView::OnUpdateSubareaHighlightlinksacosssubarea(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHighlightSubareaBoundaryLinks);
}

void CTLiteView::OnOdmatrixViewtop50odpairsonly()
{
	m_bShowTop10ODOnly = !m_bShowTop10ODOnly;
	Invalidate();

}

void CTLiteView::OnUpdateOdmatrixViewtop50odpairsonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowTop10ODOnly);
}


void CTLiteView::OnUpdateTransitShowtransitaccessibility(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowTransitAccessibility);
}


void CTLiteView::OnMovementHighlightprohibitedmovements(){
	m_bShowProhibitedMovements = !m_bShowProhibitedMovements;
	Invalidate();
}

void CTLiteView::OnUpdateMovementHighlightprohibitedmovements(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowProhibitedMovements);
}





void CTLiteView::OnUpdateTransitShowtransitlinksonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowTransitLinksOnly);
}

void CTLiteView::OnUpdateTransitShowwalklinksonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowWalkLinksOnly);
}

void CTLiteView::OnFilePrint()
{
	// TODO: Add your command handler code here
}

void CTLiteView::OnFilePrintPreview()
{
	// TODO: Add your command handler code here
}

void CTLiteView::OnFilePrintSetup()
{
	// TODO: Add your command handler code here
}

void CTLiteView::OnAgentdataShowcompletetrajectory()
{
		m_bShowCompleteTrajectory = !m_bShowCompleteTrajectory;
		Invalidate();

}

void CTLiteView::OnUpdateAgentdataShowcompletetrajectory(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowCompleteTrajectory);
}

void CTLiteView::OnAgentdataShow()
{
	m_bShowAllCompleteTrajectory = !m_bShowAllCompleteTrajectory;
}

void CTLiteView::OnUpdateAgentdataShow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAllCompleteTrajectory);
}


void CTLiteView::OnDetectorUnlock()
{
	if (m_ToolMode != sensor_location_move_tool)
		m_ToolMode = sensor_location_move_tool;
	else
		m_ToolMode = move_tool;

}


void CTLiteView::OnUpdateDetectorUnlock(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == sensor_location_move_tool ? 1 : 0);

}


void CTLiteView::OnDetectorShowsensordescription33769()
{
	m_bShowSensorDecription = !m_bShowSensorDecription;
	Invalidate();
}


void CTLiteView::OnUpdateDetectorShowsensordescription33769(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowSensorDecription == true ? 1 : 0);
}


void CTLiteView::OnDetectorShowmatchedlinkforsensor()
{
	m_bShowSensorMatchedLink = !m_bShowSensorMatchedLink;
	Invalidate();

}


void CTLiteView::OnUpdateDetectorShowmatchedlinkforsensor(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowSensorMatchedLink == true ? 1 : 0);
}


void CTLiteView::OnGridUnlock()
{


	if (m_ToolMode != network_coordinate_tool)
		m_ToolMode = network_coordinate_tool;
	else
		m_ToolMode = move_tool;

	m_bShowGrid = true;
	
	if (m_bNetworkCooridinateHints == false)
	{
		AfxMessageBox("Hints:\n1. Please use mouse to move the reference network and shift its coordinates;\n2. Use mouse wheel to adjust the size of the reference network (zoom);\n3. Click shift or control key together to change the horizonal or vertical size, respectively;\n4. Click on the other toolbar button to turn on disable this moving-reference-network mode.", MB_ICONINFORMATION);
		m_bNetworkCooridinateHints = false;
	}

	Invalidate();

}


void CTLiteView::OnUpdateGridUnlock(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == network_coordinate_tool ? 1 : 0);
}


void CTLiteView::OnLinkShowhidereferencenetwork()
{
	m_bShowReferenceNetwork = !m_bShowReferenceNetwork;
}


void CTLiteView::OnUpdateLinkShowhidereferencenetwork(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowReferenceNetwork ? 1 : 0);
}


void CTLiteView::OnEditMovereferencenetworkcoordinates()
{
	if (m_ToolMode != network_coordinate_tool)
	{
		m_MoveLayerNo = 1; //reference layer
		m_bShowGrid = true;
		if (m_bNetworkCooridinateHints == false)
		{
			AfxMessageBox("Hints:\n1. Please use mouse to move the reference network and shift its coordinates;\n2. Use mouse wheel to adjust the size of the reference network (zoom);\n3. Click shift or control key together to change the horizonal or vertical size, respectively;\n4. Click on the other toolbar button to turn on disable this moving-reference-network mode.", MB_ICONINFORMATION);
			m_bNetworkCooridinateHints = true;
		}
		m_ToolMode = network_coordinate_tool;
	}
	else
		m_ToolMode = move_tool;

	Invalidate();
}


void CTLiteView::OnUpdateEditMovereferencenetworkcoordinates(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == network_coordinate_tool ? 1 : 0);
}

void CTLiteView::OnDetectorShowfixedcountdetectorlocation()
{
	m_bShowFixedDetectorLocation = !m_bShowFixedDetectorLocation;
	Invalidate();

}


void CTLiteView::OnDetectorShowspeedsensorlocation()
{
	m_bShowSpeedSensorLocation = !m_bShowSpeedSensorLocation;
	Invalidate();
}


void CTLiteView::OnUpdateDetectorShowfixedcountdetectorlocation(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowFixedDetectorLocation ? 1 : 0);
}


void CTLiteView::OnUpdateDetectorShowspeedsensorlocation(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowSpeedSensorLocation ? 1 : 0);
}




void CTLiteView::OnShowallowedlinksfordemandtypeDemandtype1()
{
	m_DislayedAgentType = 1;
	Invalidate();
}



void CTLiteView::OnShowallowedlinksfordemandtypeDemandtype2()
{
	m_DislayedAgentType = 2;
	Invalidate();
}


void CTLiteView::OnShowallowedlinksfordemandtypeDemandtype3()
{
	m_DislayedAgentType = 3;
	Invalidate();
}


void CTLiteView::OnShowallowedlinksfordemandtypeDemandtype4()
{
	m_DislayedAgentType = 4;
	Invalidate();
}


void CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype1(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_DislayedAgentType ==1);
}


void CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype2(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_DislayedAgentType == 2);
}


void CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype3(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_DislayedAgentType == 3);
}


void CTLiteView::OnUpdateShowallowedlinksfordemandtypeDemandtype4(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_DislayedAgentType == 4);
}


void CTLiteView::OnShowallowedlinksfordemandtypeShowall()
{
	m_DislayedAgentType = 0;
	Invalidate();
}


void CTLiteView::OnUpdateShowallowedlinksfordemandtypeShowall(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_DislayedAgentType == 0);
}


void CTLiteView::OnReferencelineShowreferencelineno()
{
	m_DislayReferenceLineNo = true;
	Invalidate();

}


void CTLiteView::OnBnClickedButtonSearch()
{
	OnSearchFindlink();
}
