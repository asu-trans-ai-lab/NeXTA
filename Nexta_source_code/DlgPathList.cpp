// DlgPathList.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "CSVParser.h"
#include "DlgPathList.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"
#include "Dlg_VehicleClassification.h"
#include "CDlg_UserInput.h"
#include <string>
#include <sstream>
#include <vector>

extern void g_SelectColorCode(CDC* pDC, int ColorCount);
extern float g_Simulation_Time_Stamp;
extern CPen s_PenSimulationClock;

extern void g_SelectSuperThickPenColor(CDC* pDC, int ColorCount);
extern void g_SelectThickPenColor(CDC* pDC, int ColorCount);
extern void g_SelectBrushColor(CDC* pDC, int ColorCount);


extern CDlgPathList* g_pPathListDlg;
// CDlgPathList dialog

IMPLEMENT_DYNAMIC(CDlgPathList, CDialog)

CDlgPathList::CDlgPathList(CWnd* pParent /*=NULL*/)
: CBaseDialog(CDlgPathList::IDD, pParent)
, m_StrPathMOE(_T(""))
{
	m_TimeLeft = 0;
	m_TimeRight = 1440;
	m_MOEAggregationIntervalInMin = 1;
}


CDlgPathList::~CDlgPathList()
{
	g_pPathListDlg = NULL;
}

void CDlgPathList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ListCtrl);
	DDX_Check(pDX, IDC_CHECK_ZOOM_TO_SELECTED_LINK, m_ZoomToSelectedLink);
	DDX_Text(pDX, IDC_PATHMOE, m_StrPathMOE);
}


BEGIN_MESSAGE_MAP(CDlgPathList, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()

	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CDlgPathList::OnLvnItemchangedList)
	ON_BN_CLICKED(IDOK, &CDlgPathList::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgPathList::OnBnClickedCancel)
	ON_BN_CLICKED(ID_FIND_RANDOM_ROUTE, &CDlgPathList::OnBnClickedFindRandomRoute)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgPathList::OnCbnSelchangeCombo1)
	ON_COMMAND(ID_PATH_DATA_EXPORT_CSV, &CDlgPathList::OnPathDataExportCSV)
	ON_BN_CLICKED(IDC_CHECK_ZOOM_TO_SELECTED_LINK, &CDlgPathList::OnBnClickedCheckZoomToSelectedLink)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlgPathList::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDDATA_Analysis, &CDlgPathList::OnBnClickedDataAnalysis)
	ON_COMMAND(ID_DATA_CLEANALLPATHS, &CDlgPathList::OnDataCleanallpaths)
	ON_CBN_SELCHANGE(IDC_COMBO_AggIntrevalList, &CDlgPathList::OnCbnSelchangeComboAggintrevallist)
	ON_CBN_SELCHANGE(IDC_COMBO_PLOT_TYPE, &CDlgPathList::OnCbnSelchangeComboPlotType)
	ON_BN_CLICKED(IDDATA_Analysis2, &CDlgPathList::OnBnClickedAnalysis2)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGELANECAPACITY, &CDlgPathList::OnChangeattributesforlinksalongpathChangelanecapacity)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGE, &CDlgPathList::OnChangeattributesforlinksalongpathChange)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGELINKTYPE, &CDlgPathList::OnChangeattributesforlinksalongpathChangelinktype)
	ON_BN_CLICKED(IDDATA_DYNAMIC_Density_Contour, &CDlgPathList::OnBnClickedDynamicDensityContour)
	ON_BN_CLICKED(IDDATA_DYNAMIC_Speed_Contour, &CDlgPathList::OnBnClickedDynamicSpeedContour)
//	ON_BN_CLICKED(IDDATA_DYNAMIC_Flow_Contour, &CDlgPathList::OnBnClickedDynamicFlowContour)
	ON_COMMAND(ID_DATA_DELETESELECTEDPATH, &CDlgPathList::OnDataDeleteselectedpath)
	ON_COMMAND(ID_DATA_COCONTOURPLOT, &CDlgPathList::OnDataCocontourplot)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_FREESPEED, &CDlgPathList::OnChangeattributesforlinksalongpathFreespeed)
	ON_BN_CLICKED(IDDATA_DYNAMIC_diagram, &CDlgPathList::OnBnClickedDynamicSpacetimediagram)
END_MESSAGE_MAP()


// CDlgPathList message handlers
#define PATHDATACOLUMNSIZE 9

