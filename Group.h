#ifndef __Group_H__
#define __Group_H__

class CGroup
{
public:
	CGroup();
	void CalBeliefDegree();  // �������Ŷ�

public:
	static double weightLen;  // ����Ȩֵ
	static double weightPara;  // ƽ����Ȩֵ
	static double weightEdge;  // �߽�˵�Ȩֵ
	static double lowBeliefDegree;  // ������Ŷȷ�ֵ
	int m_groupID;  // ������
	double m_lineLength;  // �������߳�
	int m_parallels;  // ���������ƽ��������
	int m_edgeLines;  // ��������ı߽�˵�����
	double m_beliefDegree;  // �������Ŷ�
	BOOL m_believed;  // �Ƿ�����
	CPoint m_seedPoint1, m_seedPoint2;  // ���ӵ�
};

#endif