
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgLinkList.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"
#include "Dlg_VehicleClassification.h"

#include <string>
#include <sstream>


extern CDlgLinkList* g_pLinkListDlg;
// CDlgLinkList dialog


IMPLEMENT_DYNAMIC(CDlgLinkList, CDialog)

CDlgLinkList::CDlgLinkList(CWnd* pParent /*=NULL*/)
: CBaseDialog(CDlgLinkList::IDD, pParent)
, m_ZoomToSelectedLink(FALSE)
{

	m_pDoc = NULL;


}

CDlgLinkList::~CDlgLinkList()
{ 
	g_pLinkListDlg = NULL;
}

void CDlgLinkList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ListCtrl);
	DDX_Check(pDX, IDC_CHECK_ZOOM_TO_SELECTED_LINK, m_ZoomToSelectedLink);
}


BEGIN_MESSAGE_MAP(CDlgLinkList, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CDlgLinkList::OnLvnItemchangedList)
	ON_BN_CLICKED(IDOK, &CDlgLinkList::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgLinkList::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_ZOOM_TO_SELECTED_LINK, &CDlgLinkList::OnBnClickedCheckZoomToSelectedLink)
	ON_BN_CLICKED(IDBARCHARTPIECHART, &CDlgLinkList::OnBnClickedBarchartpiechart)
	ON_CBN_SELCHANGE(IDC_COMBO_Link_Selection, &CDlgLinkList::OnCbnSelchangeComboLinkSelection)
	ON_BN_CLICKED(ID_EXPORT, &CDlgLinkList::OnBnClickedExport)
	ON_STN_CLICKED(IDC_DOC_TITLE, &CDlgLinkList::OnStnClickedDocTitle)

END_MESSAGE_MAP()


// CDlgLinkList message handlers
#define LINKCOLUMNSIZE 11
#define LINKCOLUMNSIZE_AVI 7

BOOL CDlgLinkList::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);


	ReloadData();

	return true;
}
void CDlgLinkList::ReloadData()
{
	if(m_pDoc==NULL)
		return;

	CWaitCursor cursor;

	m_ListCtrl.DeleteAllItems();

	while(m_ListCtrl.GetColumnCount ()>0)
	{
		m_ListCtrl.DeleteColumn (0);
	}

	m_ListedLinkNoVector.clear();

	std::vector<std::string> m_Column_names;

	m_Column_names.push_back ("No.");
	m_Column_names.push_back ("Link Name");
	m_Column_names.push_back ("From Node");
	m_Column_names.push_back ("To Node");

	if(m_pDoc->m_bUseMileVsKMFlag )
		m_Column_names.push_back ("Length");

	m_Column_names.push_back ("Lanes");

	m_Column_names.push_back ("Free Speed");

	m_Column_names.push_back ("Lane Capacity");
	m_Column_names.push_back ("Link Type");

//	m_Column_names.push_back ("LOS");
	m_Column_names.push_back ("Total Volume");
	m_Column_names.push_back ("Hourly Volume");
	m_Column_names.push_back ("Hourly Lane Volume");
	m_Column_names.push_back("Speed");
	m_Column_names.push_back("VOC");

//	m_Column_names.push_back ("Waiting Time at Origin");


	//Add Columns and set headers
	for (size_t i=0;i<m_Column_names.size();i++)
	{

		CGridColumnTraitText* pTrait = NULL;
		//		pTrait = new CGridColumnTraitEdit();
		m_ListCtrl.InsertColumnTrait((int)i,m_Column_names.at(i).c_str(),LVCFMT_LEFT,-1,-1, pTrait);
		m_ListCtrl.SetColumnWidth((int)i,LVSCW_AUTOSIZE_USEHEADER);
	}
	m_ListCtrl.SetColumnWidth(0, 80);


	std::list<DTALink*>::iterator iLink;
	int i = 0;
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++, i++)
	{

		DTALink* pLink1 = (*iLink);
		int type  = pLink1 ->m_link_type ;
		if(m_LinkSelectionMode == eLinkSelection_SelectedLinksOnly) 
		{
			if(pLink1->m_DisplayLinkID <0)  // not selected
				continue; 
		}


		if(m_LinkSelectionMode == eLinkSelection_WithinSubarea) 
		{
			if(pLink1->m_bIncludedinSubarea == false)  // not within subarea
				continue; 
		}


		if(m_LinkSelectionMode == eLinkSelection_FreewayOnly) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsFreeway () == false) 
				continue; 
		}



		if(m_LinkSelectionMode == eLinkSelection_HighwayOnly) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsHighway () == false) 
				continue; 
		}
		if(m_LinkSelectionMode == eLinkSelection_RampOnly) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsRamp () == false) 
				continue; 
		}

		if(m_LinkSelectionMode == eLinkSelection_ArterialOnly) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsArterial () == false) 
				continue; 
		}

		if(m_LinkSelectionMode == eLinkSelection_NoConnectors) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsConnector()) 
				continue; 
		}

		char text[100];
		sprintf_s(text, "%d",pLink1->m_LinkNo );
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);
		int column_index = 1;
		m_ListCtrl.SetItemText(Index,column_index++,pLink1->m_Name .c_str ());

		sprintf_s(text, "%d",pLink1->m_FromNodeID);
		m_ListCtrl.SetItemText(Index,column_index++,text);

		sprintf_s(text, "%d",pLink1->m_ToNodeID);
		m_ListCtrl.SetItemText(Index,column_index++,text);

		sprintf_s(text, "%5.2f",pLink1->m_Length);

		m_ListCtrl.SetItemText(Index,column_index++,text);

		sprintf_s(text, "%d",pLink1->m_NumberOfLanes );
		m_ListCtrl.SetItemText(Index,column_index++,text);

			sprintf_s(text, "%4.0f",pLink1->m_FreeSpeed  );

		m_ListCtrl.SetItemText(Index,column_index++,text);

		sprintf_s(text, "%4.0f",pLink1->m_LaneCapacity  );
		m_ListCtrl.SetItemText(Index,column_index++,text);

		if(m_pDoc->m_LinkTypeMap.find(pLink1->m_link_type) != m_pDoc->m_LinkTypeMap.end())
		{
			sprintf_s(text, "%s", m_pDoc->m_LinkTypeMap[pLink1->m_link_type].link_type_name.c_str ());
			m_ListCtrl.SetItemText(Index,column_index,text);
		}

		column_index++;


		sprintf_s(text, "%.0f",pLink1->m_static_link_volume);
		m_ListCtrl.SetItemText(Index,column_index++,text);

		sprintf_s(text, "%.0f", pLink1->m_hourly_link_volume);
		m_ListCtrl.SetItemText(Index,column_index++,text);

