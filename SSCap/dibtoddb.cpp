#include "stdafx.h"
#include "dibtoddb.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HBITMAP DIBToDDB( HANDLE hDIB ) 
{ 
	LPBITMAPINFOHEADER lpbi; 
	HBITMAP hbm; 
	CPalette pal; 
	CPalette* pOldPal; 
	CClientDC dc(NULL); 
	if (hDIB == NULL) 
		return NULL; 
	lpbi = (LPBITMAPINFOHEADER)hDIB; 
	int nColors = lpbi->biClrUsed ? lpbi->biClrUsed : 
		1 << lpbi->biBitCount; 
	BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB ; 
	LPVOID lpDIBBits; 
	if( bmInfo.bmiHeader.biBitCount > 8 ) 
		lpDIBBits = (LPVOID)((LPDWORD)(bmInfo.bmiColors + 
		bmInfo.bmiHeader.biClrUsed) + 
		((bmInfo.bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0)); 
	else 
		lpDIBBits = (LPVOID)(bmInfo.bmiColors + nColors); 
	// Create and select a logical palette if needed 
	if( nColors <= 256 && dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE) 
	{ 
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors); 
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize]; 
		pLP->palVersion = 0x300; 
		pLP->palNumEntries = nColors; 
		for( int i=0; i < nColors; i++) 
		{ 
			pLP->palPalEntry[i].peRed = bmInfo.bmiColors[i].rgbRed; 
			pLP->palPalEntry[i].peGreen = bmInfo.bmiColors[i].rgbGreen; 
			pLP->palPalEntry[i].peBlue = bmInfo.bmiColors[i].rgbBlue; 
			pLP->palPalEntry[i].peFlags = 0; 
		} 
		pal.CreatePalette( pLP ); 
		delete[] pLP; 
		// Select and realize the palette 
		pOldPal = dc.SelectPalette( &pal, FALSE ); 
		dc.RealizePalette(); 
	} 
	hbm = CreateDIBitmap(dc.GetSafeHdc(), // handle to device context 
		(LPBITMAPINFOHEADER)lpbi, // pointer to bitmap info header 
		(LONG)CBM_INIT, // initialization flag 
		lpDIBBits, // pointer to initialization data 
		(LPBITMAPINFO)lpbi, // pointer to bitmap info 
		DIB_RGB_COLORS ); // color-data usage 
	if (pal.GetSafeHandle()) 
		dc.SelectPalette(pOldPal,FALSE); 
	return hbm; 
} 