BOOL CDlgPathList::OnInitDialog()
{

	CDialog::OnInitDialog();
	// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);



	std::vector<std::string> m_Column_names;

	_TCHAR *ColumnMOELabel[PATHDATACOLUMNSIZE] =
	{
		_T("No."),_T("From->To"),_T("Name"), _T("Length"), _T("Free Speed"), _T("Free-flow Travel Time (min)"),_T("# of lanes"),_T("Lane Capacity"), _T("Link Type"),  
	};


	//Add Columns and set headers
	for (size_t i=0;i< PATHDATACOLUMNSIZE;i++)
	{
		CGridColumnTrait* pTrait = NULL;
		//		pTrait = new CGridColumnTraitEdit();
		m_ListCtrl.InsertColumnTrait((int)i,ColumnMOELabel[i],LVCFMT_LEFT,-1,-1, pTrait);
		m_ListCtrl.SetColumnWidth((int)i,LVSCW_AUTOSIZE_USEHEADER);
	}
	m_ListCtrl.SetColumnWidth(0, 80);

	ReloadData();

	return true;
}
void CDlgPathList::ReloadData()
{
	CWaitCursor cursor;

	m_ListCtrl.DeleteAllItems();

	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		float total_distance = 0;
		float total_travel_time = 0;
		int number_of_sensors = 0;
		float total_count = 0;

		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{

			int column_count  = 1;
			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
			char text[100];


			sprintf_s(text, "%d",i+1);
			sprintf_s(text, "%d",i+1);
			int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);

			// from -> to
			sprintf_s(text, "%d->%d",pLink->m_FromNodeID , pLink->m_ToNodeID );
			m_ListCtrl.SetItemText(Index,column_count++,text );

			// street name
			m_ListCtrl.SetItemText(Index,column_count++,pLink->m_Name.c_str () );

			//length
			sprintf_s(text, "%7.5f",pLink->m_Length);
			m_ListCtrl.SetItemText(Index,column_count++,text);
			total_distance+= pLink->m_Length;

			//Free Speed
			sprintf_s(text, "%4.0f",pLink->m_FreeSpeed  );
			m_ListCtrl.SetItemText(Index,column_count++,text);

			//free flow travel time
			sprintf_s(text, "%7.5f",pLink->m_FreeFlowTravelTime);
			m_ListCtrl.SetItemText(Index,column_count++,text);
			total_travel_time+=pLink->m_FreeFlowTravelTime;

			//# of lanes
			sprintf_s(text, "%d",pLink->m_NumberOfLanes );
			m_ListCtrl.SetItemText(Index,column_count++,text);

			//capacity
			sprintf_s(text, "%4.2f",pLink->m_LaneCapacity);
			m_ListCtrl.SetItemText(Index,column_count++,text);

			if(m_pDoc->m_LinkTypeMap.find(pLink->m_link_type) != m_pDoc->m_LinkTypeMap.end())
			{

				if(m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.size()>=1)
				{
					sprintf_s(text, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
					m_ListCtrl.SetItemText(Index,column_count++,text);
				}
				else
				{
					sprintf_s(text, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
					m_ListCtrl.SetItemText(Index,column_count++,text);
				}

			}

	
		m_ListCtrl.SetItemText(Index,column_count++,text);


		}

			m_StrPathMOE.Format("Distance=%4.2f meters (%4.4f miles), Free-flow Travel Time=%4.2f min",
				total_distance, total_distance/1609.34, total_travel_time);
	
		
		UpdateData(0);
	}



}

void CDlgPathList::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{

	UpdateData(1);

	m_pDoc->m_SelectedLinkNo = -1;
	g_ClearLinkSelectionList();

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);
		char str[100];
		m_ListCtrl.GetItemText (nSelectedRow,2,str,20);
		int LinkNo = atoi(str);
		m_pDoc->m_SelectedLinkNo = LinkNo;
		DTALink* pLink = m_pDoc->m_LinkNoMap [LinkNo];

		g_AddLinkIntoSelectionList(pLink, LinkNo, m_pDoc->m_DocumentNo );

	}
	if(m_ZoomToSelectedLink == true)
		m_pDoc->ZoomToSelectedLink(m_pDoc->m_SelectedLinkNo);

	Invalidate();

	m_pDoc->UpdateAllViews(0);

}

void CDlgPathList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnBnClickedCancel()
{
	CDialog::OnOK();
}



void CDlgPathList::OnBnClickedFindAlternative()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnBnClickedFindRandomRoute()
{
	m_pDoc->m_RandomRoutingCoefficient = m_ComboRandomCoef.GetCurSel ()*0.2f;
	m_pDoc->Routing(false);
	m_pDoc->UpdateAllViews(NULL);

	ReloadData();
	Invalidate();


}