//		float max_density , avg_density;

//		avg_density = pLink1->GetTDDensity(MOE_start_time_in_min,MOE_end_time_in_min,max_density);

		//sprintf_s(text, "%.1f",avg_density );
		//m_ListCtrl.SetItemText(Index,column_index++,text);

		//sprintf_s(text, "%.1f",max_density );
		//m_ListCtrl.SetItemText(Index,column_index++,text);

		sprintf_s(text, "%.2f", (*iLink)->m_Length / max(0.0001, (*iLink)->m_MeanSpeed) * 60);
		m_ListCtrl.SetItemText(Index, column_index++, text);

		sprintf_s(text, "%.2f", (*iLink)->m_MeanSpeed);
		m_ListCtrl.SetItemText(Index, column_index++, text);

		sprintf_s(text, "%.2f", pLink1->m_StaticVoCRatio);
		m_ListCtrl.SetItemText(Index,column_index++,text);


		m_ListedLinkNoVector.push_back(pLink1->m_LinkNo );
	}


}

void CDlgLinkList::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{

	UpdateData(1);

	// test valid documents
	if(g_TestValidDocument(m_pDoc)==false)
		return;

	// 
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
		m_ListCtrl.GetItemText (nSelectedRow,0,str,20);
		int LinkNo = atoi(str);
		m_pDoc->m_SelectedLinkNo = LinkNo;

		DTALink* pLink = m_pDoc->m_LinkNoMap [LinkNo];

		g_AddLinkIntoSelectionList(pLink,LinkNo,m_pDoc->m_DocumentNo );

	}

	if(m_ZoomToSelectedLink == TRUE)
	{

		m_pDoc->ZoomToSelectedLink(m_pDoc->m_SelectedLinkNo);

	}

	Invalidate();

	m_pDoc->UpdateAllViews(0);

}

void CDlgLinkList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}

void CDlgLinkList::OnBnClickedCancel()
{
	CDialog::OnOK();
	g_bShowLinkList = false;

}



void CDlgLinkList::OnBnClickedCheckZoomToSelectedLink()
{
	// TODO: Add your control notification handler code here
}

void CDlgLinkList::OnBnClickedBarchartpiechart()
{

	if(m_pDoc!=NULL)
	{
		CDlg_AgentClassification dlg;
		dlg.m_pDoc = m_pDoc;
		m_pDoc->m_AgentSelectionMode = CLS_link_set;

		dlg.m_AgentSelectionNo  = CLS_link_set;
		dlg.DoModal ();
	}

}

void CDlgLinkList::OnLinkselectionShowselectedlinksonly()
{
	m_LinkSelectionMode = eLinkSelection_SelectedLinksOnly;
	ReloadData();

}

void CDlgLinkList::OnUpdateLinkselectionShowselectedlinksonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkSelectionMode == eLinkSelection_SelectedLinksOnly);
}

void CDlgLinkList::OnCbnSelchangeComboLinkSelection()
{

}

void CDlgLinkList::OnBnClickedExport()
{

}




void CDlgLinkList::OnStnClickedDocTitle()
{
	// TODO: Add your control notification handler code here
}

