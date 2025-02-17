// TLite.cpp : Defines the class behaviors for the application.
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
#include "MainFrm.h"

#include "ChildFrm.h"
#include "TLiteDoc.h"
#include "TLiteView.h"
#include "DlgMOE.h"
#include "Dlg_VehicleClassification.h"
#include "Dlg_Legend.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CDlgMOE *g_LinkMOEDlg;
extern std::vector<CDlg_AgentClassification*>	g_SummaryDialogVector;
extern CDlg_Legend* g_pLegendDlg;

// CTLiteApp

BEGIN_MESSAGE_MAP(CTLiteApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CTLiteApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN, &CTLiteApp::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_NEW_DOC, &CTLiteApp::OnFileOpenNewDoc)
	ON_COMMAND(ID_FILE_OPENMULTIPLETRAFFICDATAPROJECTS, &CTLiteApp::OnFileOpenmultipletrafficdataprojects)
	ON_COMMAND(ID_APP_EXIT, &CTLiteApp::OnAppExit)
	ON_COMMAND(ID_FILE_OPEN_NETWORK_ONLY, &CTLiteApp::OnFileOpenNetworkOnly)
END_MESSAGE_MAP()




// CTLiteApp construction



// The one and only CTLiteApp object

CTLiteApp theApp;


