// MainFrm.cpp : implementation of the CMainFrame class
//http://www.axialis.com/free/icons/
// All the icons are licensed under the Creative Commons Attribution License.
// Icons: the credits of the authors: "Axialis Team"
//

#include "stdafx.h"
#include "TLite.h"
#include "TLiteDoc.h"
#include "TLiteView.h"
#include "MainFrm.h"
#include "Dlg_Legend.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int  g_Player_Status=-1;	     //-1: edit 0: stop, 1: play, 2: pause

int g_MOEAggregationIntervalInMin = 5;

int g_SelectedAgentID = 0;
int  g_SelectedPassengerID = 0;

bool g_bShowRadarChart = false;

int g_ImpactStudyPeriodInMin = 60;
int g_SimulationDayNo = 0;
int g_SensorDayNo = 0;

int g_SimulatedLastDayNo = 0;
int g_SensorLastDayNo = 0;
int g_ImpactThreshold_QueueLengthRatioPercentage = 80;

std::map <int,bool> g_SimulatedDayDataMap;
std::map <int,bool> g_SensorDayDataMap;
extern CDlg_Legend* g_pLegendDlg;


IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_WINDOW_NEW3DVIEW, &CMainFrame::OnWindowNew3dview)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEW3DVIEW, &CMainFrame::OnUpdateWindowNew3dview)
	ON_COMMAND(ID_WINDOW_SHOW2DVIEW, &CMainFrame::OnWindowShow2dview)
	ON_COMMAND(ID_3DDISPLAY_ANIMATION, &CMainFrame::On3ddisplayAnimation)

	ON_COMMAND(ID_ANIMATION_REWIND, &CMainFrame::OnAnimationRewind)
	ON_COMMAND(ID_ANIMATION_PAUSE, &CMainFrame::OnAnimationPause)
	ON_COMMAND(ID_ANIMATION_STOP, &CMainFrame::OnAnimationStop)
	ON_COMMAND(ID_VIEW_MOETOOLBAR, &CMainFrame::OnViewMoetoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOETOOLBAR, &CMainFrame::OnUpdateViewMoetoolbar)
	ON_COMMAND(ID_WINDOW_SHOWESTIMATIONVIEW, &CMainFrame::OnWindowShowestimationview)
	
	ON_COMMAND(ID_ANIMATION_PLAY, &CMainFrame::OnAnimationPlay)
	ON_COMMAND(ID_ANIMATION_FORWARD, &CMainFrame::OnAnimationForward)
	ON_COMMAND(ID_ANIMATION_BACKWARD, &CMainFrame::OnAnimationBackward)

	ON_COMMAND(ID_ANIMATION_PLAY_Second, &CMainFrame::OnAnimationPlay_second)
	ON_COMMAND(ID_ANIMATION_FORWARD_second, &CMainFrame::OnAnimationForward_second)
	ON_COMMAND(ID_ANIMATION_BACKWARD_second, &CMainFrame::OnAnimationBackward_second)

	ON_COMMAND(ID_ANIMATION_PLAY_DecSecond, &CMainFrame::OnAnimationPlay_DecSecond)
	ON_COMMAND(ID_ANIMATION_FORWARD_DecSecond, &CMainFrame::OnAnimationForward_DecSecond)
	ON_COMMAND(ID_ANIMATION_BACKWARD_DecSecond, &CMainFrame::OnAnimationBackward_DecSecond)

	ON_COMMAND(ID_ANIMATION_SKIPFORWARD, &CMainFrame::OnAnimationSkipforward)

	ON_COMMAND(ID_ANIMATION_SKIPBACKWARD, &CMainFrame::OnAnimationSkipbackward)
	ON_COMMAND(ID_WINDOW_SYNCHRONIZEDDISPLAY, &CMainFrame::OnWindowSynchronizeddisplay)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_SYNCHRONIZEDDISPLAY, &CMainFrame::OnUpdateWindowSynchronizeddisplay)
	ON_COMMAND(ID_VIEW_GIS_Layer_Panel, &CMainFrame::OnViewGisLayerPanel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GIS_Layer_Panel, &CMainFrame::OnUpdateViewGisLayerPanel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_GISLAYER, &CMainFrame::OnLvnItemchangedListGislayer)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_GISLAYER, &CMainFrame::OnNMCustomdrawListGislayer)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_GISLAYER, &CMainFrame::OnNMRClickListGislayer)
	ON_BN_CLICKED(IDC_BUTTON_Database, &CMainFrame::OnBnClickedButtonDatabase)
	ON_COMMAND(ID_VIEW_SIMULATIONTOOLBAR, &CMainFrame::OnViewSimulationtoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SIMULATIONTOOLBAR, &CMainFrame::OnUpdateViewSimulationtoolbar)
	ON_COMMAND(ID_TOOLS_SPACE, &CMainFrame::OnToolsSpaceTimeView)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_LEFT,
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction
#define _NUM_OF_GIS_LAYERS  7 
static _TCHAR *_gLayerLabel[_NUM_OF_GIS_LAYERS] =
{
	_T("Node"),
	_T("Link"),
	_T("Path"),  
	_T("Demand"),
	_T("Link Performance"),
	_T("Background Image"),
	_T("Grid"),
};



