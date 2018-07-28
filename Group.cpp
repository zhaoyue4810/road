#include "StdAfx.h"
#include "Group.h"

double CGroup::weightLen = 0.005;
double CGroup::weightPara = 0.3;
double CGroup::weightEdge = 0.3;
double CGroup::lowBeliefDegree = 1.5;

// �ڹ��캯���г�ʼ����Ա����
CGroup::CGroup()
{
	m_groupID = -1;
	m_lineLength = 0;
	m_parallels = 0;
	m_edgeLines = 0;
	m_beliefDegree = 0;
	m_believed = FALSE;
	m_seedPoint1 = m_seedPoint2 = CPoint(-1, -1);
}

// �������Ŷ�
void CGroup::CalBeliefDegree()
{
	// �������Ŷ�
	m_beliefDegree = m_lineLength * weightLen + 
		m_parallels * weightPara + m_edgeLines * weightEdge;
	// ���Ŷȴ���������Ŷȷ�ֵ����������
	if(m_beliefDegree > lowBeliefDegree)
	{
		m_believed = TRUE;
	}
}