// CTLiteApp initialization
CTLiteApp::CTLiteApp()
{

	m_Link1Color = RGB(030,144,255);
	m_Link2Color = RGB(160,032,240); 
	m_Link3Color = RGB(034,139,034);
	m_Link4Color = RGB(102, 102, 255);
	m_Link5Color = RGB(255,0,255);
	m_Link6Color = RGB(127,255,0);

	m_BackgroundColor =  RGB(255,255,255);

	m_PickupColor = RGB(255, 0, 0);
	m_DropoffColor = RGB(0, 0, 0);

	m_NodeColor = RGB(0,0,0);
	m_NodeBrushColor = RGB(0,0,0);

	m_ZoneColor = RGB(000,191,255);

	m_AgentColor[0] = RGB(190, 190, 190);
	m_AgentColor[1] = RGB(190, 190, 190);
	m_AgentColor[2] = RGB(0, 255, 0);
	m_AgentColor[3] = RGB(255, 250, 117);
	m_AgentColor[4] = RGB(255, 250, 0);
	m_AgentColor[5] = RGB(255, 216, 0);
	m_AgentColor[6] = RGB(255, 153, 0);
	m_AgentColor[7] = RGB(255, 0, 0);

	m_NEXTA_use_flag = 0;
	m_bLoadNetworkOnly = false;
	//m_pTemplateGLView = false;
	m_pTemplateTimeTableView = false;
	m_pDocTemplate2DView = NULL;
	m_pTemplateTimeTableView = NULL;

}
BOOL CTLiteApp::InitInstance()
{
//TODO: call AfxInitRichEdit2() to initialize richedit2 library.
	  CWinApp::InitInstance();

        // Standard initialization
        SetRegistryKey(_T("NeXTA GMNS"));
        LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

		char CurrentDirectory[MAX_PATH+1];
        GetCurrentDirectory(MAX_PATH,CurrentDirectory);

		char simulation_engine[MAX_PATH + 1];

		CString NEXTASettingsPath;
		NEXTASettingsPath.Format ("%s\\NEXTA_Settings.ini", CurrentDirectory);

		g_GetProfileString("initialization", "simulation_engine", "DTALite.exe", simulation_engine, 80, NEXTASettingsPath);

		m_SimulatorString_64.Format("%s",simulation_engine);


		m_NEXTA_use_flag = (int)g_GetPrivateProfileDouble("initialization", "nexta", 0, NEXTASettingsPath);
		WritePrivateProfileString("initialization", "nexta","1",NEXTASettingsPath);

		double value;
			

		value = (DWORD )g_GetPrivateProfileDouble("initialization", "BackgroundColor", -1, NEXTASettingsPath);

		if(value<0)
			m_BackgroundColor =   RGB(255,255,255);
		else
			m_BackgroundColor = (DWORD)  value;
			

		value = (DWORD )g_GetPrivateProfileDouble("initialization", "NodeColor", -1, NEXTASettingsPath);

		if(value<0)
			m_NodeColor =   RGB(0,0,0);
		else
			m_NodeColor = (DWORD)  value;
			
		value = (DWORD )g_GetPrivateProfileDouble("initialization", "NodeBrushColor", -1, NEXTASettingsPath);

		if(value<0)
			m_NodeBrushColor =   RGB(0,0,0);
		else
			m_NodeBrushColor = (DWORD)  value;


		char additional_field[MAX_PATH + 1];
		char additional_field_name[MAX_PATH + 1];


		CString Additional_Field;

		for(int k = 1; k<=5; k++)
		{
		sprintf(additional_field_name, "field%d", k);
		g_GetProfileString("Node_additional_fields", additional_field_name, "", additional_field, 80, NEXTASettingsPath);
		m_Node_Additional_Field[k].Format("%s", additional_field);
		}

		for (int k = 1; k <= 5; k++)
		{
			sprintf(additional_field_name, "field%d", k);
			g_GetProfileString("Link_additional_fields", additional_field_name, "", additional_field, 80, NEXTASettingsPath);
			m_Link_Additional_Field[k].Format("%s", additional_field);
		}


		//m_AgentColor[1] = (DWORD)g_GetPrivateProfileDouble("initialization", "AgentColor1", -1, NEXTASettingsPath);
		//m_AgentColor[2] = (DWORD)g_GetPrivateProfileDouble("initialization", "AgentColor2", -1, NEXTASettingsPath);
		//m_AgentColor[3] = (DWORD)g_GetPrivateProfileDouble("initialization", "AgentColor3", -1, NEXTASettingsPath);
		//m_AgentColor[4] = (DWORD)g_GetPrivateProfileDouble("initialization", "AgentColor4", -1, NEXTASettingsPath);


   char lpbuffer[_MAX_PATH];
   
   int visualization_template = 1;


			m_pDocTemplate2DView = new CMultiDocTemplate(IDR_TLiteTYPE1,
				RUNTIME_CLASS(CTLiteDoc),
				RUNTIME_CLASS(CChildFrame), // custom MDI child frame
				RUNTIME_CLASS(CTLiteView));
				

		if (!m_pDocTemplate2DView)
                return FALSE;
        AddDocTemplate(m_pDocTemplate2DView);
		
    //The AddDocTemplate() call generated by AppWizard established the primary document/frame/view combination for the application that is effective 
        //when the program starts. 
        //The template object below is a secondary template that can be activated in response to the New GLView Window menu item.



        // create main MDI Frame window
        CMainFrame* pMainFrame = new CMainFrame;
		

		if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
        {
                delete pMainFrame;
                return FALSE;
        }
        m_pMainWnd = pMainFrame;
        // call DragAcceptFiles only if there's a suffix
        //  In an MDI app, this should occur immediately after setting m_pMainWnd

        // Parse command line for standard shell commands, DDE, file open
        CCommandLineInfo cmdInfo;
        ParseCommandLine(cmdInfo);


        // Dispatch commands specified on the command line.  Will return FALSE if
        // app was launched with /RegServer, /Register, /Unregserver or /Unregister.
        if (!ProcessShellCommand(cmdInfo))
                return FALSE;

        GetCurrentDirectory(MAX_PATH,pMainFrame->m_CurrentDirectory);

		pMainFrame->SetTitle ("NeXTA-GMNS v0.9.12022020");

        // The main window has been initialized, so show and update it
        pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
        pMainFrame->UpdateWindow();

        return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CTLiteApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CTLiteApp message handlers


void CTLiteApp::OnFileOpen()
{
	static char BASED_CODE szFilter[] = "GMNS Data Files (node.csv)|node.csv|";

	 
   CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_ENABLESIZING , szFilter,NULL, 0, true);

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString NetworkFile = pMainFrame->m_CurrentDirectory;
	if(theApp.m_NEXTA_use_flag == 0)
	{
	dlg.m_ofn.lpstrInitialDir = NetworkFile;
	theApp.m_NEXTA_use_flag = 1;
	}else
	{// elsewise use the last used folder
		dlg.m_ofn.lpstrInitialDir = "";

	}

	

   if(dlg.DoModal() == IDOK)
   {
	  CString PathName =   dlg.GetPathName(); 
      POSITION p = m_pDocManager->GetFirstDocTemplatePosition();
      CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(p);
      CTLiteDoc* pDoc = (CTLiteDoc*)pTemplate->OpenDocumentFile(0);
	  pDoc->m_bLoadNetworkDataOnly = m_bLoadNetworkOnly;
	  pDoc->OnOpenDocument(PathName, m_bLoadNetworkOnly);
   }
}

