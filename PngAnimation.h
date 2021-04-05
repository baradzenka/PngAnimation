#pragma once


#pragma warning(push)
#pragma warning(disable : 4458)   // declaration of 'nativeCap' hides class member.
#include <gdiplus.h>
#pragma warning(pop)
#pragma comment (lib, "Gdiplus.lib")

#include <mmsystem.h>
#pragma comment (lib, "Winmm.lib")   // for Multimedia Timers.

#include <new>
#include <cassert>


struct PngAnimation
{
///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
	PngAnimation()
	{	m_pBmp = NULL;
	}
	~PngAnimation()
	{	Destroy();
	}
		// 
	bool Create(HMODULE moduleRes/*or null*/, unsigned int idPngRes, int numberFrame)
	{	assert(numberFrame>1);
			// 
		Destroy();
		if( !LoadImage(moduleRes,idPngRes,&m_pBmp/*out*/) )
			return false;
			// 
		m_iNumberFrame = numberFrame;
		m_szFrame.cx = m_pBmp->GetWidth() / numberFrame;
		m_szFrame.cy = m_pBmp->GetHeight();
			// 
		return true;
	}
	void Destroy()
	{	m_Timer.KillTimer();
		if(m_pBmp)
		{	::delete m_pBmp;
			m_pBmp = NULL;
		}		
	}
	bool IsCreated() const
	{	return m_pBmp!=NULL;
	}
		// 
	bool Start(HWND parent, int x, int y, COLORREF clrBack, int time/*ms*/, bool infinitely)
	{	assert( IsCreated() );
		assert( ::IsWindow(parent) );
		assert(time>0);
			// 
		if( !IsCreated() )
			return false;
			// 
		m_Timer.KillTimer();
		m_hParentWnd = parent;
		m_ptFrame.x = x;
		m_ptFrame.y = y;
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
		// 
	RECT GetArea() const
	{	const RECT rect = {m_ptFrame.x,m_ptFrame.y,m_ptFrame.x+m_szFrame.cx,m_ptFrame.y+m_szFrame.cy};
		return rect;
	}

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
	void OnTimer()   // callback from Timer object.
	{	HDC hDC = ::GetDC(m_hParentWnd);
		if(hDC)
		{	VirtualWindow virtwnd(hDC,m_ptFrame,m_szFrame);
				// 
				// Background filling.
			RECT rcFill = {0,0,m_szFrame.cx,m_szFrame.cy};
			if(virtwnd==hDC)   // VirtualWindow wasn't created.
				::OffsetRect(&rcFill,m_ptFrame.x,m_ptFrame.y);
			HBRUSH brush = ::CreateSolidBrush(m_clrBack);
			::FillRect(virtwnd,&rcFill,brush);
			::DeleteObject(brush);
				// Frame rendering.
			Gdiplus::Graphics gr(virtwnd);
			gr.DrawImage(m_pBmp,rcFill.left,rcFill.top,m_szFrame.cx*m_iCurFrame,0,m_szFrame.cx,m_szFrame.cy,Gdiplus::UnitPixel);
				// 
			if(++m_iCurFrame == m_iNumberFrame)
			{	m_iCurFrame = 0;
				if(!m_bInfinitely)
					m_Timer.KillTimer();
			}
			virtwnd.Release();   // show frame.
			::ReleaseDC(m_hParentWnd,hDC);
		}
	}

private:
	bool LoadImage(HMODULE moduleRes/*or null*/, unsigned int resID, Gdiplus::Bitmap **bmp/*out*/) const
	{	assert(resID);
			// 
		if(!moduleRes)
			moduleRes = ::GetModuleHandle(NULL);
		if(moduleRes)
		{	HRSRC hRsrc = ::FindResource(moduleRes,MAKEINTRESOURCE(resID),_T("PNG"));
			if(hRsrc)
			{	HGLOBAL hGlobal = ::LoadResource(moduleRes,hRsrc);
				if(hGlobal)
				{	const void *lpBuffer = ::LockResource(hGlobal);
					if(lpBuffer)
					{	const unsigned int uiSize = static_cast<unsigned int>( ::SizeofResource(moduleRes,hRsrc) );
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
								::GlobalUnlock(lpResBuffer);
							}
							::GlobalFree(hRes);
						}
						UnlockResource(hGlobal);
					}
					::FreeResource(hGlobal);
				}
			}
		}
		if(*bmp && (*bmp)->GetLastStatus()!=Gdiplus::Ok)
		{	::delete *bmp;
			*bmp = NULL;
			return false;
		}
		return (*bmp)!=NULL;
	}

