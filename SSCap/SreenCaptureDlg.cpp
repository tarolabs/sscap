#include "stdafx.h"
#include "SSCap.h"
#include "SreenCaptureDlg.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSreenCaptureDlg dialog

CSreenCaptureDlg::CSreenCaptureDlg(LPCTSTR lpszWorkingDir,CWnd* pParent /*=NULL*/)
	: CDialog(CSreenCaptureDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSreenCaptureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

//*******************************************************************************
    //初始化像皮筋类,新增的resizeMiddle 类型
	m_rectTracker.m_nStyle=CMyTracker::resizeMiddle|CMyTracker::solidLine;  
	m_rectTracker.m_rect.SetRect(-1,-2,-3,-4);
	//设置矩形颜色
	m_rectTracker.SetRectColor(RGB(10,100,130));
	//设置矩形调整时光标,默认的太小了,弄了个大点的
	m_rectTracker.SetResizeCursor(IDC_CURSOR_SIZE4,IDC_CURSOR_SIZE3,IDC_CURSOR_SIZE2,IDC_CURSOR_SIZE1,IDC_CURSOR_MOVE );

    m_hCursor=AfxGetApp()->LoadCursor(IDC_CURSOR_ARROW);  
	
	m_bDraw=FALSE;
	m_bFirstDraw=FALSE;
	m_bQuit=FALSE;
	m_bShowMsg=FALSE;
    m_startPt=0;
    
	//获取屏幕分辩率
	m_xScreen = GetSystemMetrics(SM_CXSCREEN);
	m_yScreen = GetSystemMetrics(SM_CYSCREEN);

	//截取屏幕到位图中
	CRect rect(0, 0,m_xScreen,m_yScreen);
	m_pBitmap=CBitmap::FromHandle(CopyScreenToBitmap(&rect));
    
	//初始化刷新窗口区域 m_rgn
    m_rgn.CreateRectRgn(0,0,50,50);
//*******************************************************************************

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strFilePath = CString ( lpszWorkingDir ) +CString(_T("\\temp\\")) + CString(TEMP_QRCODE_FILE);
}

void CSreenCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSreenCaptureDlg)
	DDX_Control(pDX, IDC_EDIT_CAPTURE_INFO, m_tipEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSreenCaptureDlg, CDialog)
	//{{AFX_MSG_MAP(CSreenCaptureDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSreenCaptureDlg message handlers

BOOL CSreenCaptureDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
//**************************************************************************
	//把对化框设置成全屏顶层窗口
	SetWindowPos(&wndTopMost,0,0,m_xScreen,m_yScreen,SWP_SHOWWINDOW);
    
	//移动操作提示窗口
	CRect rect;
	m_tipEdit.GetWindowRect(&rect);
    m_tipEdit.MoveWindow(10,10,rect.Width(),rect.Height());

	//显示操作提示窗口文字
	DrawTip();
	
	//捕获按键消息窗口,将对话框的句柄传递到CCatchScreenApp中
	// 主要用于通过键盘来控制大小.
	//((CCatchScreenApp *)AfxGetApp())->m_hwndDlg=m_hWnd;
//**************************************************************************

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	//KillTimer(1001);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSreenCaptureDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
//**************************************************************************
		CPaintDC dc(this);
         
		//显示截取矩形大小信息
		if(m_bShowMsg&&m_bFirstDraw)
		{
			//得到当前矩形大小
			CRect rect;
			m_rectTracker.GetTrueRect(&rect);
			//传递CPaintDC 是为了不在了窗口上画信息
			DrawMessage(rect,&dc);
		}

		//画出像皮筋矩形
		if(m_bFirstDraw)
		{
			m_rectTracker.Draw(&dc);
		}

//*************************************************************************
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSreenCaptureDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSreenCaptureDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//CDialog::OnOK();
}

void CSreenCaptureDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
//***************************************************************
	if(m_bFirstDraw)
	{
		//取消已画矩形变量
		m_bFirstDraw=FALSE;
		m_bDraw=FALSE;
		m_rectTracker.m_rect.SetRect(-1,-1,-1,-1);
		PaintWindow();
	}
	else
	{
		CDialog::OnCancel();
	}
//*******************************************************************
}

void CSreenCaptureDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
//**************************************************************************************
	   if(m_bDraw)
	   {
			//动态调整矩形大小,并刷新画出
		    m_rectTracker.m_rect.SetRect(m_startPt.x+4,m_startPt.y+4,point.x,point.y);
			PaintWindow();
	   }
	   
	   //弥补调整大小和位置时,接收不到MouseMove消息
	   CRect rect;
	   m_tipEdit.GetWindowRect(&rect);
	   if(rect.PtInRect(point))
	   m_tipEdit.SendMessage(WM_MOUSEMOVE);
       
	   DrawTip();
	   //ChangeRGB();
//*****************************************************************************************
	CDialog::OnMouseMove(nFlags, point);
}

void CSreenCaptureDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
//*****************************************************************************************
	int nHitTest;
	nHitTest=m_rectTracker.HitTest(point);

    //判断击中位置
	if(nHitTest<0)
	{
		if(!m_bFirstDraw)
		{
			//第一次画矩形
			m_startPt=point;
			m_bDraw=TRUE;
			m_bFirstDraw=TRUE;
			//设置当当鼠标按下时最小的矩形大小
			m_rectTracker.m_rect.SetRect(point.x,point.y,point.x+4,point.y+4);	
			
			//保证当鼠标当下时立刻显示信息
			if(m_bFirstDraw)
			  m_bShowMsg=TRUE;		
			DrawTip();
			PaintWindow();
		}
	}
	else
	{
		//保证当鼠标当下时立刻显示信息
		m_bShowMsg=TRUE;		
		PaintWindow();
		
		if(m_bFirstDraw)
		{
			//调束大小时,Track会自动调整矩形大小,在些期间,消息归CRectTracker内部处理
			m_rectTracker.Track(this,point,TRUE);
			//SendMessage(WM_LBUTTONUP,NULL,NULL);
			PaintWindow();

		}
	}
//****************************************************************************************
	CDialog::OnLButtonDown(nFlags, point);
}

void CSreenCaptureDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
//****************************************************************************************
    
	m_bShowMsg=FALSE;
	m_bDraw=FALSE;

	// 如果允许编辑截图
#ifdef _ENABLE_EDIT
	DrawTip();
	PaintWindow();
#else 
	CopyScreenToBitmap(m_rectTracker.m_rect,TRUE);
	EndDialog( IDOK );
	return;
#endif

//****************************************************************************************
	CDialog::OnLButtonUp(nFlags, point);
}

void CSreenCaptureDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	int nHitTest;
	nHitTest=m_rectTracker.HitTest(point);
    //如果在是矩形内部双击
	if(nHitTest==8)      
	{
        //保存位图到粘贴板中,bSave 为TRUE,
		CopyScreenToBitmap(m_rectTracker.m_rect,TRUE);
	    //PostQuitMessage(0);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CSreenCaptureDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//****************************************************************************************
	if(m_bFirstDraw)
	{
		//如果已经截取矩则清除截取矩形
		m_bFirstDraw=FALSE;
		//清除矩形大小
		m_rectTracker.m_rect.SetRect(-1,-1,-1,-1);
		DrawTip();
		PaintWindow();
	}
	else
	{
		 //关闭程序
		 //PostQuitMessage(0);
		EndDialog( IDCANCEL );
	}
//****************************************************************************************
	CDialog::OnRButtonUp(nFlags, point);
}

HBRUSH CSreenCaptureDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	//HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	// TODO: Return a different brush if the default is not desired
//***********************************************************
	pDC->SetBkMode(TRANSPARENT);

	//设置操作提示窗口文本颜色
	if(pWnd->GetDlgCtrlID()==IDC_EDIT_CAPTURE_INFO)
	{
		pDC->SetTextColor(RGB(255,255,255));
	}
//***************************************************************
	// TODO: Return a different brush if the default is not desired
	return NULL;
	//return hbr;
}

BOOL CSreenCaptureDlg::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
//**************************************************************************************
	//用整个桌面填充全屏对话框背景
	BITMAP bmp;
	m_pBitmap->GetBitmap(&bmp);

	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);

	dcCompatible.SelectObject(m_pBitmap);

	CRect rect;
	GetClientRect(&rect);
	pDC->BitBlt(0,0,rect.Width(),rect.Height(),&dcCompatible,0,0,SRCCOPY);

	return TRUE;
//**************************************************************************************
	//return CDialog::OnEraseBkgnd(pDC);
}