CMainFrame::CMainFrame()
{

	 m_simulation_time_step = 1.0f;
	 simulation_animation_resolution_in_mili_second = 1000/25;
	m_bFeatureInfoInitialized  = false;
	m_bShowGISLayerToolBar = true;
	m_bShowMOEToolBar = true;
	m_bShowSimulationoolBar = true;
	m_bShowDataToolBar = true;
	m_iSelectedLayer = layer_link;
	bLayerInitialized = false;

}

CMainFrame::~CMainFrame()
{
}
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

	return OnCreate_TrafficNetwork(lpCreateStruct);

	return 0;
}
int CMainFrame::OnCreate_TrafficNetwork(LPCREATESTRUCT lpCreateStruct)
{

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(IDR_MAINFRAME2))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}

	if (!m_wndSimulationToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY ) ||
			!m_wndSimulationToolBar.LoadToolBar(IDR_MAINFRAME4))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}
		if(!m_wndPlayerSeekBar.Create(this))
		{
			TRACE0("Failed to create m_wndPlayerSeekBar toolbar\n");
			return -1;     // fail to create
		}

		m_wndPlayerSeekBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
			CBRS_TOOLTIPS | CBRS_FLYBY);

		if (!m_MOEToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_MOEToolBar.LoadToolBar(IDR_VIEW_TOOLBAR))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}
		if (!m_wndStatusBar.Create(this) ||
			!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators)/sizeof(UINT)))
		{
			TRACE0("Failed to create status bar\n");
			return -1;      // fail to create
		}

			m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_LEFT, SBPS_NORMAL, 300);


		if (!m_wndReBar.Create(this) ||
			!m_wndReBar.AddBar(&m_wndToolBar)||
			!m_wndReBar.AddBar(&m_wndSimulationToolBar) ||
			!m_wndReBar.AddBar(&m_wndPlayerSeekBar)
			)
		{
			TRACE0("Failed to create rebar\n");
			return -1;      // fail to create
		}



		std::vector<CString> strCHSVector;


	for(int i=0; i<= 13; i++)
	{
	
		CString strFormat;
		switch (i)
		{
		case 0: strFormat.LoadString(IDS_STRING10100); break;
		case 1: strFormat.LoadString(IDS_STRING10101); break;
		case 2: strFormat.LoadString(IDS_STRING10102); break;
		case 3: strFormat.LoadString(IDS_STRING10103); break;
		case 4: strFormat.LoadString(IDS_STRING10104); break;
		case 5: strFormat.LoadString(IDS_STRING10105); break;
		case 6: strFormat.LoadString(IDS_STRING10106); break;
		case 7: strFormat.LoadString(IDS_STRING10107); break;
		case 8: strFormat.LoadString(IDS_STRING10108); break;
		case 9: strFormat.LoadString(IDS_STRING10109); break;
		case 10: strFormat.LoadString(IDS_STRING10110); break;
		case 11: strFormat.LoadString(IDS_STRING10111); break;
		case 12: strFormat.LoadString(IDS_STRING10112); break;
		case 13: strFormat.LoadString(IDS_STRING10113); break;
		}

		strCHSVector.push_back (strFormat);

	}



		int count = 0;

		m_MOEToolBar.SetButtonText(count++,"Network");
		count++ ; // separator
		m_MOEToolBar.SetButtonText(count++,"Animation");
		m_MOEToolBar.SetButtonText(count++,"Volume");
		m_MOEToolBar.SetButtonText(count++,"Density");
		m_MOEToolBar.SetButtonText(count++,"Velocity");
		m_MOEToolBar.SetButtonText(count++,"QueueL");
		count++;
		m_MOEToolBar.SetButtonText(count++,"Link");
		m_MOEToolBar.SetButtonText(count++,"Path");
		m_MOEToolBar.SetButtonText(count++,"Assignment");
		m_MOEToolBar.SetButtonText(count++,"Summary");
		////m_MOEToolBar.SetButtonText(count++, "AgBM");

		m_MOEToolBar.SetSizes(CSize(42,55),CSize(16,15));

		m_MOEToolBar.EnableDocking(CBRS_ALIGN_ANY);
		EnableDocking(CBRS_ALIGN_ANY);
		DockControlBar(&m_MOEToolBar);


		SetTimer(0, simulation_animation_resolution_in_mili_second, NULL); // simulation reflesh timer
		m_wndPlayerSeekBar.Enable(true);
		//	   m_wndPlayerSeekBar.SetRange(0,100);
		m_wndPlayerSeekBar.SetRange(0,g_Simulation_Time_Horizon);

		//	// TODO: Delete these three lines if you don't want the toolbar to be dockable
		//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
		//	EnableDocking(CBRS_ALIGN_ANY);
		//	DockControlBar(&m_wndToolBar);

	if (!m_GISLayerBar.Create(this, IDD_DIALOG_GISLayer,
		CBRS_LEFT|CBRS_TOOLTIPS|CBRS_FLYBY, IDD_DIALOG_GISLayer))
	{
		TRACE0("Failed to create DlgBar\n");
		return -1;      // fail to create
	}