void CDlgPathList::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnBnClickedExportRoute()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnPathDataExportCSV()
{
	// calculate time-dependent travel time
	int time_step = 1;

	CString export_file_name;
	export_file_name = m_pDoc->m_ProjectDirectory + "path.csv";
	FILE* st_path;
	fopen_s(&st_path, export_file_name, "w");
	if (st_path != NULL)
	{
		fprintf(st_path, "agent_id,o_zone_id,d_zone_id,path_id,o_node_id,d_node_id,agent_type,demand_period,ratio,node_sequence\n");

		for (unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			if(path_element.m_LinkVector.size() >=1)
			{ 
				DTALink* pFirstLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[0]];
				DTALink* pLastLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[path_element.m_LinkVector.size()-1]];
				fprintf(st_path, "0,0,0,0,%d,%d,0,0,1,", pFirstLink->m_FromNodeID, pLastLink->m_ToNodeID);
			for (int i = 0; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if (pLink != NULL)
				{
					if (i == 0)
					{
						fprintf(st_path, "%d;", pLink->m_FromNodeID);
					}
					fprintf(st_path, "%d;", pLink->m_ToNodeID);
				}

			}
			}
		}

		fclose(st_path);
	}
	else
	{
	AfxMessageBox("File path.csv cannot be opened.");
	return;
	}
	m_pDoc->OpenCSVFileInExcel(export_file_name);



	export_file_name = m_pDoc->m_ProjectDirectory +"path_performance.csv";
	// save demand here

	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		fprintf(st,"Summary\n");
		fprintf(st,"path index,\n");


		fprintf(st,"\n\nPart I,time-dependent travel time");
		// 

		time_step= 15;


		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			fprintf(st,"\nTime,,,");

			for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
			}

			fprintf(st,"\nPath %d, %s,travel time,",p+1,path_element.m_path_name .c_str ());

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentTravelTime[t]);

			}  // for each time


		}

		// travel time index

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			float total_free_flow_travel_time = 0;
			for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				total_free_flow_travel_time += pLink->m_FreeFlowTravelTime ;
			}
			m_pDoc->m_PathDisplayList[p].total_free_flow_travel_time = total_free_flow_travel_time;
		}

		fprintf(st,"\n\nTravel Time Tndex,,");

		for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
		{
			fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
		}

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];


			fprintf(st,"\nPath %d, %s,",p+1, path_element.m_path_name .c_str ());
			for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentTravelTime[t]/max(1,path_element.total_free_flow_travel_time));

			}  // for each time

		}

		fprintf(st,"\n\nPart II,link sequence\n\n");

		fprintf(st, "path_id,path_node_sequence\n");


		for (unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			fprintf(st, "%d,", p+1);

			for (int i = 0; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if (pLink != NULL)
				{
					if (i == 0)
					{
						fprintf(st, "%d;", pLink->m_FromNodeID);
					}
					fprintf(st, "%d;", pLink->m_ToNodeID);
				}

			}
			fprintf(st, ",\n");
		} //for each path

		fprintf(st, "\npath_id,link_sequence_no,link_id,from_node_id->to_node_id,from_node_id,to_node_id,name,length,free speed,free-flow travel_time,# of lanes,Lane Saturation Flow Rate,Lane Capacity,Link Type\n");

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink != NULL)
				{

					fprintf(st,"%d,%d,%s,\"[%d,%d]\",%d,%d,%s,%5.3f,%5.0f,%5.3f,%d,%5.0f,%5.1f,",
						p+1,i+1, pLink->m_LinkID.c_str(), pLink->m_FromNodeID , pLink->m_ToNodeID, pLink->m_FromNodeID , pLink->m_ToNodeID,   pLink->m_Name.c_str (), pLink->m_Length ,
						pLink->m_FreeSpeed, pLink->m_FreeFlowTravelTime , pLink->m_NumberOfLanes,  pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane ,pLink->m_LaneCapacity );

					if(m_pDoc->m_LinkTypeMap.find(pLink->m_link_type) != m_pDoc->m_LinkTypeMap.end())
					{
						fprintf(st, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
					}
				}

				fprintf(st,"\n");


			}
		} //for each path

		//// part II: time-dependent speed contour
		//int step_size = 1;

		//fprintf(st,"\n\nPart IV");

		//// 

		//time_step= 15;


		//for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		//{
		//	DTAPath path_element = m_pDoc->m_PathDisplayList[p];

		//	fprintf(st,"\nTime,,,");

		//	for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
		//	{
		//		fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
		//	}

		//	// for each time
		//	fprintf(st,"\n");

		//}


		fclose(st);
	}else
	{
		AfxMessageBox("File path_performance.csv cannot be opened.");
		return;
	}
	m_pDoc->OpenCSVFileInExcel (export_file_name);
}


void CDlgPathList::OnBnClickedCheckZoomToSelectedLink()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnLbnSelchangeList1()
{
	m_pDoc->m_SelectPathNo = 0;

	ReloadData();

	Invalidate();
	m_pDoc->UpdateAllViews (0);

}

void CDlgPathList::OnBnClickedDataAnalysis()
{
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{

		(*iLink)-> m_bIncludedBySelectedPath = false;
		(*iLink)-> m_bFirstPathLink = false;
		(*iLink)-> m_bLastPathLink = false;
	}

	// mark all links in the selected path
	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{

			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
			pLink-> m_bIncludedBySelectedPath = true;

			if(i==0)
			{
				pLink-> m_bFirstPathLink = true;
			}
			if(i== m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size()-1)
			{
				pLink-> m_bLastPathLink = true;
			}
		}
	}

	CDlg_AgentClassification* m_pDlg = new CDlg_AgentClassification; 

	m_pDlg->m_PresetChartTitle.Format ("Path No.%d %s", m_pDoc->m_SelectPathNo+1, m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_path_name .c_str());

	m_pDlg->m_pDoc = m_pDoc;
	m_pDoc->m_AgentSelectionMode = CLS_path_trip;
	m_pDlg->m_XSelectionNo = CLS_time_interval_15_min;
	m_pDlg->m_AgentSelectionNo  = CLS_path_trip;
	m_pDlg->SetModelessFlag(true); // voila! this is all it takes to make your dlg modeless!
	m_pDlg->Create(IDD_DIALOG_Summary); 
	m_pDlg->ShowWindow(SW_SHOW); 

}


