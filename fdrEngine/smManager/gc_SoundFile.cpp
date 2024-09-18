#include "gc_SoundFile.h"
#include "../interface/fileOperate.h"


#ifndef SOUNDMANAGER_CLOSE
#	define SOUNDMANAGER_CLOSE		1
#endif

# pragma comment(lib,"dsound")


//////////////////////////////////////////////////////////////////////////
/** \brief
*  构造函数
*/
fdWaveFile::fdWaveFile()
{
	m_dwSize    =	0;
	m_pwfx		=	NULL;
}

/** \brief
*  析构函数
*/
fdWaveFile::~fdWaveFile()
{
	if(m_pwfx)
	{
		delete[] m_pwfx;
		m_pwfx = NULL;
	}
}

/** \brief
*  得到文件的大小
* 
*  \return   DWORD         返回媒体数据的大小 
*/
DWORD fdWaveFile::GetSize()
{
	guard;
	return m_dwSize;
	unguard;
}

/** \brief
* 获得wave的格式和数据部分
*
* \param  pRes        整个wave文件在内存中的位置
* \param  pWaveHeader 返回wave文件的格式
* \param  pbWaveData  wave的数据部分
* \param  cbWaveSize  wave数据部分的长度
* 
* \return BOOL        是否读取wave数据成功
*/
BOOL fdWaveFile::GetWaveData(void * pRes, WAVEFORMATEX * & pWaveHeader, void * & pbWaveData, DWORD & cbWaveSize) 
{
	if(m_pwfx){ delete m_pwfx;m_pwfx = NULL;}
	char* pFilePtr	=	(char*)pRes;
    MMCKINFO		m_ckRiff;      // Use in opening a WAVE file
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       
	memcpy(&m_ckRiff,pFilePtr,sizeof(m_ckRiff.ckid)+sizeof(m_ckRiff.cksize)+sizeof(m_ckRiff.fccType));
	int xx = sizeof(m_ckRiff.ckid)+sizeof(m_ckRiff.cksize);
//	pFilePtr+=sizeof(m_ckRiff.ckid)+sizeof(m_ckRiff.cksize)+sizeof(m_ckRiff.fccType);
	m_ckRiff.dwDataOffset	=	sizeof(m_ckRiff.ckid)+sizeof(m_ckRiff.cksize)+sizeof(m_ckRiff.fccType)-sizeof(m_ckRiff.ckid);
	m_ckRiff.dwFlags		=	0;
	DWORD	uuu = FOURCC_RIFF;
	DWORD	www = mmioFOURCC('W', 'A', 'V', 'E');
    if( (m_ckRiff.ckid != FOURCC_RIFF) ||
        (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
	{
		return false;
	}
 	www = mmioFOURCC('f', 'm', 't', ' ');
	//! 跨过上一个chunk头
	pFilePtr+=m_ckRiff.dwDataOffset+sizeof(m_ckRiff.ckid);
	memcpy(&ckIn,pFilePtr,sizeof(ckIn.ckid)+sizeof(ckIn.cksize)+sizeof(ckIn.fccType));
	ckIn.dwDataOffset	=	m_ckRiff.dwDataOffset+(sizeof(ckIn.ckid)+sizeof(ckIn.cksize)+sizeof(ckIn.fccType));
	ckIn.dwFlags		=	0;
    if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
	{
        return false;
	}
    // Read the 'fmt ' chunk into <pcmWaveFormat>.
//	pFilePtr+=ckIn.dwDataOffset+sizeof(m_ckRiff.ckid);
	pFilePtr	=	(char*)pRes;
	pFilePtr+=sizeof(MMCKINFO);
	memcpy(&pcmWaveFormat,pFilePtr,sizeof(PCMWAVEFORMAT));
	pFilePtr+=ckIn.dwDataOffset;
    if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
    {
		m_pwfx = (WAVEFORMATEX*)new char[ sizeof(WAVEFORMATEX) ];
        if( NULL == m_pwfx )
            return false;
        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = 0;
    }else
	{
        WORD cbExtraBytes = 0L;
		memcpy(&cbExtraBytes,pFilePtr,sizeof(WORD));
		pFilePtr+=sizeof(WORD);
        m_pwfx = (WAVEFORMATEX*)new char[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
        if( NULL == m_pwfx )
            return false;
        memcpy(m_pwfx,&pcmWaveFormat,sizeof(pcmWaveFormat));
        m_pwfx->cbSize = cbExtraBytes;
		memcpy((CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)),pFilePtr,cbExtraBytes);
 		pFilePtr+=cbExtraBytes;
	}
	pWaveHeader	=	m_pwfx;	

	m_dwSize	=	0;
	pWaveHeader = 0;
	pbWaveData	= 0;
	cbWaveSize	= 0;

	DWORD * pdw		= (DWORD *)pRes;
	DWORD dwRiff	= *pdw++;
	DWORD dwLength	= *pdw++;
	DWORD dwType	= *pdw++;

	if( dwRiff != mmioFOURCC('R', 'I', 'F', 'F') )
		return FALSE;      // not a RIFF

	if( dwType != mmioFOURCC('W', 'A', 'V', 'E') )
		return FALSE;      // not a WAV

	DWORD * pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

	while( pdw < pdwEnd ) {
		dwType = *pdw++;
		dwLength = *pdw++;
		switch( dwType ) {
			case mmioFOURCC('f', 'm', 't', ' '):
				if( !pWaveHeader ) {
					if( dwLength < sizeof(WAVEFORMAT) )
						return FALSE;      // not a WAV

					pWaveHeader = (WAVEFORMATEX *)pdw;
                    CopyMemory( &m_wfx, pWaveHeader, sizeof(WAVEFORMATEX) );
					if( pbWaveData && cbWaveSize )
						return TRUE;
				}
				break;

			case mmioFOURCC('d', 'a', 't', 'a'):
				pbWaveData = LPVOID(pdw);
				m_dwSize = cbWaveSize = dwLength;

				if( pWaveHeader )
					return TRUE;
				break;
		}
		pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
	}

	return FALSE;
}

# define MAX_SOUNDFILESIZE (768*1024)
PBYTE gc_SoundFile::m_pbStart = NULL;

bool gc_SoundFile::load( const char* lpszFileName/* = NULL*/ )
{
	DWORD dwWavSize;
	WAVEFORMATEX wfx;
	float fPlayTime = 0.0f;

	if( lpszFileName != NULL )
		m_szName	=	lpszFileName;

	ZeroMemory( &m_DSBD, sizeof(DSBUFFERDESC) );
	m_DSBD.dwSize			= sizeof(DSBUFFERDESC);
    DWORD	dwDSBufferSize	= NULL;



	m_pbData  = NULL;
	int iSize = get_fileSize(const_cast<char*>(m_szName.c_str() ));
	//! River @ 2010-4-27:降低挂机错误发生的概率
	if( iSize <= 0 )
		return false;

	// River @ 2011-2-14:没有必要再使用全局的那个缓冲区，创建一个缓冲区，然后播放.
	if( !m_pbStart )
		m_pbStart	=	new BYTE[MAX_SOUNDFILESIZE];
	if( iSize > MAX_SOUNDFILESIZE )
	{
		delete[] m_pbStart;
		m_pbStart	=	new BYTE[iSize];
	}

	read_fileToBuf(const_cast<char*>( m_szName.c_str() ),m_pbStart,iSize);	
    fdWaveFile	_pWaveFile;

	_pWaveFile.GetWaveData(m_pbStart, m_DSBD.lpwfxFormat,m_pbData,dwDSBufferSize);
	
	// Blank wave file
	if ( dwDSBufferSize == 0 )
		return false;

	// 3D控制时声道数目不能超过1
	if (m_DSBD.lpwfxFormat->nChannels == 1)
	{
		m_DSBD.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME|
			DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCDEFER| DSBCAPS_MUTE3DATMAXDISTANCE;
		//!only used when DSBCAPS_CTRL3D is set
		m_DSBD.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
	}
	else if(m_DSBD.lpwfxFormat->nChannels >= 2)
	{
		// 多声道音效控制标识
		m_DSBD.dwFlags =  DSBCAPS_CTRLVOLUME|DSBCAPS_GLOBALFOCUS; 
		osDebugOut("SOUND: 设置3D控制时声道数不能超过1!");
		m_DSBD.guid3DAlgorithm = DS3DALG_DEFAULT;
	}
	//!should be 0 for primary buffer 
    m_DSBD.dwBufferBytes   = dwDSBufferSize;

	// 计算音效的播放时间
	dwWavSize = _pWaveFile.GetSize();
	wfx = _pWaveFile.GetFormat();
	m_fSoundPlayTime = dwWavSize/(float)wfx.nAvgBytesPerSec;


	return true;
}
bool gc_SoundFile::destory()
{
	m_pbData = NULL;
	return true;
}
