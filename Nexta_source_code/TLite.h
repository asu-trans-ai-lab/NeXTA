// TLite.h : main header file for the TLite application
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
#pragma once

#define _LANGUAGE_CN_

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif
#pragma once
#include "resource.h"       // main symbols
#include "resource.h"       // main symbols

#include <list>


enum eLanguageSupport
{
     LANG_EN_EN=0,
     LANG_CN_SIMPLIFIED
} ;



extern void g_ClearLinkSelectionList();
extern void g_ClearTrajectorySelectionList();
enum _cursor_type {_cursor_standard_arrow=0,_cursor_movement_network,_cursor_create_link,_cursor_create_node,_cursor_create_subarea,_cursor_create_zone,_cursor_wait};



extern double g_GetPrivateProfileDouble( LPCTSTR section, LPCTSTR key, double def_value, LPCTSTR filename) ;
extern int g_GetProfileString( LPCTSTR section, LPCTSTR key, LPCTSTR lpdefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR filename);
extern HINSTANCE g_OpenDocument(LPCTSTR url, int showcmd);
// CTLiteApp:
// See TLite.cpp for the implementation of this class
//

class CTLiteApp : public CWinApp
{
public:

	CString m_Simulator;

eLanguageSupport m_LanguageSupport;
	int m_NEXTA_use_flag;

	COLORREF m_Link1Color;
	COLORREF m_Link2Color;
	COLORREF m_Link3Color;
	COLORREF m_Link4Color;
	COLORREF m_Link5Color;
	COLORREF m_Link6Color, m_ParkingandRideColor;
	COLORREF m_PickupColor, m_DropoffColor;

	COLORREF m_AgentColor[10];

	COLORREF m_BackgroundColor;

	COLORREF m_NodeColor;
	COLORREF m_NodeBrushColor;
	
	COLORREF m_ZoneColor;

	CString m_SimulatorString_32;
	CString m_SimulatorString_64;

	CString m_Node_Additional_Field[6];
	CString m_Link_Additional_Field[6];

	CTLiteApp();

	void UpdateAllViews();

	CMultiDocTemplate* m_pDocTemplate2DView;
	CMultiDocTemplate* m_pTemplateGLView;
	CMultiDocTemplate* m_pTemplateTimeTableView;
// Overrides
public:

	bool m_bLoadNetworkOnly;
	virtual BOOL InitInstance();


// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenNewDoc();
	virtual int ExitInstance();
	afx_msg void OnFileOpenmultipletrafficdataprojects();
	afx_msg void OnAppExit();
	afx_msg void OnFileOpenNetworkOnly();
	afx_msg void OnHelpUserguide();
	afx_msg void OnHelpChecklatestsoftwarerelease();
};

#define MAX_MOE_DISPLAYCOLOR 6

extern long g_Simulation_Time_Horizon;
extern int g_Data_Time_Interval;
extern int g_Number_of_Weekdays;
extern bool g_bShowLinkList;
extern bool g_bShowAgentPathDialog;


extern CTLiteApp theApp;