void CDlgPathList::OnDataCleanallpaths()
{
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
	{
		m_pDoc->m_PathDisplayList[p].m_LinkVector.clear ();
	}
	m_pDoc->m_PathDisplayList.clear ();

	m_ListCtrl.DeleteAllItems();

	m_pDoc->m_ONodeNo = -1;
	m_pDoc->m_DNodeNo = -1;
	m_StrPathMOE.Format ("");
	UpdateData(0);
	m_pDoc->UpdateAllViews(0);

}


void CDlgPathList::OnCbnSelchangeComboAggintrevallist()
{
}

void CDlgPathList::OnCbnSelchangeComboPlotType()
{
	Invalidate();
}
void CDlgPathList::OnSize(UINT nType, int cx, int cy)
{
	RedrawWindow();

	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}


void CDlgPathList::OnBnClickedAnalysis2()
{
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{

		(*iLink)-> m_bIncludedBySelectedPath = false;
		(*iLink)-> m_bFirstPathLink = false;
		(*iLink)-> m_bLastPathLink = false;

	}

	// mark all links in the selected path
	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{

			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
			pLink-> m_bIncludedBySelectedPath = true;

			if(i==0)
			{
				pLink-> m_bFirstPathLink = true;
			}
			if(i== m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size()-1)
			{
				pLink-> m_bLastPathLink = true;
			}

		}
	}

	CDlg_AgentClassification* m_pDlg = new CDlg_AgentClassification; 

	m_pDlg->m_PresetChartTitle.Format ("End-to-End Path No.%d", m_pDoc->m_SelectPathNo+1);

	m_pDlg->m_pDoc = m_pDoc;
	m_pDoc->m_AgentSelectionMode = CLS_path_partial_trip;
	m_pDlg->m_XSelectionNo = CLS_time_interval_15_min;
	m_pDlg->m_AgentSelectionNo  = CLS_path_partial_trip;
	m_pDlg->SetModelessFlag(true); // voila! this is all it takes to make your dlg modeless!
	m_pDlg->Create(IDD_DIALOG_Summary); 
	m_pDlg->ShowWindow(SW_SHOW); 

}




void CDlgPathList::OnChangeattributesforlinksalongpathChangelanecapacity()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_lane_capacity;
	ChangeLinkAttributeDialog();
}

void CDlgPathList::OnChangeattributesforlinksalongpathChange()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_number_of_lanes;
	ChangeLinkAttributeDialog();

}

void CDlgPathList::OnChangeattributesforlinksalongpathChangelinktype()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_link_type;
	ChangeLinkAttributeDialog();
}






void CDlgPathList::ChangeLinkAttributeDialog()
{
	CDlg_UserInput dlg;



	float value = 0;

	switch (m_ChangeLinkAttributeMode)
	{
	case eChangeLinkAttribute_lane_capacity:
		dlg.m_StrQuestion = "Please input the value for lane capacity:";
		dlg.m_InputValue = "2000";
		break;
	case eChangeLinkAttribute_number_of_lanes: 
		dlg.m_StrQuestion = "Please input the number of lanes:";
		dlg.m_InputValue = "3"; 
		break;
	case eChangeLinkAttribute_link_type: 
		dlg.m_StrQuestion = "Please input the value for link type:";
		dlg.m_InputValue = "1"; 
		break;
	case eChangeLinkAttribute_free_speed:
		dlg.m_StrQuestion = "Please input the value for free speed:";
		dlg.m_InputValue = "65"; 
		break;
	}

	if (dlg.DoModal() == IDOK)
	{
		value = atof(dlg.m_InputValue);


		if (AfxMessageBox("Are you sure to make the change?", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{
			ChangeLinkAttributeAlongPath(value, dlg.m_InputValue);
		}
	}


}


void CDlgPathList::ChangeLinkAttributeAlongPath(float value, CString value_string)
{
		m_pDoc->m_SelectPathNo = 0;

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(m_pDoc->m_SelectPathNo!=p)
				continue;
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			
			int i;

			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{

				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink != NULL)
				{
					m_pDoc->Modify (true);

					int ToNodeNo; 
					DTANode* pNode =NULL;
					switch(m_ChangeLinkAttributeMode)
					{
					case eChangeLinkAttribute_lane_capacity: pLink->m_LaneCapacity = value; break;
					case eChangeLinkAttribute_number_of_lanes: pLink->m_NumberOfLanes = value; break;
					case eChangeLinkAttribute_link_type: pLink->m_link_type = value; break;
					case eChangeLinkAttribute_free_speed : pLink->m_FreeSpeed = value; break;
					
					}


				}  // for all links
			}
		}

		m_pDoc->UpdateAllViews(0);
}


