// GeoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CeXianYouHua.h"
#include "GeoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoDialog dialog


CGeoDialog::CGeoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGeoDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGeoDialog)
	m_Lat = 0.0f;
	m_Log = 0.0f;
	//}}AFX_DATA_INIT
}


void CGeoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeoDialog)
	DDX_Text(pDX, IDC_LATITUDE, m_Lat);
	DDX_Text(pDX, IDC_LONGITUDE, m_Log);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGeoDialog, CDialog)
	//{{AFX_MSG_MAP(CGeoDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoDialog message handlers
