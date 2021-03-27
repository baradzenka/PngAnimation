// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(100, OnUpdateBut7Anim)
	ON_BN_CLICKED(100, OnBnClickedBut7Anim)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// 
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}



ULONG_PTR token;

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{	if(CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	if( !m_But7Anim.Create(_T("Start"),WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,CRect(145,202,211,225),this,100) )
		return -1;
	CFont *font = CFont::FromHandle( static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)) );
	m_But7Anim.SetFont(font);


	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if( GdiplusStartup(&token,&gdiplusStartupInput,NULL)!=Gdiplus::Ok )
		return -1;


	anim1.Create(NULL,IDB_PNG1,31);
	anim1.Start(m_hWnd, 50,50, ::GetSysColor(COLOR_WINDOW), 40,true);

	anim2.Create(NULL,IDB_PNG2,35);
	anim2.Start(m_hWnd, 150,50, ::GetSysColor(COLOR_WINDOW), 40,true);

	anim3.Create(NULL,IDB_PNG3,50);
	anim3.Start(m_hWnd, 250,50, ::GetSysColor(COLOR_WINDOW), 40,true);

	anim4.Create(NULL,IDB_PNG4,75);
	anim4.Start(m_hWnd, 350,50, ::GetSysColor(COLOR_WINDOW), 40,true);

	anim5.Create(NULL,IDB_PNG5,15);
	anim5.Start(m_hWnd, 450,50, ::GetSysColor(COLOR_WINDOW), 40,true);

	anim6.Create(NULL,IDB_PNG6,12);
	anim6.Start(m_hWnd, 45,150, ::GetSysColor(COLOR_WINDOW), 120,true);

	anim7.Create(NULL,IDB_PNG7,20);
	OnBnClickedBut7Anim();

	anim8.Create(NULL,IDB_PNG8,65);
	anim8.Start(m_hWnd, 260,130, ::GetSysColor(COLOR_WINDOW), 40,true);

	anim9.Create(NULL,IDB_PNG9,22);
	anim9.Start(m_hWnd, 360,135, ::GetSysColor(COLOR_WINDOW), 80,true);


	SetTimer(1,100,NULL);

	return 0;
}


void CChildView::OnTimer(UINT_PTR nIDEvent)
{	UpdateDialogControls(this,FALSE);
	CWnd::OnTimer(nIDEvent);
}


void CChildView::OnUpdateBut7Anim(CCmdUI *pCmdUI)
{	pCmdUI->Enable( !anim7.IsActive() );
}

void CChildView::OnBnClickedBut7Anim()
{	anim7.Start(m_hWnd, 145,130, ::GetSysColor(COLOR_WINDOW), 64,false);
}


void CChildView::OnSize(UINT nType,int cx,int cy)
{	CWnd::OnSize(nType,cx,cy);

	// To prevent flickering when the window is resized.
	RECT rc = anim1.GetArea();
	ValidateRect(&rc);
	rc = anim2.GetArea();
	ValidateRect(&rc);
	rc = anim3.GetArea();
	ValidateRect(&rc);
	rc = anim4.GetArea();
	ValidateRect(&rc);
	rc = anim5.GetArea();
	ValidateRect(&rc);
	rc = anim6.GetArea();
	ValidateRect(&rc);
	if( anim7.IsActive() )
	{	rc = anim7.GetArea();
		ValidateRect(&rc);
	}
	rc = anim8.GetArea();
	ValidateRect(&rc);
	rc = anim9.GetArea();
	ValidateRect(&rc);
}


