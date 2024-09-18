//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osMoveRecord.h
 *  
 *  His:      river Created 2004-8-30.
 *
 *  Des:      用于录制场景中的图象到avi文件.
 *  
 *   "微雨过,小荷翻,榴花开欲然,
 *    玉盆纤手弄清泉,琼珠碎却圆."
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osMoveRecord.h"

# if __MOVIE_RECORD__

bool NVMovie::BeginRecord(char* strFileName, LPDIRECT3DSURFACE9 pBackBuffer)
{
	HRESULT hr;
	D3DSURFACE_DESC SurfaceDesc;
	AVISTREAMINFO StreamHeader;
	AVICOMPRESSOPTIONS CompressOptions;
	DWORD dwPitch;
	DWORD BitCount;
	DWORD dwVersion;

	if (m_bRecording)
	{
		if (!EndRecord())
			return false;
	}

	// Check VFW
	dwVersion = HIWORD(VideoForWindowsVersion());
	if (dwVersion < 0x010a)
	{
		return false;
	}

	// Create the movie
	AVIFileInit();
	hr = AVIFileOpen(&m_pFile, strFileName, OF_CREATE | OF_WRITE, NULL);
	if (hr != AVIERR_OK)
	{
		MessageBox(NULL, "Could not open file!", "ERROR", MB_ICONEXCLAMATION);
		Free();
		return false;
	}

	pBackBuffer->GetDesc(&SurfaceDesc);

	// Get the pitch of the backbuffer
	switch(SurfaceDesc.Format)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		dwPitch = SurfaceDesc.Width * 4;
		BitCount = 32;
		break;
	default:
		MessageBox(NULL, "Require 32 bit desktop!", "ERROR", MB_ICONEXCLAMATION);
		return false;
	}

	ZeroMemory(&StreamHeader, sizeof(StreamHeader));
	StreamHeader.fccType                = streamtypeVIDEO;// stream type
	StreamHeader.fccHandler             = 0;
	StreamHeader.dwSuggestedBufferSize  = 1000*dwPitch * SurfaceDesc.Height;
	StreamHeader.dwScale                = 1;
	StreamHeader.dwRate                 = 15;

	SetRect(&StreamHeader.rcFrame, 0, 0,		    // rectangle for stream
		(int) SurfaceDesc.Width,
		(int) SurfaceDesc.Height);

	// Create stream
	hr = AVIFileCreateStream(m_pFile, &m_pStream, &StreamHeader);	    
	if (hr != AVIERR_OK) 
	{
		Free();
		return false;
	}

	// Create a compressed version of the source stream
	ZeroMemory(&CompressOptions, sizeof(CompressOptions));
	CompressOptions.dwFlags = AVICOMPRESSF_VALID;
	CompressOptions.fccType = streamtypeVIDEO;
	CompressOptions.fccHandler = comptypeDIB;
	CompressOptions.dwQuality = 7400;
	CompressOptions.dwKeyFrameEvery = 1;
	hr = AVIMakeCompressedStream(&m_pCompressedStream, m_pStream, &CompressOptions, NULL); 
	if (hr != AVIERR_OK) 
	{
		Free();
		return false;
	}

	// Generate bitmap data    
	ZeroMemory(&m_BitmapInfoHeader, sizeof(BITMAPINFOHEADER));
	m_BitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfoHeader.biWidth = SurfaceDesc.Width;
	m_BitmapInfoHeader.biHeight = SurfaceDesc.Height;
	m_BitmapInfoHeader.biPlanes = 1;
	m_BitmapInfoHeader.biBitCount = WORD(BitCount);
	m_BitmapInfoHeader.biCompression = BI_RGB;
	m_BitmapInfoHeader.biSizeImage = dwPitch * SurfaceDesc.Height;
	m_BitmapInfoHeader.biXPelsPerMeter = 1000;
	m_BitmapInfoHeader.biYPelsPerMeter = 1000;
	m_BitmapInfoHeader.biClrUsed = 0;
	m_BitmapInfoHeader.biClrImportant = 0;
	hr = AVIStreamSetFormat(m_pCompressedStream, 0, &m_BitmapInfoHeader,	    // stream format
		m_BitmapInfoHeader.biSize);
	if (hr != AVIERR_OK) 
	{
		Free();
		return false;
	}


	m_pBackBuffer = pBackBuffer;
	m_pBackBuffer->AddRef();

	m_pBuffer = new BYTE[((dwPitch + 3) & ~3) * SurfaceDesc.Height];
	memset( m_pBuffer,0,sizeof( BYTE )*((dwPitch + 3) & ~3) * SurfaceDesc.Height );

	m_FrameCount = 0;
	m_bRecording = true;

	return true;
}

bool NVMovie::AddFrame()
{
	D3DSURFACE_DESC SurfaceDesc;
	D3DLOCKED_RECT LockedRect;
	DWORD dwDestPitch;
	DWORD dwSourcePitch;

	if (!m_bRecording)
		return false;

	m_pBackBuffer->GetDesc(&SurfaceDesc);

	// Get the pitch of the backbuffer
	switch(SurfaceDesc.Format)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		dwDestPitch = SurfaceDesc.Width * 4;
		break;
	case D3DFMT_R5G6B5:
	case D3DFMT_A1R5G5B5:
		dwDestPitch = SurfaceDesc.Width * 2;
		break;
	}

	m_pBackBuffer->LockRect(&LockedRect, NULL, D3DLOCK_READONLY);
	dwSourcePitch = dwDestPitch;
	if (LockedRect.Pitch != 0)
		dwSourcePitch = LockedRect.Pitch;

	BYTE* pDest = m_pBuffer;
	for (int i = SurfaceDesc.Height - 1; i >= 0; i--)
	{
		LPBYTE pRow = (LPBYTE)LockedRect.pBits + (i * dwSourcePitch);
		memcpy(pDest, pRow, dwDestPitch);
		pDest += ((dwDestPitch + 3) & ~3);
	}

	m_pBackBuffer->UnlockRect();

	HRESULT hr = AVIStreamWrite(m_pCompressedStream,	    
		m_FrameCount++,
		1, 
		m_pBuffer,
		((dwDestPitch + 3) & ~3) * SurfaceDesc.Height,
		AVIIF_KEYFRAME,
		NULL, NULL);



	if (hr != AVIERR_OK)
	{
		return false;	
	}

	return true;
}

bool NVMovie::EndRecord()
{
	if (!m_bRecording)
		return false;

	Free();

	m_bRecording = false;

	return true;
}

# endif 