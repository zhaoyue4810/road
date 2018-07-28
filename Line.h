#ifndef __Line_H__
#define __Line_H__

#include <afxtempl.h>

class CLine
{
public:
	CLine();

public:
	static int groupID;
	int m_groupID;  // 线段ID
	CArray<CPoint,CPoint> m_pointArray;  // 存放线段点
	double m_lineLength;  // 线段长度
	int m_parallels;  // 平行线数量
	int m_edgeLines;  // 边界端点数量
	double m_aveHuiDu;  // 线段平均灰度
	double m_aveGradMag;  // 线段平均梯度
};

#endif