BOOL CSreenCaptureDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
//***********************************************************************
	//设置改变截取矩形大小时光标
	if (pWnd == this &&m_rectTracker.SetCursor(this, nHitTest)
		             &&!m_bDraw &&m_bFirstDraw) //此处判断保截取时当标始中为彩色光标
    {
		return TRUE; 
	}

	//设置彩色光标
	SetCursor(m_hCursor);
	return TRUE;

//*******************************************************************
	//return CDialog::OnSetCursor(pWnd, nHitTest, message);
}


//*********************增加的函数**********************************************************
//考贝屏幕,这段代码是拿来主义 呵呵
HBITMAP CSreenCaptureDlg::CopyScreenToBitmap(LPRECT lpRect,BOOL bSave)
//lpRect 代表选定区域
{
	HDC       hScrDC, hMemDC;      
	// 屏幕和内存设备描述表
	HBITMAP    hBitmap, hOldBitmap;   
	// 位图句柄
	int       nX, nY, nX2, nY2;      
	// 选定区域坐标
	int       nWidth, nHeight;
	
	// 确保选定区域不为空矩形
	if (IsRectEmpty(lpRect))
		return NULL;
	//为屏幕创建设备描述表
	hScrDC = CreateDC( _T("DISPLAY"), NULL, NULL, NULL);

	//为屏幕设备描述表创建兼容的内存设备描述表
	hMemDC = CreateCompatibleDC(hScrDC);
	// 获得选定区域坐标
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//确保选定区域是可见的
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > m_xScreen)
		nX2 = m_xScreen;
	if (nY2 > m_yScreen)
		nY2 = m_yScreen;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;
	// 创建一个与屏幕设备描述表兼容的位图
	hBitmap = CreateCompatibleBitmap
		(hScrDC, nWidth, nHeight);
	// 把新位图选到内存设备描述表中
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// 把屏幕设备描述表拷贝到内存设备描述表中
	if(bSave)
	{
		//创建军兼容DC,当bSave为中时把开始保存的全屏位图,按截取矩形大小保存
		CDC dcCompatible;
		dcCompatible.CreateCompatibleDC(CDC::FromHandle(hMemDC));
		dcCompatible.SelectObject(m_pBitmap);
        
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			dcCompatible, nX, nY, SRCCOPY);

	}
	else
	{
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			hScrDC, nX, nY, SRCCOPY);
	}

	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	//得到屏幕位图的句柄
	//清除 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	
	if(bSave)
	{
				
		if (OpenClipboard()) 
		{
        //清空剪贴板
        EmptyClipboard();
        //把屏幕内容粘贴到剪贴板上,
        //hBitmap 为刚才的屏幕位图句柄
        SetClipboardData(CF_BITMAP, hBitmap);

		// 写PNG文件
		WritePngToFile( hBitmap );

        //关闭剪贴板
        CloseClipboard();
      }
	}
	// 返回位图句柄

	return hBitmap;
}

//显示操作提示信息
void CSreenCaptureDlg::DrawTip()
{
    //得当前坐标像素,
	CPoint pt;
	GetCursorPos(&pt);
	
	//当到当前R,G,B,各像素值
	COLORREF color;
	CClientDC dc(this);
	color=dc.GetPixel(pt);
	BYTE rValue,gValue,bValue;
	rValue=GetRValue(color);
	gValue=GetGValue(color);
	bValue=GetGValue(color);
	
	//按格式排放字符串
	CString string;
	CString strTemp;
	string.Format( lm_u82u16_s(_("\r\n\r\n Current RGB  (%d,%d,%d)\r\n")),rValue,gValue,bValue);
    
	if(!m_bDraw&&!m_bFirstDraw)
	{
		strTemp=lm_u82u16_s(_("\r\n Press and drag your mouse to select\r\n\r\n Press ESC or right click to exit"));
	}
	else
	{
		if(m_bDraw&&m_bFirstDraw)
		{
			strTemp=lm_u82u16_s(_("\r\n Release your mouse key to confirm selection\r\n\r\n Press ESC to exit"));
		}
		else if(m_bFirstDraw)
		{
			strTemp=lm_u82u16_s( _("\r\n Use left key to adjust selection\r\n\r\n Double click in selection to confirm and exit\r\n\r\n Right click to reselect") );
		}
	}
	string+=strTemp;
	//显示到编缉框中,操作提示窗口
	m_tipEdit.SetWindowText(string);
}

