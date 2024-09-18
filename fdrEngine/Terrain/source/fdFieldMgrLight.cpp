//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgrLight.cpp
 *
 *  His:      River created @ 2004-5-9
 *
 *  Des:      �����еĹ��պͶ�̬����صĺ�������
 *   
 *  "ʥ�˲����Ѳ�����δ�����������ң���δ��"
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include  "../../mfpipe/include/deviceInit.h"
# include  "../../mfpipe/include/osCharacter.h"


//! �Գ����еĶ�����ɫ����ʹ�û�����ϵ�����д���
void osc_TGManager::reset_ambientCoeff( void )
{
	guard;


	for( int t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{
		osassert( this->m_arrPtrTG[t_i] );



		m_arrPtrTG[t_i]->set_ambientLCoeff( g_fAmbiR,
			g_fAmbiG,g_fAmbiB );


	}



	return;

	unguard;
}

void osc_TGManager::start_sceneLight(float _times,float frequency,int flashtype,float flashTime,osColor _color /*= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f)*/)
{
	m_SceneFlashLightMgr->start_sceneLight(_times,frequency,flashtype,flashTime,_color);

}
void osc_TGManager::stop_sceneLight()
{
	m_SceneFlashLightMgr->stop();
}
/** \brief
*  �ı䳡���Ļ�����ϵ����
*
*  \param _r,_g,_b  �����л������ϵ����ʹ��������ֵ���ų����еĻ����⡣
*
*  ATTENTION :  �������Ʒ��Ӧ�Ļ�����Ĵ���ȫ��ֻ����һ���ط������޸�ȫ�ֹ⡣
*/
void osc_TGManager::change_amibentCoeff( float _r,float _g,float _b )
{
	osassert( (_r>=0)&(_b>=0)&&(_g>=0) );


	g_fAmbiR = _r;
	g_fAmbiG = _g;
	g_fAmbiB = _b;


	// �Ȼָ�����̬�⴦����ĸ��ӵ���ɫ��
	//
	// ��ÿһ��tg����֡��ĳ�ʼ����
	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		osassert( m_arrPtrTG[t_i] );
		
		m_arrPtrTG[t_i]->frame_setTG();			

	}

	reset_ambientCoeff();

	// ��Ʒ�Ļ�������ء�

	// skinMesh�Ļ�������ء�
	m_skinMeshMgr->reset_ambient();
}


//! ����animation Mesh�Ķ�̬�����ݡ�
bool osc_TGManager::process_aniMeshDlight( void )
{
	guard;

	osc_dlight*    t_ptrDL;
	os_dLightData   t_dlight;

	t_dlight.m_sAmbientLight.a = 1.0f;
	t_dlight.m_sAmbientLight.r = g_fAmbiR;
	t_dlight.m_sAmbientLight.g = g_fAmbiG;
	t_dlight.m_sAmbientLight.b = g_fAmbiB;

	// 
	// ��ÿһ����̬������ǰ��̬���ambientMesh��Ӱ�졣
	for( int t_i=0;t_i<MAX_LIGHTNUM;t_i++ )
	{
		t_ptrDL = &m_dlightMgr.m_vecDlight[t_i];
		if( !t_ptrDL->m_bInuse )
			continue;

		t_dlight.m_fAttenuationDis = t_ptrDL->m_fRadius;
		t_dlight.m_sDLight = t_ptrDL->m_dlightColor;
		t_dlight.m_vec3DLightPos = t_ptrDL->m_vec3Pos;
		
		m_middlePipe.process_amDlight( t_dlight );

	}

	return true;

	unguard;
}


/** \brief
*  ����̬�⡣
*/
BOOL osc_newTileGround::process_dLight( osc_dlight* _dl ,int _lightID)
{
	return true;
	//return m_sMapTerrain.SetDynamic_Light(_dl,_lightID);
}

/** \brief
*  ʹ�ø����ĵƹ�ϵ���Գ�����ÿһ������ĵƹ�������š�
*/
void osc_newTileGround::set_ambientLCoeff( float _r,float _g,float _b )
{
	guard;
//	m_sMapTerrain.SetambientLCoeff(_r,_g,_b);
	unguard;
}