//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSky.h
 *
 *  Desc:     ���������е������ʾ, ������ʾ��ͬ�����.
 *
 *  His:      River created @ 2004-3-15
 *
 *  "��������������ο��Ķ�����Ī���ڲ����Լ�����Ϊ������Ȩ�������������ˡ�"
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"
# include "../../mfpipe/include/osPolygon.h"

//! �Կ���Բϸ�ֵ���Ŀ.
# define  SKY_DIV_NUM        24



class osc_middlePipe;

/** \brief
 *  ��ʼ�������Ҫ�����ݽṹ.
 */
struct os_skyInit
{
	osVec3D   m_vec3CloudDir;

	float     m_fSkyMoveSpeed;

	//! �����������֡�
	char      m_szSkyName[128];

	//! ���osa�ļ�������
	char      m_szBkSkyOsa[128];

	//! Զ��osa�ļ�������.
	char      m_szBkTerrOsa[128];

	//! ����shader��Ҫ��ini�ļ���
	char      m_szIni[128];

	//! ��������ĸ߶���������յĸ߶�.
	float     m_fSkyHeight;

	osc_middlePipe* m_mdPipe;
	//@{
	// Windy mod @ 2005-9-11 10:06:48
	//Desc: 3 sky name ��������յ����� ��������ղ������ɣ�
///	char	  m_szSkyOsa3[128];
	//@}
};

// ATTENTION TO OPP:���벢��ʾ��������ա�osa��Ȼ���ٴ���������յĹ��ɡ�

class   osc_camera;

/** \brief
 *  �򵥵Ĳ�����յ���.
 *
 *  Ŀǰ���ʹ��һ������������,��ʹ�ö������,�Ժ����ʹ�ö������Ͷ����ͼ.
 *  
 */
class osc_fdSkyMgr
{
private:
	/** \brief
	 *  ������յļ�������.
	 */
	os_polygon        m_rPolygon;
	int               m_indexNum;

	//! �����м�ܵ���ָ��.
	osc_middlePipe*   m_mdPipe;

	//! �Ʋ��ƶ��ķ���,ֻ����x,z�������ƶ�.
	osVec3D          m_vec3MovSpd;

	//! �Ʋ��ƶ����ٶ�.
	float            m_fMovSpeed;

	//! ��յĸ߶�.
	float            m_fSkyHeight;

	//! ��ǰ�Ʋ��õ������������
	char             m_szShaderName[128];


	//! ���shader�õ���ini���ļ�����
	char             m_szIniName[128];

	//! ��ǰ��յ�shaderId.
	int              m_iSId;


	int              m_iSkyVerNum;
	//! ��ǰ����õ��Ķ���,��һ������Ϊ���ĵ�.
	os_billBoardVertex     m_vecSkyVer[SKY_DIV_NUM*2+1];


	//! ��ǰ�����Ⱦ��Ҫ�������ε�����.
	WORD                  m_arrSkyTriIdx[SKY_DIV_NUM*3*3];


	//! ��Ҫ��Ⱦ��0�ݺ͵��������,���ε���dip.
	bool                  m_bRenderTw;


public:
	osc_fdSkyMgr();
	~osc_fdSkyMgr();

	/** \brief
	 *  ��ʼ�����ǵ�skyMgr.
	 */
	void             init_skyMgr( os_skyInit& _sinit );

	/** \brief
	 *  ʹ���м�ܵ�����ս�����Ⱦ.
	 *
	 *  \param _camyaw ��ǰ�����ˮƽ�������ת�Ƕ�,�����Ż���յ���Ⱦ.
	 *  
	 */
	void             render_sky( osc_middlePipe* _pipe,float _tm,osVec3D _center,
		              float _yaw,float _fardis,bool _reflect );


	//! ����Զ����osa�ļ�.
	void             reset_osaFarTerr( const char* _bkTerr );

	/** \brief
	 *  ��������õ�����ͼ��
	 *
	 *  \param _bkSky ���������,ʹ�õ���osa�ļ�.
	 *  \param _bkSky3 �ǵ�������յ�OSA�ļ�
	 */
	void             reset_skyTexture( const char* _bkSky,const char* _bkSky1,const char* _bkSky2 ,const char* _bolt = NULL,float rate = 1.0f,float _angle=0.0f,osColor _color = osColor(1.0f,1.0f,1.0f,1.0f),float _boltheight = 20.0f);

	//! ������յ��ƶ��ٶȡ�
	void             reset_skyMoveSpeed( float _spd );

	//! �õ������صĲ�����
	const char*       get_skyTexture( void );

	//! �õ����������صĲ���.
	const char*       get_bkSkyOsa( void );

	//! �õ���յ����ٶȡ�
	float            get_skyMoveSpeed( void );

	const char*		get_bkSkyOsa1();
	const char*		get_bkSkyOsa2();
	const char*		get_bolt();
	float			get_boltRate();


};