//显示截取矩形信息
void CSreenCaptureDlg::DrawMessage(CRect &inRect,CDC * pDC)
{
	//截取矩形大小信息离鼠标间隔
	const int space=3;
    
	//设置字体颜色大小
	
	CPoint pt;
	CPen pen(PS_SOLID,1,RGB(147,147,147));

	//dc.SetTextColor(RGB(147,147,147));
	CFont font;
	CFont * pOldFont;
	font.CreatePointFont(90,_T("宋体") );
	pOldFont=pDC->SelectObject(&font);

	//得到字体宽度和高度
	GetCursorPos(&pt);
	int OldBkMode;
	OldBkMode=pDC->SetBkMode(TRANSPARENT);

	TEXTMETRIC tm;
	int charHeight;
	CSize size;
	int	lineLength;
	pDC->GetTextMetrics(&tm);
	charHeight = tm.tmHeight+tm.tmExternalLeading;

	int textlength = _tcslen( lm_u82u16_s( _("  Corner position  ")));
	size=pDC->GetTextExtent( lm_u82u16_s( _("  Corner position  ")), textlength );

	lineLength=size.cx;
    
	//初始化矩形, 以保证写下六行文字
	CRect rect(pt.x+space,pt.y-charHeight*6-space,pt.x+lineLength+space,pt.y-space);
    
    //创建临时矩形
    CRect rectTemp;
	//当矩形到达桌面边缘时调整方向和大小
	if((pt.x+rect.Width())>=m_xScreen)
	{
		//桌面上方显示不下矩形
		rectTemp=rect;
		rectTemp.left=rect.left-rect.Width()-space*2;
		rectTemp.right=rect.right-rect.Width()-space*2;;
		rect=rectTemp;
	}

	if((pt.y-rect.Height())<=0)
	{
		//桌面右方显示不下矩形
		rectTemp=rect;
		rectTemp.top=rect.top+rect.Height()+space*2;;
		rectTemp.bottom=rect.bottom+rect.Height()+space*2;;
		rect=rectTemp;
		
	}

	//创建空画刷画矩形
	CBrush * pOldBrush;
    pOldBrush=pDC->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));
	
	pDC->Rectangle(rect);
   	rect.top+=2;
	//在矩形中显示文字
	CRect outRect(rect.left,rect.top,rect.left+lineLength,rect.top+charHeight);
	CString string( lm_u82u16_s(_("  Corner position  ") ) );
	pDC->DrawText(string,outRect,DT_CENTER);
    
	outRect.SetRect(rect.left,rect.top+charHeight,rect.left+lineLength,charHeight+rect.top+charHeight);
	string.Format( _T("(%d,%d)"),inRect.left,inRect.top);
	pDC->DrawText(string,outRect,DT_CENTER);
	

	outRect.SetRect(rect.left,rect.top+charHeight*2,rect.left+lineLength,charHeight+rect.top+charHeight*2);
	string= lm_u82u16_s( _("  Rectangle size  " )) ;
	pDC->DrawText(string,outRect,DT_CENTER);

	outRect.SetRect(rect.left,rect.top+charHeight*3,rect.left+lineLength,charHeight+rect.top+charHeight*3);
	string.Format( _T("(%d,%d)"),inRect.Width(),inRect.Height());
    pDC->DrawText(string,outRect,DT_CENTER);

	outRect.SetRect(rect.left,rect.top+charHeight*4,rect.left+lineLength,charHeight+rect.top+charHeight*4);
	string = lm_u82u16_s( _( "  Cursor position  " ));
    pDC->DrawText(string,outRect,DT_CENTER);

	outRect.SetRect(rect.left,rect.top+charHeight*5,rect.left+lineLength,charHeight+rect.top+charHeight*5);
	string.Format( _T("(%d,%d)"),pt.x,pt.y);
	pDC->DrawText(string,outRect,DT_CENTER);
    
	pDC->SetBkMode(OldBkMode);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldBrush);
	
}
//重画窗口
void CSreenCaptureDlg::PaintWindow()
{
	//获取当全屏对话框窗口大小
	CRect rect1;
	GetWindowRect(rect1);

	//获取编辑框窗口大小
	CRect rect2;
	m_tipEdit.GetWindowRect(rect2);

	CRgn rgn1,rgn2;
	rgn1.CreateRectRgnIndirect(rect1);
	rgn2.CreateRectRgnIndirect(rect2);

	//获取更新区域,就是除了编辑框窗口不更新
	m_rgn.CombineRgn(&rgn1,&rgn2,RGN_DIFF);
	
	InvalidateRgn(&m_rgn);
}
//改变操作提示窗口当RGB值
void CSreenCaptureDlg::ChangeRGB()
{
	//保存旧的RGB值字符串
	static CString strOld( _T("" ));

	CPoint pt;
	GetCursorPos(&pt);

	//当到当前R,G,B,各像素值
	COLORREF color;
	CClientDC dc(this);
	color=dc.GetPixel(pt);
	BYTE rValue,gValue,bValue;
	rValue=GetRValue(color);
	gValue=GetGValue(color);
	bValue=GetGValue(color);
	
	//按格式排放字符串
	CString string;

	string.Format( lm_u82u16_s(_(" Pixel RGB  (%d,%d,%d)")),rValue,gValue,bValue);
//如果当前颜色没变则不刷新RGB值,以免窗口有更多闪烁
    if(strOld!=string)
	{
	    //得到RGB文本那一行的文本长度
		int LineLength=m_tipEdit.LineLength( 6 );
		//复选RGB值文本,也就是(255,255,255)形式
	    m_tipEdit.SetSel( 6, LineLength  );
        
		//替换RGB内容
		m_tipEdit.ReplaceSel(string);
    }
	
	strOld=string;

}

