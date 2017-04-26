// BitmapEx.cpp: implementation of the CBitmapEx class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "BitmapEx.h"
#include <io.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBitmapEx::CBitmapEx()
{
	// Init members
	m_lpData = NULL;
	m_dwSize = 0;
	m_iPaletteEntries = 0;
	m_ResampleMode = RM_NEARESTNEIGHBOUR;
	m_ColorMode = CM_RGB;
	m_lpFire = NULL;
	m_pFireBitmap = NULL;
	m_lpWaterHeightField1 = NULL;
	m_lpWaterHeightField2 = NULL;
	m_bWaterFlip = FALSE;
	m_iDamp = 5;
	m_iLightModifier = 3;
	m_pWaterBitmap = NULL;
	m_lpSmokeField = NULL;
	m_pSmokeBitmap = NULL;
}

CBitmapEx::~CBitmapEx()
{
	// Deinit members
	if (m_lpData != NULL)
	{
		free(m_lpData);
		m_lpData = NULL;
	}
	if (m_lpFire != NULL)
	{
		free(m_lpFire);
		m_lpFire = NULL;
	}
	if (m_pFireBitmap != NULL)
	{
		delete m_pFireBitmap;
		m_pFireBitmap = NULL;
	}
	if (m_lpWaterHeightField1 != NULL)
	{
		free(m_lpWaterHeightField1);
		m_lpWaterHeightField1 = NULL;
	}
	if (m_lpWaterHeightField2 != NULL)
	{
		free(m_lpWaterHeightField2);
		m_lpWaterHeightField2 = NULL;
	}
	if (m_pWaterBitmap != NULL)
	{
		delete m_pWaterBitmap;
		m_pWaterBitmap = NULL;
	}
	if (m_lpSmokeField != NULL)
	{
		free(m_lpSmokeField);
		m_lpSmokeField = NULL;
	}
	if (m_pSmokeBitmap != NULL)
	{
		delete m_pSmokeBitmap;
		m_pSmokeBitmap = NULL;
	}
}

void CBitmapEx::Create(long width, long height)
{
	// Deinit members
	if (m_lpData != NULL)
	{
		free(m_lpData);
		m_lpData = NULL;
	}

	// Format BITMAPINFOHEADER info
	m_iBpp = _BITS_PER_PIXEL_32 >> 3;
	memset(&m_bih, 0, sizeof(BITMAPINFOHEADER));
	m_bih.biSize = sizeof(BITMAPINFOHEADER);
	m_bih.biWidth = max(0, width);
	m_bih.biHeight = max(0, height);
	m_bih.biPlanes = 1;
	m_bih.biBitCount = (WORD)(m_iBpp << 3);
	m_iPaletteEntries = 0;
	m_iPitch = m_bih.biWidth * m_iBpp;
	m_dwSize = m_iPitch * m_bih.biHeight;
	m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));

	// Format BITMAPFILEHEADER info
	memset(&m_bfh, 0, sizeof(BITMAPFILEHEADER));
	m_bfh.bfType = 'MB';
	m_bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_dwSize;
	m_bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
}

void CBitmapEx::Create(CBitmapEx& bitmapEx)
{
	// Check for valid bitmap
	if (bitmapEx.IsValid())
	{
		// Deinit members
		if (m_lpData != NULL)
		{
			free(m_lpData);
			m_lpData = NULL;
		}

		// Calculate bitmap params
		m_iBpp = bitmapEx.GetBpp() >> 3;
		m_iPitch = bitmapEx.GetPitch();
		memcpy(&m_bfh, bitmapEx.GetFileInfo(), sizeof(BITMAPFILEHEADER));
		memcpy(&m_bih, bitmapEx.GetInfo(), sizeof(BITMAPINFOHEADER));
		m_dwSize = bitmapEx.GetSize();
		m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
		memcpy(m_lpData, bitmapEx.GetData(), m_dwSize*sizeof(BYTE));
		m_iPaletteEntries = bitmapEx.GetPaletteEntries();
		memcpy(m_lpPalette, bitmapEx.GetPalette(), m_iPaletteEntries*sizeof(RGBQUAD));
		m_ResampleMode = bitmapEx.GetResampleMode();
	}
}

void CBitmapEx::Create(CBitmapEx* pBitmapEx)
{
	// Check for valid bitmap
	if ((pBitmapEx != NULL) && (pBitmapEx->IsValid()))
	{
		// Deinit members
		if (m_lpData != NULL)
		{
			free(m_lpData);
			m_lpData = NULL;
		}

		// Calculate bitmap params
		m_iBpp = pBitmapEx->GetBpp() >> 3;
		m_iPitch = pBitmapEx->GetPitch();
		memcpy(&m_bfh, pBitmapEx->GetFileInfo(), sizeof(BITMAPFILEHEADER));
		memcpy(&m_bih, pBitmapEx->GetInfo(), sizeof(BITMAPINFOHEADER));
		m_dwSize = pBitmapEx->GetSize();
		m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
		memcpy(m_lpData, pBitmapEx->GetData(), m_dwSize*sizeof(BYTE));
		m_iPaletteEntries = pBitmapEx->GetPaletteEntries();
		memcpy(m_lpPalette, pBitmapEx->GetPalette(), m_iPaletteEntries*sizeof(RGBQUAD));
		m_ResampleMode = pBitmapEx->GetResampleMode();
	}
}

bool  CBitmapEx::Load( const char* lpszBitmapFile )
{
	assert( lpszBitmapFile != NULL );

	static wchar_t buf[4096];
	memset( buf,0,4096 );

	MultiByteToWideChar (CP_ACP,0,lpszBitmapFile,-1,(LPWSTR)buf,4095);

	return Load( buf );

}
bool  CBitmapEx::Load(LPTSTR lpszBitmapFile)
{
	// Check for valid .BMP file path
	if (lpszBitmapFile != NULL)
	{
		// Open .BMP file
		FILE* file = _tfopen(lpszBitmapFile, _T("rb"));
		if (file != NULL)
		{
			// Deinit members
			if (m_lpData != NULL)
			{
				free(m_lpData);
				m_lpData = NULL;
			}

			// Read BITMAPFILEHEADER info
			fread(&m_bfh, 1, sizeof(BITMAPFILEHEADER), file);

			// Read BITMAPINFOHEADER info
			fread(&m_bih, 1, sizeof(BITMAPINFOHEADER), file);

			// Calculate pitch
			m_iBpp = m_bih.biBitCount >> 3;
			m_iPitch = m_iBpp * m_bih.biWidth;
			while ((m_iPitch & 3) != 0)
				m_iPitch++;

			// Check for bit-depth (8, 16, 24 and 32bpp only)
			if (m_bih.biBitCount >= 8)
			{
				if (m_bih.biBitCount == 8)
				{
					// Calculate palette entries
					m_iPaletteEntries = m_bih.biClrUsed;
					if (m_iPaletteEntries == 0)
						m_iPaletteEntries = 256;

					// Read palette info
					fread(m_lpPalette, m_iPaletteEntries, sizeof(RGBQUAD), file);
				}

				// Read image data
				m_dwSize = m_iPitch * m_bih.biHeight;
				m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
				fread(m_lpData, m_dwSize, sizeof(BYTE), file);

				// Convert to 32bpp bitmap
				_ConvertTo32Bpp();
			}

			// Close .BMP file
			fclose(file);
		}
		else return false;
	} 
	else return false;

	return true;
}

bool  CBitmapEx::Load(LPBYTE lpBitmapData)
{
	// Check for valid bitmap data buffer
	if (lpBitmapData != NULL)
	{
		// Deinit members
		if (m_lpData != NULL)
		{
			free(m_lpData);
			m_lpData = NULL;
		}

		// Read BITMAPFILEHEADER info
		memcpy(&m_bfh, lpBitmapData, sizeof(BITMAPFILEHEADER));

		// Read BITMAPINFOHEADER info
		memcpy(&m_bih, lpBitmapData+sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));

		// Calculate pitch
		m_iBpp = m_bih.biBitCount >> 3;
		m_iPitch = m_iBpp * m_bih.biWidth;
		while ((m_iPitch & 3) != 0)
			m_iPitch++;

		// Check for bit-depth (8, 16, 24 and 32bpp only)
		if (m_bih.biBitCount >= 8)
		{
			if (m_bih.biBitCount == 8)
			{
				// Calculate palette entries
				m_iPaletteEntries = m_bih.biClrUsed;
				if (m_iPaletteEntries == 0)
					m_iPaletteEntries = 256;

				// Read palette info
				memcpy(m_lpPalette, lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), m_iPaletteEntries*sizeof(RGBQUAD));
			}

			// Read image data
			m_dwSize = m_iPitch * m_bih.biHeight;
			m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
			memcpy(m_lpData, lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+m_iPaletteEntries*sizeof(RGBQUAD), m_dwSize*sizeof(BYTE));

			// Convert to 32bpp bitmap
			_ConvertTo32Bpp();
		}
	}
	else return false;
	return true;
}

bool  CBitmapEx::Load(HBITMAP hBitmap)
{
	// Check for valid bitmap handle
	if (hBitmap != NULL)
	{
		// Deinit members
		if (m_lpData != NULL)
		{
			free(m_lpData);
			m_lpData = NULL;
		}

		// Get bitmap info
		BITMAP bmp;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		// Check for valid bitmap format (16, 24 and 32bpp only)
		if (bmp.bmBitsPixel > 8)
		{
			// Get bitmap data
			m_iBpp = bmp.bmBitsPixel >> 3;
			m_iPitch = bmp.bmWidthBytes;
			m_dwSize = bmp.bmHeight * m_iPitch;
			m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
			GetBitmapBits(hBitmap, m_dwSize*sizeof(BYTE), m_lpData);

			// Format BITMAPFILEHEADER info
			memset(&m_bfh, 0, sizeof(BITMAPFILEHEADER));
			m_bfh.bfType = 'MB';
			m_bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_dwSize;
			m_bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

			// Format BITMAPINFOHEADER info
			memset(&m_bih, 0, sizeof(BITMAPINFOHEADER));
			m_bih.biSize = sizeof(BITMAPINFOHEADER);
			m_bih.biWidth = bmp.bmWidth;
			m_bih.biHeight = bmp.bmHeight;
			m_bih.biPlanes = 1;
			m_bih.biBitCount = bmp.bmBitsPixel;

			// Convert to 32bpp bitmap
			_ConvertTo32Bpp();
			FlipVertical();
		}
	}
	return false;
	return	true;
}

void CBitmapEx::Save(LPTSTR lpszBitmapFile)
{
	// Check for valid .BMP file path
	if (lpszBitmapFile != NULL)
	{
		// Convert to 24bpp bitmap
		_ConvertTo24Bpp();

		// Open .BMP file
		FILE* file = _tfopen(lpszBitmapFile, _T("wb"));
		if (file != NULL)
		{
			// Write BITMAPFILEHEADER info
			fwrite(&m_bfh, 1, sizeof(BITMAPFILEHEADER), file);

			// Write BITMAPINFOHEADER info
			fwrite(&m_bih, 1, sizeof(BITMAPINFOHEADER), file);

			// Check for palettized bitmap
			if (m_bih.biBitCount == 8)
			{
				// Write palette info
				fwrite(m_lpPalette, m_iPaletteEntries, sizeof(RGBQUAD), file);
			}

			// Write image data
			fwrite(m_lpData, m_dwSize, sizeof(BYTE), file);

			// Close .BMP file
			fclose(file);
		}

		// Convert to 32bpp bitmap
		_ConvertTo32Bpp();
	}
}

void CBitmapEx::Save(LPBYTE lpBitmapData)
{
	// Check for valid bitmap data buffer
	if (lpBitmapData != NULL)
	{
		// Convert to 24bpp bitmap
		_ConvertTo24Bpp();

		// Write BITMAPFILEHEADER info
		memcpy(lpBitmapData, &m_bfh, sizeof(BITMAPFILEHEADER));

		// Write BITMAPINFOHEADER info
		memcpy(lpBitmapData+sizeof(BITMAPFILEHEADER), &m_bih, sizeof(BITMAPINFOHEADER));

		// Check for palettized bitmap
		if (m_bih.biBitCount == 8)
		{
			// Write palette info
			memcpy(lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), m_lpPalette, m_iPaletteEntries*sizeof(RGBQUAD));
		}

		// Write image data
		memcpy(lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+m_iPaletteEntries*sizeof(RGBQUAD), m_lpData, m_dwSize*sizeof(BYTE));

		// Convert to 32bpp bitmap
		_ConvertTo32Bpp();
	}
}

void CBitmapEx::Save(HBITMAP& hBitmap)
{
	// Check for valid bitmap data buffer and handle
	if ((IsValid()) && (hBitmap == NULL))
	{
		// Write bitmap data
		FlipVertical();
		hBitmap = CreateBitmap(m_bih.biWidth, m_bih.biHeight, 1, m_bih.biBitCount, m_lpData);
		FlipVertical();
	}
}

float CBitmapEx::_ARG(float xa, float ya)
{
	float _arg = 0.0f;

	// Calculate ARG
	if ((xa == 0.0f) && (ya == 0.0f))
		_arg = 0.0f;
	if ((xa == 0.0f) && (ya >= 0.0f))
		_arg = _PI / 2;
	if ((ya == 0.0f) && (xa < 0.0f))
		_arg = _PI;
	if ((xa == 0.0f) && (ya < 0.0f))
		_arg = -_PI / 2;
	if (xa > 0.0f)
		_arg = atan(ya / xa);
	if ((xa < 0.0f) && (ya >= 0.0f))
		_arg = _PI - atan(-ya / xa);
	if ((xa < 0.0f) && (ya < 0.0f))
		_arg = -_PI + atan(-ya / -xa);

	return _arg;
}

float CBitmapEx::_MOD(float x, float y, float z)
{
	float _mod = 0.0f;

	// Calculate MOD
	_mod = sqrt(x*x + y*y + z*z);

	return _mod;
}

void CBitmapEx::_ConvertTo32Bpp()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate new params
		long _bpp = (_BITS_PER_PIXEL_32 >> 3);
		long _pitch = m_bih.biWidth * _bpp;

		// Create temporary bitmap
		DWORD dwSize = _pitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Convert bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = 0;
			dwSrcHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				switch (m_bih.biBitCount)
				{
					case 8:
						{
							BYTE red = m_lpPalette[m_lpData[dwSrcTotalOffset]].rgbRed;
							BYTE green = m_lpPalette[m_lpData[dwSrcTotalOffset]].rgbGreen;
							BYTE blue = m_lpPalette[m_lpData[dwSrcTotalOffset]].rgbBlue;
							lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
						}
						break;

					case 16:
						{
							LPWORD lpSrcData = (LPWORD)m_lpData;
							BYTE red = (lpSrcData[dwSrcTotalOffset>>1] & 0x7C00) >> 10;
							BYTE green = (lpSrcData[dwSrcTotalOffset>>1] & 0x03E0) >> 5;
							BYTE blue = lpSrcData[dwSrcTotalOffset>>1] & 0x001F;
							lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
						}
						break;

					case 24:
						{
							lpDstData[dwDstTotalOffset>>2] = _RGB(m_lpData[dwSrcTotalOffset+2], m_lpData[dwSrcTotalOffset+1], m_lpData[dwSrcTotalOffset]);
						}
						break;

					case 32:
						{
							LPDWORD lpSrcData = (LPDWORD)m_lpData;
							lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
						}
						break;
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += _bpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_iBpp = _bpp;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
		m_bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_dwSize;
		m_bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		m_bih.biBitCount = _BITS_PER_PIXEL_32;
	}
}

void CBitmapEx::_ConvertTo24Bpp()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate new params
		long _bpp = (_BITS_PER_PIXEL_24 >> 3);
		long _pitch = m_bih.biWidth * _bpp;
		while ((_pitch & 3) != 0)
			_pitch++;

		// Create temporary bitmap
		DWORD dwSize = _pitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Convert bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = 0;
			dwSrcHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpData[dwDstTotalOffset+2] = _GetRValue(lpSrcData[dwSrcTotalOffset>>2]);
				lpData[dwDstTotalOffset+1] = _GetGValue(lpSrcData[dwSrcTotalOffset>>2]);
				lpData[dwDstTotalOffset] = _GetBValue(lpSrcData[dwSrcTotalOffset>>2]);

				// Update destination horizontal offset
				dwDstHorizontalOffset += _bpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_iBpp = _bpp;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
		m_bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_dwSize;
		m_bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		m_bih.biBitCount = _BITS_PER_PIXEL_24;
	}
}