void CTLiteApp::OnFileOpenNewDoc()
{
	m_bLoadNetworkOnly = false;
	OnFileOpen();
	//reset back
}


int CTLiteApp::ExitInstance()
{

	// Xuesong: potential memory leak
	//if(m_pDocTemplate2DView!=NULL)
	//delete m_pDocTemplate2DView;

	//if(m_pTemplateTimeTableView!=NULL)
	//delete m_pTemplateTimeTableView;
	//collect memory


	return CWinApp::ExitInstance();
}


void CTLiteApp::UpdateAllViews()
	{


	POSITION posTempl;
	POSITION posDoc;

	CMultiDocTemplate *pDocTempl;
	CDocument *pDoc;


	posTempl = GetFirstDocTemplatePosition(); 

	while(posTempl != NULL)
	{
		pDocTempl = (CMultiDocTemplate *) GetNextDocTemplate(posTempl); // first TEMPLATE
		posDoc = pDocTempl->GetFirstDocPosition();

		while(posDoc != NULL)
		{
			pDoc = pDocTempl->GetNextDoc(posDoc); 
			pDoc->UpdateAllViews (0);

		}
	} 


	if(g_LinkMOEDlg  && g_LinkMOEDlg ->GetSafeHwnd ())
	{
		g_LinkMOEDlg->Invalidate (true);
	}

	

}



void CTLiteApp::OnFileOpenmultipletrafficdataprojects()
{
   CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		   _T("Project Index File(*.pif)|*.pif|"));
   if(dlg.DoModal() == IDOK)
   {
	  /* read project index
	  // for each project
		  // project type: resolution (macro, micro, micro-scopic)
		  // with/without real-world data
	      // with/without prediction

	   different layers of data (data framework)
	  { import data to a new project
	     network
		 traffic control file
		 demand
		 scenario file (VMS, incident, ramp metering )
		 simulated data
		 observed data
			 detector sensor (multiple days)
		 	 AVI sensor (node to node, AVI)
			 corridor file (node to node file, )
		 probe sensor (local X) including NGSIM data
		 optional CSV files
		 demand
		 Agent trajectory
	   signal data
	   }	

	   multiple views

	   1) spatial 2d network
			MOE view (color coded)
	   2) spatial 3d network
			color coded + height (MOE (e.g. emissions)
	   3) time series 
			3.1 link MOE
				(speed, travel time, density, flow, queue length)
			3.2 network MOE
			3.3 path 
				(fuel efficiency)
	   4) OD to path view
	   5) time-space view 
			data vs. simulated
			point sensor, AVI, probe, estimated
	   6) quick summary of scenaro
			impacted vehicels by scenarios
			selected OD, selected path

		AMS Tools
		   0) basic GIS viewing tools
		      view, zoom-in, zoom-out, add and delete links, input scenaro and demand data

		   1) exporting data for selected subarea
			Synchro data
			DTALite
			TRANSIMS
			CORSIM
			
		   2) on-line GIS	google fusion tables -> KML

		   3)
			generate additional MOE
				emissions
				safety
				reliability
			4) dynamic OD demand estimation

			5) generate capacity using quick analysis tables

			6) communicate with data bus (upload and download dat from central data bus, communicate with simulation clock)
			simulator warpper (call external excutable, exchange data with data bus)
				{
				DYNASMART
				DTALite
				TRANSIMS
				}
	
			7) scenario comparison 
				estimation vs. prediction
				different resolutions
				data vs. estimation results, faulty sensors, imcompartable 

	   // end of each project

	  */


      POSITION p = m_pDocManager->GetFirstDocTemplatePosition();
      CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(p);
      CTLiteDoc* pDoc = (CTLiteDoc*)pTemplate->OpenDocumentFile(0);
      pDoc->OnOpenDocument(dlg.GetPathName());
   }
}


void CTLiteApp::OnAppExit()
{

	exit(0);
}

void CTLiteApp::OnFileOpenNetworkOnly()
{
	m_bLoadNetworkOnly = true;
	OnFileOpen();
	//reset back
	m_bLoadNetworkOnly = false;}


void CTLiteApp::OnHelpUserguide()
{
	g_OpenDocument("https://docs.google.com/document/d/14tUa1I6Xf62zsiWf4lLfngqGqGJlIM_MSehLFMVXass/edit", SW_SHOW);
}

void CTLiteApp::OnHelpChecklatestsoftwarerelease()
{

	g_OpenDocument("http://code.google.com/p/nexta/downloads/list", SW_SHOW);

}

