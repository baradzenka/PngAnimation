#pragma once


#include "PngAnimation.h"


// 
class CChildView : public CWnd
{	PngAnimation anim1, anim2, anim3, anim4, anim5, 
		anim6, anim7, anim8, anim9;
	CButton m_But7Anim;

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateBut7Anim(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedBut7Anim();
};

