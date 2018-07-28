// CeXianYouHuaDoc.h : interface of the CCeXianYouHuaDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CEXIANYOUHUADOC_H__7C1CD884_798F_4AB5_AE43_65A32BBB8678__INCLUDED_)
#define AFX_CEXIANYOUHUADOC_H__7C1CD884_798F_4AB5_AE43_65A32BBB8678__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DibObject.h"

class CCeXianYouHuaDoc : public CDocument
{
public:
	BOOL LoadImageToDocument();  // ����ͼ��
	BOOL GetGrayDibObject();  // ��ͼ��m_pSubDibObject��ȡ�Ҷ�ͼ��m_pGrayDibObject
	void InteractRoadExtract(CPoint roadSeed1, CPoint roadSeed2);  // ��·������ȡ
	void InteractLakeExtract(CPoint lakeSeed);  // ˮ�彻����ȡ
	void InteractAllLakeExtract(CPoint lakeSeed);  // ˮ�彻����ȡ
	void DrawPolygon(CArray<CPoint,CPoint> &polygonPoints);  // �������
	void Earser(CPoint point);  // ����
	void EraseArea(CPoint point1, CPoint point2);  // ����ѡ�е�����
	void DrawLines(CArray<CPoint,CPoint> &linePoints);  // ����

protected: // create from serialization only
	CCeXianYouHuaDoc();
	DECLARE_DYNCREATE(CCeXianYouHuaDoc)
	ResetExtractDibObject();  // ��ȡ����ͼ��m_pSubDibObject��ƥ��Ŀհ�ͼ��m_pExtractDibObject
	BOOL GetSubDibObject(CRect &);  // ��ȡ�ָ���ͼ������m_pSubDibObject��

// Attributes
public:
	BOOL m_bImageLoaded;
	CDibObject *m_pDibObject;
	CDibObject *m_pSubDibObject;   // ���ڴ�ŷָ���ͼ��
	CDibObject *m_pGrayDibObject;  // ���ڴ�ŻҶ�ͼ��
	CDibObject *m_pExtractDibObject;  // ���ڴ����ȡ�����ͼ��
	BOOL m_onlyDrawExtractResult;  // �Ƿ�ֻ��ʾ��ȡ���
	int m_moveCX, m_moveCY;  // �ƶ�����
	CPoint m_startPoint;  // ͼƬ��ȡ�����ϵ�
	CSize m_jieQuSize;  // ͼƬ��ȡ�Ĵ�С
	float m_scale;  // ͼƬ���ű���

protected:
	WORD m_nChannelIndex;   // ���ɻҶ�ͼ��ʱ��ʹ�õ�ͨ��	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeXianYouHuaDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCeXianYouHuaDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCeXianYouHuaDoc)
	afx_msg void OnAutoRoadExtract();
	afx_msg void OnAutoLakeExtract();
	afx_msg void OnScaleSmall();
	afx_msg void OnScaleBig();
	afx_msg void OnCanny();
	afx_msg void OnGuassFilter();
	afx_msg void OnRobert();
	afx_msg void OnPrewitt();
	afx_msg void OnSobel();
	afx_msg void OnLaplacian();
	afx_msg void OnZoom();
	//}}AFX_MSG
	afx_msg void OnChannelChange(WORD nID);
	afx_msg void OnUpdateChannelChange(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEXIANYOUHUADOC_H__7C1CD884_798F_4AB5_AE43_65A32BBB8678__INCLUDED_)
