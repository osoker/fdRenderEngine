//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: dlight.h
 *
 *  Desc:     ���������еĶ�̬�⣬���̬����Ҫ�ͳ������ϡ�
 *
 *  His:      River created @ 2004-3-3
 *
 *  "������ʿ,Ϊ��֮��"  
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once


# include "../../interface/osInterface.h"


# include "../../mfpipe/include/osShader.h"                 // ʹ��ShaderMgr������ص��豸�����Դ��
# include "../../mfpipe/include/middlePipe.h"


//! �Ƿ�ʹ��ʵʱ�ĵƹ�,���Ҫ����lightMap�������ö���ɫ��ģ��ƹ⣬�˴�Ϊ0
# define REALTIME_LIGHT  1


//! ��̬��Ӱ������Χ����4�������϶���6������,����18�ס�
# define MAXTILE_DLEFFECT    8



/** \brief
 *  �����ж�̬������ݽṹ����
 *  
 */
struct osc_dlight
{
	//! ��̬��λ�á�
	osVec3D    m_vec3Pos;

	//! ��ǰ��̬�����ɫ��
	osColor    m_dlightColor;

	//! ��ǰ��̬���Ӱ�췶Χ��
	float      m_fRadius;

	//! ��ǰ�ṹ�Ƿ���ʹ���С�
	BOOL       m_bInuse;


	//@{
	//  ��̬���ֵ��ء�
	//! Դ������ɫ��Ŀ�������ɫ��
	osColor            m_sSrcColor;
	osColor            m_sDstColor;
	//! �ܹ���Ҫ��ֵ��ʱ�䣬�͵�ǰ���ڵ�ʱ��ֵ��
	float              m_fTolLerpTime;
	float              m_fCurTime;
	//! ��ǰ�Ƿ��ڲ�ֵ״̬��
	BOOL               m_bInLerpState;
	//@} 

public:
	osc_dlight();

	//! ��������̬�������֮����в�ֵ��
	void                lerp_DLight( float _time );

	//! ��ʼ��̬���ֵ��
	void                start_dLightFadeOut( float _totalTime ); 

};
typedef std::vector< osc_dlight >  VEC_dlight;



//! ȫ�ֵĶ�̬����Ϣ
struct os_dlInfo
{
	int                m_iDLNum;

	//! ÿһ����̬���Ӧ�Ķ�̬��ָ��.
	const osc_dlight*  m_vecDLPtr[MAX_LIGHTNUM];
	//! ÿһ����̬���ڶ�̬��������ڵ�ID
	int                m_iDLId[MAX_LIGHTNUM];
};

/** \brief
 *  ����̬��Ĺ�������
 *  
 *  ATTENTION TO FIX: ��������̬���ʱ�򣬿��ܻ��ڴ�������⣬�����ض��������²��ܳ��֣�
 *                     ��������ʱ���ԡ�
 */
class osc_dlightMgr
{
	friend class     osc_TGManager;
private:
	//! ��ǰ�Ķ�̬�����ݡ�
	osc_dlight       m_vecDlight[MAX_LIGHTNUM];

	//! ��ǰlightMgr�б�����м�ܵ���ָ�롣
	osc_middlePipe*   m_ptrMpipe;

	//! ��̬���õ�������ʹ�õ�shaderid.
	int              m_iShaderId;

	//! dx�ĵƹ�ṹ����ͨ��Ʒʹ����Щ�ṹ��
	D3DLIGHT9        m_vecDXLight[MAX_LIGHTNUM];
	//! ��ǰ�õ���dx�ƹ���Ŀ��
	int             m_iDxLightNum;

private:

	void fill_dxGDirLight( D3DLIGHT9& _dxL );

public:
	osc_dlightMgr();
	~osc_dlightMgr();


	//! ��ʼ����̬���������
	void          init_dlightMgr( osc_middlePipe* _mpipe );

	//! �ͷŶ�̬�������
	void          reset_dlightMgr( void );

	//! ֡��move��̬�������( ������ɾ����̬��ʱ����Ҫ����)
	void          frame_moveDLMgr( void );

	/** \brief
	 *  ����һ����̬�⡣
	 *
	 *  \return int  �������-1,�򴴽�ʧ�ܡ�
	 */
	int           create_dynamicLight( os_dlightStruct& _dl );

	/** \brief
	 *  �õ�һ����̬��ĸ��������
	 */
	void          get_dynamicLight( int _id,os_dlightStruct& _dl );

	/** \brief
	 *  FrameMoveһ����̬�⡣
	 */
	void          set_dynamicLight( int _id,os_dlightStruct& _dl );

	/** \brief
	 *  ���ö�̬���λ�á�
	 */
	void          set_dLightPos( int _id,osVec3D& _pos );


	/** \brief
	 *  ɾ��һ����̬�⡣
	 */
	void          delete_dynamicLight( int _id,float _fadeTime = 0.6f );

	/** \brief
	 *  ���õ�ǰ�����еĶ�̬��Ϊdx���ݸ�ʽ��
	 *
	 *  \param _enable ��ֵΪtrueʱ���򿪵ƹ⣬����رյƹ⡣
	 *  \param bool _fullAmbi �Ƿ��㻷����.
	 */
	void          dxlight_enable( bool _enable,bool _fullAmbi = false );

	//! �õ���̬��������ڵ���Ϣ
	void          get_dlINfo( os_dlInfo& _info );

	//! ����һ����̬��
	void          set_pointLight( os_meshDLight& _dl,bool _enable = true );

	


};


extern osc_dlightMgr*   g_ptrDLightMgr;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ��̬��λ�õĸı䡣
/** \brief
*  ���ö�̬���λ�á�
*/
inline void osc_dlightMgr::set_dLightPos( int _id,osVec3D& _pos )
{
	osc_dlight*  t_ptrL;

	osassert( (_id>=0)&&(_id<MAX_LIGHTNUM) );

	t_ptrL = &this->m_vecDlight[_id];
	osassert( t_ptrL->m_bInuse );

	t_ptrL->m_vec3Pos = _pos;

}