void CDlgPathList::OnBnClickedDynamicDensityContour()
{
	CString PathTitle;
	SetCurrentDirectory(m_pDoc->m_ProjectDirectory);

	int ytics_stepsize  = 1;

	CString export_file_name, export_plt_file_name;

	export_file_name = m_pDoc->m_ProjectDirectory +"export_path_density.txt";

	export_plt_file_name = m_pDoc->m_ProjectDirectory +"export_path_density.plt";

	int yrange = 0;
	int xrange = m_TimeRight - m_TimeLeft +1;


	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		
	int step_size = 1;

	
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			PathTitle.Format("Path %d %s", p+1,  path_element.m_path_name.c_str () );

			if(path_element.m_LinkVector.size() >=15)
				ytics_stepsize = 4;

			if(path_element.m_LinkVector.size() >=30)
				ytics_stepsize = 10;

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							 
							CString label = pLink->m_Name .c_str ();

							//if(pLink->m_Name  == "(null)")
							//{
							//label.Format ("%d->%d",pLink->m_FromNodeID , pLink->m_ToNodeID);
							//}

							//fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeID , pLink->m_ToNodeID , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "%.1f ", pLink->GetSimulatedDensity (t));

							}
							fprintf(st,"\n");

							yrange++;
						}

					}


				}

							// last line with zero
							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "0.0 ");

							}

							fprintf(st,"\n");

			} //for each path
		
	fclose(st);
	}


	FILE* st_plt;
	fopen_s(&st_plt,export_plt_file_name,"w");
	if(st_plt!=NULL)
	{

		CString xtics_str; 

		fprintf(st_plt,"set title \"Dynamic Density Contour (%s) Unit: veh/mile/lane\" \n", PathTitle);

		fprintf(st_plt,"set xlabel \"Time Horizon\"\n");
		fprintf(st_plt,"set ylabel \"Space (Node Sequence)\"  offset -1\n");

		int xtics_stepsize  = 30;

		if(xrange/xtics_stepsize >20)
			xtics_stepsize = 120;  // 2 hour interval
		else if(xrange/xtics_stepsize >10)
			xtics_stepsize = 60;   // 1 hour interval
		else if(xrange/xtics_stepsize < 5)
			xtics_stepsize = 10;   // 1 hour interval

			 
		for(int t = m_TimeLeft ; t<= m_TimeRight; t+= xtics_stepsize)  
		{
			CString str;
			str.Format("\"%s\" %d ",m_pDoc->GetTimeStampString24HourFormat (t), t-m_TimeLeft);

			if(t+ xtics_stepsize> m_TimeRight ) 
				xtics_str += str;
			else 
			{
				xtics_str += str ;
				xtics_str += "," ;
			}
		}

		fprintf(st_plt,"set xtics (%s) \n",xtics_str);

		CString ytics_str; 

		int yrange_i  = 0;

		CString last_node_number = " ";

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{


			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							CString label = pLink->m_Name .c_str ();


							if(pLink->m_Name  == "(null)" || pLink->m_Name.size() ==0)
							{
							label.Format ("%d",pLink->m_FromNodeID);

							last_node_number.Format ("%d",pLink->m_ToNodeID);
							}

							if(s==0 && (i%ytics_stepsize) ==0)   // first segment 
							{
							CString str;
							str.Format("\"%s\" %d, ",label, yrange_i) ;

							ytics_str += str;
							
							}

							yrange_i++;
						}

					}


				}

				
			} //for each path


		CString str;
		str.Format("\"%s\" %d", last_node_number, yrange_i);
		ytics_str +=str;
		fprintf(st_plt,"set ytics (%s)\n",ytics_str);

		fprintf(st_plt,"set xrange [0:%d] \n",xrange);
		fprintf(st_plt,"set yrange [0:%d] \n",yrange);

		fprintf(st_plt,"set palette defined (0 \"white\", 10 \"green\", 30 \"yellow\", 50 \"red\")\n");


		fprintf(st_plt,"set pm3d map\n");
		fprintf(st_plt,"splot '%s' matrix\ notitle\n",export_file_name);

		fclose(st_plt);
	}else
	{
	AfxMessageBox("File export_path_density.plt cannot be opened. Please check");
	}

	HINSTANCE result = ShellExecute(NULL, _T("open"), export_plt_file_name, NULL,NULL, SW_SHOW);
	

}
void CDlgPathList::OnBnClickedDynamicSpeedContour()
{
	m_TimeLeft = m_pDoc->m_DemandLoadingStartTimeInMin;
	m_TimeRight = m_pDoc->m_DemandLoadingEndTimeInMin;

	CString export_file_name, export_plt_file_name;

	export_file_name = m_pDoc->m_ProjectDirectory +"export_path_speed.txt";
	// 
	SetCurrentDirectory(m_pDoc->m_ProjectDirectory);

	export_plt_file_name = m_pDoc->m_ProjectDirectory +"export_path_speed.plt";

	int yrange = 0;

	CString PathTitle;

	int ytics_stepsize  = 1;
	bool bFreewayFlag = true;
	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		
	int step_size = 5;
	
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			PathTitle.Format("Path %d %s", p+1,  path_element.m_path_name.c_str () );


			if(path_element.m_LinkVector.size() >=15)
				ytics_stepsize = 4;

			if(path_element.m_LinkVector.size() >=30)
				ytics_stepsize = 10;
				//for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
				//{
				//	fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
				//}

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						if(m_pDoc->m_LinkTypeMap[pLink->m_link_type].IsFreeway () == false)
							bFreewayFlag = false;

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{
							 
							CString label = pLink->m_Name .c_str ();

							//if(pLink->m_Name  == "(null)")
							//{
							//label.Format ("%d->%d",pLink->m_FromNodeID , pLink->m_ToNodeID);
							//}

							//fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeID , pLink->m_ToNodeID , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "%.1f ", pLink->GetDynamicSpeed (t));

							}
							fprintf(st,"\n");
							yrange++;
						}

					}


				}
							// last line with zero
							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "0.0 ");

							}

							fprintf(st,"\n");

			} //for each path
		
	fclose(st);
	}

	int xrange = m_TimeRight - m_TimeLeft +1;


	FILE* st_plt;
	fopen_s(&st_plt,export_plt_file_name,"w");
	if(st_plt!=NULL)
	{

		CString xtics_str; 

		fprintf(st_plt,"set title \"Dynamic Speed Contour (%s) Unit: mph\" \n", PathTitle);


		fprintf(st_plt,"set xlabel \"Time Horizon\"\n");
		fprintf(st_plt,"set ylabel \"Space (Node Sequence)\"  offset -1\n");

		int xtics_stepsize  = 30;

		if(xrange/xtics_stepsize >20)
			xtics_stepsize = 120;  // 2 hour interval
		else if(xrange/xtics_stepsize >10)
			xtics_stepsize = 60;   // 1 hour interval
		else if(xrange/xtics_stepsize < 5)
			xtics_stepsize = 10;   // 1 hour interval

			 
		for(int t = m_TimeLeft ; t<= m_TimeRight; t+= xtics_stepsize)  
		{
			CString str;
			str.Format("\"%s\" %d ",m_pDoc->GetTimeStampString24HourFormat (t), t-m_TimeLeft);

			if(t+ xtics_stepsize> m_TimeRight ) 
				xtics_str += str;
			else 
			{
				xtics_str += str ;
				xtics_str += "," ;
			}
		}

		fprintf(st_plt,"set xtics (%s) \n",xtics_str);

		CString ytics_str; 

		int yrange_i  = 0;

	CString last_node_number = " ";

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							CString label = pLink->m_Name .c_str ();


							if(pLink->m_Name  == "(null)" || pLink->m_Name.size() ==0)
							{
							label.Format ("%d",pLink->m_FromNodeID);

							last_node_number.Format ("%d",pLink->m_ToNodeID);
							}

							if(s==0 && (i%ytics_stepsize) ==0)   // first segment 
							{
							CString str;
							str.Format("\"%s\" %d, ",label, yrange_i) ;

							ytics_str += str;
							
							}

							yrange_i++;
						}

					}


				}

				
			} //for each path


		CString str;
		str.Format("\"%s\" %d", last_node_number, yrange_i);
		ytics_str +=str;
		fprintf(st_plt,"set ytics (%s)\n",ytics_str);

		fprintf(st_plt,"set xrange [0:%d] \n",xrange);
		fprintf(st_plt,"set yrange [0:%d] \n",yrange);

		if(bFreewayFlag)
			fprintf(st_plt,"set palette defined (0 \"white\", 0.1 \"red\", 40 \"yellow\", 50 \"green\")\n");
		else
			fprintf(st_plt,"set palette defined (0 \"white\", 0.1 \"red\", 20 \"yellow\", 40 \"green\")\n");

		fprintf(st_plt,"set pm3d map\n");
		fprintf(st_plt,"splot '%s' matrix\ notitle\n",export_file_name);

		fclose(st_plt);
	}else
	{
	AfxMessageBox("File export_path_speed.plt cannot be opened. Please check");
	}



	HINSTANCE result = ShellExecute(NULL, _T("open"), export_plt_file_name, NULL,NULL, SW_SHOW);
}

