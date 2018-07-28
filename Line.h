#ifndef __Line_H__
#define __Line_H__

#include <afxtempl.h>

class CLine
{
public:
	CLine();

public:
	static int groupID;
	int m_groupID;  // �߶�ID
	CArray<CPoint,CPoint> m_pointArray;  // ����߶ε�
	double m_lineLength;  // �߶γ���
	int m_parallels;  // ƽ��������
	int m_edgeLines;  // �߽�˵�����
	double m_aveHuiDu;  // �߶�ƽ���Ҷ�
	double m_aveGradMag;  // �߶�ƽ���ݶ�
};

#endif