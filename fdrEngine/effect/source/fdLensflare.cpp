//===================================================================
/** \file  
* Filename :   fdLensflare.cpp
* Desc     :   
* His      :   Windy create @2005-9-14 14:33:34
*/
//===================================================================
# include "stdafx.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../backpipe/include/osCamera.h"
# include "../../mfpipe/include/osShader.h"
# include "../../mfpipe/include/deviceInit.h"
# include "../../terrain/include/fdfieldmgr.h"
#include "../include/fdLensflare.h"

//========================================================================================
os_Lensflare::os_Lensflare() : m_SunRealPos(0.0f, 0.0f, 1000.0f),m_IsRender(true)
{

	m_ClipSize = 0.0f;
	m_numFlareImages = 0;
	m_baseAlpha = 1.0f;
	m_srcPic.blend_mode = 3;

	m_srcPic.m_scrVer[0].m_vecUv= D3DXVECTOR2(0.0f, 0.0f);
	m_srcPic.m_scrVer[1].m_vecUv= D3DXVECTOR2(0.0f, 1.0f);
	m_srcPic.m_scrVer[2].m_vecUv= D3DXVECTOR2(1.0f, 1.0f);
	m_srcPic.m_scrVer[3].m_vecUv= D3DXVECTOR2(1.0f, 0.0f);

}
//========================================================================================
os_Lensflare::~os_Lensflare() 
{
}
//========================================================================================
bool os_Lensflare::init()
{
	m_ptrScene		= get_sceneMgr();
	osassert(m_ptrScene);

	return true;//syq

}
//========================================================================================
void os_Lensflare::clear()
{
	m_numFlareImages = 0;
	m_ClipSize = 0.0f;
	m_FlareDefList.clear();
	m_FlareTexIDList.clear();

}
//========================================================================================
void os_Lensflare::loadStdFlare(const char * _fname)
{
	clear();
	if(!_fname)
		return;
	if( _fname[0] == 0 )
		return;

	if (!file_exist((char *)_fname)){ 
		return;
	}
	std::string filename = _fname;
	m_fname = _fname;
	int         t_i;
	//CIni fini;
	CIniFile    t_file;
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_i = read_fileToBuf( (char*)filename.c_str(),t_ptrStart,TMP_BUFSIZE );
	t_file.OpenMemRead( t_ptrStart,t_i );
	if( !t_file.ReadSection( "FlareImage" ) )
	{
		osassert( false );
	}
	t_file.ReadLine( "NumFlareImages",&m_numFlareImages);
	//fini.Open(filename.c_str());
	///读取基本纹理的个数,和它们的文件名
	//fini.SetSection("FlareImage");
	//fini.ReadLine("NumFlareImages",&m_numFlareImages);
	
	std::string texfileID,texfilename;
	texfileID.reserve(10);
	texfilename.reserve(MAX_PATH);
	m_FlareTexIDList.reserve(m_numFlareImages);
	osassert( 24 >= m_numFlareImages );
	int i;
	for (i = 0 ;i < m_numFlareImages ;i++){

		sprintf((char *)texfileID.c_str(),"%s%d","FlareID",i);
		t_file.ReadLine( (char *)texfileID.c_str(),(char *)texfilename.c_str(),MAX_PATH);
//		fini.ReadLine((char *)texfileID.c_str(),(char *)texfilename.c_str(),MAX_PATH);
		strcpy( m_vecFlareTexName[i],texfilename.c_str() );
	}
	///读取定义的Lens Flare的个数,和各个的描述,并保存到m_FlareDefList列表中 
	int FlareDefNum = 0 ;
	std::string flaresection;
	FlareDef flaredef;
	flaresection.reserve(20);
	if( !t_file.ReadSection( "FlareDefine" ) )
	{
		osassert( false );
	}
	t_file.ReadLine( "FlareNum",&FlareDefNum);

	//fini.SetSection("FlareDefine");
	//fini.ReadLine("FlareNum",&FlareDefNum);
	for ( i = 0 ; i < FlareDefNum ; ++i )
	{
		sprintf((char *)flaresection.c_str(),"%s%d","FlareDefine",i);
		//fini.SetSection(flaresection.c_str());
		if( !t_file.ReadSection( (char*)flaresection.c_str() ) )
		{
			osassert( false );
		}
		///读取各个定义值
		t_file.ReadLine( "Pos",&flaredef.m_fPos);
		//fini.ReadLine("Pos",&flaredef.m_fPos);
		t_file.ReadLine( "TexID",&flaredef.m_nIndex);

		//fini.ReadLine("TexID",&flaredef.m_nIndex);

		t_file.ReadLine( "Size",&flaredef.m_fSize);
		t_file.ReadLine( "Red",&flaredef.m_fRed);
		t_file.ReadLine( "Green",&flaredef.m_fGreen);
		t_file.ReadLine( "Blue",&flaredef.m_fBlue);
		t_file.ReadLine( "Alpha",&flaredef.m_fAlpha);
		

		m_FlareDefList.push_back(flaredef);
		if (flaredef.m_fPos == 0.0f&&m_ClipSize<flaredef.m_fSize)
		{
			m_ClipSize = flaredef.m_fSize;
		}
	}
	t_file.CloseFile();

	END_USEGBUF( t_iGBufIdx );

	// 创建设备相关的资源
	for( int t_i=0;t_i<m_numFlareImages;t_i ++ )
		m_FlareTexIDList[t_i] = m_ptrScene->create_scrTexture( m_vecFlareTexName[t_i] );

	m_ClipSize*=100.0f;	// 缩放值.编辑器和引擎固定的
}
//========================================================================================
void os_Lensflare::setVertsPosSize(float _fPosX, float _fPosY, float _fSize)
{
	//!精确的应该用FOV来计算dy，简单的就不用了
	float	dx = _fSize,
			dy = _fSize ;

	m_srcPic.m_scrVer[0].m_vecPos = D3DXVECTOR4(_fPosX-dx, _fPosY-dy,  0.0f,1.0f);
	m_srcPic.m_scrVer[1].m_vecPos = D3DXVECTOR4(_fPosX-dx, _fPosY+dy,  0.0f,1.0f);
	m_srcPic.m_scrVer[2].m_vecPos = D3DXVECTOR4(_fPosX+dx, _fPosY+dy,  0.0f,1.0f); 
	m_srcPic.m_scrVer[3].m_vecPos = D3DXVECTOR4(_fPosX+dx, _fPosY-dy,  0.0f,1.0f);

}
//========================================================================================
void os_Lensflare::setVertsColor(float _fRed, float _fGreen, float _fBlue, float _fAlpha)
{
	DWORD col = D3DCOLOR_COLORVALUE(_fRed, _fGreen, _fBlue, _fAlpha);

	m_srcPic.m_scrVer[0].m_dwDiffuse = col;
	m_srcPic.m_scrVer[1].m_dwDiffuse = col;
	m_srcPic.m_scrVer[2].m_dwDiffuse = col;
	m_srcPic.m_scrVer[3].m_dwDiffuse = col;
}
//========================================================================================
void os_Lensflare::drawFlarePart(FlareDef & _flareDef,int step)
{
	if(_flareDef.m_fPos != 0.0f){
		if (step == 2)
		{
			drawFlarePart(_flareDef.m_fPos, _flareDef.m_nIndex,_flareDef.m_fSize,_flareDef.m_fRed, _flareDef.m_fGreen, _flareDef.m_fBlue, _flareDef.m_fAlpha*m_baseAlpha);
		}
	}
	else {
		
		if (step==1)
		{
			drawFlarePart(_flareDef.m_fPos, _flareDef.m_nIndex,_flareDef.m_fSize,_flareDef.m_fRed, _flareDef.m_fGreen, _flareDef.m_fBlue, _flareDef.m_fAlpha);
		}
	}

}
//========================================================================================
void os_Lensflare::drawFlarePart(float _fPos, int _nIndex, float _fSize, float _fRed, float _fGreen, float _fBlue, float _fAlpha)
{
	D3DXVECTOR2 center((float)(m_viewWidth/2.0f),(float)(m_viewHeight/2.0f));
	D3DXVECTOR2 d = center - m_screenPos;
	D3DXVECTOR2 p = m_screenPos + _fPos * d;

	setVertsPosSize(p.x,p.y, _fSize*100);
	setVertsColor(_fRed*_fAlpha, _fGreen*_fAlpha, _fBlue*_fAlpha, 1.0f);
	
	m_srcPic.tex_id = m_FlareTexIDList[_nIndex];

	m_ptrScene->push_scrDisEle(&m_srcPic);
}
//========================================================================================
void os_Lensflare::render(I_camera * pCam,int step)
{
	guard;
	if (!m_IsRender){
		return;
	}
	//!避免变换后产生镜像问题（两个太阳）进行z值的比较
	osVec3D campos,camlook,vec3ScrPos,focuspos;
	
	pCam->get_curpos(&campos);
	pCam->get_camFocus(focuspos);
	m_SunRealPos = focuspos + g_vec3LPos*1000;
	pCam->get_godLookVec(camlook);
	D3DXVECTOR3 sunlook = m_SunRealPos - campos;
	if(D3DXVec3Dot(&camlook,&g_vec3LPos)<0){
		return;
	}
	
	pCam->get_scrCoodFromVec3(m_SunRealPos,vec3ScrPos);
	m_screenPos.x = vec3ScrPos.x;
	m_screenPos.y = vec3ScrPos.y;
	osc_camera *posc_cam = (osc_camera *)pCam;
	m_viewWidth = posc_cam->get_viewportwidth();
	m_viewHeight =posc_cam->get_viewportheight();

	float lensFlareClipSize = m_ClipSize;

	/**/
	if(  m_screenPos.x<(0.0f-lensFlareClipSize) || 
	     m_screenPos.x>(m_viewWidth+lensFlareClipSize) || 
		 m_screenPos.y<(0.0f-lensFlareClipSize) || 
		 m_screenPos.y>(m_viewHeight+lensFlareClipSize))
		return;
	
	osVec3D start,direction,intersect;
	osc_TGManager * SceMgrPtr = (osc_TGManager *)m_ptrScene;
	
	float scaleX = 1.0f-fabs((m_screenPos.x/m_viewWidth)*2.0f-1.0f);
	float scaleY = 1.0f-fabs((m_screenPos.y/m_viewHeight)*2.0f-1.0f);
	
	m_baseAlpha = max(scaleX*scaleY,0.0f);
	if(SceMgrPtr->get_rayInterTerrPos(campos,sunlook,intersect))
	{
		m_baseAlpha = 0.0f;
	}

	
	///render
	for (size_t FlarePartID = 0 ; FlarePartID != m_FlareDefList.size();FlarePartID++){
		drawFlarePart(m_FlareDefList[FlarePartID],step);
	}
	//!第二个参数表示，渲染的二维图片用LensFlare的渲染状态。

	
	m_ptrScene->render_scene( true ,true);
	
	unguard;

}
//========================================================================================
int	 os_Lensflare::get_partNum()
{
	return (int)m_FlareDefList.size();
}
//========================================================================================
void os_Lensflare::addOnePart()
{
	FlareDef flaredef;
	flaredef.m_fSize = 1.0f;
	flaredef.m_fAlpha = 1.0f;
	flaredef.m_nIndex = 0;
	flaredef.m_fPos = 0.0f;
	flaredef.m_fRed = flaredef.m_fGreen = flaredef.m_fBlue = 1.0f;
	m_FlareDefList.push_back(flaredef);
}
//========================================================================================
void os_Lensflare::delOnePart(int iPartID)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	std::vector<FlareDef>::iterator iter;
	iter = m_FlareDefList.begin() + iPartID;
	m_FlareDefList.erase(iter);
}
//========================================================================================
void os_Lensflare::save_file(const char * _fname)
{
	//FlareDef part;
	CIni fini;
	fini.Open(m_fname.c_str());
	
	int FlareDefNum = (int)m_FlareDefList.size() ;
	std::string flaresection;
	FlareDef flaredef;
	flaresection.reserve(20);
	fini.SetSection("FlareDefine");
	fini.WriteLine("FlareNum",FlareDefNum);
	for ( int i = 0 ; i < FlareDefNum ; ++i )
	{
		sprintf((char *)flaresection.c_str(),"%s%d","FlareDefine",i);
		fini.SetSection(flaresection.c_str());
		///读取各个定义值
		fini.WriteLine("Pos",m_FlareDefList[i].m_fPos);
		fini.WriteLine("TexID",m_FlareDefList[i].m_nIndex);
		fini.WriteLine("Size",m_FlareDefList[i].m_fSize);
		fini.WriteLine("Red",m_FlareDefList[i].m_fRed);
		fini.WriteLine("Green",m_FlareDefList[i].m_fGreen);
		fini.WriteLine("Blue",m_FlareDefList[i].m_fBlue);
		fini.WriteLine("Alpha",m_FlareDefList[i].m_fAlpha);
		//m_FlareDefList.push_back(flaredef);
	}

	


}
//========================================================================================
void os_Lensflare::set_pos(int iPartID,const float & _fpos)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	m_FlareDefList[iPartID].m_fPos = _fpos;

}
//========================================================================================
void os_Lensflare::get_pos(int iPartID,float & _fpos)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	_fpos = m_FlareDefList[iPartID].m_fPos;
}
//========================================================================================
void os_Lensflare::set_texid(int iPartID,const int & _texID)
{
	osassert(0 <= iPartID&&iPartID<(int)m_FlareDefList.size());
	if (0 <= _texID && _texID <m_numFlareImages){
		m_FlareDefList[iPartID].m_nIndex = _texID;
	}
}
//========================================================================================
void os_Lensflare::get_texid(int iPartID,int & _texID)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	_texID = m_FlareDefList[iPartID].m_nIndex;
}
//========================================================================================
void os_Lensflare::get_size(int iPartID,float & _size)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	_size = m_FlareDefList[iPartID].m_fSize;
}
//========================================================================================
void os_Lensflare::set_size(int iPartID,const float & _size)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	m_FlareDefList[iPartID].m_fSize = _size;
}
//========================================================================================
void os_Lensflare::set_color(int iPartID,const D3DXCOLOR & _color)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	m_FlareDefList[iPartID].m_fRed = _color.b;
	m_FlareDefList[iPartID].m_fGreen = _color.g;
	m_FlareDefList[iPartID].m_fBlue = _color.r;
}
//========================================================================================
void os_Lensflare::get_color(int iPartID,D3DXCOLOR & _color)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	_color.b = m_FlareDefList[iPartID].m_fRed;
	_color.g = m_FlareDefList[iPartID].m_fGreen;
	_color.r = m_FlareDefList[iPartID].m_fBlue;
}
//========================================================================================
void os_Lensflare::set_alpha(int iPartID,const float & _alpha)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	m_FlareDefList[iPartID].m_fAlpha = _alpha;
}
//========================================================================================
void os_Lensflare::get_alpha(int iPartID,float & _alpha)
{
	osassert(0<=iPartID&&iPartID<(int)m_FlareDefList.size());
	_alpha = m_FlareDefList[iPartID].m_fAlpha;
}