CListCtrl * pGISLayerList = (CListCtrl *)m_GISLayerBar.GetDlgItem(IDC_LIST_GISLAYER);

pGISLayerList->InsertColumn(0,"Layer",LVCFMT_LEFT,170);

	LV_ITEM lvi;
	for(int i = 0; i < _NUM_OF_GIS_LAYERS; i++)
	{
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;

	CString strFormat;
	switch (i)
	{
	case 0: strFormat.LoadString(IDS_STRING10000); break;
	case 1: strFormat.LoadString(IDS_STRING10001); break;
	case 2: strFormat.LoadString(IDS_STRING10002); break;
	case 3: strFormat.LoadString(IDS_STRING10003); break;
	case 4: strFormat.LoadString(IDS_STRING10004); break;
	case 5: strFormat.LoadString(IDS_STRING10005); break;
	case 6: strFormat.LoadString(IDS_STRING10006); break;
	case 7: strFormat.LoadString(IDS_STRING10007); break;
	case 8: strFormat.LoadString(IDS_STRING10008); break;
	case 9: strFormat.LoadString(IDS_STRING10009); break;
	case 10: strFormat.LoadString(IDS_STRING10010); break;
	case 11: strFormat.LoadString(IDS_STRING10011); break;
	case 12: strFormat.LoadString(IDS_STRING10012); break;
	case 13: strFormat.LoadString(IDS_STRING10013); break;
	case 14: strFormat.LoadString(IDS_STRING10014); break;
	case 15: strFormat.LoadString(IDS_STRING10015); break;
	case 16: strFormat.LoadString(IDS_STRING10016); break;
	case 17: strFormat.LoadString(IDS_STRING10017); break;
	case 18: strFormat.LoadString(IDS_STRING10018); break;

	}

	char str[20];
	sprintf(str,"%s", strFormat);

		lvi.pszText = _gLayerLabel[i];

		pGISLayerList->InsertItem(&lvi);
		m_bShowLayerMap[(layer_mode)(i)] = false;

	}



	pGISLayerList->SetExtendedStyle(LVS_EX_CHECKBOXES);


	m_bShowLayerMap[layer_node] = true;
	m_bShowLayerMap[layer_link] = true;
	m_bShowLayerMap[layer_link_MOE] = true;
	m_bShowLayerMap[layer_grid] = true;
	m_bShowLayerMap[layer_path] = true;
	m_bShowLayerMap[layer_background_image] = true;
	

	for(int i = 0; i < _NUM_OF_GIS_LAYERS; i++)  // do not include the last 2 (reseach) layers for now
	{
	pGISLayerList->SetCheck(i,m_bShowLayerMap[(layer_mode)(i)]);
	}

	m_wndSimulationToolBar.ShowWindow (m_bShowSimulationoolBar);
	bLayerInitialized = true;
	return 0;
}