private:
	struct VirtualWindow
	{	VirtualWindow(HDC dcDst, POINT ptDst, SIZE szDst)
		{	assert(dcDst);
				// 
			m_bmpSrc = NULL;
			m_dcSrc = ::CreateCompatibleDC(dcDst);
			if(m_dcSrc)
			{	m_bmpSrc = ::CreateCompatibleBitmap(dcDst,szDst.cx,szDst.cy);
				if(!m_bmpSrc)
				{	::DeleteDC(m_dcSrc);
					m_dcSrc = dcDst;
				}
				else
				{	::SelectObject(m_dcSrc,m_bmpSrc);
					m_dcDst = dcDst;
					m_ptDst = ptDst;
					m_szDst = szDst;
				}
			}
			else
				m_dcSrc = dcDst;
		}
		~VirtualWindow()
		{	Release();
		}
			// 
		operator HDC() const
		{	return m_dcSrc;
		}
		void Release()
		{	if(m_bmpSrc)
			{	::BitBlt(m_dcDst,m_ptDst.x,m_ptDst.y,m_szDst.cx,m_szDst.cy,m_dcSrc,0,0,SRCCOPY);
				::DeleteObject(m_bmpSrc);
				::DeleteDC(m_dcSrc);
				m_bmpSrc = NULL;
			}
		}

	private:
		HDC m_dcSrc, m_dcDst;
		POINT m_ptDst;
		SIZE m_szDst;
		HBITMAP m_bmpSrc;
	};

private:
	struct Timer
	{	Timer()
		{	m_hTimerWnd = NULL;
			m_uTimerID = 0;
		}
		~Timer()
		{	KillTimer();
		}
			// 
		bool SetTimer(PngAnimation *obj, void(PngAnimation::*func)(), int time/*ms*/)
		{	m_pNotifyObj = obj;
			m_pNotifyFunc = func;
				// 
			if( (m_hTimerWnd = CreateTimerWindow())==NULL ||
				(m_uTimerID = ::timeSetEvent(time,0,TimerProc,reinterpret_cast<DWORD_PTR>(this),TIME_PERIODIC))==0)
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
			if(m_hTimerWnd)
			{	::DestroyWindow(m_hTimerWnd);
				m_hTimerWnd = NULL;
			}
		}
			// 
		bool IsActive() const
		{	return m_hTimerWnd!=NULL;
		}

	protected:
		PngAnimation *m_pNotifyObj;
		void(PngAnimation::*m_pNotifyFunc)();
		HWND m_hTimerWnd;
		unsigned int m_uTimerID;

	protected:
		static void __stdcall TimerProc(unsigned int /*uID*/, unsigned int /*uMsg*/, DWORD_PTR dwUser, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
		{	Timer const *timer = reinterpret_cast<Timer const *>(dwUser);
			::SendMessage(timer->m_hTimerWnd,WM_USER,dwUser,0);
		}
			// 
		HWND CreateTimerWindow() const
		{	const TCHAR className[] = _T("{77E7379C-051F-43F7-8B17-D73117493DBD}");
			HMODULE module = ::GetModuleHandle(NULL);
			WNDCLASS wc;
			memset(&wc,0,sizeof(wc));
			if(::GetClassInfo(module,className,&wc/*out*/)==0)
			{	wc.lpfnWndProc = WindowProc;
				wc.hInstance = module;
				wc.lpszClassName = className;
				if(::RegisterClass(&wc)==0)
					return NULL;
			}
			return ::CreateWindowEx(0,className,_T(""),WS_POPUP,0,0,0,0,NULL,NULL,module,NULL);
		}
			// 
		static LRESULT __stdcall WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
		{	const LRESULT res = ::DefWindowProc(hwnd, uMsg, wParam, lParam);
			if(uMsg==WM_USER)
			{	Timer *timer = reinterpret_cast<Timer *>(wParam);
				(timer->m_pNotifyObj->*timer->m_pNotifyFunc)();
			}
			return res;
		}
	};

private:
	Gdiplus::Bitmap *m_pBmp;
	Timer m_Timer;
	HWND m_hParentWnd;
	int m_iCurFrame,m_iNumberFrame, m_iTime;
	POINT m_ptFrame;
	SIZE m_szFrame;
	COLORREF m_clrBack;
	bool m_bInfinitely;
};