void CDlgPathList::OnBnClickedDynamicFlowContour()
{
	int ytics_stepsize = 1;
	CString export_file_name, export_plt_file_name;
	SetCurrentDirectory(m_pDoc->m_ProjectDirectory);

	export_file_name = m_pDoc->m_ProjectDirectory +"export_path_v_over_c.txt";


	export_plt_file_name = m_pDoc->m_ProjectDirectory +"export_path_v_over_c.plt";

	int yrange = 0;
	int xrange = m_TimeRight - m_TimeLeft +1;

	CString PathTitle;

	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		
	int step_size = 1;
	
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{



			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			PathTitle.Format("Path %d %s", p+1,  path_element.m_path_name.c_str () );

			if(path_element.m_LinkVector.size() >=15)
				ytics_stepsize = 4;

			if(path_element.m_LinkVector.size() >=30)
				ytics_stepsize = 10;

				//for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
				//{
				//	fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
				//}

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{
							 
							CString label = pLink->m_Name .c_str ();

							//if(pLink->m_Name  == "(null)")
							//{
							//label.Format ("%d->%d",pLink->m_FromNodeID , pLink->m_ToNodeID);
							//}

							//fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeID , pLink->m_ToNodeID , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{
								float LaneClosurePercentage = pLink->GetImpactedFlag(t); // check capacity reduction event;
								float Capacity=  pLink->m_NumberOfLanes * pLink->m_LaneCapacity *(1-	LaneClosurePercentage);		

								float volume  = pLink->GetSimulatedLinkOutVolume (t);
								float voc = volume/max(1,Capacity);
								if(voc>1.01 && m_pDoc->m_traffic_flow_model!=0)  // not BPR function 
									voc = 1.0;
								fprintf(st, "%.2f ", voc);

								if( pLink->m_FromNodeID == 30 && pLink->m_ToNodeID == 31)
								{
						
								TRACE("\n%f; %f; %f", volume, Capacity, voc);
								}

							}
							fprintf(st,"\n");

							yrange++;
						}

					}


				}

						//last line with zero
							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "0.00 ");

							}

							fprintf(st,"\n");
			} //for each path
		
	fclose(st);
	}


	FILE* st_plt;
	fopen_s(&st_plt,export_plt_file_name,"w");
	if(st_plt!=NULL)
	{

		CString xtics_str; 

		fprintf(st_plt,"set title \"Dynamic Volume Over Capcity Contour (%s)\" \n", PathTitle);


		fprintf(st_plt,"set xlabel \"Time Horizon\"\n");
		fprintf(st_plt,"set ylabel \"Space (Node Sequence)\"  offset -1\n");

		int xtics_stepsize  = 30;

		if(xrange/xtics_stepsize >20)
			xtics_stepsize = 120;  // 2 hour interval
		else if(xrange/xtics_stepsize >10)
			xtics_stepsize = 60;   // 1 hour interval
		else if(xrange/xtics_stepsize < 5)
			xtics_stepsize = 10;   // 1 hour interval

			 
		for(int t = m_TimeLeft ; t<= m_TimeRight; t+= xtics_stepsize)  
		{
			CString str;
			str.Format("\"%s\" %d ",m_pDoc->GetTimeStampString24HourFormat (t), t-m_TimeLeft);

			if(t+ xtics_stepsize> m_TimeRight ) 
				xtics_str += str;
			else 
			{
				xtics_str += str ;
				xtics_str += "," ;
			}
		}

		fprintf(st_plt,"set xtics (%s) \n",xtics_str);

		CString ytics_str; 

		int yrange_i  = 0;

		CString last_node_number = " ";

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							CString label = pLink->m_Name .c_str ();

							last_node_number.Empty ();
							if(pLink->m_Name  == "(null)" || pLink->m_Name.size() ==0)
							{
							label.Format ("%d",pLink->m_FromNodeID);

							last_node_number.Format ("%d",pLink->m_ToNodeID);
							}

							if(s==0 && (i%ytics_stepsize) ==0)   // first segment 
							{
							CString str;
							str.Format("\"%s\" %d, ",label, yrange_i) ;

							ytics_str += str;
							
							}

							yrange_i++;
						}

					}


				}

				
			} //for each path


		CString str;
		str.Format("\"%s\" %d", last_node_number, yrange_i);
		ytics_str +=str;
		fprintf(st_plt,"set ytics (%s)\n",ytics_str);

		fprintf(st_plt,"set xrange [0:%d] \n",xrange);
		fprintf(st_plt,"set yrange [0:%d] \n",yrange);

		fprintf(st_plt,"set palette defined (0 \"white\", 0.4 \"green\", 0.6 \"yellow\", 1 \"red\")\n");
		fprintf(st_plt,"set pm3d map\n");
		fprintf(st_plt,"splot '%s' matrix\ notitle\n",export_file_name);

		fclose(st_plt);
	}else
	{
	AfxMessageBox("File export_path_v_over_c.plt cannot be opened. Please check");
	}

	HINSTANCE result = ShellExecute(NULL, _T("open"), export_plt_file_name, NULL,NULL, SW_SHOW);
}