int CMainFrame::OnCreate_RailNetwork(LPCREATESTRUCT lpCreateStruct)
{

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(IDR_AMS_TOOLBAR))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}

	
		if (!m_wndStatusBar.Create(this) ||
			!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators)/sizeof(UINT)))
		{
			TRACE0("Failed to create status bar\n");
			return -1;      // fail to create
		}

		if (!m_wndReBar.Create(this) ||
			!m_wndReBar.AddBar(&m_wndToolBar)
			)
		{
			TRACE0("Failed to create rebar\n");
			return -1;      // fail to create
		}



	return 0;
}
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers







void CMainFrame::OnWindowNew3dview()
{
	//CMDIChildWnd* pActiveChild = MDIGetActive();
	//CDocument* pDocument;
	//if (pActiveChild == NULL || (pDocument = pActiveChild->GetActiveDocument()) == NULL) {
	//	TRACE("Warning:  No active document for WindowNew command\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Command failed
	//}
	//// Otherwise, we have a new frame!
	//CDocTemplate* pTemplate = ((CTLiteApp*) AfxGetApp())->m_pTemplateGLView;
	//ASSERT_VALID(pTemplate);

	//CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	//if (pFrame == NULL)
	//{
	//	TRACE("Warning:  failed to create new frame\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Command failed
	//	// make it visisable
	//}
	//pTemplate->InitialUpdateFrame(pFrame, pDocument);

}



void CMainFrame::OnUpdateWindowNew3dview(CCmdUI *pCmdUI)
{

}


void CMainFrame::OnWindowShow2dview()
{
	MDITile(MDITILE_VERTICAL);
}

void CMainFrame::On3ddisplayAnimation()
{
	// TODO: Add your command handler code here
}
void CMainFrame::OnTimer(UINT_PTR  nIDEvent)
{

	if(g_Player_Status == 1) // play
	{
		if(g_Simulation_Time_Stamp < g_Simulation_Time_Horizon)
		{
			g_Simulation_Time_Stamp += m_simulation_time_step;  // min
		}else
		{
			g_Simulation_Time_Stamp = g_Simulation_Time_Horizon;
			g_Player_Status = 0; // automatically stops when we reach the end of simulation horizon

		}

		m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

		UpdateAllViews();
	}

}

void CMainFrame::OnAnimationRewind()
{
	g_Player_Status = 1;
	g_Simulation_Time_Stamp = g_SimulationStartTime_in_min;
	m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

	UpdateAllViews();
}

void CMainFrame::OnAnimationPause()
{
	g_Player_Status = 2;
}

void CMainFrame::OnAnimationStop()
{
	g_Player_Status = 0;
	g_Simulation_Time_Stamp = g_SimulationStartTime_in_min;
	m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

	UpdateAllViews();
}

void CMainFrame::OnViewMoetoolbar()
{
	m_bShowMOEToolBar= !m_bShowMOEToolBar;
	if(m_bShowMOEToolBar)
		m_MOEToolBar.ShowWindow (true);
	else
		m_MOEToolBar.ShowWindow (false);
}

void CMainFrame::OnUpdateViewMoetoolbar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bShowMOEToolBar);
}


void CMainFrame::UpdateLegendView()
{
	if( g_pLegendDlg!=NULL && g_pLegendDlg->GetSafeHwnd())
	{
		g_pLegendDlg->Invalidate (true);
	}	
}

