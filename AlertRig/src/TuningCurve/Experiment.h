// Experiment.h: interface for the Experiment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXPERIMENT_H__FA6FB5CE_97DE_4702_8D81_FC33FE4B6840__INCLUDED_)
#define AFX_EXPERIMENT_H__FA6FB5CE_97DE_4702_8D81_FC33FE4B6840__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Experiment  
{
public:
	int DistanceToScreen;
	Experiment();
	virtual ~Experiment();

private:
	CString ParameterFileName;
};

#endif // !defined(AFX_EXPERIMENT_H__FA6FB5CE_97DE_4702_8D81_FC33FE4B6840__INCLUDED_)
