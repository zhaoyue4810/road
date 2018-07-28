// ZoomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CeXianYouHua.h"
#include "ZoomDlg.h"
#include "CeXianYouHuaDoc.h"
#include "MainFrm.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ZoomDlg dialog


ZoomDlg::ZoomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ZoomDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ZoomDlg)
	m_ratioX = 0.0f;
	m_ratioY = 0.0f;
	//}}AFX_DATA_INIT
}


void ZoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ZoomDlg)
	DDX_Text(pDX, IDC_EDIT_X, m_ratioX);
	DDX_Text(pDX, IDC_EDIT_Y, m_ratioY);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ZoomDlg, CDialog)
	//{{AFX_MSG_MAP(ZoomDlg)
	ON_BN_CLICKED(IDC_Zoom_Adapt, OnZoomAdapt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ZoomDlg message handlers

void ZoomDlg::OnZoomAdapt() 
{
	CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
	//获取子框架窗口指针
	CChildFrame *pChild = (CChildFrame *) pFrame->MDIGetActive();
	float re_ratio;
	float ratio;
	int WidthIn=pChild->m_nWidth;
	int HeightIn=pChild->m_nHeight;
	if(WidthIn>=1000 && HeightIn>=700)
	{
		re_ratio=(WidthIn/1000.0) > (HeightIn/700.0) ? (WidthIn/1000.0) : (HeightIn/700.0);
		ratio=1.0/re_ratio;
		m_ratioX=ratio;
		m_ratioY=ratio;
		UpdateData(FALSE);
	}
	else
	{
		m_ratioX=1.0;
		m_ratioY=1.0;
		UpdateData(FALSE);
	}
}
