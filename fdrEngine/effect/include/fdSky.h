//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSky.h
 *
 *  Desc:     处理引擎中的天空显示, 用于显示不同的天空.
 *
 *  His:      River created @ 2004-3-15
 *
 *  "世界上最虚弱最不牢靠的东西，莫过于不以自己力量为基础的权力带来的声誉了。"
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"
# include "../../mfpipe/include/osPolygon.h"

//! 对开空圆细分的数目.
# define  SKY_DIV_NUM        24



class osc_middlePipe;

/** \brief
 *  初始化天空需要的数据结构.
 */
struct os_skyInit
{
	osVec3D   m_vec3CloudDir;

	float     m_fSkyMoveSpeed;

	//! 天空纹理的名字。
	char      m_szSkyName[128];

	//! 天空osa文件的名字
	char      m_szBkSkyOsa[128];

	//! 远景osa文件的文字.
	char      m_szBkTerrOsa[128];

	//! 创建shader需要的ini文件。
	char      m_szIni[128];

	//! 根据相机的高度来决定天空的高度.
	float     m_fSkyHeight;

	osc_middlePipe* m_mdPipe;
	//@{
	// Windy mod @ 2005-9-11 10:06:48
	//Desc: 3 sky name 第三层天空的名字 第三层天空不做过渡，
///	char	  m_szSkyOsa3[128];
	//@}
};

// ATTENTION TO OPP:加入并显示背景的天空。osa。然后再处理两个天空的过渡。

class   osc_camera;

/** \brief
 *  简单的测试天空的类.
 *
 *  目前天空使用一个正方形做出,但使用多个顶点,以后测试使用多个顶点和多层贴图.
 *  
 */
class osc_fdSkyMgr
{
private:
	/** \brief
	 *  保存天空的几何数据.
	 */
	os_polygon        m_rPolygon;
	int               m_indexNum;

	//! 保存中间管道的指针.
	osc_middlePipe*   m_mdPipe;

	//! 云层移动的方向,只能在x,z方向上移动.
	osVec3D          m_vec3MovSpd;

	//! 云层移动的速度.
	float            m_fMovSpeed;

	//! 天空的高度.
	float            m_fSkyHeight;

	//! 当前云层用到的纹理的名字
	char             m_szShaderName[128];


	//! 天空shader用到的ini的文件名。
	char             m_szIniName[128];

	//! 当前天空的shaderId.
	int              m_iSId;


	int              m_iSkyVerNum;
	//! 当前天空用到的顶点,第一个顶点为中心点.
	os_billBoardVertex     m_vecSkyVer[SKY_DIV_NUM*2+1];


	//! 当前天空渲染需要的三角形的索引.
	WORD                  m_arrSkyTriIdx[SKY_DIV_NUM*3*3];


	//! 需要渲染第0份和第三份天空,两次调用dip.
	bool                  m_bRenderTw;


public:
	osc_fdSkyMgr();
	~osc_fdSkyMgr();

	/** \brief
	 *  初始化我们的skyMgr.
	 */
	void             init_skyMgr( os_skyInit& _sinit );

	/** \brief
	 *  使用中间管道对天空进行渲染.
	 *
	 *  \param _camyaw 当前相机在水平方向的旋转角度,用于优化天空的渲染.
	 *  
	 */
	void             render_sky( osc_middlePipe* _pipe,float _tm,osVec3D _center,
		              float _yaw,float _fardis,bool _reflect );


	//! 重设远景的osa文件.
	void             reset_osaFarTerr( const char* _bkTerr );

	/** \brief
	 *  重设天空用到的贴图。
	 *
	 *  \param _bkSky 背景的天空,使用的是osa文件.
	 *  \param _bkSky3 是第三层天空的OSA文件
	 */
	void             reset_skyTexture( const char* _bkSky,const char* _bkSky1,const char* _bkSky2 ,const char* _bolt = NULL,float rate = 1.0f,float _angle=0.0f,osColor _color = osColor(1.0f,1.0f,1.0f,1.0f),float _boltheight = 20.0f);

	//! 重设天空的移动速度。
	void             reset_skyMoveSpeed( float _spd );

	//! 得到天空相关的参数。
	const char*       get_skyTexture( void );

	//! 得到背景天空相关的参数.
	const char*       get_bkSkyOsa( void );

	//! 得到天空的移速度。
	float            get_skyMoveSpeed( void );

	const char*		get_bkSkyOsa1();
	const char*		get_bkSkyOsa2();
	const char*		get_bolt();
	float			get_boltRate();


};