void CMainFrame::UpdateAllViews()
{

	CTLiteApp* pApp;
	pApp = (CTLiteApp *) AfxGetApp(); 
	pApp->UpdateAllViews();

}
void CMainFrame::OnWindowShowestimationview()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL || (pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warning:  No active document for WindowNew command\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Command failed
	}
	// Otherwise, we have a new frame!
	//CDocTemplate* pTemplate = ((CTLiteApp*) AfxGetApp())->m_pTemplateGLView;
	//ASSERT_VALID(pTemplate);

	//CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	//if (pFrame == NULL)
	//{
	//	TRACE("Warning:  failed to create new frame\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Command failed
	//	// make it visisable
	//}
	//pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

void CMainFrame::OnViewDatatoolbar()
{
}

void CMainFrame::OnUpdateViewDatatoolbar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bShowDataToolBar);
}

void CMainFrame::OnAnimationPlay()
{
	g_Player_Status = 1;
	m_simulation_time_step = 1.0f;

	UpdateAllViews();
}


void CMainFrame::OnAnimationForward()
{
	m_wndPlayerSeekBar.SetPos(min(g_Simulation_Time_Stamp+1,g_Simulation_Time_Horizon));
	UpdateAllViews();

}

void CMainFrame::OnAnimationBackward()  //min
{
	m_wndPlayerSeekBar.SetPos(max(g_Simulation_Time_Stamp-1,0));
	UpdateAllViews();
}


void CMainFrame::OnAnimationPlay_second()
{
	g_Player_Status = 1;
	m_simulation_time_step = 0.1/60.0f;  // 0.1 second

	UpdateAllViews();
}


void CMainFrame::OnAnimationForward_second()
{
	m_wndPlayerSeekBar.SetPos(min(g_Simulation_Time_Stamp+0.1/60.0f,g_Simulation_Time_Horizon));
	UpdateAllViews();

}

void CMainFrame::OnAnimationBackward_second()
{
	m_wndPlayerSeekBar.SetPos(max(g_Simulation_Time_Stamp-0.1/60.0f,0));
	UpdateAllViews();
}

void CMainFrame::OnAnimationPlay_DecSecond()
{
	g_Player_Status = 1;
	m_simulation_time_step = 0.1 / 60.0f;  // 0.1 seconds

	UpdateAllViews();
}


void CMainFrame::OnAnimationForward_DecSecond()
{
	simulation_animation_resolution_in_mili_second  = simulation_animation_resolution_in_mili_second/2;
	if (simulation_animation_resolution_in_mili_second <= 50)
		simulation_animation_resolution_in_mili_second = 50;

	SetTimer(0, simulation_animation_resolution_in_mili_second, NULL); // simulation reflesh timer

	UpdateAllViews();

}

void CMainFrame::OnAnimationBackward_DecSecond()
{
	simulation_animation_resolution_in_mili_second = simulation_animation_resolution_in_mili_second * 2;
	if (simulation_animation_resolution_in_mili_second >= 100000)
		simulation_animation_resolution_in_mili_second = 100000;
	SetTimer(0, simulation_animation_resolution_in_mili_second, NULL); // simulation reflesh timer

	UpdateAllViews();
}

void CMainFrame::OnAnimationSkipforward()
{
	m_wndPlayerSeekBar.SetPos(min(g_Simulation_Time_Stamp+5,g_Simulation_Time_Horizon));
	UpdateAllViews();
}

void CMainFrame::OnAnimationSkipbackward()
{
	m_wndPlayerSeekBar.SetPos(max(g_Simulation_Time_Stamp-5,0));
	UpdateAllViews();
}

void CMainFrame::OnWindowSynchronizeddisplay()
{
	m_bSynchronizedDisplay = !m_bSynchronizedDisplay;
}

void CMainFrame::OnUpdateWindowSynchronizeddisplay(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bSynchronizedDisplay);
}


void CMainFrame::OnViewGisLayerPanel()
{
	m_bShowGISLayerToolBar= !m_bShowGISLayerToolBar;
	if(m_bShowGISLayerToolBar)
		ShowControlBar(&m_GISLayerBar,true,false);
	else
		ShowControlBar(&m_GISLayerBar,false,false);
}

