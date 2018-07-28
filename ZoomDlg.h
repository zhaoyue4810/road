#if !defined(AFX_ZOOMDLG_H__0A2FCA29_750B_405B_9515_D039159A6905__INCLUDED_)
#define AFX_ZOOMDLG_H__0A2FCA29_750B_405B_9515_D039159A6905__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZoomDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ZoomDlg dialog

class ZoomDlg : public CDialog
{
// Construction
public:
	ZoomDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ZoomDlg)
	enum { IDD = IDD_ZOOM };
	float	m_ratioX;
	float	m_ratioY;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ZoomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ZoomDlg)
	afx_msg void OnZoomAdapt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZOOMDLG_H__0A2FCA29_750B_405B_9515_D039159A6905__INCLUDED_)
