#if !defined(AFX_SHOW_CONTRAST_H__275FBF7E_428E_4468_ACA0_9CDAD043340C__INCLUDED_)
#define AFX_SHOW_CONTRAST_H__275FBF7E_428E_4468_ACA0_9CDAD043340C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Show_Contrast.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShow_Contrast dialog

class CShow_Contrast : public CDialog
{
// Construction
public:
	CShow_Contrast(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShow_Contrast)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShow_Contrast)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShow_Contrast)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	CBrush   m_bkBrush;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOW_CONTRAST_H__275FBF7E_428E_4468_ACA0_9CDAD043340C__INCLUDED_)