void CBitmapEx::_ScaleNearestNeighbour(long horizontalPercent, long verticalPercent)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _horizontalPercent = max(1, horizontalPercent);
		long _verticalPercent = max(1, verticalPercent);
		float dx = 100.0f / (float)_horizontalPercent;
		float dy = 100.0f / (float)_verticalPercent;
		long _width = (long)((1.0f/dx) * (float)m_bih.biWidth + 0.5f);
		long _height = (long)((1.0f/dy) * (float)m_bih.biHeight + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_ScaleBilinear(long horizontalPercent, long verticalPercent)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _horizontalPercent = max(1, horizontalPercent);
		long _verticalPercent = max(1, verticalPercent);
		float dx = 100.0f / (float)_horizontalPercent;
		float dy = 100.0f / (float)_verticalPercent;
		long _width = (long)((1.0f/dx) * (float)m_bih.biWidth + 0.5f);
		long _height = (long)((1.0f/dy) * (float)m_bih.biHeight + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
				if (n >= m_bih.biWidth-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
				if (m >= m_bih.biHeight-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
				if ((n >= m_bih.biWidth-1) || (m >= m_bih.biHeight-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_ScaleBicubic(long horizontalPercent, long verticalPercent)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _horizontalPercent = max(1, horizontalPercent);
		long _verticalPercent = max(1, verticalPercent);
		float dx = 100.0f / (float)_horizontalPercent;
		float dy = 100.0f / (float)_verticalPercent;
		long _width = (long)((1.0f/dx) * (float)m_bih.biWidth + 0.5f);
		long _height = (long)((1.0f/dy) * (float)m_bih.biHeight + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
				if ((m < 1) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
				if ((m < 1) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
				if (n >= m_bih.biWidth-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
				if (n >= m_bih.biWidth-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
				if (m >= m_bih.biHeight-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
				if (m >= m_bih.biHeight-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
				BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
				BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::Scale(long horizontalPercent, long verticalPercent)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Scale bitmap using nearest neighbour interpolation algorithm
				_ScaleNearestNeighbour(horizontalPercent, verticalPercent);
			}
			break;

		case RM_BILINEAR:
			{
				// Scale bitmap using bilinear interpolation algorithm
				_ScaleBilinear(horizontalPercent, verticalPercent);
			}
			break;

		case RM_BICUBIC:
			{
				// Scale bitmap using bicubic interpolation algorithm
				_ScaleBicubic(horizontalPercent, verticalPercent);
			}
			break;
	}
}

void CBitmapEx::_ScaleNearestNeighbour2(long width, long height)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _width = max(1, width);
		long _height = max(1, height);
		float dx = (float)m_bih.biWidth / (float)_width;
		float dy = (float)m_bih.biHeight / (float)_height;
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_ScaleBilinear2(long width, long height)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _width = max(1, width);
		long _height = max(1, height);
		float dx = (float)m_bih.biWidth / (float)_width;
		float dy = (float)m_bih.biHeight / (float)_height;
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
				if (n >= m_bih.biWidth-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
				if (m >= m_bih.biHeight-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
				if ((n >= m_bih.biWidth-1) || (m >= m_bih.biHeight-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_ScaleBicubic2(long width, long height)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _width = max(1, width);
		long _height = max(1, height);
		float dx = (float)m_bih.biWidth / (float)_width;
		float dy = (float)m_bih.biHeight / (float)_height;
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
				if ((m < 1) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
				if ((m < 1) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
				if (n >= m_bih.biWidth-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
				if (n >= m_bih.biWidth-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
				if (m >= m_bih.biHeight-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
				if (m >= m_bih.biHeight-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
				BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
				BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::Scale2(long width, long height)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Scale bitmap using nearest neighbour interpolation algorithm
				_ScaleNearestNeighbour2(width, height);
			}
			break;

		case RM_BILINEAR:
			{
				// Scale bitmap using bilinear interpolation algorithm
				_ScaleBilinear2(width, height);
			}
			break;

		case RM_BICUBIC:
			{
				// Scale bitmap using bicubic interpolation algorithm
				_ScaleBicubic2(width, height);
			}
			break;
	}
}

void CBitmapEx::_RotateNearestNeighbour(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate rotating params
		float _angle = ((float)-degrees/180.0f) * _PI;
		long _width = (long)(abs((float)m_bih.biWidth*cos(_angle)) + abs((float)m_bih.biHeight*sin(_angle)) + 0.5f);
		long _height = (long)(abs((float)m_bih.biWidth*sin(_angle)) + abs((float)m_bih.biHeight*cos(_angle)) + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_0_5 = ftofx(0.5f);
		fixed f_H = itofx(m_bih.biHeight/2);
		fixed f_W = itofx(m_bih.biWidth/2);
		fixed f_cos = ftofx(cos(-_angle));
		fixed f_sin = ftofx(sin(-_angle));

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rotate bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i-_height/2);
				fixed f_j = itofx(j-_width/2);
				fixed f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
				fixed f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
				long m = fxtoi(f_m);
				long n = fxtoi(f_n);
				if ((m > 0) && (m < m_bih.biHeight-1) && (n > 0) && (n < m_bih.biWidth-1))
				{
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_RotateBilinear(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate rotating params
		float _angle = ((float)-degrees/180.0f) * _PI;
		long _width = (long)(abs((float)(m_bih.biWidth*cos(_angle))) + abs((float)(m_bih.biHeight*sin(_angle))) + 0.5f);
		long _height = (long)(abs((float)(m_bih.biWidth*sin(_angle))) + abs((float)(m_bih.biHeight*cos(_angle))) + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_0_5 = ftofx(0.5f);
		fixed f_H = itofx(m_bih.biHeight/2);
		fixed f_W = itofx(m_bih.biWidth/2);
		fixed f_cos = ftofx(cos(-_angle));
		fixed f_sin = ftofx(sin(-_angle));
		fixed f_1 = itofx(1);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rotate bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i-_height/2);
				fixed f_j = itofx(j-_width/2);
				fixed f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
				fixed f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
				long m = fxtoi(f_m);
				long n = fxtoi(f_n);
				if ((m > 0) && (m < m_bih.biHeight-1) && (n > 0) && (n < m_bih.biWidth-1))
				{
					fixed f_f = f_m - itofx(m);
					fixed f_g = f_n - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((n >= m_bih.biWidth-1) || (m >= m_bih.biHeight-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					fixed f_red = Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4);
					fixed f_green = Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4);
					fixed f_blue = Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4);
					BYTE red = (BYTE)max(0, min(255, fxtoi(f_red)));
					BYTE green = (BYTE)max(0, min(255, fxtoi(f_green)));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(f_blue)));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_RotateBicubic(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate rotating params
		float _angle = ((float)-degrees/180.0f) * _PI;
		long _width = (long)(abs((float)m_bih.biWidth*cos(_angle)) + abs((float)m_bih.biHeight*sin(_angle)) + 0.5f);
		long _height = (long)(abs((float)m_bih.biWidth*sin(_angle)) + abs((float)m_bih.biHeight*cos(_angle)) + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_0_5 = ftofx(0.5f);
		fixed f_H = itofx(m_bih.biHeight/2);
		fixed f_W = itofx(m_bih.biWidth/2);
		fixed f_cos = ftofx(cos(-_angle));
		fixed f_sin = ftofx(sin(-_angle));
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rotate bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i-_height/2);
				fixed f_j = itofx(j-_width/2);
				fixed f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
				fixed f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
				long m = fxtoi(f_m);
				long n = fxtoi(f_n);
				if ((m > 0) && (m < m_bih.biHeight-1) && (n > 0) && (n < m_bih.biWidth-1))
				{
					fixed f_f = f_m - itofx(m);
					fixed f_g = f_n - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
					if (n >= m_bih.biWidth-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
					if (m >= m_bih.biHeight-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
					BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::Rotate(long degrees, _PIXEL bgColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Rotate bitmap using nearest neighbour interpolation algorithm
				_RotateNearestNeighbour(degrees, bgColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Rotate bitmap using bilinear interpolation algorithm
				_RotateBilinear(degrees, bgColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Rotate bitmap using bicubic interpolation algorithm
				_RotateBicubic(degrees, bgColor);
			}
			break;
	}
}

void CBitmapEx::Crop(long x, long y, long width, long height)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate cropping params
		long _y1 = max(0, min(m_bih.biHeight, y));
		long _x1 = max(0, min(m_bih.biWidth, x));
		long _y2 = max(0, min(m_bih.biHeight, height+_y1));
		long _x2 = max(0, min(m_bih.biWidth, width+_x1));
		long _height = _y2 - _y1;
		long _width = _x2 - _x1;

		// Check cropping region
		if ((_width == 0) || (_height == 0))
			return;

		// Create temporary bitmap
		long iPitch = _width * m_iBpp;
		while ((iPitch & 3) != 0)
			iPitch++;
		DWORD dwSize = iPitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Crop bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		DWORD dwSrcHorizontalStartOffset = _x1 * m_iBpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (m_bih.biHeight-_y2) * m_iPitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		m_dwSize = dwSize;
		m_iPitch = iPitch;
		m_bih.biHeight = _height;
		m_bih.biWidth = _width;
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Shear(long degreesX, long degreesY, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate shearing params
		long _degreesY = max(-89, min(89, degreesY));
		long _degreesX = max(-89, min(89, degreesX));

		// Shear bitmap horizontaly
		_ShearHorizontal(_degreesX, bgColor);

		// Shear bitmap verticaly
		_ShearVertical(_degreesY, bgColor);
	}
}

void CBitmapEx::_ShearVerticalNearestNeighbour(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate shearing params
		float _alpha = tan((float)degrees/180.0f*_PI);
		long _offset = (long)((float)m_bih.biWidth * _alpha + 0.5f);
		long _height = m_bih.biHeight + abs(_offset);
		long _width = m_bih.biWidth;
		fixed f_alpha = ftofx(_alpha);
		fixed f_offset = ftofx(_offset);

		// Create temporary bitmap
		long _pitch = _width * m_iBpp;
		while ((_pitch & 3) != 0)
			_pitch++;
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Shear bitmap verticaly
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_y;
				if (f_offset >= 0)
					f_y = f_i - Mulfx(f_j,f_alpha);
				else
					f_y = (f_i+f_offset) - Mulfx(f_j,f_alpha);
				fixed f_x = f_j;
				long m = fxtoi(f_y);
				long n = fxtoi(f_x);
				if ((m >= 0) && (m <= m_bih.biHeight-1) && (n >= 0) && (n <= m_bih.biWidth-1))
				{
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_dwSize = dwSize;
		m_iPitch = _pitch;
		m_bih.biHeight = _height;
		m_bih.biWidth = _width;
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::_ShearVerticalBilinear(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate shearing params
		float _alpha = tan((float)degrees/180.0f*_PI);
		long _offset = (long)((float)m_bih.biWidth * _alpha + 0.5f);
		long _height = m_bih.biHeight + abs(_offset);
		long _width = m_bih.biWidth;
		fixed f_alpha = ftofx(_alpha);
		fixed f_offset = ftofx(_offset);
		fixed f_1 = itofx(1);

		// Create temporary bitmap
		long _pitch = _width * m_iBpp;
		while ((_pitch & 3) != 0)
			_pitch++;
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Shear bitmap verticaly
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_y;
				if (f_offset >= 0)
					f_y = f_i - Mulfx(f_j,f_alpha);
				else
					f_y = (f_i+f_offset) - Mulfx(f_j,f_alpha);
				fixed f_x = f_j;
				long m = fxtoi(f_y);
				long n = fxtoi(f_x);
				if ((m >= 0) && (m <= m_bih.biHeight-1) && (n >= 0) && (n <= m_bih.biWidth-1))
				{
					fixed f_f = f_y - itofx(m);
					fixed f_g = f_x - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((n >= m_bih.biWidth-1) || (m >= m_bih.biHeight-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					fixed f_red = Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4);
					fixed f_green = Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4);
					fixed f_blue = Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4);
					BYTE red = (BYTE)max(0, min(255, fxtoi(f_red)));
					BYTE green = (BYTE)max(0, min(255, fxtoi(f_green)));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(f_blue)));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_dwSize = dwSize;
		m_iPitch = _pitch;
		m_bih.biHeight = _height;
		m_bih.biWidth = _width;
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::_ShearVerticalBicubic(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate shearing params
		float _alpha = tan((float)degrees/180.0f*_PI);
		long _offset = (long)((float)m_bih.biWidth * _alpha + 0.5f);
		long _height = m_bih.biHeight + abs(_offset);
		long _width = m_bih.biWidth;
		fixed f_alpha = ftofx(_alpha);
		fixed f_offset = ftofx(_offset);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Create temporary bitmap
		long _pitch = _width * m_iBpp;
		while ((_pitch & 3) != 0)
			_pitch++;
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Shear bitmap verticaly
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_y;
				if (f_offset >= 0)
					f_y = f_i - Mulfx(f_j,f_alpha);
				else
					f_y = (f_i+f_offset) - Mulfx(f_j,f_alpha);
				fixed f_x = f_j;
				long m = fxtoi(f_y);
				long n = fxtoi(f_x);
				if ((m >= 0) && (m <= m_bih.biHeight-1) && (n >= 0) && (n <= m_bih.biWidth-1))
				{
					fixed f_f = f_y - itofx(m);
					fixed f_g = f_x - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
					if (n >= m_bih.biWidth-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
					if (m >= m_bih.biHeight-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
					BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_dwSize = dwSize;
		m_iPitch = _pitch;
		m_bih.biHeight = _height;
		m_bih.biWidth = _width;
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::_ShearVertical(long degrees, _PIXEL bgColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Shear bitmap verticaly using nearest neighbour interpolation algorithm
				_ShearVerticalNearestNeighbour(degrees, bgColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Shear bitmap verticaly using bilinear interpolation algorithm
				_ShearVerticalBilinear(degrees, bgColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Shear bitmap verticaly using bicubic interpolation algorithm
				_ShearVerticalBicubic(degrees, bgColor);
			}
			break;
	}
}

void CBitmapEx::_ShearHorizontalNearestNeighbour(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate shearing params
		float _alpha = tan((float)degrees/180.0f*_PI);
		long _offset = (long)((float)m_bih.biHeight * _alpha + 0.5f);
		long _height = m_bih.biHeight;
		long _width = m_bih.biWidth + abs(_offset);
		fixed f_alpha = ftofx(_alpha);
		fixed f_offset = ftofx(_offset);

		// Create temporary bitmap
		long _pitch = _width * m_iBpp;
		while ((_pitch & 3) != 0)
			_pitch++;
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Shear bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_y = f_i;
				fixed f_x;
				if (f_offset >= 0)
					f_x = f_j - Mulfx(f_i,f_alpha);
				else
					f_x = (f_j+f_offset) - Mulfx(f_i,f_alpha);
				long m = fxtoi(f_y);
				long n = fxtoi(f_x);
				if ((m >= 0) && (m <= m_bih.biHeight-1) && (n >= 0) && (n <= m_bih.biWidth-1))
				{
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_dwSize = dwSize;
		m_iPitch = _pitch;
		m_bih.biHeight = _height;
		m_bih.biWidth = _width;
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::_ShearHorizontalBilinear(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate shearing params
		float _alpha = tan((float)degrees/180.0f*_PI);
		long _offset = (long)((float)m_bih.biHeight * _alpha + 0.5f);
		long _height = m_bih.biHeight;
		long _width = m_bih.biWidth + abs(_offset);
		fixed f_alpha = ftofx(_alpha);
		fixed f_offset = ftofx(_offset);
		fixed f_1 = itofx(1);

		// Create temporary bitmap
		long _pitch = _width * m_iBpp;
		while ((_pitch & 3) != 0)
			_pitch++;
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Shear bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_y = f_i;
				fixed f_x;
				if (f_offset >= 0)
					f_x = f_j - Mulfx(f_i,f_alpha);
				else
					f_x = (f_j+f_offset) - Mulfx(f_i,f_alpha);
				long m = fxtoi(f_y);
				long n = fxtoi(f_x);
				if ((m >= 0) && (m <= m_bih.biHeight-1) && (n >= 0) && (n <= m_bih.biWidth-1))
				{
					fixed f_f = f_y - itofx(m);
					fixed f_g = f_x - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((n >= m_bih.biWidth-1) || (m >= m_bih.biHeight-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					fixed f_red = Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4);
					fixed f_green = Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4);
					fixed f_blue = Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4);
					BYTE red = (BYTE)max(0, min(255, fxtoi(f_red)));
					BYTE green = (BYTE)max(0, min(255, fxtoi(f_green)));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(f_blue)));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_dwSize = dwSize;
		m_iPitch = _pitch;
		m_bih.biHeight = _height;
		m_bih.biWidth = _width;
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::_ShearHorizontalBicubic(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate shearing params
		float _alpha = tan((float)degrees/180.0f*_PI);
		long _offset = (long)((float)m_bih.biHeight * _alpha + 0.5f);
		long _height = m_bih.biHeight;
		long _width = m_bih.biWidth + abs(_offset);
		fixed f_alpha = ftofx(_alpha);
		fixed f_offset = ftofx(_offset);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Create temporary bitmap
		long _pitch = _width * m_iBpp;
		while ((_pitch & 3) != 0)
			_pitch++;
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Shear bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_y = f_i;
				fixed f_x;
				if (f_offset >= 0)
					f_x = f_j - Mulfx(f_i,f_alpha);
				else
					f_x = (f_j+f_offset) - Mulfx(f_i,f_alpha);
				long m = fxtoi(f_y);
				long n = fxtoi(f_x);
				if ((m >= 0) && (m <= m_bih.biHeight-1) && (n >= 0) && (n <= m_bih.biWidth-1))
				{
					fixed f_f = f_y - itofx(m);
					fixed f_g = f_x - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
					if (n >= m_bih.biWidth-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
					if (m >= m_bih.biHeight-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
					BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_dwSize = dwSize;
		m_iPitch = _pitch;
		m_bih.biHeight = _height;
		m_bih.biWidth = _width;
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::_ShearHorizontal(long degrees, _PIXEL bgColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Shear bitmap horizontaly using nearest neighbour interpolation algorithm
				_ShearHorizontalNearestNeighbour(degrees, bgColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Shear bitmap horizontaly using bilinear interpolation algorithm
				_ShearHorizontalBilinear(degrees, bgColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Shear bitmap horizontaly using bicubic interpolation algorithm
				_ShearHorizontalBicubic(degrees, bgColor);
			}
			break;
	}
}

void CBitmapEx::FlipHorizontal()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate flipping params
		long _width = m_bih.biWidth / 2;

		// Flip bitmap horizontaly
		DWORD dwDstHorizontalStartOffset = 0;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = (m_bih.biWidth-1) * m_iBpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixelTmp = lpDstData[dwDstTotalOffset>>2];
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
				lpSrcData[dwSrcTotalOffset>>2] = pixelTmp;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset -= m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::FlipVertical()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate flipping params
		long _height = m_bih.biHeight / 2;

		// Flip bitmap verticaly
		DWORD dwDstTotalOffset = 0;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = (m_bih.biHeight-1) * m_iPitch;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpTmpData = (LPDWORD)malloc(m_iPitch*sizeof(BYTE));
		for (long i=0; i<_height; i++)
		{
			// Update bitmap
			memcpy(lpTmpData, lpDstData+(dwDstTotalOffset>>2), m_iPitch*sizeof(BYTE));
			memcpy(lpDstData+(dwDstTotalOffset>>2), lpSrcData+(dwSrcTotalOffset>>2), m_iPitch*sizeof(BYTE));
			memcpy(lpSrcData+(dwSrcTotalOffset>>2), lpTmpData, m_iPitch*sizeof(BYTE));

			// Update destination total offset
			dwDstTotalOffset += m_iPitch;

			// Update source total offset
			dwSrcTotalOffset -= m_iPitch;
		}
		free(lpTmpData);
	}
}

void CBitmapEx::MirrorLeft()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _width = m_bih.biWidth / 2;

		// Mirror bitmap left
		DWORD dwDstHorizontalStartOffset = (m_bih.biWidth-1) * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = 0;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset -= m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::MirrorRight()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _width = m_bih.biWidth / 2;

		// Mirror bitmap right
		DWORD dwDstHorizontalStartOffset = 0;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = (m_bih.biWidth-1) * m_iBpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset -= m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::MirrorTop()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _height = m_bih.biHeight / 2;

		// Mirror bitmap top
		DWORD dwDstTotalOffset = 0;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = (m_bih.biHeight-1) * m_iPitch;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<_height; i++)
		{
			// Update bitmap
			memcpy(lpDstData+(dwDstTotalOffset>>2), lpSrcData+(dwSrcTotalOffset>>2), m_iPitch*sizeof(BYTE));

			// Update destination total offset
			dwDstTotalOffset += m_iPitch;

			// Update source total offset
			dwSrcTotalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::MirrorBottom()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _height = m_bih.biHeight / 2;

		// Mirror bitmap bottom
		DWORD dwDstTotalOffset = (m_bih.biHeight-1) * m_iPitch;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<_height; i++)
		{
			// Update bitmap
			memcpy(lpDstData+(dwDstTotalOffset>>2), lpSrcData+(dwSrcTotalOffset>>2), m_iPitch*sizeof(BYTE));

			// Update destination total offset
			dwDstTotalOffset -= m_iPitch;

			// Update source total offset
			dwSrcTotalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::Clear(_PIXEL clearColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
/*		// Clear bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				lpDstData[dwTotalOffset>>2] = clearColor;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Clear bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		__asm {
			mov edi, lpDstData
			mov eax, clearColor
			mov ecx, dwSize
			rep stosd
		}
	}
}

void CBitmapEx::Negative()
{
	// Check for valid bitmap
	if (IsValid())
	{
/*		// Negative bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				_PIXEL oldPixel = lpDstData[dwTotalOffset>>2];
				_PIXEL newPixel = _RGB(255-_GetRValue(oldPixel),255-_GetGValue(oldPixel),255-_GetBValue(oldPixel));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Negative bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		__asm {
			mov edi, lpDstData
			mov ecx, 0
label_loop:
			mov eax, [edi]
			xor eax, 0x00FFFFFF
			mov [edi], eax
			add edi, 4
			inc ecx
			cmp ecx, dwSize
			jl label_loop
		}
	}
}

void CBitmapEx::Grayscale()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate grayscale params
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);

/*		// Grayscale bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
				fixed f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
				fixed f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_value),fxtoi(f_value),fxtoi(f_value));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Grayscale bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		__asm {
			mov edi, lpDstData
			mov ecx, 0
label_loop:
			mov ebx, [edi]
			mov eax, ebx
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			push eax
			xor edx, edx
			pop eax
			mov edx, eax
			pop eax
			add edx, eax
			pop eax
			add edx, eax
			shr edx, 8
			and edx, 0x000000FF
			mov eax, edx
			shl eax, 8
			or eax, edx
			shl eax, 8
			or eax, edx
			mov [edi], eax
			add edi, 4
			inc ecx
			cmp ecx, dwSize
			jl label_loop
		}
	}
}

void CBitmapEx::Sepia(long depth)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate sepia params
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);
		long _depth = max(0, min(100, depth));
		fixed f_depth = itofx(_depth);
		fixed f_depth2 = Mulfx(itofx(2),f_depth);

/*		// Sepia bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
				fixed f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
				fixed f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				BYTE red = (BYTE)fxtoi(f_value+f_depth2);
				if (red < ((_depth<<1)-1))
					red = 255;
				BYTE green = (BYTE)fxtoi(f_value+f_depth);
				if (green < (_depth-1))
					green = 255;
				BYTE blue = (BYTE)fxtoi(f_value);
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Sepia bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		DWORD _depthm1 = _depth - 1;
		DWORD _depth2 = _depth << 1;
		DWORD _depth2m1 = _depth2 - 1;
		__asm {
			mov edi, lpDstData
			mov ecx, 0
label_loop:
			mov ebx, [edi]
			mov eax, ebx
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			push eax
			xor edx, edx
			pop eax
			mov edx, eax
			pop eax
			add edx, eax
			pop eax
			add edx, eax
			shr edx, 8
			and edx, 0x000000FF
			xor eax, eax
			mov ebx, edx
			add ebx, _depth2
			and ebx, 0x000000FF
			cmp ebx, _depth2m1
			jge next1
			mov ebx, 255
next1:		mov eax, ebx
			mov ebx, edx
			add ebx, _depth
			and ebx, 0x000000FF
			cmp ebx, _depthm1
			jge next2
			mov ebx, 255
next2:		shl eax, 8
			or eax, ebx
			shl eax, 8
			or eax, edx
			mov [edi], eax
			add edi, 4
			inc ecx
			cmp ecx, dwSize
			jl label_loop
		}
	}
}

void CBitmapEx::Emboss()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate emboss params
		fixed f_w[9] = {itofx(-1), itofx(-1), itofx(0),
						itofx(-1), itofx(0), itofx(1),
						itofx(0), itofx(1), itofx(1)
		};
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);
		fixed f_128 = itofx(128);
		fixed f_255 = itofx(255);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Emboss bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffsets[9];
				dwSrcOffsets[0] = dwTotalOffset - m_iPitch - m_iBpp;
				if ((i < 1) || (j < 1))
					dwSrcOffsets[0] = dwTotalOffset;
				dwSrcOffsets[1] = dwTotalOffset - m_iPitch;
				if (i < 1)
					dwSrcOffsets[1] = dwTotalOffset;
				dwSrcOffsets[2] = dwTotalOffset - m_iPitch + m_iBpp;
				if ((i < 1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[2] = dwTotalOffset;
				dwSrcOffsets[3] = dwTotalOffset - m_iBpp;
				if (j < 1)
					dwSrcOffsets[3] = dwTotalOffset;
				dwSrcOffsets[4] = dwTotalOffset;
				dwSrcOffsets[5] = dwTotalOffset + m_iBpp;
				if (j >= m_bih.biWidth-1)
					dwSrcOffsets[5] = dwTotalOffset;
				dwSrcOffsets[6] = dwTotalOffset + m_iPitch - m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j < 1))
					dwSrcOffsets[6] = dwTotalOffset;
				dwSrcOffsets[7] = dwTotalOffset + m_iPitch;
				if (i >= m_bih.biHeight-1)
					dwSrcOffsets[7] = dwTotalOffset;
				dwSrcOffsets[8] = dwTotalOffset + m_iPitch + m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[8] = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=0; k<9; k++)
				{
					fixed f_r = itofx(_GetRValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_g = itofx(_GetGValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_b = itofx(_GetBValue(lpSrcData[dwSrcOffsets[k]>>2]));
					f_red += Mulfx(f_r,f_w[k]);
					f_green += Mulfx(f_g,f_w[k]);
					f_blue += Mulfx(f_b,f_w[k]);
				}
				f_red = max(0, min(f_255, f_red+f_128));
				f_green = max(0, min(f_255, f_green+f_128));
				f_blue = max(0, min(f_255, f_blue+f_128));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_value),fxtoi(f_value),fxtoi(f_value));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Engrave()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate engrave params
		fixed f_w[9] = {itofx(1), itofx(1), itofx(0),
						itofx(1), itofx(0), itofx(-1),
						itofx(0), itofx(-1), itofx(-1)
		};
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);
		fixed f_128 = itofx(128);
		fixed f_255 = itofx(255);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Engrave bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffsets[9];
				dwSrcOffsets[0] = dwTotalOffset - m_iPitch - m_iBpp;
				if ((i < 1) || (j < 1))
					dwSrcOffsets[0] = dwTotalOffset;
				dwSrcOffsets[1] = dwTotalOffset - m_iPitch;
				if (i < 1)
					dwSrcOffsets[1] = dwTotalOffset;
				dwSrcOffsets[2] = dwTotalOffset - m_iPitch + m_iBpp;
				if ((i < 1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[2] = dwTotalOffset;
				dwSrcOffsets[3] = dwTotalOffset - m_iBpp;
				if (j < 1)
					dwSrcOffsets[3] = dwTotalOffset;
				dwSrcOffsets[4] = dwTotalOffset;
				dwSrcOffsets[5] = dwTotalOffset + m_iBpp;
				if (j >= m_bih.biWidth-1)
					dwSrcOffsets[5] = dwTotalOffset;
				dwSrcOffsets[6] = dwTotalOffset + m_iPitch - m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j < 1))
					dwSrcOffsets[6] = dwTotalOffset;
				dwSrcOffsets[7] = dwTotalOffset + m_iPitch;
				if (i >= m_bih.biHeight-1)
					dwSrcOffsets[7] = dwTotalOffset;
				dwSrcOffsets[8] = dwTotalOffset + m_iPitch + m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[8] = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=0; k<9; k++)
				{
					fixed f_r = itofx(_GetRValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_g = itofx(_GetGValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_b = itofx(_GetBValue(lpSrcData[dwSrcOffsets[k]>>2]));
					f_red += Mulfx(f_r,f_w[k]);
					f_green += Mulfx(f_g,f_w[k]);
					f_blue += Mulfx(f_b,f_w[k]);
				}
				f_red = max(0, min(f_255, f_red+f_128));
				f_green = max(0, min(f_255, f_green+f_128));
				f_blue = max(0, min(f_255, f_blue+f_128));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_value),fxtoi(f_value),fxtoi(f_value));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Pixelize(long size)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate pixelize params
		long _size = max(1, min((m_bih.biWidth>>4), size));
		fixed f_size = Divfx(itofx(1),itofx(_size*_size));

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Pixelize bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i+=_size)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j+=_size)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=0; k<_size; k++)
				{
					long m = i + k;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=0; l<_size; l++)
					{
						long n = j + l;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						f_red += itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
						f_green += itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
						f_blue += itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
					}
				}
				f_red = Mulfx(f_size,f_red);
				f_green = Mulfx(f_size,f_green);
				f_blue = Mulfx(f_size,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_red),fxtoi(f_green),fxtoi(f_blue));
				for (long k=0; k<_size; k++)
				{
					long m = i + k;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=0; l<_size; l++)
					{
						long n = j + l;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						lpDstData[dwSrcOffset>>2] = newPixel;
					}
				}

				// Update horizontal offset
				dwHorizontalOffset += (_size*m_iBpp);
			}

			// Update vertical offset
			dwVerticalOffset += (_size*m_iPitch);
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Brightness(long brightness)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate brightness params
		long _brightness = max(-255, min(255, brightness));

		// Change bitmap brightness
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				BYTE red = _GetRValue(lpDstData[dwTotalOffset>>2]);
				BYTE green = _GetGValue(lpDstData[dwTotalOffset>>2]);
				BYTE blue = _GetBValue(lpDstData[dwTotalOffset>>2]);
				red = (BYTE)max(0, min(red+_brightness, 255));
				green = (BYTE)max(0, min(green+_brightness, 255));
				blue = (BYTE)max(0, min(blue+_brightness, 255));
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::Contrast(long contrast)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate contrast params
		long _contrast = max(1, min(100, contrast));
		fixed f_contrast = ftofx(1.0f/_contrast);
		fixed f_128 = itofx(128);

		// Change bitmap contrast
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
				fixed f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
				fixed f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
				f_red = Mulfx(f_red-f_128, f_contrast) + f_128;
				f_green = Mulfx(f_green-f_128, f_contrast) + f_128;
				f_blue = Mulfx(f_blue-f_128, f_contrast) + f_128;
				BYTE red = (BYTE)max(0, min(fxtoi(f_red), 255));
				BYTE green = (BYTE)max(0, min(fxtoi(f_green), 255));
				BYTE blue = (BYTE)max(0, min(fxtoi(f_blue), 255));
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::Blur()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate blur params
		fixed f_9 = itofx(9);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Blur bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<=1; k++)
				{
					long m = i + k;
					if (m < 0)
						m = 0;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=-1; l<=1; l++)
					{
						long n = j + l;
						if (n < 0)
							n = 0;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						f_red += itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
						f_green += itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
						f_blue += itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
					}
				}
				BYTE red = (BYTE)max(0, min(fxtoi(Divfx(f_red,f_9)), 255));
				BYTE green = (BYTE)max(0, min(fxtoi(Divfx(f_green,f_9)), 255));
				BYTE blue = (BYTE)max(0, min(fxtoi(Divfx(f_blue,f_9)), 255));
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::GaussianBlur()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate blur params
		fixed f_16 = itofx(16);
		fixed f_4 = itofx(4);
		fixed f_2 = itofx(2);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Gaussian blur bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<=1; k++)
				{
					long m = i + k;
					if (m < 0)
						m = 0;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=-1; l<=1; l++)
					{
						long n = j + l;
						if (n < 0)
							n = 0;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						if ((k == 0) && (l == 0))
						{
							f_red += Mulfx(itofx(_GetRValue(lpSrcData[dwSrcOffset>>2])),f_4);
							f_green += Mulfx(itofx(_GetGValue(lpSrcData[dwSrcOffset>>2])),f_4);
							f_blue += Mulfx(itofx(_GetBValue(lpSrcData[dwSrcOffset>>2])),f_4);
						}
						else if (((k == -1) && (l == 0)) || ((k == 0) && (l == -1)) || ((k == 0) && (l == 1)) || ((k == 1) && (l == 0)))
						{
							f_red += Mulfx(itofx(_GetRValue(lpSrcData[dwSrcOffset>>2])),f_2);
							f_green += Mulfx(itofx(_GetGValue(lpSrcData[dwSrcOffset>>2])),f_2);
							f_blue += Mulfx(itofx(_GetBValue(lpSrcData[dwSrcOffset>>2])),f_2);
						}
						else
						{
							f_red += itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
							f_green += itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
							f_blue += itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
						}
					}
				}
				BYTE red = (BYTE)max(0, min(fxtoi(Divfx(f_red,f_16)), 255));
				BYTE green = (BYTE)max(0, min(fxtoi(Divfx(f_green,f_16)), 255));
				BYTE blue = (BYTE)max(0, min(fxtoi(Divfx(f_blue,f_16)), 255));
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Sharp()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate sharp params
		fixed f_6 = ftofx(6.0f);
		fixed f_1_2 = ftofx(1.0f/2.0f);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Sharp bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<=1; k++)
				{
					long m = i + k;
					if (m < 0)
						m = 0;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=-1; l<=1; l++)
					{
						long n = j + l;
						if (n < 0)
							n = 0;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						if ((k == 0) && (l == 0))
						{
							f_red += Mulfx(itofx(_GetRValue(lpSrcData[dwSrcOffset>>2])),f_6);
							f_green += Mulfx(itofx(_GetGValue(lpSrcData[dwSrcOffset>>2])),f_6);
							f_blue += Mulfx(itofx(_GetBValue(lpSrcData[dwSrcOffset>>2])),f_6);
						}
						else if (((k == -1) && (l == 0)) || ((k == 0) && (l == -1)) || ((k == 0) && (l == 1)) || ((k == 1) && (l == 0)))
						{
							f_red -= itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
							f_green -= itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
							f_blue -= itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
						}
					}
				}
				BYTE red = (BYTE)max(0, min(fxtoi(Mulfx(f_red,f_1_2)), 255));
				BYTE green = (BYTE)max(0, min(fxtoi(Mulfx(f_green,f_1_2)), 255));
				BYTE blue = (BYTE)max(0, min(fxtoi(Mulfx(f_blue,f_1_2)), 255));
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Colorize(_PIXEL color)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate colorization params
		fixed f_sred = itofx(_GetRValue(color));
		fixed f_sgreen = itofx(_GetGValue(color));
		fixed f_sblue = itofx(_GetBValue(color));
		_PIXEL _sColor = _RGB2HSV(color);
		fixed f_value = Divfx(itofx(_GetBValue(_sColor)), itofx(255));

		// Colorize bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_dred = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
				f_dred = f_dred + Mulfx(f_value, f_sred-f_dred);
				f_dgreen = f_dgreen + Mulfx(f_value, f_sgreen-f_dgreen);
				f_dblue = f_dblue + Mulfx(f_value, f_sblue-f_dblue);
				lpDstData[dwTotalOffset>>2] = _RGB(fxtoi(f_dred), fxtoi(f_dgreen), fxtoi(f_dblue));

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::Rank(BOOL bMinimum)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate rank params
		fixed f_255 = itofx(255);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rank bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_redMin=f_255, f_redMax=0;
				fixed f_greenMin=f_255, f_greenMax=0;
				fixed f_blueMin=f_255, f_blueMax=0;
				for (long k=0; k<3; k++)
				{
					long m = i + k;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=0; l<3; l++)
					{
						long n = j + l;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						fixed f_red = itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
						fixed f_green = itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
						fixed f_blue = itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
						if (bMinimum)
						{
							if (f_red < f_redMin)
								f_redMin = f_red;
							if (f_green < f_greenMin)
								f_greenMin = f_green;
							if (f_blue < f_blueMin)
								f_blueMin = f_blue;
						}
						else
						{
							if (f_red > f_redMax)
								f_redMax = f_red;
							if (f_green > f_greenMax)
								f_greenMax = f_green;
							if (f_blue > f_blueMax)
								f_blueMax = f_blue;
						}
					}
				}
				if (bMinimum)
					lpDstData[dwTotalOffset>>2] = _RGB(fxtoi(f_redMin), fxtoi(f_greenMin), fxtoi(f_blueMin));
				else
					lpDstData[dwTotalOffset>>2] = _RGB(fxtoi(f_redMax), fxtoi(f_greenMax), fxtoi(f_blueMax));

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Spread(long distanceX, long distanceY)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate spread params
		long _distanceY = max(1, min(m_bih.biHeight/2-1, distanceY));
		long _distanceX = max(1, min(m_bih.biWidth/2-1, distanceX));

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rank bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				long m = i + (rand()%_distanceY) - _distanceY/2;
				if (m < 0)
					m = 0;
				else if (m >= m_bih.biHeight-1)
					m = m_bih.biHeight - 1;
				long n = j + (rand()%_distanceX) - _distanceX/2;
				if (n < 0)
					n = 0;
				else if (n >= m_bih.biWidth-1)
					n = m_bih.biWidth - 1;
				DWORD dwDstOffset = m*m_iPitch + n*m_iBpp;
				lpDstData[dwDstOffset>>2] = lpSrcData[dwSrcOffset>>2];
				lpDstData[dwSrcOffset>>2] = lpSrcData[dwDstOffset>>2];

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Offset(long offsetX, long offsetY)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate offset params
		long _offsetY = max(-m_bih.biHeight/2+1, min(m_bih.biHeight/2-1, offsetY));
		long _offsetX = max(-m_bih.biWidth/2+1, min(m_bih.biWidth/2-1, offsetX));

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rank bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				long m = (i + _offsetY) % m_bih.biHeight;
				long n = (j + _offsetX) % m_bih.biWidth;
				DWORD dwSrcOffset = m*m_iPitch + n*m_iBpp;
				lpDstData[dwTotalOffset>>2] = lpSrcData[dwSrcOffset>>2];

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::BlackAndWhite(long offset)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate black-and-white params
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);
		long _offset = max(0, min(255, offset));
		fixed f_offset = itofx(_offset);

		// Black-And-White bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
				fixed f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
				fixed f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				if (f_value > f_offset)
					lpDstData[dwTotalOffset>>2] = _RGB(0,0,0);
				else
					lpDstData[dwTotalOffset>>2] = _RGB(255,255,255);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::EdgeDetect()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate blur params
		fixed f_8 = itofx(8);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Edge detect bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<=1; k++)
				{
					long m = i + k;
					if (m < 0)
						m = 0;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=-1; l<=1; l++)
					{
						long n = j + l;
						if (n < 0)
							n = 0;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						if ((k == 0) && (l == 0))
						{
							f_red += Mulfx(itofx(_GetRValue(lpSrcData[dwSrcOffset>>2])),f_8);
							f_green += Mulfx(itofx(_GetGValue(lpSrcData[dwSrcOffset>>2])),f_8);
							f_blue += Mulfx(itofx(_GetBValue(lpSrcData[dwSrcOffset>>2])),f_8);
						}
						else
						{
							f_red -= itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
							f_green -= itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
							f_blue -= itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
						}
					}
				}
				BYTE red = (BYTE)max(0, min(fxtoi(f_red), 255));
				BYTE green = (BYTE)max(0, min(fxtoi(f_green), 255));
				BYTE blue = (BYTE)max(0, min(fxtoi(f_blue), 255));
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::GlowingEdges(long blur, long threshold, long scale)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate glowing edge params
		float _threshold = max(0, min(100, threshold)) / 100.0f;
		long _scale = max(0, min(100, scale));
		fixed f_treshold = ftofx(_threshold);
		fixed f_scale = ftofx(_scale);
		fixed f_255 = itofx(255);

		// Create temporary bitmap
		CBitmapEx tempBitmap;
		tempBitmap.Create(this);
		for (long k=0; k<blur; k++)
			tempBitmap.GaussianBlur();
		tempBitmap.EdgeDetect();

		// Glowing edge bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD)tempBitmap.GetData();
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = Divfx(itofx(_GetRValue(lpSrcData[dwTotalOffset>>2])),f_255);
				fixed f_green = Divfx(itofx(_GetGValue(lpSrcData[dwTotalOffset>>2])),f_255);
				fixed f_blue = Divfx(itofx(_GetBValue(lpSrcData[dwTotalOffset>>2])),f_255);
				if ((f_red > f_treshold) || (f_green > f_treshold) || (f_blue > f_treshold))
				{
					f_red = Mulfx(f_red,f_scale);
					f_green = Mulfx(f_green,f_scale);
					f_blue = Mulfx(f_blue,f_scale);
				}
				f_red = min(f_255, Mulfx(f_red,f_255));
				f_green = min(f_255, Mulfx(f_green,f_255));
				f_blue = min(f_255, Mulfx(f_blue,f_255));
				lpDstData[dwTotalOffset>>2] =_RGB(fxtoi(f_red), fxtoi(f_green), fxtoi(f_blue));

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::EqualizeHistogram(long levels)
{
	// Check for valid bitmap
	if (IsValid())
	{
		long i, j, k;

		// Convert image to HSV colro model
		ConvertToHSV();

		// Calculate histogram equalization params
		long _levels = max(0, min(255, levels));

		// Build histogram
		long lpRedHistogram[256], lpGreenHistogram[256], lpBlueHistogram[256];
		memset(lpRedHistogram, 0, 256*sizeof(long));
		memset(lpGreenHistogram, 0, 256*sizeof(long));
		memset(lpBlueHistogram, 0, 256*sizeof(long));
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update channel histograms
				BYTE red = _GetRValue(lpSrcData[dwTotalOffset>>2]);
				BYTE green = _GetGValue(lpSrcData[dwTotalOffset>>2]);
				BYTE blue = _GetBValue(lpSrcData[dwTotalOffset>>2]);
				lpRedHistogram[red]++;
				lpGreenHistogram[green]++;
				lpBlueHistogram[blue]++;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Build cumulative frequency
		long lpRedCumulativeFrequency[256], lpGreenCumulativeFrequency[256], lpBlueCumulativeFrequency[256];
		memset(lpRedCumulativeFrequency, 0, 256*sizeof(long));
		memset(lpGreenCumulativeFrequency, 0, 256*sizeof(long));
		memset(lpBlueCumulativeFrequency, 0, 256*sizeof(long));
		lpRedCumulativeFrequency[0] = lpRedHistogram[0];
		lpGreenCumulativeFrequency[0] = lpGreenHistogram[0];
		lpBlueCumulativeFrequency[0] = lpBlueHistogram[0];
		for (k=1; k<256; k++)
		{
			lpRedCumulativeFrequency[k] = lpRedHistogram[k] + lpRedCumulativeFrequency[k-1];
			lpGreenCumulativeFrequency[k] = lpGreenHistogram[k] + lpGreenCumulativeFrequency[k-1];
			lpBlueCumulativeFrequency[k] = lpBlueHistogram[k] + lpBlueCumulativeFrequency[k-1];
		}

		// Calculate histogram stretching params
		float _histoCoef = (float)_levels / (float)(m_bih.biWidth*m_bih.biHeight);

		// Equalize image histogram
		dwVerticalOffset = 0;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				BYTE hue = _GetRValue(lpDstData[dwTotalOffset>>2]);
				BYTE saturation = _GetGValue(lpDstData[dwTotalOffset>>2]);
				BYTE value = _GetBValue(lpDstData[dwTotalOffset>>2]);
				value = (BYTE)((float)lpBlueCumulativeFrequency[value] * _histoCoef);
				lpDstData[dwTotalOffset>>2] = _RGB(hue, saturation, value);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Convert image to RGB color model
		ConvertToRGB();
	}
}

void CBitmapEx::Median()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate median params
		fixed f_8 = itofx(8);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Median filter bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		BYTE lpRed[9], lpGreen[9], lpBlue[9];
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				long s = 0;
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<=1; k++)
				{
					long m = i + k;
					if (m < 0)
						m = 0;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=-1; l<=1; l++)
					{
						long n = j + l;
						if (n < 0)
							n = 0;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						lpRed[s] = _GetRValue(lpSrcData[dwSrcOffset>>2]);
						lpGreen[s] = _GetGValue(lpSrcData[dwSrcOffset>>2]);
						lpBlue[s] = _GetBValue(lpSrcData[dwSrcOffset>>2]);
						s++;
					}
				}

				// Sort source pixels
				for (long p=0; p<8; p++)
				{
					for (long q=p+1; q<9; q++)
					{
						if (lpRed[p] > lpRed[q])
						{
							BYTE tmp = lpRed[p];
							lpRed[p] = lpRed[q];
							lpRed[q] = tmp;
						}
						if (lpGreen[p] > lpGreen[q])
						{
							BYTE tmp = lpGreen[p];
							lpGreen[p] = lpGreen[q];
							lpGreen[q] = tmp;
						}
						if (lpBlue[p] > lpBlue[q])
						{
							BYTE tmp = lpBlue[p];
							lpBlue[p] = lpBlue[q];
							lpBlue[q] = tmp;
						}
					}
				}
				lpDstData[dwTotalOffset>>2] = _RGB(lpRed[4], lpGreen[4], lpBlue[4]);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Posterize(long levels)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate posterize params
		long _levels = max(2, min(16, levels));
		long _offset = 256 / _levels;
		BYTE lpRedPalette[16], lpGreenPalette[16], lpBluePalette[16];
		for (long k=0; k<_levels; k++)
		{
			lpRedPalette[k] = (BYTE)(k * _offset);
			lpGreenPalette[k] = (BYTE)(k * _offset);
			lpBluePalette[k] = (BYTE)(k * _offset);
		}

		// Posterize bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				BYTE red = _GetRValue(lpDstData[dwTotalOffset>>2]);
				BYTE green = _GetGValue(lpDstData[dwTotalOffset>>2]);
				BYTE blue = _GetBValue(lpDstData[dwTotalOffset>>2]);
				red = (BYTE)(red / _offset);
				green = (BYTE)(green / _offset);
				blue = (BYTE)(blue /_offset);
				lpDstData[dwTotalOffset>>2] = _RGB(lpRedPalette[red], lpGreenPalette[green], lpBluePalette[blue]);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::Solarize(long threshold)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate solarization params
		long _threshold = max(0, min(255, threshold));
		fixed f_treshold = itofx(_threshold);
		fixed f_255 = itofx(255);

		// Solarize bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				BYTE red = _GetRValue(lpDstData[dwTotalOffset>>2]);
				BYTE green = _GetGValue(lpDstData[dwTotalOffset>>2]);
				BYTE blue = _GetBValue(lpDstData[dwTotalOffset>>2]);
				fixed f_red = itofx(red);
				fixed f_green = itofx(green);
				fixed f_blue = itofx(blue);
				if (f_red > 0)
					f_red = Mulfx(Divfx(f_treshold-f_red,f_red),f_255);
				if (f_green > 0)
					f_green = Mulfx(Divfx(f_treshold-f_green,f_green),f_255);
				if (f_blue > 0)
					f_blue = Mulfx(Divfx(f_treshold-f_blue,f_blue),f_255);
				red = (BYTE)max(0, min(255, fxtoi(f_red)));
				green = (BYTE)max(0, min(255, fxtoi(f_green)));
				blue = (BYTE)max(0, min(255, fxtoi(f_blue)));
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::Draw(HDC hDC)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Draw bitmap
		::SetDIBitsToDevice(hDC, 0, 0, m_bih.biWidth, m_bih.biHeight, 0, 0, 0, m_bih.biHeight, m_lpData, (LPBITMAPINFO)&m_bih, DIB_RGB_COLORS);
	}
}

void CBitmapEx::Draw(HDC hDC, long dstX, long dstY)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Draw bitmap
		::SetDIBitsToDevice(hDC, dstX, dstY, m_bih.biWidth, m_bih.biHeight, 0, 0, 0, m_bih.biHeight, m_lpData, (LPBITMAPINFO)&m_bih, DIB_RGB_COLORS);
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			mov eax, [esi]
			mov [edi], eax
			add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);
		fixed f_1malpha = f_1 - f_alpha;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			push edx
			push ebx
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
			pop ebx
			pop edx
			add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									SetPixel(j, i, bitmapEx.GetPixel(n, m));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									pixel1 =  _RGB(red, green, blue);
									pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									_PIXEL pixel1 =  _RGB(red, green, blue);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									SetPixel(j, i, bitmapEx.GetPixel(n, m));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									pixel1 =  _RGB(red, green, blue);
									pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									_PIXEL pixel1 =  _RGB(red, green, blue);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::_DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
				if ((n >= _width-1) || (m >= _height-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
				if ((m < 1) || (n >= _width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
				if ((m < 1) || (n >= _width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
				if (n >= _width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
				if ((m >= _height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
				if ((m >= _height-1) || (n >= _width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
				if ((m >= _height-1) || (n >= _width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
				if ((m >= _height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
				if (m >= _height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
				if ((m >= _height-2) || (n >= _width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
				if ((m >= _height-2) || (n >= _width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
				BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
				BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight);
			}
			break;
	}
}

void CBitmapEx::_DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
				if ((n >= _width-1) || (m >= _height-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				fixed f_sred = (Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				fixed f_sgreen = (Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				fixed f_sblue = (Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4, f_srcPixel;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov f_srcPixel, eax
			mov eax, f_srcPixel
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		fixed f_alpha = ftofx(_alphaPercent);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
				if ((m < 1) || (n >= _width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
				if ((m < 1) || (n >= _width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
				if (n >= _width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
				if ((m >= _height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
				if ((m >= _height-1) || (n >= _width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
				if ((m >= _height-1) || (n >= _width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
				if ((m >= _height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
				if (m >= _height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
				if ((m >= _height-2) || (n >= _width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
				if ((m >= _height-2) || (n >= _width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				fixed f_sred = itofx(max(0, min(255, fxtoi(Mulfx(f_red,f_gama)))));
				fixed f_sgreen = itofx(max(0, min(255, fxtoi(Mulfx(f_green,f_gama)))));
				fixed f_sblue = itofx(max(0, min(255, fxtoi(Mulfx(f_blue,f_gama)))));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha);
			}
			break;
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
					lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			mov [edi], eax
next1:		add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									_PIXEL pixel = bitmapEx.GetPixel(n, m);
									if (pixel != transparentColor)
										SetPixel(j, i, pixel);
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel = bitmapEx.GetPixel(n, m);
									if (pixel != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel = bitmapEx.GetPixel(n, m);
									if (pixel != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);
		fixed f_1malpha = f_1 - f_alpha;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel1 != transparentColor)
				{
					_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
					fixed f_sred = itofx(_GetRValue(pixel1));
					fixed f_sgreen = itofx(_GetGValue(pixel1));
					fixed f_sblue = itofx(_GetBValue(pixel1));
					fixed f_dred = itofx(_GetRValue(pixel2));
					fixed f_dgreen = itofx(_GetGValue(pixel2));
					fixed f_dblue = itofx(_GetBValue(pixel2));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			push edx
			push ebx
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
next1:		pop ebx
			pop edx
			add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										pixel1 =  _RGB(red, green, blue);
										pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										_PIXEL pixel1 =  _RGB(red, green, blue);
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::_DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
					lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			mov [edi], eax
next1:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
					if ((n >= _width-1) || (m >= _height-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
					BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next4
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov [edi], eax
next4:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
					if ((m < 1) || (n >= _width-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
					if ((m < 1) || (n >= _width-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
					if (n >= _width-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
					if ((m >= _height-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
					if ((m >= _height-1) || (n >= _width-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
					if ((m >= _height-1) || (n >= _width-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset - _pitch + _pitch - _bpp;
					if ((m >= _height-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset - _pitch + _pitch;
					if (m >= _height-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset - _pitch + _pitch + _bpp;
					if ((m >= _height-2) || (n >= _width-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset - _pitch + _pitch + _bpp + _bpp;
					if ((m >= _height-2) || (n >= _width-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
					BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawTransparentNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, transparentColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawTransparentBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, transparentColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawTransparentBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, transparentColor);
			}
			break;
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
										SetPixel(j, i, pixel1);
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::_DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel1 != transparentColor)
				{
					_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
					fixed f_sred = itofx(_GetRValue(pixel1));
					fixed f_sgreen = itofx(_GetGValue(pixel1));
					fixed f_sblue = itofx(_GetBValue(pixel1));
					fixed f_dred = itofx(_GetRValue(pixel2));
					fixed f_dgreen = itofx(_GetGValue(pixel2));
					fixed f_dblue = itofx(_GetBValue(pixel2));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
next1:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
					if ((n >= _width-1) || (m >= _height-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					fixed f_sred = (Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
					fixed f_sgreen = (Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
					fixed f_sblue = (Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
					fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4, f_srcPixel;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next4
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov f_srcPixel, eax
			mov eax, f_srcPixel
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
next4:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		fixed f_alpha = ftofx(_alphaPercent);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
					if ((m < 1) || (n >= _width-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
					if ((m < 1) || (n >= _width-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
					if (n >= _width-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
					if ((m >= _height-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
					if ((m >= _height-1) || (n >= _width-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
					if ((m >= _height-1) || (n >= _width-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
					if ((m >= _height-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
					if (m >= _height-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
					if ((m >= _height-2) || (n >= _width-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
					if ((m >= _height-2) || (n >= _width-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					fixed f_sred = itofx(max(0, min(255, fxtoi(Mulfx(f_red,f_gama)))));
					fixed f_sgreen = itofx(max(0, min(255, fxtoi(Mulfx(f_green,f_gama)))));
					fixed f_sblue = itofx(max(0, min(255, fxtoi(Mulfx(f_blue,f_gama)))));
					fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawTransparentNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha, transparentColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawTransparentBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha, transparentColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawTransparentBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha, transparentColor);
			}
			break;
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										pixel1 =  _RGB(red, green, blue);
										pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										_PIXEL pixel1 =  _RGB(red, green, blue);
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::DrawBlended(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		fixed f_1 = itofx(1);
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;
		float _heightHalf = (float)_height / 2;
		float _widthHalf = (float)_width / 2;
		float _heightSquare = (float)(_height*_height);
		float _widthSquare = (float)(_width*_width);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_width;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_height;
		float _gamma = 2.2f;
		if (!(mode & GM_GAMMA))
			_gamma = 1.0f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if ((mode & 0x001F) == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, sqrt((_alphaV*_gamma)*(_alphaV*_gamma) + (_alphaH*_gamma)*(_alphaH*_gamma)));
				else if ((mode & 0x001F) == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if ((mode & 0x001F) == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if ((mode & 0x001F) == GM_FDIAGONAL)
				{
					_alpha = ((float)i/(float)_height + (float)j/(float)_width)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_BDIAGONAL)
				{
					_alpha = ((float)i/(float)_height + (float)(_width-j-1)/(float)_width)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(_endAlphaPercent, 1.0f-_alpha));
					else
						_alpha = min(_startAlphaPercent, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}
	}
}

void CBitmapEx::_DrawBlendedNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Check drawing region
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float _heightHalf = (float)_dstHeight / 2;
		float _widthHalf = (float)_dstWidth / 2;
		float _heightSquare = (float)(_dstHeight*_dstHeight);
		float _widthSquare = (float)(_dstWidth*_dstWidth);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstWidth;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstHeight;
		float _gamma = 2.2f;
		if (!(mode & GM_GAMMA))
			_gamma = 1.0f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if ((mode & 0x001F) == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, sqrt((_alphaV*_gamma)*(_alphaV*_gamma) + (_alphaH*_gamma)*(_alphaH*_gamma)));
				else if ((mode & 0x001F) == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if ((mode & 0x001F) == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if ((mode & 0x001F) == GM_FDIAGONAL)
				{
					_alpha = ((float)i/(float)_dstHeight + (float)j/(float)_dstWidth)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_BDIAGONAL)
				{
					_alpha = ((float)i/(float)_dstHeight + (float)(_dstWidth-j-1)/(float)_dstWidth)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(_endAlphaPercent, 1.0f-_alpha));
					else
						_alpha = min(_startAlphaPercent, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::_DrawBlendedBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Check drawing region
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float _heightHalf = (float)_dstHeight / 2;
		float _widthHalf = (float)_dstWidth / 2;
		float _heightSquare = (float)(_dstHeight*_dstHeight);
		float _widthSquare = (float)(_dstWidth*_dstWidth);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstWidth;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstHeight;
		float _gamma = 2.2f;
		if (!(mode & GM_GAMMA))
			_gamma = 1.0f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if ((mode & 0x001F) == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, sqrt((_alphaV*_gamma)*(_alphaV*_gamma) + (_alphaH*_gamma)*(_alphaH*_gamma)));
				else if ((mode & 0x001F) == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if ((mode & 0x001F) == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if ((mode & 0x001F) == GM_FDIAGONAL)
				{
					_alpha = ((float)i/(float)_dstHeight + (float)j/(float)_dstWidth)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_BDIAGONAL)
				{
					_alpha = ((float)i/(float)_dstHeight + (float)(_dstWidth-j-1)/(float)_dstWidth)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(_endAlphaPercent, 1.0f-_alpha));
					else
						_alpha = min(_startAlphaPercent, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
				if ((n >= _width-1) || (m >= _height-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				fixed f_sred = (Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				fixed f_sgreen = (Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				fixed f_sblue = (Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::_DrawBlendedBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Check drawing region
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float _heightHalf = (float)_dstHeight / 2;
		float _widthHalf = (float)_dstWidth / 2;
		float _heightSquare = (float)(_dstHeight*_dstHeight);
		float _widthSquare = (float)(_dstWidth*_dstWidth);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstWidth;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstHeight;
		float _gamma = 2.2f;
		if (!(mode & GM_GAMMA))
			_gamma = 1.0f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if ((mode & 0x001F) == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, sqrt((_alphaV*_gamma)*(_alphaV*_gamma) + (_alphaH*_gamma)*(_alphaH*_gamma)));
				else if ((mode & 0x001F) == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if ((mode & 0x001F) == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if ((mode & 0x001F) == GM_FDIAGONAL)
				{
					_alpha = ((float)i/(float)_dstHeight + (float)j/(float)_dstWidth)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_BDIAGONAL)
				{
					_alpha = ((float)i/(float)_dstHeight + (float)(_dstWidth-j-1)/(float)_dstWidth)/2.0f * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(_startAlphaPercent, min(_endAlphaPercent, _alpha));
					else
						_alpha = max(_endAlphaPercent, min(_startAlphaPercent, 1.0f-_alpha));
				}
				else if ((mode & 0x001F) == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(_endAlphaPercent, 1.0f-_alpha));
					else
						_alpha = min(_startAlphaPercent, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
				if ((m < 1) || (n >= _width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
				if ((m < 1) || (n >= _width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
				if (n >= _width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
				if ((m >= _height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
				if ((m >= _height-1) || (n >= _width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
				if ((m >= _height-1) || (n >= _width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
				if ((m >= _height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
				if (m >= _height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
				if ((m >= _height-2) || (n >= _width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
				if ((m >= _height-2) || (n >= _width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				fixed f_sred = itofx(max(0, min(255, fxtoi(Mulfx(f_red,f_gama)))));
				fixed f_sgreen = itofx(max(0, min(255, fxtoi(Mulfx(f_green,f_gama)))));
				fixed f_sblue = itofx(max(0, min(255, fxtoi(Mulfx(f_blue,f_gama)))));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::DrawBlended(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawBlendedNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, startAlpha, endAlpha, mode);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawBlendedBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, startAlpha, endAlpha, mode);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawBlendedBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, startAlpha, endAlpha, mode);
			}
			break;
	}
}

void CBitmapEx::DrawMasked(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
					lpDstData[dwDstTotalOffset>>2] = _RGB(0,0,0);
				else
					lpDstData[dwDstTotalOffset>>2] = _RGB(255,255,255);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}
	}
}

void CBitmapEx::DrawAlpha(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha, _PIXEL alphaColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);
		fixed f_1malpha = f_1 - f_alpha;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_alphaSrc = f_1 - ftofx((float)_GetRValue(pixel1)/255.0f);
				fixed f_alphaDst = f_1 - Mulfx(f_alphaSrc, f_alpha);
				fixed f_sred = itofx(_GetRValue(alphaColor));
				fixed f_sgreen = itofx(_GetGValue(alphaColor));
				fixed f_sblue = itofx(_GetBValue(alphaColor));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_dred, f_alphaDst) + Mulfx(f_sred, f_1-f_alphaDst));
				BYTE green = (BYTE)fxtoi(Mulfx(f_dgreen, f_alphaDst) + Mulfx(f_sgreen, f_1-f_alphaDst));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_dblue, f_alphaDst) + Mulfx(f_sblue, f_1-f_alphaDst));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}
	}
}

void CBitmapEx::DrawCombined(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixelSrc = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixelDst = lpDstData[dwDstTotalOffset>>2];
				_PIXEL pixel;
				if (mode == CM_SRC_AND_DST)
					pixel = pixelSrc & pixelDst;
				else if (mode == CM_SRC_OR_DST)
					pixel = pixelSrc | pixelDst;
				else if (mode == CM_SRC_XOR_DST)
					pixel = pixelSrc ^ pixelDst;
				else if (mode == CM_SRC_AND_DSTI)
					pixel = pixelSrc & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_OR_DSTI)
					pixel = pixelSrc | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_XOR_DSTI)
					pixel = pixelSrc ^ (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_AND_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) & pixelDst;
				else if (mode == CM_SRCI_OR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) | pixelDst;
				else if (mode == CM_SRCI_XOR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ pixelDst;
				else if (mode == CM_SRCI_AND_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_OR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_XOR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ (pixelDst ^ 0x00FFFFFF);
				lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}
	}
}

void CBitmapEx::_DrawCombinedNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixelSrc = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixelDst = lpDstData[dwDstTotalOffset>>2];
				_PIXEL pixel;
				if (mode == CM_SRC_AND_DST)
					pixel = pixelSrc & pixelDst;
				else if (mode == CM_SRC_OR_DST)
					pixel = pixelSrc | pixelDst;
				else if (mode == CM_SRC_XOR_DST)
					pixel = pixelSrc ^ pixelDst;
				else if (mode == CM_SRC_AND_DSTI)
					pixel = pixelSrc & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_OR_DSTI)
					pixel = pixelSrc | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_XOR_DSTI)
					pixel = pixelSrc ^ (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_AND_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) & pixelDst;
				else if (mode == CM_SRCI_OR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) | pixelDst;
				else if (mode == CM_SRCI_XOR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ pixelDst;
				else if (mode == CM_SRCI_AND_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_OR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_XOR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ (pixelDst ^ 0x00FFFFFF);
				lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::_DrawCombinedBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
				if ((n >= _width-1) || (m >= _height-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				_PIXEL pixelSrc = _RGB(red, green, blue);
				_PIXEL pixelDst = lpDstData[dwDstTotalOffset>>2];
				_PIXEL pixel;
				if (mode == CM_SRC_AND_DST)
					pixel = pixelSrc & pixelDst;
				else if (mode == CM_SRC_OR_DST)
					pixel = pixelSrc | pixelDst;
				else if (mode == CM_SRC_XOR_DST)
					pixel = pixelSrc ^ pixelDst;
				else if (mode == CM_SRC_AND_DSTI)
					pixel = pixelSrc & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_OR_DSTI)
					pixel = pixelSrc | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_XOR_DSTI)
					pixel = pixelSrc ^ (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_AND_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) & pixelDst;
				else if (mode == CM_SRCI_OR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) | pixelDst;
				else if (mode == CM_SRCI_XOR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ pixelDst;
				else if (mode == CM_SRCI_AND_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_OR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_XOR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ (pixelDst ^ 0x00FFFFFF);
				lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::_DrawCombinedBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp() >> 3;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
				if ((m < 1) || (n >= _width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
				if ((m < 1) || (n >= _width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
				if (n >= _width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
				if ((m >= _height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
				if ((m >= _height-1) || (n >= _width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
				if ((m >= _height-1) || (n >= _width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
				if ((m >= _height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
				if (m >= _height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
				if ((m >= _height-2) || (n >= _width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
				if ((m >= _height-2) || (n >= _width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
				BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
				BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
				_PIXEL pixelSrc = _RGB(red, green, blue);
				_PIXEL pixelDst = lpDstData[dwDstTotalOffset>>2];
				_PIXEL pixel;
				if (mode == CM_SRC_AND_DST)
					pixel = pixelSrc & pixelDst;
				else if (mode == CM_SRC_OR_DST)
					pixel = pixelSrc | pixelDst;
				else if (mode == CM_SRC_XOR_DST)
					pixel = pixelSrc ^ pixelDst;
				else if (mode == CM_SRC_AND_DSTI)
					pixel = pixelSrc & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_OR_DSTI)
					pixel = pixelSrc | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRC_XOR_DSTI)
					pixel = pixelSrc ^ (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_AND_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) & pixelDst;
				else if (mode == CM_SRCI_OR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) | pixelDst;
				else if (mode == CM_SRCI_XOR_DST)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ pixelDst;
				else if (mode == CM_SRCI_AND_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) & (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_OR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) | (pixelDst ^ 0x00FFFFFF);
				else if (mode == CM_SRCI_XOR_DSTI)
					pixel = (pixelSrc ^ 0x00FFFFFF) ^ (pixelDst ^ 0x00FFFFFF);
				lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::DrawCombined(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, DWORD mode)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw combined bitmaps using nearest neighbour interpolation algorithm
				_DrawCombinedNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, mode);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw combined bitmaps using bilinear interpolation algorithm
				_DrawCombinedBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, mode);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw combined bitmaps using bicubic interpolation algorithm
				_DrawCombinedBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, mode);
			}
			break;
	}
}

void CBitmapEx::DrawTextA(long dstX, long dstY, LPSTR lpszText, _PIXEL textColor, long textAlpha, LPTSTR lpszFontName, long fontSize, BOOL bBold, BOOL bItalic)
{
	// Check for valid bitmap
	if (IsValid() && (lpszText != NULL))
	{
		// Draw text on the bitmap
		long iLen = (long)strlen(lpszText);
		HDC hDC = ::GetDC(NULL);
		long iFontHeight = -MulDiv(fontSize, ::GetDeviceCaps(hDC, LOGPIXELSY), 72);
		long iWeight = FW_NORMAL;
		if (bBold)
			iWeight = FW_BOLD;
		HFONT hFont = ::CreateFont(iFontHeight, 0, 0, 0, iWeight, bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, lpszFontName);
		HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
		SIZE sz;
		::GetTextExtentExPointA(hDC, lpszText, iLen, 0, NULL, NULL, &sz);
		GLYPHMETRICS gm;
		MAT2 m2 = {{0,1}, {0,0}, {0,0}, {0,1}};
		long iLetterOffset = 0;
		long iLineOffset = 0;
		for (long k=0; k<iLen; k++)
		{
			// Check for libe-break character
			if (lpszText[k] == '\n')
			{
				iLineOffset += sz.cy;
				iLetterOffset = 0;
			}
			else
			{
				// Get font letter info
				long iSize = ::GetGlyphOutlineA(hDC, lpszText[k], GGO_GRAY8_BITMAP, &gm, 0, NULL, &m2);

				// Check for valid letter
				if (lpszText[k] != ' ')
				{
					// Allocate font letter buffer
					LPBYTE lpFontBitmap = (LPBYTE)malloc(iSize*sizeof(BYTE));

					// Get font letter data
					::GetGlyphOutlineA(hDC, lpszText[k], GGO_GRAY8_BITMAP, &gm, iSize, lpFontBitmap, &m2);

					// Calculate letter params
					long iVerticalSkip = sz.cy - gm.gmBlackBoxY + (gm.gmBlackBoxY-gm.gmptGlyphOrigin.y);
					long iHorizontalSkip = gm.gmptGlyphOrigin.x;
					long pitch = gm.gmBlackBoxX;
					while ((pitch & 3) != 0)
						pitch++;

					// Draw single letter
					long iVerticalOffset = 0;
					long cy = (long)gm.gmBlackBoxY;
					long cx = (long)gm.gmBlackBoxX;
					for (long i=0; i<cy; i++)
					{
						long iHorizontalOffset = 0;
						for (long j=0; j<cx; j++)
						{
							long index = iVerticalOffset + iHorizontalOffset;
							_PIXEL pixel = GetPixel(dstX+j+iLetterOffset+iHorizontalSkip, dstY+i+iVerticalSkip+iLineOffset);
							fixed f_value = ftofx(lpFontBitmap[index]);
							fixed f_alpha = Mulfx(Divfx(f_value,ftofx(65.0f)),Divfx(itofx(textAlpha),ftofx(100.0f)));
							fixed f_1malpha = itofx(1) - f_alpha;
							fixed f_sred = itofx(_GetRValue(textColor));
							fixed f_sgreen = itofx(_GetGValue(textColor));
							fixed f_sblue = itofx(_GetBValue(textColor));
							fixed f_dred = itofx(_GetRValue(pixel));
							fixed f_dgreen = itofx(_GetGValue(pixel));
							fixed f_dblue = itofx(_GetBValue(pixel));
							BYTE red = (BYTE)fxtoi(Mulfx(f_alpha,f_sred) + Mulfx(f_1malpha,f_dred));
							BYTE green = (BYTE)fxtoi(Mulfx(f_alpha,f_sgreen) + Mulfx(f_1malpha,f_dgreen));
							BYTE blue = (BYTE)fxtoi(Mulfx(f_alpha,f_sblue) + Mulfx(f_1malpha,f_dblue));
							SetPixel(dstX+j+iLetterOffset+iHorizontalSkip, dstY+i+iVerticalSkip+iLineOffset, _RGB(red, green, blue));

							// Increment horizontal offset
							iHorizontalOffset++;
						}

						// Increment vertical offset
						iVerticalOffset += pitch;
					}

					// Free font letter buffer
					free(lpFontBitmap);
				}

				// Increment letter horizontal offset
				iLetterOffset += gm.gmCellIncX;
			}
		}
		::SelectObject(hDC, hOldFont);
		::DeleteObject(hFont);
		::ReleaseDC(NULL, hDC);
	}
}

void CBitmapEx::DrawTextW(long dstX, long dstY, LPWSTR lpszText, _PIXEL textColor, long textAlpha, LPTSTR lpszFontName, long fontSize, BOOL bBold, BOOL bItalic)
{
	// Check for valid bitmap
	if (IsValid() && (lpszText != NULL))
	{
		// Draw text on the bitmap
		long iLen = (long)wcslen(lpszText);
		HDC hDC = ::GetDC(NULL);
		long iFontHeight = -MulDiv(fontSize, ::GetDeviceCaps(hDC, LOGPIXELSY), 72);
		long iWeight = FW_NORMAL;
		if (bBold)
			iWeight = FW_BOLD;
		HFONT hFont = ::CreateFont(iFontHeight, 0, 0, 0, iWeight, bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, lpszFontName);
		HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
		SIZE sz;
		::GetTextExtentExPointW(hDC, lpszText, iLen, 0, NULL, NULL, &sz);
		GLYPHMETRICS gm;
		MAT2 m2 = {{0,1}, {0,0}, {0,0}, {0,1}};
		long iLetterOffset = 0;
		long iLineOffset = 0;
		for (long k=0; k<iLen; k++)
		{
			// Check for libe-break character
			if (lpszText[k] == '\n')
			{
				iLineOffset += sz.cy;
				iLetterOffset = 0;
			}
			else
			{
				// Get font letter info
				long iSize = ::GetGlyphOutlineW(hDC, lpszText[k], GGO_GRAY8_BITMAP, &gm, 0, NULL, &m2);

				// Check for valid letter
				if (lpszText[k] != ' ')
				{
					// Allocate font letter buffer
					LPBYTE lpFontBitmap = (LPBYTE)malloc(iSize*sizeof(BYTE));

					// Get font letter data
					::GetGlyphOutlineW(hDC, lpszText[k], GGO_GRAY8_BITMAP, &gm, iSize, lpFontBitmap, &m2);

					// Calculate letter params
					long iVerticalSkip = sz.cy - gm.gmBlackBoxY + (gm.gmBlackBoxY-gm.gmptGlyphOrigin.y);
					long iHorizontalSkip = gm.gmptGlyphOrigin.x;
					long pitch = gm.gmBlackBoxX;
					while ((pitch & 3) != 0)
						pitch++;

					// Draw single letter
					long iVerticalOffset = 0;
					long cy = (long)gm.gmBlackBoxY;
					long cx = (long)gm.gmBlackBoxX;
					for (long i=0; i<cy; i++)
					{
						long iHorizontalOffset = 0;
						for (long j=0; j<cx; j++)
						{
							long index = iVerticalOffset + iHorizontalOffset;
							_PIXEL pixel = GetPixel(dstX+j+iLetterOffset+iHorizontalSkip, dstY+i+iVerticalSkip+iLineOffset);
							fixed f_value = ftofx(lpFontBitmap[index]);
							fixed f_alpha = Mulfx(Divfx(f_value,ftofx(65.0f)),Divfx(itofx(textAlpha),ftofx(100.0f)));
							fixed f_1malpha = itofx(1) - f_alpha;
							fixed f_sred = itofx(_GetRValue(textColor));
							fixed f_sgreen = itofx(_GetGValue(textColor));
							fixed f_sblue = itofx(_GetBValue(textColor));
							fixed f_dred = itofx(_GetRValue(pixel));
							fixed f_dgreen = itofx(_GetGValue(pixel));
							fixed f_dblue = itofx(_GetBValue(pixel));
							BYTE red = (BYTE)fxtoi(Mulfx(f_alpha,f_sred) + Mulfx(f_1malpha,f_dred));
							BYTE green = (BYTE)fxtoi(Mulfx(f_alpha,f_sgreen) + Mulfx(f_1malpha,f_dgreen));
							BYTE blue = (BYTE)fxtoi(Mulfx(f_alpha,f_sblue) + Mulfx(f_1malpha,f_dblue));
							SetPixel(dstX+j+iLetterOffset+iHorizontalSkip, dstY+i+iVerticalSkip+iLineOffset, _RGB(red, green, blue));

							// Increment horizontal offset
							iHorizontalOffset++;
						}

						// Increment vertical offset
						iVerticalOffset += pitch;
					}

					// Free font letter buffer
					free(lpFontBitmap);
				}

				// Increment letter horizontal offset
				iLetterOffset += gm.gmCellIncX;
			}
		}
		::SelectObject(hDC, hOldFont);
		::DeleteObject(hFont);
		::ReleaseDC(NULL, hDC);
	}
}

void CBitmapEx::SetPixel(long x, long y, _PIXEL pixel)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Set pixel color
		long _x = max(0, min(m_bih.biWidth-1, x));
		long _y = max(0, min(m_bih.biHeight-1, y));
		DWORD dwTotalOffset = (m_bih.biHeight-1-_y)*m_iPitch + _x*m_iBpp;
		LPDWORD lpData = (LPDWORD)m_lpData;
		lpData[dwTotalOffset>>2] = pixel;
	}
}

_PIXEL CBitmapEx::GetPixel(long x, long y)
{
	_PIXEL pixel = _RGB(0,0,0);

	// Check for valid bitmap
	if (IsValid())
	{
		// Get pixel color
		long _x = max(0, min(m_bih.biWidth-1, x));
		long _y = max(0, min(m_bih.biHeight-1, y));
		DWORD dwTotalOffset = (m_bih.biHeight-1-_y)*m_iPitch + _x*m_iBpp;
		LPDWORD lpData = (LPDWORD)m_lpData;
		pixel = lpData[dwTotalOffset>>2];
	}

	return pixel;
}

_PIXEL CBitmapEx::_RGB2HSV(_PIXEL rgbPixel)
{
	_PIXEL hsvPixel = 0;

	// Convert RGB value to HSV value
	BYTE red = _GetRValue(rgbPixel);
	BYTE green = _GetGValue(rgbPixel);
	BYTE blue = _GetBValue(rgbPixel);
	BYTE temp = (red + green + blue) / 3;
	float xa = (green - red) / sqrt(2.0f);
	float ya = (2*blue - red - green) / sqrt(6.0f);
	float _hue = _ARG(xa, ya) * (180.0f / _PI) + 150.0f;
	float _saturation = _ARG(temp, _MOD((float)(red-temp), (float)(green-temp), (float)(blue-temp))) * 100.0f / atan(sqrt(6.0f));
	float _value = (float)temp / 2.55f;
	if ((_saturation == 0.0f) || (_value == 0.0f))
		_hue = 0.0f;
	if (_hue < 0.0f)
		_hue = _hue + 360.0f;
	if (_hue >= 360.0f)
		_hue = _hue - 360.0f;
	BYTE hue = (BYTE)((_hue / 360.0f) * 255.0f);
	BYTE saturation = (BYTE)((_saturation / 100.0f) * 255.0f);
	BYTE value = (BYTE)((_value / 100.0f) * 255.0f);
	hsvPixel = _RGB(hue, saturation, value);

	return hsvPixel;
}

_PIXEL CBitmapEx::_HSV2RGB(_PIXEL hsvPixel)
{
	_PIXEL rgbPixel = 0;

	// Convert HSV value to RGB value
	BYTE hue = _GetRValue(hsvPixel);
	BYTE saturation = _GetGValue(hsvPixel);
	BYTE value = _GetBValue(hsvPixel);
	float _hue = ((float)hue / 255.0f) * 360.0f;
	float _saturation = ((float)saturation / 255.0f) * 100.0f;
	float _value = ((float)value / 255.0f) * 100.0f;
	float _angle = (_hue - 150.0f) * _PI / 180.0f;
	float _ur = _value * 2.55f;
	float _radius = _ur * tan(_saturation * atan(sqrt(6.0f)) / 100.0f);
	float _vr = _radius * cos(_angle) / sqrt(2.0f);
	float _wr = _radius * sin(_angle) / sqrt(6.0f);
	float _red = _ur - _vr - _wr;
	float _green = _ur + _vr - _wr;
	float _blue = _ur + 2*_wr;
	float _rdim;
	if (_red < 0.0f)
	{
		_rdim = _ur / (_vr + _wr);
		_red = 0.0f;
		_green = _ur + (_vr - _wr) * _rdim;
		_blue = _ur + 2*_wr * _rdim;
	}
	else if (_green < 0.0f)
	{
		_rdim = -_ur / (_vr - _wr);
		_red = _ur - (_vr + _wr) * _rdim;
		_green = 0.0f;
		_blue = _ur + 2*_wr * _rdim;
	}
	else if (_blue < 0.0f)
	{
		_rdim = -_ur / (2*_wr);
		_red = _ur - (_vr + _wr) * _rdim;
		_green = _ur + (_vr - _wr) * _rdim;
		_blue = 0.0f;
	}
	if (_red > 255.0f)
	{
		_rdim = (_ur - 255.0f) / (_vr + _wr);
		_red = 255.0f;
		_green = _ur + (_vr - _wr) * _rdim;
		_blue = _ur + 2*_wr * _rdim;
	}
	if (_green > 255.0f)
	{
		_rdim = (255.0f - _ur) / (_vr - _wr);
		_red = _ur - (_vr + _wr) * _rdim;
		_green = 255.0f;
		_blue = _ur + 2*_wr * _rdim;
	}
	if (_blue > 255.0f)
	{
		_rdim = (255.0f - _ur) / (2*_wr);
		_red = _ur - (_vr + _wr) * _rdim;
		_green = _ur + (_vr - _wr) * _rdim;
		_blue = 255.0f;
	}
	BYTE red = (BYTE)_red;
	BYTE green = (BYTE)_green;
	BYTE blue = (BYTE)_blue;
	rgbPixel = _RGB(red, green, blue);

	return rgbPixel;
}

void CBitmapEx::ConvertToHSV()
{
	// Check for valid bitmap
	if ((IsValid()) && (m_ColorMode == CM_RGB))
	{
		// Convert RGB bitmap to HSV
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				lpDstData[dwTotalOffset>>2] = _RGB2HSV(lpDstData[dwTotalOffset>>2]);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update color mode
		m_ColorMode = CM_HSV;
	}
}

void CBitmapEx::ConvertToRGB()
{
	// Check for valid bitmap
	if ((IsValid()) && (m_ColorMode == CM_HSV))
	{
		// Convert HSV bitmap to RGB
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				lpDstData[dwTotalOffset>>2] = _HSV2RGB(lpDstData[dwTotalOffset>>2]);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update color mode
		m_ColorMode = CM_RGB;
	}
}

void CBitmapEx::ReplaceColor(long x, long y, _PIXEL newColor, long alpha, long error, BOOL bImage)
{
	// Check for valid bitmap
	if ((IsValid()) && (m_ColorMode == CM_RGB))
	{
		// Calculate color replacement params
		_PIXEL reffPixel = _RGB2HSV(GetPixel(x, y));
		BYTE reffRed = _GetRValue(reffPixel);
		BYTE reffGreen = _GetGValue(reffPixel);
		BYTE reffBlue = _GetBValue(reffPixel);
		float _alpha = max(0, min(alpha, 100)) / 100.0f;
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alpha);
		fixed f_dred = itofx(_GetRValue(newColor));
		fixed f_dgreen = itofx(_GetGValue(newColor));
		fixed f_dblue = itofx(_GetBValue(newColor));
		float _error = (float)max(0, min(error, 442));
		_error = _error * _error;

		// Check for proccessing flag
		if (bImage)
		{
			// Replace bitmap image color
			DWORD dwHorizontalOffset;
			DWORD dwVerticalOffset = 0;
			DWORD dwTotalOffset;
			LPDWORD lpDstData = (LPDWORD)m_lpData;
			for (long i=0; i<m_bih.biHeight; i++)
			{
				dwHorizontalOffset = 0;
				for (long j=0; j<m_bih.biWidth; j++)
				{
					// Update total offset
					dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

					// Update bitmap
					_PIXEL currPixel = _RGB2HSV(lpDstData[dwTotalOffset>>2]);
					BYTE currRed = _GetRValue(currPixel);
					BYTE currGreen = _GetGValue(currPixel);
					BYTE currBlue = _GetBValue(currPixel);
					float _hueDiff = (float)((currRed-reffRed)*(currRed-reffRed));
					float _satDiff = (float)((currGreen-reffGreen)*(currGreen-reffGreen));
					float _valDiff = (float)((currBlue-reffBlue)*(currBlue-reffBlue));
					float _diff = _hueDiff + _satDiff + _valDiff;
					if (_diff < _error)
					{
						fixed f_sred = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
						fixed f_sgreen = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
						fixed f_sblue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
						BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_1-f_alpha) + Mulfx(f_dred, f_alpha));
						BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_1-f_alpha) + Mulfx(f_dgreen, f_alpha));
						BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_1-f_alpha) + Mulfx(f_dblue, f_alpha));
						lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);
					}

					// Update horizontal offset
					dwHorizontalOffset += m_iBpp;
				}

				// Update vertical offset
				dwVerticalOffset += m_iPitch;
			}
		}
		else
		{
			// Calculate starting offset
			long _x = max(0, min(m_bih.biWidth-1, x));
			long _y = max(0, min(m_bih.biHeight-1, m_bih.biHeight-1-y));

			// Create mask
			_LPPOINT pPixels = (_LPPOINT)malloc(m_bih.biHeight*m_bih.biWidth*sizeof(_POINT));
			long count = 0;
			pPixels[count].x = _x;
			pPixels[count].y = _y;
			count++;
			LPBOOL pMask = (LPBOOL)malloc(m_bih.biHeight*m_bih.biWidth*sizeof(BOOL));
			memset(pMask, FALSE, m_bih.biHeight*m_bih.biWidth*sizeof(BOOL));
			pMask[pPixels[count-1].y*m_bih.biHeight+pPixels[count-1].x] = TRUE;

			// Replace bitmap region color
			LPDWORD lpDstData = (LPDWORD)m_lpData;
			do
			{
				// Get last point
				count--;
				_x = pPixels[count].x;
				_y = pPixels[count].y;
				DWORD dwTotalOffset = _y*m_iPitch + _x*m_iBpp;
				_PIXEL currPixel = _RGB2HSV(lpDstData[dwTotalOffset>>2]);
				BYTE currRed = _GetRValue(currPixel);
				BYTE currGreen = _GetGValue(currPixel);
				BYTE currBlue = _GetBValue(currPixel);
				float _hueDiff = (float)((currRed-reffRed)*(currRed-reffRed));
				float _satDiff = (float)((currGreen-reffGreen)*(currGreen-reffGreen));
				float _valDiff = (float)((currBlue-reffBlue)*(currBlue-reffBlue));
				float _diff = _hueDiff + _satDiff + _valDiff;
				if ((_diff < _error) && (pMask[pPixels[count].y*m_bih.biHeight+pPixels[count].x]))
				{
					fixed f_sred = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
					fixed f_sgreen = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
					fixed f_sblue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_1-f_alpha) + Mulfx(f_dred, f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_1-f_alpha) + Mulfx(f_dgreen, f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_1-f_alpha) + Mulfx(f_dblue, f_alpha));
					lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Check neighbouring pixels
				if (_x > 0)
				{
					DWORD dwLeftOffset = dwTotalOffset - m_iBpp;
					_PIXEL currPixel = _RGB2HSV(lpDstData[dwLeftOffset>>2]);
					BYTE currRed = _GetRValue(currPixel);
					BYTE currGreen = _GetGValue(currPixel);
					BYTE currBlue = _GetBValue(currPixel);
					float _hueDiff = (float)((currRed-reffRed)*(currRed-reffRed));
					float _satDiff = (float)((currGreen-reffGreen)*(currGreen-reffGreen));
					float _valDiff = (float)((currBlue-reffBlue)*(currBlue-reffBlue));
					float _diff = _hueDiff + _satDiff + _valDiff;
					if ((_diff < _error) && (!pMask[_y*m_bih.biHeight+_x-1]))
					{
						pPixels[count].x = _x - 1;
						pPixels[count].y = _y;
						count++;
						pMask[_y*m_bih.biHeight+_x-1] = TRUE;
					}
				}
				if (_y > 0)
				{
					DWORD dwTopOffset = dwTotalOffset - m_iPitch;
					_PIXEL currPixel = _RGB2HSV(lpDstData[dwTopOffset>>2]);
					BYTE currRed = _GetRValue(currPixel);
					BYTE currGreen = _GetGValue(currPixel);
					BYTE currBlue = _GetBValue(currPixel);
					float _hueDiff = (float)((currRed-reffRed)*(currRed-reffRed));
					float _satDiff = (float)((currGreen-reffGreen)*(currGreen-reffGreen));
					float _valDiff = (float)((currBlue-reffBlue)*(currBlue-reffBlue));
					float _diff = _hueDiff + _satDiff + _valDiff;
					if ((_diff < _error) && (!pMask[(_y-1)*m_bih.biHeight+_x]))
					{
						pPixels[count].x = _x;
						pPixels[count].y = _y - 1;
						count++;
						pMask[(_y-1)*m_bih.biHeight+_x] = TRUE;
					}
				}
				if (_x < m_bih.biWidth-1)
				{
					DWORD dwRightOffset = dwTotalOffset + m_iBpp;
					_PIXEL currPixel = _RGB2HSV(lpDstData[dwRightOffset>>2]);
					BYTE currRed = _GetRValue(currPixel);
					BYTE currGreen = _GetGValue(currPixel);
					BYTE currBlue = _GetBValue(currPixel);
					float _hueDiff = (float)((currRed-reffRed)*(currRed-reffRed));
					float _satDiff = (float)((currGreen-reffGreen)*(currGreen-reffGreen));
					float _valDiff = (float)((currBlue-reffBlue)*(currBlue-reffBlue));
					float _diff = _hueDiff + _satDiff + _valDiff;
					if ((_diff < _error) && (!pMask[_y*m_bih.biHeight+_x+1]))
					{
						pPixels[count].x = _x + 1;
						pPixels[count].y = _y;
						count++;
						pMask[_y*m_bih.biHeight+_x+1] = TRUE;
					}
				}
				if (_y < m_bih.biHeight-1)
				{
					DWORD dwBottomOffset = dwTotalOffset + m_iPitch;
					_PIXEL currPixel = _RGB2HSV(lpDstData[dwBottomOffset>>2]);
					BYTE currRed = _GetRValue(currPixel);
					BYTE currGreen = _GetGValue(currPixel);
					BYTE currBlue = _GetBValue(currPixel);
					float _hueDiff = (float)((currRed-reffRed)*(currRed-reffRed));
					float _satDiff = (float)((currGreen-reffGreen)*(currGreen-reffGreen));
					float _valDiff = (float)((currBlue-reffBlue)*(currBlue-reffBlue));
					float _diff = _hueDiff + _satDiff + _valDiff;
					if ((_diff < _error) && (!pMask[(_y+1)*m_bih.biHeight+_x]))
					{
						pPixels[count].x = _x;
						pPixels[count].y = _y + 1;
						count++;
						pMask[(_y+1)*m_bih.biHeight+_x] = TRUE;
					}
				}

			} while (count > 0);

			// Free mask
			free(pPixels);
			free(pMask);
		}
	}
}

void CBitmapEx::CreateFireEffect()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Create fire buffer
		if (m_lpFire != NULL)
			free(m_lpFire);
		m_lpFire = (LPBYTE)malloc(m_bih.biHeight*m_bih.biWidth*sizeof(BYTE));
		memset(m_lpFire, 0, m_bih.biHeight*m_bih.biWidth*sizeof(BYTE));
		if (m_pFireBitmap != NULL)
			delete m_pFireBitmap;
		m_pFireBitmap = new CBitmapEx();
		m_pFireBitmap->Create(this);

		// Create fire palette
		for (long i=0; i<256; i++)
		{
			_PIXEL _pixel = _HSV2RGB(_RGB(i/3, 255, min(255, 2*i)));
			m_lpFirePalette[i].rgbRed = _GetRValue(_pixel);
			m_lpFirePalette[i].rgbGreen = _GetGValue(_pixel);
			m_lpFirePalette[i].rgbBlue = _GetBValue(_pixel);
			m_lpFirePalette[i].rgbReserved = 0;
		}
	}
}

void CBitmapEx::UpdateFireEffect(BOOL bLarge, long iteration, long height)
{
	// Check for valid bitmap
	if (IsValid() && (m_lpFire != NULL))
	{
		long i, j;
		long _height = max(4, min(128, height));
		long _heightSize = 4*_height + 1;
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);

		// Update fire buffer
		DWORD dwOffset;
		srand(GetTickCount());
		for (long k=0; k<iteration; k++)
		{
			// Randomize bottom row of the fire buffer
			dwOffset = (m_bih.biHeight-1)*m_bih.biWidth;
			for(j=0; j<m_bih.biWidth; j++)
			{
				m_lpFire[dwOffset] = abs(32768 + rand()) % 256;
				dwOffset++;
			}

			// Recalculate fire buffer
			dwOffset = 0;
			for (i=0; i<m_bih.biHeight; i++)
			{
				for (j=0; j<m_bih.biWidth; j++)
				{
					if (bLarge)
					{
						// Large fire
						long _x1 = ((j - 1) + m_bih.biWidth) % m_bih.biWidth;
						long _y1 = (i + 1) % m_bih.biHeight;
						long _x2 = j % m_bih.biWidth;
						long _y2 = (i + 2) % m_bih.biHeight;
						long _x3 = (j + 1) % m_bih.biWidth;
						long _y3 = (i + 1) % m_bih.biHeight;
						long _x4 = j % m_bih.biWidth;
						long _y4 = (i + 3) % m_bih.biHeight;
						DWORD dwOffset1 = (_y1 * m_bih.biWidth) + _x1;
						DWORD dwOffset2 = (_y2 * m_bih.biWidth) + _x2;
						DWORD dwOffset3 = (_y3 * m_bih.biWidth) + _x3;
						DWORD dwOffset4 = (_y4 * m_bih.biWidth) + _x4;
						m_lpFire[dwOffset] = (BYTE)(((m_lpFire[dwOffset1] + m_lpFire[dwOffset2] + m_lpFire[dwOffset3] + m_lpFire[dwOffset4]) * _height) / _heightSize);
						dwOffset++;
					}
					else
					{
						// Small fire
						long _x1 = j % m_bih.biWidth;
						long _y1 = (i + 1) % m_bih.biHeight;
						long _x2 = j % m_bih.biWidth;
						long _y2 = (i + 2) % m_bih.biHeight;
						long _x3 = j % m_bih.biWidth;
						long _y3 = (i + 2) % m_bih.biHeight;
						long _x4 = j % m_bih.biWidth;
						long _y4 = (i + 3) % m_bih.biHeight;
						DWORD dwOffset1 = (_y1 * m_bih.biWidth) + _x1;
						DWORD dwOffset2 = (_y2 * m_bih.biWidth) + _x2;
						DWORD dwOffset3 = (_y3 * m_bih.biWidth) + _x3;
						DWORD dwOffset4 = (_y4 * m_bih.biWidth) + _x4;
						m_lpFire[dwOffset] = (BYTE)(((m_lpFire[dwOffset1] + m_lpFire[dwOffset2] + m_lpFire[dwOffset3] + m_lpFire[dwOffset4]) * _height) / _heightSize);
						dwOffset++;
					}
				}
			}
		}

		// Update fire
		dwOffset = 0;
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = (m_bih.biHeight-1) * m_iPitch;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD)m_pFireBitmap->GetData();
		for (i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = itofx(m_lpFirePalette[m_lpFire[dwOffset]].rgbRed);
				fixed f_green = itofx(m_lpFirePalette[m_lpFire[dwOffset]].rgbGreen);
				fixed f_blue = itofx(m_lpFirePalette[m_lpFire[dwOffset]].rgbBlue);
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				fixed f_alpha = Divfx(f_value,ftofx(255.0f));
				fixed f_1malpha = itofx(1) - f_alpha;
				fixed f_sred = itofx(_GetRValue(lpSrcData[dwTotalOffset>>2]));
				fixed f_sgreen = itofx(_GetGValue(lpSrcData[dwTotalOffset>>2]));
				fixed f_sblue = itofx(_GetBValue(lpSrcData[dwTotalOffset>>2]));
				fixed f_dred = Mulfx(f_1malpha,f_sred) + Mulfx(f_alpha,f_red);
				fixed f_dgreen = Mulfx(f_1malpha,f_sgreen) + Mulfx(f_alpha,f_green);
				fixed f_dblue = Mulfx(f_1malpha,f_sblue) + Mulfx(f_alpha,f_blue);
				lpDstData[dwTotalOffset>>2] = _RGB(fxtoi(f_dred), fxtoi(f_dgreen), fxtoi(f_dblue));
				dwOffset++;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::CreateWaterEffect()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Create water buffers
		if (m_lpWaterHeightField1 != NULL)
			free(m_lpWaterHeightField1);
		m_lpWaterHeightField1 = (long*)malloc(m_bih.biHeight*m_bih.biWidth*sizeof(long));
		if (m_lpWaterHeightField2 != NULL)
			free(m_lpWaterHeightField2);
		m_lpWaterHeightField2 = (long*)malloc(m_bih.biHeight*m_bih.biWidth*sizeof(long));
		memset(m_lpWaterHeightField1, 0, m_bih.biHeight*m_bih.biWidth*sizeof(long));
		memset(m_lpWaterHeightField2, 0, m_bih.biHeight*m_bih.biWidth*sizeof(long));
		if (m_pWaterBitmap != NULL)
			delete m_pWaterBitmap;
		m_pWaterBitmap = new CBitmapEx();
		m_pWaterBitmap->Create(this);
	}
}

void CBitmapEx::UpdateWaterEffect(long iteration)
{
	// Check for valid bitmap
	if (IsValid() && (m_lpWaterHeightField1 != NULL) && (m_lpWaterHeightField2 != NULL))
	{
		for (long k=0; k<iteration; k++)
		{
			// Calculate water params
			long *lpOldWaterMap, *lpNewWaterMap;
			if (m_bWaterFlip == FALSE)
			{
				lpNewWaterMap = &m_lpWaterHeightField1[0];
				lpOldWaterMap = &m_lpWaterHeightField2[0];
			}
			else
			{
				lpNewWaterMap = &m_lpWaterHeightField2[0];
				lpOldWaterMap = &m_lpWaterHeightField1[0];
			}

			// Update water buffers
			DWORD dwHorizontalOffset;
			DWORD dwVerticalOffset = m_bih.biWidth;
			DWORD dwTotalOffset;
			for (long i=1; i<m_bih.biHeight-1; i++)
			{
				dwHorizontalOffset = 1;
				for (long j=1; j<m_bih.biWidth-1; j++)
				{
					// Update total offset
					dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

					// Update buffers
					DWORD dwOffset1 = dwTotalOffset + m_bih.biWidth;
					DWORD dwOffset2 = dwTotalOffset - m_bih.biWidth;
					DWORD dwOffset3 = dwTotalOffset + 1;
					DWORD dwOffset4 = dwTotalOffset - 1;
					DWORD dwOffset5 = dwTotalOffset - m_bih.biWidth - 1;
					DWORD dwOffset6 = dwTotalOffset - m_bih.biWidth + 1;
					DWORD dwOffset7 = dwTotalOffset + m_bih.biWidth - 1;
					DWORD dwOffset8 = dwTotalOffset + m_bih.biWidth + 1;
					long newHeight = ((lpOldWaterMap[dwOffset1] + 
						lpOldWaterMap[dwOffset2] + 
						lpOldWaterMap[dwOffset3] + 
						lpOldWaterMap[dwOffset4] + 
						lpOldWaterMap[dwOffset5] + 
						lpOldWaterMap[dwOffset6] + 
						lpOldWaterMap[dwOffset7] + 
						lpOldWaterMap[dwOffset8]) >> 2) 
						- lpNewWaterMap[dwTotalOffset];
					lpNewWaterMap[dwTotalOffset] = newHeight - (newHeight >> m_iDamp);

					// Update horizontal offset
					dwHorizontalOffset++;
				}

				// Update vertical offset
				dwVerticalOffset += m_bih.biWidth;
			}

			// Flip water buffers
			m_bWaterFlip = !m_bWaterFlip;
		}

		// Update water
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = m_bih.biWidth;
		DWORD dwSrcTotalOffset;
		long* lpWaterData = &m_lpWaterHeightField1[0];
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD)m_pWaterBitmap->GetData();
		long iIndex;
		long iTotalSize = m_bih.biWidth * m_bih.biHeight;
		DWORD dwOffset;
		for (long i=1; i<m_bih.biHeight-1; i++)
		{
			dwSrcHorizontalOffset = 1;
			dwDstHorizontalOffset = m_iBpp;
			for (long j=1; j<m_bih.biWidth-1; j++)
			{
				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				long dy = lpWaterData[dwSrcTotalOffset] - lpWaterData[dwSrcTotalOffset+m_bih.biWidth];
				long dx = lpWaterData[dwSrcTotalOffset] - lpWaterData[dwSrcTotalOffset+1];
				iIndex = dwSrcTotalOffset + (dy>>m_iLightModifier)*m_bih.biWidth + (dx>>m_iLightModifier);
				if ((iIndex >= 0) && (iIndex < iTotalSize))
				{
					dwOffset = dwDstTotalOffset + (dy>>m_iLightModifier)*m_iPitch + (dx>>m_iLightModifier)*m_iBpp;
					if (dwOffset < m_dwSize)
					{
						BYTE red = _GetRValue(lpSrcData[dwOffset>>2]);
						BYTE green = _GetGValue(lpSrcData[dwOffset>>2]);
						BYTE blue = _GetBValue(lpSrcData[dwOffset>>2]);
						red = (BYTE)max(0, min(255, red-dx));
						green = (BYTE)max(0, min(255, green-dx));
						blue = (BYTE)max(0, min(255, blue-dx));
						lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
					}
				}

				// Update source horizontal offset
				dwSrcHorizontalOffset++;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update source vertical offset
			dwSrcVerticalOffset += m_bih.biWidth;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::MakeWaterBlob(long x, long y, long size, long height)
{
	// Check for valid bitmap
	if (IsValid() && (m_lpWaterHeightField1 != NULL) && (m_lpWaterHeightField2 != NULL))
	{
		// Calculate water params
		long *lpOldWaterMap, *lpNewWaterMap;
		if (m_bWaterFlip == FALSE)
		{
			lpNewWaterMap = &m_lpWaterHeightField1[0];
			lpOldWaterMap = &m_lpWaterHeightField2[0];
		}
		else
		{
			lpNewWaterMap = &m_lpWaterHeightField2[0];
			lpOldWaterMap = &m_lpWaterHeightField1[0];
		}

		// Make water blob
		long _x = max(1, min(m_bih.biWidth-2, x));
		long _y = max(1, min(m_bih.biHeight-2, y));
		long qsize = size * size;
		long left = -size;
		if((_x-size) < 1)
			left -= (_x - size - 1);
		long right = size;
		if((_x+size) > m_bih.biWidth-1)
			right -= (_x + size - m_bih.biWidth + 1);
		long top = -size;
		if((_y-size) < 1)
			top -= (_y - size - 1);
		long bottom = size;
		if((_y+size) > m_bih.biHeight-1)
			bottom -= (_y + size - m_bih.biHeight + 1);
		for(long cy=top; cy<bottom; cy++)
		{
			long cyq = cy * cy;
			for(long cx=left; cx<right; cx++)
			{
				if(cx*cx + cyq < qsize)
				{
					DWORD dwOffset = (cy+_y)*m_bih.biWidth + (cx+_x);
					lpNewWaterMap[dwOffset] += height;
				}
			}
		}
	}
}

void CBitmapEx::CreateSmokeEffect()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Create smoke buffer
		if (m_lpSmokeField != NULL)
			free(m_lpSmokeField);
		m_lpSmokeField = (float*)malloc(_NOISE_HEIGHT*_NOISE_WIDTH*_NOISE_DEPTH*sizeof(float));
		if (m_pSmokeBitmap != NULL)
			delete m_pSmokeBitmap;
		m_pSmokeBitmap = new CBitmapEx();
		m_pSmokeBitmap->Create(this);
		srand(GetTickCount());
		DWORD dwOffset = 0;
		for (long i=0; i<_NOISE_HEIGHT; i++)
		{
			for (long j=0; j<_NOISE_WIDTH; j++)
			{
				for (long k=0; k<_NOISE_DEPTH; k++)
				{
					m_lpSmokeField[dwOffset] = (float)(rand() % 32768) / 32768.0f;
					dwOffset++;
				}
			}
		}
	}
}

void CBitmapEx::UpdateSmokeEffect(long offsetX, long offsetY, long offsetZ)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Update smoke buffer
		long k = GetTickCount() / offsetZ;
		long shiftY = GetTickCount() / offsetY;
		long shiftX = GetTickCount() / offsetX;
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_pSmokeBitmap->GetData();
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Smooth noise
				long size = 128;
				float initSize = (float)size;
				float value = 0.0f;
				for (long _size=size; _size>=1; _size=_size>>1)
				{
					float _i = (float)(i+shiftY) / (float)_size;
					float _j = (float)(j+shiftX) / (float)_size;
					float _k = (float)k / (float)_size;
					float fractY = _i - (long)_i;
					float fractX = _j - (long)_j;
					float fractZ = _k - (long)_k;
					long y1 = (long(_i) + _NOISE_HEIGHT) % _NOISE_HEIGHT;
					long x1 = (long(_j) + _NOISE_WIDTH) % _NOISE_WIDTH;
					long z1 = (long(_k) + _NOISE_DEPTH) % _NOISE_DEPTH;
					long y2 = (y1 + _NOISE_HEIGHT - 1) % _NOISE_HEIGHT;
					long x2 = (x1 + _NOISE_WIDTH - 1) % _NOISE_WIDTH;
					long z2 = (z1 + _NOISE_DEPTH - 1) % _NOISE_DEPTH;
					DWORD dwOffset1 = (y1*_NOISE_WIDTH + x1)*_NOISE_DEPTH + z1;
					DWORD dwOffset2 = (y2*_NOISE_WIDTH + x1)*_NOISE_DEPTH + z1;
					DWORD dwOffset3 = (y1*_NOISE_WIDTH + x2)*_NOISE_DEPTH + z1;
					DWORD dwOffset4 = (y2*_NOISE_WIDTH + x2)*_NOISE_DEPTH + z1;
					DWORD dwOffset5 = (y1*_NOISE_WIDTH + x1)*_NOISE_DEPTH + z2;
					DWORD dwOffset6 = (y2*_NOISE_WIDTH + x1)*_NOISE_DEPTH + z2;
					DWORD dwOffset7 = (y1*_NOISE_WIDTH + x2)*_NOISE_DEPTH + z2;
					DWORD dwOffset8 = (y2*_NOISE_WIDTH + x2)*_NOISE_DEPTH + z2;
					float smoothValue = 0.0f;
					smoothValue += fractX * fractY * fractZ * m_lpSmokeField[dwOffset1];
					smoothValue += fractX * (1.0f-fractY) * fractZ * m_lpSmokeField[dwOffset2];
					smoothValue += (1.0f-fractX) * fractY * fractZ * m_lpSmokeField[dwOffset3];
					smoothValue += (1.0f-fractX) * (1.0f-fractY) * fractZ * m_lpSmokeField[dwOffset4];
					smoothValue += fractX * fractY * (1.0f-fractZ) * m_lpSmokeField[dwOffset5];
					smoothValue += fractX * (1.0f-fractY) * (1.0f-fractZ) * m_lpSmokeField[dwOffset6];
					smoothValue += (1.0f-fractX) * fractY * (1.0f-fractZ) * m_lpSmokeField[dwOffset7];
					smoothValue += (1.0f-fractX) * (1.0f-fractY) * (1.0f-fractZ) * m_lpSmokeField[dwOffset8];
					value += smoothValue * _size;
				}
				value = (128.0f * value) / initSize;
				fixed f_value = ftofx(value);
				fixed f_alpha = Divfx(f_value,ftofx(255.0f));
				fixed f_1malpha = itofx(1) - f_alpha;
				fixed f_sred = itofx(_GetRValue(lpSrcData[dwTotalOffset>>2]));
				fixed f_sgreen = itofx(_GetGValue(lpSrcData[dwTotalOffset>>2]));
				fixed f_sblue = itofx(_GetBValue(lpSrcData[dwTotalOffset>>2]));
				fixed f_dred = Mulfx(f_1malpha,f_sred) + Mulfx(f_alpha,f_value);
				fixed f_dgreen = Mulfx(f_1malpha,f_sgreen) + Mulfx(f_alpha,f_value);
				fixed f_dblue = Mulfx(f_1malpha,f_sblue) + Mulfx(f_alpha,f_value);
				lpDstData[dwTotalOffset>>2] = _RGB(fxtoi(f_dred), fxtoi(f_dgreen), fxtoi(f_dblue));

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

_SIZE CBitmapEx::MeasureTextA(LPSTR lpszText, LPTSTR lpszFontName, long fontSize, BOOL bBold, BOOL bItalic)
{
	_SIZE szText = {0, 0};

	// Check for valid bitmap
	if (IsValid() && (lpszText != NULL))
	{
		// Measure text size
		long iLen = (long)strlen(lpszText);
		HDC hDC = ::GetDC(NULL);
		long iFontHeight = -MulDiv(fontSize, ::GetDeviceCaps(hDC, LOGPIXELSY), 72);
		long iWeight = FW_NORMAL;
		if (bBold)
			iWeight = FW_BOLD;
		HFONT hFont = ::CreateFont(iFontHeight, 0, 0, 0, iWeight, bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, lpszFontName);
		HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
		SIZE sz;
		::GetTextExtentExPointA(hDC, lpszText, iLen, 0, NULL, NULL, &sz);
		GLYPHMETRICS gm;
		MAT2 m2 = {{0,1}, {0,0}, {0,0}, {0,1}};
		long iLetterOffset = 0;
		long iLineOffset = 0;
		for (long k=0; k<iLen; k++)
		{
			// Check for libe-break character
			if (lpszText[k] == '\n')
			{
				if (szText.cx < iLetterOffset)
					szText.cx = iLetterOffset;
				szText.cy += sz.cy;
				iLineOffset += sz.cy;
				iLetterOffset = 0;
			}
			else
			{
				// Get font letter info
				::GetGlyphOutlineA(hDC, lpszText[k], GGO_METRICS, &gm, 0, NULL, &m2);

				// Increment letter horizontal offset
				iLetterOffset += gm.gmCellIncX;
			}
		}
		if (szText.cx < iLetterOffset)
			szText.cx = iLetterOffset;
		szText.cy += sz.cy;
		::SelectObject(hDC, hOldFont);
		::DeleteObject(hFont);
		::ReleaseDC(NULL, hDC);
	}

	return szText;
}

_SIZE CBitmapEx::MeasureTextW(LPWSTR lpszText, LPTSTR lpszFontName, long fontSize, BOOL bBold, BOOL bItalic)
{
	_SIZE szText = {0, 0};

	// Check for valid bitmap
	if (IsValid() && (lpszText != NULL))
	{
		// Measure text size
		long iLen = (long)wcslen(lpszText);
		HDC hDC = ::GetDC(NULL);
		long iFontHeight = -MulDiv(fontSize, ::GetDeviceCaps(hDC, LOGPIXELSY), 72);
		long iWeight = FW_NORMAL;
		if (bBold)
			iWeight = FW_BOLD;
		HFONT hFont = ::CreateFont(iFontHeight, 0, 0, 0, iWeight, bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, lpszFontName);
		HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
		SIZE sz;
		::GetTextExtentExPointW(hDC, lpszText, iLen, 0, NULL, NULL, &sz);
		GLYPHMETRICS gm;
		MAT2 m2 = {{0,1}, {0,0}, {0,0}, {0,1}};
		long iLetterOffset = 0;
		long iLineOffset = 0;
		for (long k=0; k<iLen; k++)
		{
			// Check for libe-break character
			if (lpszText[k] == '\n')
			{
				if (szText.cx < iLetterOffset)
					szText.cx = iLetterOffset;
				szText.cy += sz.cy;
				iLineOffset += sz.cy;
				iLetterOffset = 0;
			}
			else
			{
				// Get font letter info
				::GetGlyphOutlineW(hDC, lpszText[k], GGO_METRICS, &gm, 0, NULL, &m2);

				// Increment letter horizontal offset
				iLetterOffset += gm.gmCellIncX;
			}
		}
		if (szText.cx < iLetterOffset)
			szText.cx = iLetterOffset;
		szText.cy += sz.cy;
		::SelectObject(hDC, hOldFont);
		::DeleteObject(hFont);
		::ReleaseDC(NULL, hDC);
	}

	return szText;
}

void CBitmapEx::GetRedChannel(LPBYTE lpBuffer)
{
	// Check for valid bitmap
	if (IsValid() && (lpBuffer != NULL))
	{
		// Get red channel
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Get red component value
				lpBuffer[dwTotalOffset>>2] = _GetRValue(lpSrcData[dwTotalOffset>>2]);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::GetGreenChannel(LPBYTE lpBuffer)
{
	// Check for valid bitmap
	if (IsValid() && (lpBuffer != NULL))
	{
		// Get green channel
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Get green component value
				lpBuffer[dwTotalOffset>>2] = _GetGValue(lpSrcData[dwTotalOffset>>2]);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::GetBlueChannel(LPBYTE lpBuffer)
{
	// Check for valid bitmap
	if (IsValid() && (lpBuffer != NULL))
	{
		// Get blue channel
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Get blue component value
				lpBuffer[dwTotalOffset>>2] = _GetBValue(lpSrcData[dwTotalOffset>>2]);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::GetRedChannelHistogram(long lpBuffer[256], BOOL bPercent)
{
	// Check for valid bitmap
	if (IsValid() && (lpBuffer != NULL))
	{
		// Clear red channel histogram
		memset(lpBuffer, 0, 256*sizeof(long));

		// Get red channel histogram
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update red channel histogram
				BYTE red = _GetRValue(lpSrcData[dwTotalOffset>>2]);
				lpBuffer[red]++;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Calculate red channel histogram in percents
		if (bPercent)
		{
			long imageArea = m_bih.biWidth * m_bih.biHeight;
			for (long k=0; k<256; k++)
				lpBuffer[k] = (long)((float)lpBuffer[k] / (float)imageArea * 10000.0f);
		}
	}
}

void CBitmapEx::GetGreenChannelHistogram(long lpBuffer[256], BOOL bPercent)
{
	// Check for valid bitmap
	if (IsValid() && (lpBuffer != NULL))
	{
		// Clear green channel histogram
		memset(lpBuffer, 0, 256*sizeof(long));

		// Get green channel histogram
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update green channel histogram
				BYTE green = _GetGValue(lpSrcData[dwTotalOffset>>2]);
				lpBuffer[green]++;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Calculate green channel histogram in percents
		if (bPercent)
		{
			long imageArea = m_bih.biWidth * m_bih.biHeight;
			for (long k=0; k<256; k++)
				lpBuffer[k] = (long)((float)lpBuffer[k] / (float)imageArea * 10000.0f);
		}
	}
}

void CBitmapEx::GetBlueChannelHistogram(long lpBuffer[256], BOOL bPercent)
{
	// Check for valid bitmap
	if (IsValid() && (lpBuffer != NULL))
	{
		// Clear blue channel histogram
		memset(lpBuffer, 0, 256*sizeof(long));

		// Get blue channel histogram
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update blue channel histogram
				BYTE blue = _GetBValue(lpSrcData[dwTotalOffset>>2]);
				lpBuffer[blue]++;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Calculate blue channel histogram in percents
		if (bPercent)
		{
			long imageArea = m_bih.biWidth * m_bih.biHeight;
			for (long k=0; k<256; k++)
				lpBuffer[k] = (long)((float)lpBuffer[k] / (float)imageArea * 10000.0f);
		}
	}
}