//*******************************************************************************************


void CSreenCaptureDlg::WritePngToFile( HBITMAP hBitmap )
{
	CImage img;
	img.Attach( hBitmap );

	img.Save( m_strFilePath, Gdiplus::ImageFormatPNG );
	return;
}

void CSreenCaptureDlg::WriteBmpToFile(HBITMAP hBitmap)
{
	HDC hDC =::CreateDC( _T("DISPLAY"),NULL,NULL,NULL); 
	int iBits = ::GetDeviceCaps(hDC, BITSPIXEL) * ::GetDeviceCaps(hDC, PLANES);//当前分辨率下每个像素所占字节数  
	::DeleteDC(hDC);

	WORD   wBitCount;   //位图中每个像素所占字节数    
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else if (iBits <= 24)
		wBitCount = 24;
	else
		wBitCount = iBits;

	DWORD   dwPaletteSize=0;	//调色板大小， 位图中像素字节大小 
	if (wBitCount <= 8)		
		dwPaletteSize = (1 << wBitCount) *	sizeof(RGBQUAD);	


	BITMAP  bm;        //位图属性结构
	::GetObject(hBitmap, sizeof(bm), (LPSTR)&bm);  


	BITMAPINFOHEADER   bi;       //位图信息头结构     
	bi.biSize            = sizeof(BITMAPINFOHEADER);  
	bi.biWidth           = bm.bmWidth;
	bi.biHeight          = bm.bmHeight;
	bi.biPlanes          = 1;
	bi.biBitCount        = wBitCount;
	bi.biCompression     = BI_RGB; //BI_RGB表示位图没有压缩
	bi.biSizeImage       = 0;
	bi.biXPelsPerMeter   = 0;
	bi.biYPelsPerMeter   = 0;
	bi.biClrUsed         = 0;
	bi.biClrImportant    = 0;

	DWORD dwBmBitsSize = ((bm.bmWidth * wBitCount+31)/32) * 4 * bm.bmHeight;    
	HANDLE hDib  = ::GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));  //为位图内容分配内存
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	HANDLE hPal = ::GetStockObject(DEFAULT_PALETTE);  // 处理调色板 
	HANDLE  hOldPal=NULL; 
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = SelectPalette(hDC,(HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}
	::GetDIBits(hDC, hBitmap, 0, (UINT) bm.bmHeight,(LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSize,(BITMAPINFO*)lpbi,DIB_RGB_COLORS);// 获取该调色板下新的像素值
	if (hOldPal)//恢复调色板
	{
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}	

	BITMAPFILEHEADER   bmfHdr; //位图文件头结构     
	bmfHdr.bfType = 0x4D42;  // "BM"  	// 设置位图文件头
	DWORD dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;  
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

	HANDLE hFile = CreateFile(m_strFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);//创建位图文件   
	DWORD dwWritten;
	WriteFile(hFile, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);	// 写入位图文件头
	WriteFile(hFile, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);// 写入位图文件其余内容

	GlobalUnlock(hDib);   //清除   
	GlobalFree(hDib);
	CloseHandle(hFile); 	
}
