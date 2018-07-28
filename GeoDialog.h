#if !defined(AFX_GEODIALOG_H__E872B355_5AC8_40A9_A93F_14FFF7448604__INCLUDED_)
#define AFX_GEODIALOG_H__E872B355_5AC8_40A9_A93F_14FFF7448604__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GeoDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGeoDialog dialog

class CGeoDialog : public CDialog
{
// Construction
public:
	CGeoDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGeoDialog)
	enum { IDD = IDD_JWDIALOG };
	float	m_Lat;
	float	m_Log;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGeoDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEODIALOG_H__E872B355_5AC8_40A9_A93F_14FFF7448604__INCLUDED_)
