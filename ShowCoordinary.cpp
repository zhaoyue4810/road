// ShowCoordinary.cpp : implementation file
//

#include "stdafx.h"
#include "CeXianYouHua.h"
#include "ShowCoordinary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShowCoordinary dialog


CShowCoordinary::CShowCoordinary(CWnd* pParent /*=NULL*/)
	: CDialog(CShowCoordinary::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShowCoordinary)
	m_Height_Show = 0;
	m_Width_Show = 0;
	//}}AFX_DATA_INIT
}


void CShowCoordinary::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShowCoordinary)
	DDX_Text(pDX, IDC_HEIGHT_IN, m_Height_Show);
	DDX_Text(pDX, IDC_WIDTH_IN, m_Width_Show);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShowCoordinary, CDialog)
	//{{AFX_MSG_MAP(CShowCoordinary)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShowCoordinary message handlers
