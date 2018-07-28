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
	BOOL LoadImageToDocument();  // 载入图像
	BOOL GetGrayDibObject();  // 由图像m_pSubDibObject获取灰度图像m_pGrayDibObject
	void InteractRoadExtract(CPoint roadSeed1, CPoint roadSeed2);  // 道路交互提取
	void InteractLakeExtract(CPoint lakeSeed);  // 水体交互提取
	void InteractAllLakeExtract(CPoint lakeSeed);  // 水体交互提取
	void DrawPolygon(CArray<CPoint,CPoint> &polygonPoints);  // 画多边形
	void Earser(CPoint point);  // 擦除
	void EraseArea(CPoint point1, CPoint point2);  // 擦除选中的区域
	void DrawLines(CArray<CPoint,CPoint> &linePoints);  // 画线

protected: // create from serialization only
	CCeXianYouHuaDoc();
	DECLARE_DYNCREATE(CCeXianYouHuaDoc)
	ResetExtractDibObject();  // 获取与子图像m_pSubDibObject相匹配的空白图像m_pExtractDibObject
	BOOL GetSubDibObject(CRect &);  // 获取分割后的图像存放于m_pSubDibObject中

// Attributes
public:
	BOOL m_bImageLoaded;
	CDibObject *m_pDibObject;
	CDibObject *m_pSubDibObject;   // 用于存放分割后的图像
	CDibObject *m_pGrayDibObject;  // 用于存放灰度图像
	CDibObject *m_pExtractDibObject;  // 用于存放提取结果的图像
	BOOL m_onlyDrawExtractResult;  // 是否只显示提取结果
	int m_moveCX, m_moveCY;  // 移动距离
	CPoint m_startPoint;  // 图片截取的左上点
	CSize m_jieQuSize;  // 图片截取的大小
	float m_scale;  // 图片缩放比例

protected:
	WORD m_nChannelIndex;   // 生成灰度图像时所使用的通道	

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
