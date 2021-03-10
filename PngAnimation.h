#pragma once


#pragma warning(push)
#pragma warning(disable : 4458)   // declaration of 'nativeCap' hides class member.
#include <gdiplus.h>
#pragma warning(pop)
#pragma comment (lib, "Gdiplus.lib")

#include <mmsystem.h>
#pragma comment (lib, "Winmm.lib")   // for Multimedia Timers.

#include <cassert>


struct PngAnimation
{
///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
	PngAnimation()
	{	m_bmp = NULL;
	}
	~PngAnimation()
	{	Destroy();
	}
		// 
	bool Create(HMODULE moduleRes/*or null*/, UINT idPngRes, int numberFrame)
	{	assert(numberFrame>1 && time>0);
			// 
		Destroy();
		if( !LoadImage(moduleRes,idPngRes,&m_bmp/*out*/) )
			return false;
			// 
		m_iNumberFrame = numberFrame;
		m_szFrame.SetSize(m_bmp->GetWidth()/numberFrame,m_bmp->GetHeight());
			// 
		return true;
	}
	void Destroy()
	{	m_Timer.KillTimer();
		if(m_bmp)
		{	::delete m_bmp;
			m_bmp = NULL;
		}		
	}
	bool IsCreated() const
	{	return m_bmp!=NULL;
	}
		// 
	bool Start(CWnd *parent, int x, int y, COLORREF clrBack, int time/*ms*/, bool infinitely)
	{	assert( IsCreated() );
		assert(parent && ::IsWindow(parent->m_hWnd));
			// 
		m_Timer.KillTimer();
		m_parentWnd = parent;
		m_ptFrame.SetPoint(x,y);
		m_clrBack = clrBack;
		m_iTime = time;
		m_bInfinitely = infinitely;
		m_iCurFrame = 0;
		if( !m_Timer.SetTimer(this,&PngAnimation::OnTimer,m_iTime) )
			return false;
		OnTimer();
		return true;
	}
	void Stop()
	{	m_Timer.KillTimer();
	}
	bool IsActive() const
	{	return m_Timer.IsActive();
	}

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
	void OnTimer()   // callback from Timer object.
	{	CClientDC dc(m_parentWnd);
		VirtualWindow virtwnd(&dc, CRect(m_ptFrame,m_szFrame) );
			// 
			// Background filling.
		CBrush brush(m_clrBack);
		CRect rcFill(0,0,m_szFrame.cx,m_szFrame.cy);
		if(virtwnd.m_hDC==dc.m_hDC)   // error creating VirtualWindow.
			rcFill.OffsetRect(m_ptFrame);
		virtwnd.FillRect(&rcFill,&brush);
			// Frame rendering.
		Gdiplus::Graphics gr(virtwnd.m_hDC);
		gr.DrawImage(m_bmp,rcFill.left,rcFill.top,m_szFrame.cx*m_iCurFrame,0,m_szFrame.cx,m_szFrame.cy,Gdiplus::UnitPixel);
			// 
		if(++m_iCurFrame == m_iNumberFrame)
		{	m_iCurFrame = 0;
			if(!m_bInfinitely)
				m_Timer.KillTimer();
		}
	}

private:
	bool LoadImage(HMODULE moduleRes/*or null*/, UINT resID, Gdiplus::Bitmap **bmp/*out*/) const
	{	assert(resID);
			// 
		if(!moduleRes)
			moduleRes = AfxFindResourceHandle(MAKEINTRESOURCE(resID),_T("PNG"));
		if(moduleRes)
		{	HRSRC hRsrc = ::FindResource(moduleRes,MAKEINTRESOURCE(resID),_T("PNG"));
			if(hRsrc)
			{	HGLOBAL hGlobal = ::LoadResource(moduleRes,hRsrc);
				if(hGlobal)
				{	const void *lpBuffer = ::LockResource(hGlobal);
					if(lpBuffer)
					{	const UINT uiSize = static_cast<UINT>( ::SizeofResource(moduleRes,hRsrc) );
						HGLOBAL hRes = ::GlobalAlloc(GMEM_MOVEABLE, uiSize);
						if(hRes)
						{	void *lpResBuffer = ::GlobalLock(hRes);
							if(lpResBuffer)
							{	memcpy(lpResBuffer, lpBuffer, uiSize);
								IStream *pStream = NULL;
								if(::CreateStreamOnHGlobal(hRes, FALSE, &pStream/*out*/)==S_OK)
								{	*bmp = ::new (std::nothrow) Gdiplus::Bitmap(pStream,FALSE);
									pStream->Release();
								}
							}
						}
						::UnlockResource(hGlobal);
					}
					::FreeResource(hGlobal);
				}
			}
		}
		if(*bmp && (*bmp)->GetLastStatus()!=Gdiplus::Ok)
		{	::delete *bmp;
			return false;
		}
		return (*bmp)!=NULL;
	}

private:
	struct VirtualWindow : CDC
	{	VirtualWindow(CDC *dcDst, CRect const &rcDst)
		{	assert(dcDst && dcDst->m_hDC);
			assert(rcDst);
				// 
			if(CreateCompatibleDC(dcDst) && m_bmpSrc.CreateCompatibleBitmap(dcDst,rcDst.Width(),rcDst.Height()))
			{	SelectObject(&m_bmpSrc);
				m_pDcDst = dcDst;
				m_rcDst = rcDst;
			}
			else
			{	if(m_hDC)
					DeleteDC();
				Attach(dcDst->m_hDC);
			}
			SetBkMode(TRANSPARENT);
		}
		~VirtualWindow()
		{	if(m_bmpSrc.m_hObject)
				m_pDcDst->BitBlt(m_rcDst.left,m_rcDst.top,m_rcDst.Width(),m_rcDst.Height(),this,0,0,SRCCOPY);
			else
				Detach();
		}

