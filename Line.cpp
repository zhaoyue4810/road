#include "StdAfx.h"
#include "Line.h"

int CLine::groupID = 0;

CLine::CLine()
{
	m_groupID = groupID++;
	m_lineLength = 0;
	m_parallels =0;
	m_edgeLines = 0;
	m_aveHuiDu = 0;
	m_aveGradMag = 0;
}