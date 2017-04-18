// BitmapEx.h: interface for the CBitmapEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAPEX_H__80F20A52_B43F_42C5_B182_AC8D27BF5C0E__INCLUDED_)
#define AFX_BITMAPEX_H__80F20A52_B43F_42C5_B182_AC8D27BF5C0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>

#define _PI	3.1415926f											// Value of PI
#define _BITS_PER_PIXEL_32	32									// 32-bit color depth
#define _BITS_PER_PIXEL_24	24									// 24-bit color depth
#define _PIXEL	DWORD											// Pixel
#define _RGB(r,g,b)	(((r) << 16) | ((g) << 8) | (b))			// Convert to RGB
#define _GetRValue(c)	((BYTE)(((c) & 0x00FF0000) >> 16))		// Red color component
#define _GetGValue(c)	((BYTE)(((c) & 0x0000FF00) >> 8))		// Green color component
#define _GetBValue(c)	((BYTE)((c) & 0x000000FF))				// Blue color component
#define _NOISE_WIDTH	192
#define _NOISE_HEIGHT	192
#define _NOISE_DEPTH	64


typedef long fixed;												// Our new fixed point type
#define itofx(x) ((x) << 8)										// Integer to fixed point
#define ftofx(x) (long)((x) * 256)								// Float to fixed point
#define dtofx(x) (long)((x) * 256)								// Double to fixed point
#define fxtoi(x) ((x) >> 8)										// Fixed point to integer
#define fxtof(x) ((float) (x) / 256)							// Fixed point to float
#define fxtod(x) ((double)(x) / 256)							// Fixed point to double
#define Mulfx(x,y) (((x) * (y)) >> 8)							// Multiply a fixed by a fixed
#define Divfx(x,y) (((x) << 8) / (y))							// Divide a fixed by a fixed


typedef struct __POINT
{
	long x;
	long y;

} _POINT, *_LPPOINT;

typedef struct __SIZE
{
	long cx;
	long cy;

} _SIZE, *_LPSIZE;

typedef struct __QUAD
{
	_POINT p1;
	_POINT p2;
	_POINT p3;
	_POINT p4;

} _QUAD, *_LPQUAD;

typedef enum __RESAMPLE_MODE
{
	RM_NEARESTNEIGHBOUR = 0,
	RM_BILINEAR,
	RM_BICUBIC,

} _RESAMPLE_MODE;

typedef enum __GRADIENT_MODE
{
	GM_NONE = 0x0000,
	GM_HORIZONTAL = 0x0001,
	GM_VERTICAL = 0x0002,
	GM_FDIAGONAL = 0x0004,
	GM_BDIAGONAL = 0x0008,
	GM_RADIAL = 0x0010,
	GM_GAMMA = 0x0020

} _GRADIENT_MODE;

typedef enum __COLOR_MODE
{
	CM_RGB = 0,
	CM_HSV

} _COLOR_MODE;

typedef enum __COMBINE_MODE
{
	CM_SRC_AND_DST = 0,
	CM_SRC_OR_DST,
	CM_SRC_XOR_DST,
	CM_SRC_AND_DSTI,
	CM_SRC_OR_DSTI,
	CM_SRC_XOR_DSTI,
	CM_SRCI_AND_DST,
	CM_SRCI_OR_DST,
	CM_SRCI_XOR_DST,
	CM_SRCI_AND_DSTI,
	CM_SRCI_OR_DSTI,
	CM_SRCI_XOR_DSTI

} _COMBINE_MODE;


class CBitmapEx
{
public:
	// Public methods
	CBitmapEx();
	virtual ~CBitmapEx();
	void Create(long width, long height);
	void Create(CBitmapEx& bitmapEx);
	void Create(CBitmapEx* pBitmapEx);

	bool Load( const char *lpszBitmapFile );
	bool Load( LPTSTR lpszBitmapFile);
	bool Load( LPBYTE lpBitmapData);
	bool Load( HBITMAP hBitmap);