	private:
		CDC *m_pDcDst;
		CRect m_rcDst;
		CBitmap m_bmpSrc;
	};

private:
	struct Timer : private CWnd
	{	Timer()
		{	m_uTimerID = 0;
		}
		~Timer()
		{	KillTimer();
		}
			// 
		bool SetTimer(PngAnimation *obj, void(PngAnimation::*func)(), int time/*ms*/)
		{	m_pNotifyObj = obj;
			m_pNotifyFunc = func;
				// 
			if( !CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL,WS_POPUP,0,0,0,0,NULL,0) ||
				(m_uTimerID = ::timeSetEvent(time,0,TimeProc,reinterpret_cast<DWORD_PTR>(m_hWnd),TIME_PERIODIC))==0)
			{
				KillTimer();
				return false;
			}
			return true;
		}
		void KillTimer()
		{	if(m_uTimerID)
			{	::timeKillEvent(m_uTimerID);
				m_uTimerID = 0;
			}
			if(m_hWnd)
				DestroyWindow();
		}
			// 
		bool IsActive() const
		{	return m_hWnd!=NULL;
		}

	protected:
		PngAnimation *m_pNotifyObj;
		void(PngAnimation::*m_pNotifyFunc)();
		UINT m_uTimerID;

	protected:
		static void CALLBACK TimeProc(UINT /*uID*/, UINT /*uMsg*/, DWORD_PTR dwUser, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
		{	::SendMessage(reinterpret_cast<HWND>(dwUser),WM_USER,0,0);
		}
			// 
		virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
		{	const LRESULT res = CWnd::WindowProc(message,wParam,lParam);
			if(message==WM_USER)
				(m_pNotifyObj->*m_pNotifyFunc)();
			return res;
		}
	};

private:
	Gdiplus::Bitmap *m_bmp;
	Timer m_Timer;
	CWnd *m_parentWnd;
	int m_iCurFrame,m_iNumberFrame, m_iTime;
	CPoint m_ptFrame;
	CSize m_szFrame;
	COLORREF m_clrBack;
	bool m_bInfinitely;
};



