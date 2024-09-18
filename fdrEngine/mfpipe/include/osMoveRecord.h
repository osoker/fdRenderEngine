//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osTexture.h
 *
 *   Desc:     O.S.O.K 引擎的纹理管理器,对中间管理开放.
 *
 *   His:      River Created @ 4/27 2003
 *
 *   "微雨过,小荷翻,榴花开欲然,
 *    玉盆纤手弄清泉,琼珠碎却圆."
 */
//--------------------------------------------------------------------------------------------------------
# pragma    once

# include "../../interface/osInterface.h"



#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.

# if __MOVIE_RECORD__

class NVMovie
{

public:
	NVMovie()
	: m_bRecording(false),
		m_pFile(NULL),
		m_pBackBuffer(NULL),
		m_pBuffer(NULL),
		m_pCompressedStream(NULL),
		m_pStream(NULL)
	{
	}


	virtual ~NVMovie()
	{
		Free();
	}


	virtual bool BeginRecord(char* strFileName, LPDIRECT3DSURFACE9 pBackBuffer);
	virtual bool AddFrame();
	virtual bool EndRecord();


protected:
	virtual void Free()
	{
		if (m_pCompressedStream)
		{
			AVIStreamClose(m_pCompressedStream);
			m_pCompressedStream = NULL;
		}

		if (m_pStream)
		{
			AVIStreamClose(m_pStream);
			m_pStream = NULL;
		}

		if (m_pFile)
		{
			AVIFileClose(m_pFile);
			AVIFileExit();
			m_pFile = NULL;
		}

		if (m_pBackBuffer)
		{
			m_pBackBuffer->Release();
			m_pBackBuffer = NULL;
		}

		delete []m_pBuffer;

		m_pBuffer = NULL;
	}

	BITMAPINFOHEADER m_BitmapInfoHeader;
	PAVISTREAM	m_pCompressedStream;
	PAVISTREAM	m_pStream;
    PAVIFILE	m_pFile;

	BYTE* m_pBuffer;
    
	LPDIRECT3DSURFACE9 m_pBackBuffer;
	bool m_bRecording;
	int m_FrameCount;

};

# endif 