	void Save(LPTSTR lpszBitmapFile);
	void Save(LPBYTE lpBitmapData);
	void Save(HBITMAP& hBitmap);
	void Scale(long horizontalPercent=100, long verticalPercent=100);
	void Scale2(long width, long height);
	void Rotate(long degrees=0, _PIXEL bgColor=_RGB(0,0,0));
	void Crop(long x, long y, long width, long height);
	void Shear(long degreesX, long degreesY, _PIXEL bgColor=_RGB(0,0,0));
	void FlipHorizontal();
	void FlipVertical();
	void MirrorLeft();
	void MirrorRight();
	void MirrorTop();
	void MirrorBottom();
	void Clear(_PIXEL clearColor=_RGB(0,0,0));
	void Negative();
	void Grayscale();
	void Sepia(long depth=34);
	void Emboss();
	void Engrave();
	void Pixelize(long size=4);
	void Brightness(long brightness=0);
	void Contrast(long contrast=0);
	void Blur();
	void GaussianBlur();
	void Sharp();
	void Colorize(_PIXEL color);
	void Rank(BOOL bMinimum=TRUE);
	void Spread(long distanceX=8, long distanceY=8);
	void Offset(long offsetX=16, long offsetY=16);
	void BlackAndWhite(long offset=128);
	void EdgeDetect();
	void GlowingEdges(long blur=3, long threshold=2, long scale=5);
	void EqualizeHistogram(long levels=255);
	void Median();
	void Posterize(long levels=4);
	void Solarize(long threshold=128);
	void Draw(HDC hDC);
	void Draw(HDC hDC, long dstX, long dstY);
	void Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY);
	void Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawBlended(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void DrawBlended(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void DrawMasked(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, _PIXEL transparentColor=_RGB(255,255,255));
	void DrawAlpha(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha, _PIXEL alphaColor=_RGB(0,0,0));
	void DrawCombined(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, DWORD mode=CM_SRC_AND_DST);
	void DrawCombined(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode=CM_SRC_AND_DST);
	void DrawTextA(long dstX, long dstY, LPSTR lpszText, _PIXEL textColor, long textAlpha, LPTSTR lpszFontName, long fontSize, BOOL bBold=FALSE, BOOL bItalic=FALSE);
	void DrawTextW(long dstX, long dstY, LPWSTR lpszText, _PIXEL textColor, long textAlpha, LPTSTR lpszFontName, long fontSize, BOOL bBold=FALSE, BOOL bItalic=FALSE);
	LPBITMAPFILEHEADER GetFileInfo() {return &m_bfh;}
	LPBITMAPINFOHEADER GetInfo() {return &m_bih;}
	long GetWidth() {return m_bih.biWidth;}
	long GetHeight() {return m_bih.biHeight;}
	long GetPitch() {return m_iPitch;}
	/** @brief Get bytes per pixel
	*/
	long GetBpp() {return (m_iBpp);}
	/* @bief Get bits per pixel
	*/
	long Getbpp() {return (m_iBpp<<3);}
	long GetPaletteEntries() {return m_iPaletteEntries;}
	LPRGBQUAD GetPalette() {return m_lpPalette;}
	DWORD GetSize() {return m_dwSize;}
	LPBYTE GetData() {return m_lpData;}
	void SetResampleMode(_RESAMPLE_MODE mode=RM_NEARESTNEIGHBOUR) {m_ResampleMode = mode;}
	_RESAMPLE_MODE GetResampleMode() {return m_ResampleMode;}
	BOOL IsValid() {return (m_lpData != NULL);}
	void SetPixel(long x, long y, _PIXEL pixel);
	_PIXEL GetPixel(long x, long y);
	_PIXEL _RGB2HSV(_PIXEL rgbPixel);
	_PIXEL _HSV2RGB(_PIXEL hsvPixel);
	void ConvertToHSV();
	void ConvertToRGB();
	void ReplaceColor(long x, long y, _PIXEL newColor, long alpha=20, long error=100, BOOL bImage=TRUE);
	_COLOR_MODE GetColorMode() {return m_ColorMode;}
	void CreateFireEffect();
	void UpdateFireEffect(BOOL bLarge=TRUE, long iteration=5, long height=16);
	void CreateWaterEffect();
	void UpdateWaterEffect(long iteration=5);
	void MakeWaterBlob(long x, long y, long size, long height);
	void CreateSmokeEffect();
	void UpdateSmokeEffect(long offsetX=1, long offsetY=1, long offsetZ=1);
	_SIZE MeasureTextA(LPSTR lpszText, LPTSTR lpszFontName, long fontSize, BOOL bBold=FALSE, BOOL bItalic=FALSE);
	_SIZE MeasureTextW(LPWSTR lpszText, LPTSTR lpszFontName, long fontSize, BOOL bBold=FALSE, BOOL bItalic=FALSE);
	void GetRedChannel(LPBYTE lpBuffer);
	void GetGreenChannel(LPBYTE lpBuffer);
	void GetBlueChannel(LPBYTE lpBuffer);
	void GetRedChannelHistogram(long lpBuffer[256], BOOL bPercent=FALSE);
	void GetGreenChannelHistogram(long lpBuffer[256], BOOL bPercent=FALSE);
	void GetBlueChannelHistogram(long lpBuffer[256], BOOL bPercent=FALSE);

private:
	// Private methods
	float _ARG(float xa, float ya);
	float _MOD(float x, float y, float z);
	void _ConvertTo32Bpp();
	void _ConvertTo24Bpp();
	void _ScaleNearestNeighbour(long horizontalPercent, long verticalPercent);
	void _ScaleBilinear(long horizontalPercent, long verticalPercent);
	void _ScaleBicubic(long horizontalPercent, long verticalPercent);
	void _ScaleNearestNeighbour2(long width, long height);
	void _ScaleBilinear2(long width, long height);
	void _ScaleBicubic2(long width, long height);
	void _RotateNearestNeighbour(long degrees, _PIXEL bgColor);
	void _RotateBilinear(long degrees, _PIXEL bgColor);
	void _RotateBicubic(long degrees, _PIXEL bgColor);
	void _ShearVerticalNearestNeighbour(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _ShearVerticalBilinear(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _ShearVerticalBicubic(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _ShearVertical(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _ShearHorizontalNearestNeighbour(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _ShearHorizontalBilinear(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _ShearHorizontalBicubic(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _ShearHorizontal(long degrees, _PIXEL bgColor=_RGB(0,0,0));
	void _DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void _DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void _DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void _DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void _DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void _DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void _DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawBlendedNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void _DrawBlendedBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void _DrawBlendedBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void _DrawCombinedNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode=CM_SRC_AND_DST);
	void _DrawCombinedBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode=CM_SRC_AND_DST);
	void _DrawCombinedBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode=CM_SRC_AND_DST);

private:
	// Private members
	BITMAPFILEHEADER m_bfh;
	BITMAPINFOHEADER m_bih;
	long m_iPaletteEntries;
	RGBQUAD m_lpPalette[256];
	long m_iPitch;
	long m_iBpp;
	DWORD m_dwSize;
	LPBYTE m_lpData;
	_RESAMPLE_MODE m_ResampleMode;
	_COLOR_MODE m_ColorMode;
	RGBQUAD m_lpFirePalette[256];
	LPBYTE m_lpFire;
	CBitmapEx* m_pFireBitmap;
	long* m_lpWaterHeightField1;
	long* m_lpWaterHeightField2;
	BOOL m_bWaterFlip;
	long m_iDamp;
	long m_iLightModifier;
	CBitmapEx* m_pWaterBitmap;
	float* m_lpSmokeField;
	CBitmapEx* m_pSmokeBitmap;

};

#endif // !defined(AFX_BITMAPEX_H__80F20A52_B43F_42C5_B182_AC8D27BF5C0E__INCLUDED_)
