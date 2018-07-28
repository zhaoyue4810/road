// Show_Contrast.cpp : implementation file
//

#include "stdafx.h"
#include "CeXianYouHua.h"
#include "Show_Contrast.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShow_Contrast dialog


CShow_Contrast::CShow_Contrast(CWnd* pParent /*=NULL*/)
	: CDialog(CShow_Contrast::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShow_Contrast)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CShow_Contrast::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShow_Contrast)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShow_Contrast, CDialog)
	//{{AFX_MSG_MAP(CShow_Contrast)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShow_Contrast message handlers

int CShow_Contrast::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
//	SetClassLong(GetSafeHwnd(),GCL_HBRBACKGROUND,(LONG)GetStockObject(WHITE_BRUSH));
//	SetClassLong(m_hWnd,GCL_HCURSOR,(LONG)LoadCursor(NULL,IDC_CROSS));
	return 0;
}

BOOL CShow_Contrast::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_bkBrush.CreateSolidBrush(RGB(255,255,255));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CShow_Contrast::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	 if(nCtlColor==CTLCOLOR_DLG) 
		  return   m_bkBrush;   
	   
  
	// TODO: Return a different brush if the default is not desired
	return hbr;
}