void CDlgPathList::OnDataDeleteselectedpath()
{
	std::vector<DTAPath>	m_TempPathDisplayList = m_pDoc->m_PathDisplayList;

	m_pDoc->m_PathDisplayList.clear();

	for(unsigned int p = 0; p< m_TempPathDisplayList.size(); p++)
	{
	 if(p!= m_pDoc->m_SelectPathNo)
	 {
		 m_pDoc->m_PathDisplayList.push_back (m_TempPathDisplayList[p]);
	 }

	}

	 m_pDoc->m_SelectPathNo = min( m_pDoc->m_SelectPathNo, m_pDoc->m_PathDisplayList.size()-1);
	 	ReloadData();
}




void CDlgPathList::OnDataCocontourplot()
{
}



void CDlgPathList::OnBnClickedPathDataExportCsv()
{
	// TODO: Add your control notification handler code here
}




void CDlgPathList::OnChangeattributesforlinksalongpathFreespeed()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_free_speed;
	ChangeLinkAttributeDialog();
}


void CDlgPathList::OnBnClickedDynamicSpacetimediagram()
{
	CString PathTitle;

	int ytics_stepsize = 1;

	CString export_file_name, export_plt_file_name;




	int yrange = 0;
	int xrange = m_TimeRight - m_TimeLeft + 1;

	SetCurrentDirectory(m_pDoc->m_ProjectDirectory);

	export_plt_file_name = m_pDoc->m_ProjectDirectory + "space_time_diagram.plt";
	FILE* st_plt;
	fopen_s(&st_plt, export_plt_file_name, "w");
	if (st_plt != NULL)
	{

		CString xtics_str;
		

		fprintf(st_plt, "set title \"Space time trajectory diagram\"\n");

		fprintf(st_plt, "set xlabel \"Time Horizon\"\n");
		fprintf(st_plt, "set ylabel \"Space (distance)\"  offset -1\n");

		int xtics_stepsize = 30;

		if (xrange / xtics_stepsize > 20)
			xtics_stepsize = 120;  // 2 hour interval
		else if (xrange / xtics_stepsize > 10)
			xtics_stepsize = 60;   // 1 hour interval
		else if (xrange / xtics_stepsize < 5)
			xtics_stepsize = 10;   // 1 hour interval


		for (int t = m_TimeLeft; t <= m_TimeRight; t += xtics_stepsize)
		{
			CString str;
			str.Format("\"%s\" %d ", m_pDoc->GetTimeStampString24HourFormat(t), t - m_TimeLeft);

			if (t + xtics_stepsize > m_TimeRight)
				xtics_str += str;
			else
			{
				xtics_str += str;
				xtics_str += ",";
			}
		}

		fprintf(st_plt, "set xtics (%s) \n", xtics_str);

		CString ytics_str;

		float y_distance = 0;

		CString last_node_number = " ";

		std::map<long, float> PathLinkNoStartMap;
		std::map<long, float> PathLinkNoEndMap;
		for (unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			for (int i = 0; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

				if (pLink != NULL)
				{
					PathLinkNoStartMap[pLink->m_LinkNo] = y_distance;

					y_distance += pLink->m_Length;

					PathLinkNoEndMap[pLink->m_LinkNo] = y_distance;


					CString label = pLink->m_Name.c_str();

					CString str;
					str.Format("\"%s\" %.3f, ", label, y_distance);
			

				}


			}


		} //for each path


		CString str;
		str.Format("\"%s\" %d", last_node_number, y_distance);
		ytics_str += str;
		fprintf(st_plt, "set ytics (%s)\n", ytics_str);

		fprintf(st_plt, "set xrange [0:%d] \n", xrange);
		fprintf(st_plt, "set yrange [0:%.2f] \n", y_distance);


		std::list<DTAAgent*>::iterator iAgent;

		int agent_count = 0;

		for (iAgent = m_pDoc->m_RouteAssignmentSet.begin(); iAgent != m_pDoc->m_RouteAssignmentSet.end(); iAgent++)
		{
			DTAAgent* pAgent = (*iAgent);

			bool bPassingPathFlag = false;
			for (int i = 0; i < pAgent->m_NodeSize-1; i++)
			{

				if (PathLinkNoStartMap.find(pAgent->m_NodeAry[i].LinkNo) != PathLinkNoStartMap.end())
				{
					bPassingPathFlag = true;
					break;
				}
			}


			//testing
			if (agent_count >= 4)
				break;

			if(bPassingPathFlag)
			{


				agent_count++;
				CString agent_str;
				agent_str.Format("agent%d.txt", agent_count);
				export_file_name = m_pDoc->m_ProjectDirectory + agent_str;

				FILE* st_agent;
				fopen_s(&st_agent, export_file_name, "w");
				if (st_agent != NULL)
				{
					fprintf(st_agent, "# agent_id = %d\n", pAgent->m_AgentID);
					for (int i = 0; i < pAgent->m_NodeSize - 1; i++)
					{

						if (PathLinkNoStartMap.find(pAgent->m_NodeAry[i].LinkNo) != PathLinkNoStartMap.end())
						{
							fprintf(st_agent, "%.2f     %f\n", pAgent->m_NodeAry[i].ArrivalTimeOnDSN, PathLinkNoStartMap[pAgent->m_NodeAry[i].LinkNo]);
							fprintf(st_agent, "%.2f     %f\n", pAgent->m_NodeAry[i+1].ArrivalTimeOnDSN, PathLinkNoEndMap[pAgent->m_NodeAry[i].LinkNo]);
						}
					}

					fclose(st_agent);
				}

			}



		}

		fprintf(st_plt, "plot ");

		for (int a = 1; a < agent_count; a++)
		{
			fprintf(st_plt, "\"agent%d.txt\" using 1:2 title 'agent %d'  with lines,\\\n", a, a);
		}
		fprintf(st_plt, "\"agent%d.txt\" using 1:2 title 'agent %d'  with lines\n", agent_count, agent_count);

		fclose(st_plt);
	}
	else
	{
		AfxMessageBox("File export_path_density.plt cannot be opened. Please check");
	}


	HINSTANCE result = ShellExecute(NULL, _T("open"), export_plt_file_name, NULL, NULL, SW_SHOW);

}