void CMainFrame::OnUpdateViewGisLayerPanel(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bShowGISLayerToolBar);
}

void CMainFrame::OnLvnItemchangedListGislayer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

CListCtrl * pGISLayerList = (CListCtrl *)m_GISLayerBar.GetDlgItem(IDC_LIST_GISLAYER);

// determine which layer is active
	POSITION pos = pGISLayerList->GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = pGISLayerList->GetNextSelectedItem(pos);
		 m_iSelectedLayer = (layer_mode) nItem;
		 BOOL bChecked = pGISLayerList->GetCheck(nItem); 
	 	 m_bShowLayerMap[m_iSelectedLayer] = bChecked;
		 
		 CEdit * pEdit1 = (CEdit*)(m_GISLayerBar.GetDlgItem(IDC_ACTIVE_LAYER));
		 pEdit1->SetWindowText(_gLayerLabel[m_iSelectedLayer]);

	}

	// detemine the selection status

	if(bLayerInitialized)
	{
      for(int nItem =0 ; nItem <  pGISLayerList->GetItemCount(); nItem++)
      {
		  
        BOOL bChecked =  ListView_GetCheckState(pGISLayerList->m_hWnd,nItem);
	 	m_bShowLayerMap[(layer_mode) nItem] = bChecked;

      }
	}

	pGISLayerList->Invalidate (1);
	UpdateAllViews();
}

void CMainFrame::OnNMCustomdrawListGislayer(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.

    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        *pResult = CDRF_NOTIFYITEMDRAW;
        }
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        // This is the prepaint stage for an item. Here's where we set the
        // item's text color. Our return value will tell Windows to draw the
        // item itself, but it will use the new color we set here.
       
        COLORREF crText = RGB(0,0,0);

        if (pLVCD->nmcd.dwItemSpec  == (int)(m_iSelectedLayer) ) // first line
            crText = RGB(255,0,0);
        
        // Store the color back in the NMLVCUSTOMDRAW struct.
        pLVCD->clrText = crText;

        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
        }
}


void CMainFrame::FillFeatureInfo()
{
	CListCtrl * pGISFeatureList = (CListCtrl *)m_GISLayerBar.GetDlgItem(IDC_LIST_FEATURE_INFO);

	pGISFeatureList->DeleteAllItems ();
	
	if(!m_bFeatureInfoInitialized)
	{
	pGISFeatureList->InsertColumn(0,"Attribute",LVCFMT_LEFT,100);
	pGISFeatureList->InsertColumn(1,"Data",LVCFMT_LEFT,100);
	m_bFeatureInfoInitialized = true;
	}

	LV_ITEM lvi;
	for(int i = 0; i < m_FeatureInfoVector.size(); i++)
	{

		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		char text_str[100];
		sprintf(text_str,"%s",  m_FeatureInfoVector[i].Attribute.c_str());

		lvi.pszText =  text_str;
		int Index = pGISFeatureList->InsertItem(&lvi);
		pGISFeatureList->SetItemText(Index,1,m_FeatureInfoVector[i].Data);
	}

}

void CMainFrame::OnNMRClickListGislayer(NMHDR *pNMHDR, LRESULT *pResult)
{
}

void CMainFrame::OnBnClickedButtonDatabase()
{

}

void CMainFrame::OnBnClickedButtonConfiguration()
{
	
}



void CMainFrame::OnViewSimulationtoolbar()
{
	m_bShowSimulationoolBar= !m_bShowSimulationoolBar;
	if(m_bShowSimulationoolBar)
	{
		m_wndSimulationToolBar.ShowWindow (true);
	}
	else
		m_wndSimulationToolBar.ShowWindow (false);

	
}

void CMainFrame::OnUpdateViewSimulationtoolbar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bShowSimulationoolBar);
}


void CMainFrame::OnToolsSpaceTimeView()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL || (pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warning:  No active document for WindowNew command\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Command failed
	}

	// Otherwise, we have a new frame!
	CDocTemplate* pTemplate = ((CTLiteApp*)AfxGetApp())->m_pTemplateTimeTableView;
	ASSERT_VALID(pTemplate);

	CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL)
	{
		TRACE("Warning:  failed to create new frame\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Command failed
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}
