//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdRain.h
 *
 *  Desc:     ������������Ч�Ĵ���
 *
 *  His:      River created @ 2004-7-3
 *
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"


/** \brief
 *  �����е�����Ч����
 *
 *  ����Ч��ʹ���ڳ����л��߶���ʵ�֡�
 *  
 */
class osc_rain
{
private:

	float		m_fWidth;
	float		m_fHeight;
	
	float		m_fRainLength;

	osVec3D 	m_vec3Velocity;

	//! �豸��ص����ݡ�
	osc_middlePipe*  m_ptrMPipe;

	//! ��ǰ������ݶ�Ӧ��dynamicVertex bufferID.
	int             m_iVBid;
	//! ��ǰ��������õ���Index buffer���ݡ�
	int             m_iIBid;

	//! ��ǰ��ζ�Ӧ��effectID
	int             m_iEffectId;

	//! ��Ⱦ��ǰ��Effectʹ�õ�polygon.
	os_polygon       m_sPolygon;

	//! ��ζ�Ӧ�Ķ������ݡ�
	os_sceVerDiffuse* m_vecRainVer;

	//! ��ε�λ�����ݡ�
	osVec3D*         m_vecRainPos;
	//! ÿһ����ζ�Ӧ�������ٶȡ�
	float*           m_vecSpeed;



	//! ÿһ����ζ���������µ��ı������ݹ��ɡ�
	os_sceVerDiffuse  m_vecRainVerOffset[4];

	//! ��ʼֹͣ���ꡣ
	bool            m_bStopRaining;

	//! ��ǰ�Ƿ�������
	bool            m_bRainning;


	//! ������Ч���������������ݽṹ
	os_rainStruct    m_sRainStart;
	//! �ⲿ�޸ĺ������ʼ����С
	os_sceVerDiffuse m_vecRainVerStart[4];

private:
	//! ������Ч����FrameMove����
	void       frame_moveRain( osVec3D& _campos );

	//! ������Ⱦ���Ч��ʹ�õ������
	void       set_rainRenderCam( float _pitch );

	/** \brief
	 *  ��ʼ������Ч�����豸������ݡ�
	 *
	 *  ���������init_rainData���Զ����ã��Ժ�ÿ���ͷ��˳����е��豸���
	 *  ���ݺ���Ҫ�ٵ�������������ָ�����Ч�����豸������ݡ�
	 */
	void       init_rainDevdepData( void );

public:
	osc_rain();
	virtual ~osc_rain();

	//! ��ʼ�����Ч�������ݡ�
	void       init_rainData( osc_middlePipe* _mpipe );


	/** \brief
	 *  ��Ⱦ��ǰ����Ч����
	 *
	 *  \param _pitch  ��ǰ�������������pitchֵ��
	 *  \param _pos    ��ǰ�����������λ�á�
	 */
	void       render_rain( osVec3D& _pos,float _pitch );


	//! ��ʼ����Ч����
	void       start_rain( os_rainStruct* _rs = NULL );


	//! ���������Ч����
	void       stop_rain( void );


	//! �õ��Ƿ���������.
	bool       get_rainning( void );



};


//! ����������ѩ��



//! ѩ�������ݽṹ
struct os_snowStruct
{
	//! ѩ���Ĵ�С
	float   m_fSize;
	
	//! ѩ������ķ���ͼ��ٶȡ�
	osVec3D m_vec3Velocity;

	//! ѩ����ǰ��λ��.
	osVec3D m_ve3SnowPos;

};

/** \brief
 *  ��ѩЧ���Ĺ�����
 *
 *  ��ѩ��Ҫʵ�ֵ��㷨��
 *  1: �ȳ�������������,�������㻺��������������������effectId.
 *  2: ��ʼ��ѩ�󣬳�ʼ��ѩ�������ݽṹ��
 *  3: ÿһ֡��ѩ��FrameMove.
 *  4: �����������ȷ����Ⱦѩ����
 *  5: ֹͣ��ѩ��Ĵ���
 */
class osc_snowStorm
{
private:
	//! ѩ�������ݽṹ
	os_snowStruct*    m_vecSnowData;


	//! �豸��ص����ݡ�
	osc_middlePipe*  m_ptrMPipe;
	//! ��ǰѩ�����ݶ�Ӧ��dynamicVertex bufferID.
	int             m_iVBid;
	//! ��ǰѩ�������õ���Index buffer���ݡ�
	int             m_iIBid;
	//! ��ǰѩ����Ӧ��effectID
	int             m_iEffectId;
	//! ��Ⱦ��ǰ��Effectʹ�õ�polygon.
	os_polygon       m_sPolygon;
	//! ÿһ��ѩ������������µ��ı������ݹ��ɡ�
	//os_sceVerDiffuse  m_vecSnowVerOffset[4];
	os_SnowVertex  m_vecSnowVerOffset[4];

	//! ѩ����Ӧ�Ķ�������
	//os_sceVerDiffuse* m_vecSnowRenderVer2;
	os_SnowVertex*	m_vecSnowRenderVer;
	osVec3D*			m_vecSnowPos;
	osVec3D*				m_vecSpeed;

	//! ��ʼֹͣ��ѩ��
	bool            m_bStopSnowing;

	//! ��ǰ�Ƿ�����ѩ
	bool            m_bSnowing;

	os_snow			mSnowSetting;
private:
	//! ����ѩЧ����FrameMove����
	void       frame_moveSnow( osVec3D& _campos );

	//! ������ȾѩЧ��ʹ�õ������
	void       set_snowRenderCam( float _pitch );

public:
	osc_snowStorm();
	~osc_snowStorm();

	//! ��ʼ��������ѩ�Ĺ�����
	void    init_snowStormMgr( osc_middlePipe* _mpipe );

	//! ��ʼ��ѩ
	void    start_snowStorm( os_snow& snowSetting );

	//! ������ѩ
	void    stop_snowStorm( void );

	/** \brief
	 *  ��Ⱦ��ǰ��ѩЧ����
	 *
	 *  \param _pitch  ��ǰ�������������pitchֵ��
	 *  \param _pos    ��ǰ�����������λ�á�
	 */
	void       render_snowStorm( osVec3D& _pos,float _pitch );


	//! �õ��Ƿ���ѩ
	bool       get_weatherSnowing( os_snow* snowSetting = NULL);

	//! 
private:
	//! ����ѩЧ����FrameMove����
	void       frame_moveRain( osVec3D& _campos );

};




///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  
//! ���������Ч����
inline void osc_rain::stop_rain( void )
{
	m_bStopRaining = true;
}
//! �õ��Ƿ���������.
inline bool osc_rain::get_rainning( void )
{
	return !m_bStopRaining;
}


