//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdParticle.cpp
 *
 *  Desc:     ���볡����������Ч������ݵ�ʵ���ࡣ
 *
 *  His:      River created @ 2004-5-20.
 *
 *  "�����������ж��İ�����ף���һ��������Լ���֧�䡣
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdParticle.h"
# include "../../backPipe/include/osCamera.h"
# include "../../Terrain/include/fdFieldMgr.h"

static osc_particleMgr* g_ptrParMgr = NULL;

//! ��Ⱦ��صĺ�����
void os_parRunCtlIns::reset_particle( void )
{
	m_bAlive = false;
	m_fRotation = 0.0f;

	m_fElapseTime = 0.0f;


    m_fLifeTime = 0.0f;

	m_fTval = 0.0f;


	m_iTexFrame = 0;
	m_iTexSetsIdx = -1;

	m_fSize = 0.0f;

	m_fAlpha = 1.0f;
}


//===================================================================================================
// emissionControlInstance��صĺ�����
//
os_emssionCtlIns::os_emssionCtlIns()
{
	m_ptrParScene = NULL;
	m_ptrEmissionCtl = NULL;
	m_ptrEmitter = NULL;
	m_ptrParticleRunCtl = NULL;
	
	m_bFirstFrameGenPar = FALSE;

	reset_emssionIns();

}

/** \brief
 *  ��ʼ����ǰ�����ӿ��ơ�
 *
 *  ��Ҫ��䵱ǰ���ӿ�����Ҫ��polygon���ݡ�
 */
void os_emssionCtlIns::init_emissionCtlIns( os_particleScene* _sce,
		                             os_particleEmissionCtl* _emi,float _sizeScale )
{
	guard;

	osassert( _sce );
	osassert( _emi );

	m_ptrParScene = _sce;
	m_ptrEmissionCtl = _emi;

	m_fSizeScale = _sizeScale;

	m_iNpwmr = _sce->m_iNpwmr;
	m_ptrEmitter = &_sce->m_vecParEmitter[_emi->m_iParEmitterIdx];
	m_ptrParticleRunCtl = &_sce->m_vecParRunCtl[_emi->m_iParticleRunIdx];

	for( int t_i=0;t_i<MAX_PARTICLENUM;t_i ++ )
		m_vecParticle[t_i].reset_particle();

	//
	//  ��ʼ������������.
	m_fTimeLastFrame = 0.0f;
	m_fTimeFromLS = 0.0f;

	m_fAccumulate = 0.0f;

	m_iParVertexNum = 0;

	// ��ʼ��emission�����ڡ�
	emitter_cycleStart();

	// ���Ҫ��Ⱦ�Ķ���ε�����.
	m_sPolygon.m_bUseStaticVB = true;
	m_sPolygon.m_sBufData.m_iIbufId = osc_particleMgr::m_iIbId;
	m_sPolygon.m_sBufData.m_iVbufId = osc_particleMgr::m_iVbId;

	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_billBoardVertex );
	m_sPolygon.m_iShaderId = m_ptrParticleRunCtl->m_iShaderId;
	osassert( osc_particleMgr::m_iIbId>= 0 );

	// ��ʼ�����ӷ�������λ��
	m_vec3LastEmitPos = osVec3D( 0.0f,0.0f,0.0f );;

	m_bFirstFrameGenPar = TRUE;

	return;

	unguard;
}


//! ���赱ǰ��emssionInstance.
void os_emssionCtlIns::reset_emssionIns( void )
{
	m_ptrParScene = NULL;
	m_ptrEmissionCtl = NULL;
	m_ptrEmitter = NULL;
	m_ptrParticleRunCtl = NULL;

	m_iParVertexNum = 0;

	for( int t_i=0;t_i<MAX_PARTICLENUM;t_i ++ )
		m_vecParticle[t_i].reset_particle();

	m_vec3LSStart = osVec3D( 0.0f,0.0f,0.0f );
	m_vec3LSEnd = osVec3D( 0.0f,0.0f,0.0f );

}

/** \brief
*   ��ʼ��һ������.
*/
void os_emssionCtlIns::init_particle( int _idx,osVec3D& _offset )
{
	guard;

	osassert( _idx >= 0 );

	m_vecParticle[_idx].m_bAlive = true;
	m_vecParticle[_idx].m_fElapseTime = 0.0f;
	m_vecParticle[_idx].m_fTval = 0.0f;

	//
	// �õ��������.
	m_vecParticle[_idx].m_iTexFrame = 0;
	m_vecParticle[_idx].m_iTexSetsIdx = 
		getrand_fromintscope( 0,m_ptrParticleRunCtl->m_iFrameSets - 1 );

	// ��ǰ���������ӵ���������.
	m_vecParticle[_idx].m_fLifeTime = getrand_fromfloatscope( 
		m_particleEmtIns.m_vec2ParLifeTime.x,m_particleEmtIns.m_vec2ParLifeTime.y );

	//
	// �õ����ӵļ���ʾ����.
	get_parSize( _idx );
	get_parAlpha( _idx );
	get_parColor( _idx );
	get_parRotation( _idx );

	//
	// �õ����ӳ�ʼ����velocity.
	get_ptlInitVelocity( _idx );

	//
	// �õ����ӵ�����ϵ��.
	get_particleResistance( _idx );

	//
	// �õ����ӵ�λ��.
	get_ptlInitPosition( _idx );



	// 
	// ����Ƿ�ɢ������ռ�,��Ӧ�ü�������������ռ��λ����.
	// ����Ƿ�ɢ�����ӿռ䣬����Ҫÿ�δ������ӵ��������
	if( m_ptrParScene->m_iEmitterTo == 0 )
		m_vecParticle[_idx].m_vec3ParticlePos += _offset;


	unguard;
}

/** \brief
*   �ӵ�ǰ�����Ӷ��е�,���һ����Ŀ������.
*/
void os_emssionCtlIns::replace_particle( int _rnum,osVec3D* _offset )
{
	guard;

	float   t_f;
	int     t_iMax,t_idx;

	osassert( _rnum > 0 );

	t_iMax = m_ptrParScene->m_iMaxParticleNum;
	// 
	// ������ϵ�����.
	if( (m_ptrParScene->m_iNpwmr == 1) )
	{
		for( int t_i = 0;t_i<_rnum;t_i ++ )
		{
			t_f = 0;

			// �õ�����ʱ���������.
			for( int t_j = 0;t_j<t_iMax;t_j ++ )
			{
				if( m_vecParticle[t_j].m_fElapseTime < t_f )
					continue;
				t_f = m_vecParticle[t_j].m_fElapseTime;
				t_idx = t_j;
			}

			// ��ʼ���������.
			init_particle( t_idx,_offset[t_i] );

		}
	}
	else if( m_ptrParScene->m_iNpwmr == 2 )
	{
		for( int t_i = 0;t_i<_rnum;t_i ++ )
		{
			t_idx = getrand_fromintscope( 0,t_iMax-1 );

			init_particle( t_idx,_offset[t_i] );
		}
	}
	else
	{
		//�˴�Ӧ�õ����㣬���ȴ��²�����������
		return;
	}

	return;

	unguard;
}

/** \brief
*  �����ӽ��и���.
*/
void os_emssionCtlIns::update_particle( int _idx,osVec3D& _offset )
{
	guard;

	float    t_fGravity;
	osVec3D   t_vec3Offset;

	osassert( _idx >= 0 );

	//
	// ���������.
	get_tvalue( _idx );
	get_textureFrame( _idx );
	get_particleResistance( _idx );
	get_parSize( _idx );
	get_parAlpha( _idx );
	get_parColor( _idx );
	get_parRotation( _idx );

	//
	// �������ӵ�λ��.
	t_fGravity = m_ptrParticleRunCtl->m_fGravity;
	t_vec3Offset = m_vecParticle[_idx].m_vec3Velocity*m_fTimeLastFrame;
	t_vec3Offset.y -= (t_fGravity*m_fTimeLastFrame*m_fTimeLastFrame/2.0f);
	t_vec3Offset *= m_ptrParScene->m_fMultiSize;

# if 0 // River @ 2009-5-14:Ϊ����ʱ������ǰ��Ե��������Ч����Ȼ
	   // �ܶ������Ч������������м��ƶ���Ч����̫��
	// River mod @ 2008-6-13:������ٶȷ���Ϊ��ʱ�����ӵļ��ٶ�������
	// ��ԭ�����.
	//! River mod @ 2008-6-11:Ϊ�����������м�����������Ч��
	if( vec3_equal( m_ptrEmitter->m_vec3Dir[0],osVec3D( 0.0f,0.0f,0.0f ) ) )
	{
		float t_fLength = osVec3Length( &t_vec3Offset );
		osVec3D  t_vec3;
		t_vec3 = m_vecParticle[_idx].m_vec3ParticlePos;
		t_vec3.y = 0.0f;
		osVec3Normalize( &t_vec3,&t_vec3 );
		t_vec3*=t_fLength;

		t_vec3Offset = -t_vec3;
	}
# endif 

	//@{
	// River added @ 2004-7-2:
	m_vecParticle[_idx].m_vec3Velocity.y -= (t_fGravity*m_fTimeLastFrame);
	//@} 
	m_vecParticle[_idx].m_vec3ParticlePos += t_vec3Offset;


	// River mod @ 2008-6-13:����Y�����ת��Ϣ,����ͳһ��ת���Խ�����������.
	// River @ 2009-3-30:���Ӳ�������ת�������������������������......
	//                   ��ʱ�벻�����õĽ��������
	osMatrix  t_sMat;
	osMatrixRotationY( &t_sMat,m_fTimeLastFrame*m_ptrParticleRunCtl->m_fParticleYRot );
	osVec3Transform( &m_vecParticle[_idx].m_vec3ParticlePos,
		&m_vecParticle[_idx].m_vec3ParticlePos,&t_sMat );


	return;

	unguard;
}





//! �õ���ǰʱ�̲���emitter�������ӵ���Ŀ.
int os_emssionCtlIns::get_emitterGenParNum( void )
{
	guard;

	float         t_fMax,t_fMin;

	//
	// ATTENTION TO OPP:���Բ�ʹ��accumulate�������Ӳ������.
	if( m_particleEmtIns.m_vec2EmissionRate.x > 
		     m_particleEmtIns.m_vec2EmissionRate.y )
	{
		t_fMax = m_particleEmtIns.m_vec2EmissionRate.x;
		t_fMin = m_particleEmtIns.m_vec2EmissionRate.y;
	}
	else
	{
		t_fMax = m_particleEmtIns.m_vec2EmissionRate.y;
		t_fMin = m_particleEmtIns.m_vec2EmissionRate.x;
	}


	//
	// �������������Ŀ�����ֵ����Сֵ���.
	if( (t_fMax > 1)&&( float_equal( t_fMax,t_fMin) ) )
	{
		t_fMax *= m_fTimeLastFrame;
		t_fMin = t_fMax + m_fAccumulate;
		m_fAccumulate = t_fMin - int(t_fMin);

		return int(t_fMin);
	}

	//
	// 
	if( float_equal( t_fMax,0.0f ) )
		return 0;
	else
	{
		if( t_fMax > 1.0f )
		{
			t_fMax = ::getrand_fromfloatscope( t_fMin,t_fMax );
			t_fMax *= m_fTimeLastFrame;

			t_fMin = t_fMax + m_fAccumulate;
			m_fAccumulate = t_fMin - int(t_fMin);
			return int(t_fMin);
		}
		else
			return int(getrand_fromfloat( t_fMax-t_fMin ));
	}

	osassert( false );

	return 0;

	unguard;
}


//! ��ǰ��emicontrolIns�Ƿ񼤻
bool os_emssionCtlIns::is_alive( void )
{
	guard;

	if( !m_ptrParScene )
		return false;

	if( m_particleEmtIns.m_bAlive )
		return true;

	int     t_iMax;

	t_iMax = m_ptrParScene->m_iMaxParticleNum;
	for( int t_i=0;t_i<t_iMax;t_i ++ )
	{
		if( m_vecParticle[t_i].m_bAlive )
			return true;
	}

	//
	// ֻ�����ӷ��������������ӣ��������������е�����
	// ���ڼ���ʱ������emssionϵͳ����������
	return false;

	unguard;
}


//! �����²�����ÿһ�����ӵ�λ��
void os_emssionCtlIns::cal_newParticlePos( osVec3D* _pos,osVec3D& _offset,int _num )
{
	guard;

	osassert( _pos );

	if( _num > MAX_PARTICLENUM )
		_num = MAX_PARTICLENUM;

	m_bEmitPosChanged = FALSE;
	if( !vec3_equal( m_vec3LastEmitPos,_offset ) )
		m_bEmitPosChanged = TRUE;

	// ����֮һ���������µ�λ�ò�������������ͷ����Ч��
	if( m_bEmitPosChanged )
	{
		for( int t_i=0;t_i<_num/4;t_i ++ )
			_pos[t_i] = _offset;
	}
	else
	{
		for( int t_i=0;t_i<_num;t_i ++ )
			_pos[t_i] = _offset;
		return;
	}

	//! ����һ��������ڵ�ǰ֡λ�ú���֡λ��֮���������������������Ч��
	osVec3D    t_vec3Dir = _offset - m_vec3LastEmitPos;
	float      t_fPos;
	for( int t_i=_num/4;t_i<_num;t_i ++ )
	{
		t_fPos = getrand_fromfloatscope( 0.0f,1.0f );
		_pos[t_i] = m_vec3LastEmitPos + t_vec3Dir * t_fPos;
	}

	return;

	unguard;
}


/** \brief
*  ÿһ֡�Ե�ǰ��emissionControlInstance���и��¡�
*
*  �������ӿ����ڵ�emitterIns��particleInsArray.
* 
*/
void os_emssionCtlIns::framemove_emission( float _etime,osVec3D& _offset )
{
	guard;

	int             t_iParticleNum,t_iMax,t_iNum,t_i;
	//! ����������ڼ����²�����ÿһ�����ӵ�λ��,������ӷ�����λ�øı䣬��������λ�õ�����
	static osVec3D  t_vec3NewParticlePos[MAX_PARTICLENUM];

	if( _etime <= 0.000001f )
		return;

	osassert( _etime >= 0.0f );

	// 
	// ����ǳ�ʼλ�ã�����֡λ��Ϊ��ǰλ��
	if( vec3_equal( m_vec3LastEmitPos,osVec3D( 0.0f,0.0f,0.0f ) ) )
		m_vec3LastEmitPos = _offset;

	m_fTimeFromLS += _etime;
	m_fTimeLastFrame = _etime;

	//
	// ����emitter�Ƿ���.
	if( m_particleEmtIns.m_bAlive )
		emission_cycleEnd();


	//
	// �õ���ǰʱ��emitter���������.
	if( m_particleEmtIns.m_bAlive )
	{
		get_emitterInsData();

		// �����ӿ��ƽ��д���.
		t_iParticleNum = get_emitterGenParNum();
	}
	else
	{
		t_iParticleNum = 0;
	} 

	// River added @ 2007-5-10:���һ���Ӳ����������ӵ����⣺
	if( t_iParticleNum >= m_ptrParScene->m_iMaxParticleNum )
		t_iParticleNum = m_ptrParScene->m_iMaxParticleNum;


	// �����²�����ÿһ�����ӵ�λ�á�
	cal_newParticlePos( t_vec3NewParticlePos,_offset,t_iParticleNum );

	t_iMax = m_ptrParScene->m_iMaxParticleNum;

	//
	// �ӷǼ���������,�������µ�����.
	t_iNum = 0;

	for( t_i=0;(t_i<t_iMax)&&(t_iNum<t_iParticleNum);t_i++ )
	{
		if( m_vecParticle[t_i].m_bAlive )
			continue;

		init_particle( t_i,t_vec3NewParticlePos[t_iNum] );
		t_iNum ++;

	}

	//
	// �ӵ�ǰ�����������,����������µ�����.
	if( (t_i == t_iMax)&&(t_iNum<t_iParticleNum) )
	{
		replace_particle( t_iParticleNum - t_iNum,&t_vec3NewParticlePos[t_iNum] );
	}


	//
	// �Ե�ǰ���Ӷ����е�ÿһ����������ӽ��и���.
	for( int t_i=0;t_i<t_iMax;t_i ++ )
	{
		if( m_vecParticle[t_i].m_bAlive )
			update_particle( t_i,_offset );
	} 

	// ��¼���ӷ������ϴη����������ڵ�λ��
	if( m_bEmitPosChanged ) m_vec3LastEmitPos = _offset;

	return ;

	unguard;
}

/** \brief
*  ��䵱ǰemissionControl Instance�ڵ�����Array��ָ�������ݻ������С�
*/
void os_emssionCtlIns::fill_renderVerBuf( os_billBoardVertex* _verStart,osc_camera* _cam )
{
	guard;

	float              t_fULength,t_fVLength,t_fUBase,t_fVBase;
	osVec3D            t_vec3Right,t_vec3Up,t_vec3Look;
	osVec3D            t_vec3BR,t_vec3BU;
	osVec3D*           t_ptrVec3;
	os_parRunCtlIns*    t_ptrPar;
	osMatrix           t_matRot;
	os_billBoardVertex* t_ptrVerStart;

	osassert( _verStart );

	m_iParVertexNum = 0;

	// River @ 20106-6-8:�����ϵ�������Ⱦ��ʹ�ù̶��ķ���
	if( _cam )
	{
		_cam->get_rightvec( t_vec3Right );
		_cam->get_godLookVec( t_vec3Look );
	}
	else
	{
		t_vec3Right = osVec3D( 1.0f,0.0f,0.0f );
		t_vec3Look = osVec3D( 0.0f,0.0f,1.0f );
	}

	osVec3Cross( &t_vec3Up,&t_vec3Look,&t_vec3Right );
	osVec3Normalize( &t_vec3Up,&t_vec3Up );

	t_fULength = 1.0f/m_ptrParticleRunCtl->m_iFrameNum;
	t_fVLength = 1.0f/m_ptrParticleRunCtl->m_iFrameSets;


	//
	// ��ÿһ������,���������ӵ��ĸ�����.
	int    t_iMax = m_ptrParScene->m_iMaxParticleNum;
	for( int t_i=0;t_i<t_iMax;t_i ++ )
	{
		// ������Ǽ�������,�˳�����Ⱦ.
		if( !m_vecParticle[t_i].m_bAlive )
			continue;

		t_ptrPar = &m_vecParticle[t_i];
		t_ptrVerStart = &_verStart[m_iParVertexNum];

		// 
		// ����uv.
		t_fUBase = t_ptrPar->m_iTexFrame*t_fULength;
		t_fVBase = t_ptrPar->m_iTexSetsIdx*t_fVLength;

		// 
		// ������ת�������.
		osMatrixRotationAxis( &t_matRot,&t_vec3Look,m_vecParticle[t_i].m_fRotation );
		osVec3Transform( &t_vec3BR,&t_vec3Right,&t_matRot );
		osVec3Transform( &t_vec3BU,&t_vec3Up,&t_matRot );

		//
		// ��䶥��.
		t_ptrVerStart[0].m_vec3Pos = 
			(-t_vec3BR-t_vec3BU)*t_ptrPar->m_fSize+t_ptrPar->m_vec3ParticlePos;
		t_ptrVerStart[1].m_vec3Pos = 
			(t_vec3BR-t_vec3BU)*t_ptrPar->m_fSize + t_ptrPar->m_vec3ParticlePos;
		t_ptrVerStart[2].m_vec3Pos = 
			(t_vec3BR+t_vec3BU)*t_ptrPar->m_fSize + t_ptrPar->m_vec3ParticlePos;
		t_ptrVerStart[3].m_vec3Pos = 
			(-t_vec3BR+t_vec3BU)*t_ptrPar->m_fSize + t_ptrPar->m_vec3ParticlePos;

		//
		// ���uv����.
		t_ptrVerStart[0].m_vec2Uv = 
			osVec2D( t_fUBase,t_fVBase );
		t_ptrVerStart[1].m_vec2Uv = 
			osVec2D( t_fUBase + t_fULength,t_fVBase );
		t_ptrVerStart[2].m_vec2Uv = 
			osVec2D( t_fUBase + t_fULength,t_fVBase + t_fVLength );
		t_ptrVerStart[3].m_vec2Uv = 
			osVec2D( t_fUBase,t_fVBase + t_fVLength );


		//
		// �����ɫֵ.
		t_ptrVec3 = &t_ptrPar->m_vec3Color;

		t_ptrVerStart[0].m_dwColor = 
			osColor( t_ptrVec3->x,t_ptrVec3->y,t_ptrVec3->z,t_ptrPar->m_fAlpha  );
		t_ptrVerStart[1].m_dwColor = t_ptrVerStart[2].m_dwColor = 
			t_ptrVerStart[3].m_dwColor = t_ptrVerStart[0].m_dwColor;

		//
		// ÿһ������������Ҫ���4������.
		m_iParVertexNum += 4;

	}

	unguard;
}


//! �Ե�ǰ��emiControlIns����FrameMove.
void os_emssionCtlIns::frameMove_eci( 
			osVec3D& _offset,float _etime,osc_camera* _cam )
{
	guard;

	osassert( m_ptrParticleRunCtl );

	// 
	// ���������ռ������,��ʹ��identity���������.
	// Ȼ�������������ռ��λ�üӵ����ӵĶ���λ����.
	// ��frameMove��ǰ��emissionControl,��������ӵ��ڴ档
	framemove_emission( _etime,_offset );

	fill_renderVerBuf( m_vecRenderVer,_cam );

	// ��¼�ϴ�Emitter�����������ڵ�λ��
	m_vec3LastEmitPos = _offset;


	return;

	unguard;
}


/** \brief
*  ��Ⱦ��ǰ��emissionControlInstance.
*/
bool os_emssionCtlIns::render_emissionControlIns( 
	           osc_middlePipe* _pipe,osVec3D& _offset )
{
	guard;

	osassert( _pipe );

	osassert( osc_particleMgr::m_iVbId>=0 );
	osassert( osc_particleMgr::m_iIbId>=0 );
	osassert( m_ptrParticleRunCtl );

	// 
	// �����������.emittertoΪ1ʱ���ӷ�ɢ������ռ�
	osMatrix   t_mat,t_scaleMat;
	osMatrixScaling( &t_scaleMat,m_fSizeScale,m_fSizeScale,m_fSizeScale );

	// ��ɢ������ռ�ĵ����ӣ���������ʱ��ֱ�Ӽ������ӵ�λ��λ�Ƽ���
	if( m_ptrParScene->m_iEmitterTo == 1 )
	{
		osMatrixTranslation( &t_mat,_offset.x,_offset.y,_offset.z );
		osMatrixMultiply( &t_mat,&t_scaleMat,&t_mat );
	}
	else
	{
		// ��ɢ�����ӿռ�����ӣ���Ҫ�������������ǰ��λ�ú����ź��λ�ã�
		// ������λ�õ�ƫ���������뵽�����ӵ��������ž����ڣ����������ź�
		// λ��Ҳ�����ˣ���������Ҫ����λ��û�����ŵ�Ч����
		osVec3D   t_vec3AfterScale = _offset*m_fSizeScale;
		osVec3D   t_vec3Offset = _offset - t_vec3AfterScale;
		
		t_mat = t_scaleMat;
		t_mat._41 = t_vec3Offset.x;
		t_mat._42 = t_vec3Offset.y;
		t_mat._43 = t_vec3Offset.z;
	}

	_pipe->set_worldMatrix( t_mat );


	// �����ǰemission������Ҫ��Ⱦ��particle.
	if( m_iParVertexNum > 0 )
	{
		// 
		// ���ݺͱ�����ϻ��Ǻͱ������������Ⱦ��ʽ��������Ⱦ״̬��
		if( m_ptrParticleRunCtl->m_iBlendMode == 0 )
		{
			_pipe->set_renderState( D3DRS_SRCBLEND ,D3DBLEND_ONE );
			_pipe->set_renderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		}
		else 
		{
			_pipe->set_renderState( D3DRS_SRCBLEND ,D3DBLEND_SRCALPHA );
			_pipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
		}

		//
		// ��䶥�����ݵ����㻺����������Ⱦ��ǰ���ӵĶ������ݡ�
		int     t_idx;
		t_idx = _pipe->fill_dnamicVB( osc_particleMgr::m_iVbId,
			m_vecRenderVer,sizeof( os_billBoardVertex ),m_iParVertexNum );

		m_sPolygon.m_iPriNum = m_iParVertexNum/4*2;

		m_sPolygon.m_iVerNum = m_iParVertexNum;
		m_sPolygon.m_sBufData.m_iVertexStart = t_idx;

		if( !_pipe->render_polygon( &m_sPolygon ) )
			return false;
	}

	return true;

	unguard;
}





//===================================================================================================
//
// Particle ins��غ�����
os_particleSceneIns::os_particleSceneIns()
{
	m_ptrParScene = NULL;
	m_bInuse = false;
	m_bTopLayer = false;

	m_fTimeScale = 1.0f;

	m_vec3Offset = osVec3D(0,0,0);
	m_iTrackId = -1;
	m_bHide = false;

}
os_particleSceneIns::~os_particleSceneIns()
{
	delete_parSceIns();
}



/** \brief
*  ʹ�ó���ָ������ʼ��һ��particleInstance.
*/
WORD os_particleSceneIns::init_particleIns( os_particleScene* _psce,
	osVec3D& _offset,float _scale,BOOL _inScene/* = FALSE*/,bool _topLayer/* = false*/ )
{
	guard;

	osassert( _psce );
	osassert( _scale > 0.0f );

	m_ptrParScene = _psce;

	m_bTopLayer = _topLayer;
	m_bHide = false;

	// River @ 2010-6-9:���ڽ����ϵ�������Ⱦ
	m_bRenderedInCurFrame = true;

	osassert( _psce->m_iEmissionCtlNum <= MAX_EMISSION );
	for( int t_i=0;t_i<_psce->m_iEmissionCtlNum;t_i ++ )
	{
		m_vec3Offset = _offset;

		m_arrEmissonCtlIns[t_i].init_emissionCtlIns( _psce,
			&_psce->m_vecEmssionCtl[t_i],_scale );

	}

	increase_val();
	set_insInit();

	//
	// �����ǰ��particle�����õ���track,���trackMgr�д���track.
	if( m_ptrParScene->m_bUseTrack )
	{
		m_iTrackId = g_ptrTrackMgr->create_track( 
			m_ptrParScene->m_szTrackFileName,m_ptrParScene->m_fRunSpeed,true,false );

		// ATTENTION TO FIX:River mod @ 2007-4-6:�޸�Ϊ������.
		if( m_iTrackId < 0 )
			m_ptrParScene->m_bUseTrack = FALSE;

	}

	m_bInuse = true;

	return get_val();

	unguard;
}



//! ��partileIns����framemove.
void os_particleSceneIns::frameMove_curIns( float _etime,
						osc_camera* _cam,bool _canDeleteCurIns/* = false*/ )
{
	guard;

	osassert( this->m_ptrParScene );
	m_ptrParScene->set_used();

	osVec3D    t_vec3;
	bool       t_bAct = false;

	// River mod @ 2010-6-8:�����ϵ����Ӳ���Ҫcam.
	if( !m_bTopLayer )
		osassert( _cam );


	t_vec3.x = 0.0f;t_vec3.y = 0.0f;t_vec3.z = 0.0f;

	// �õ�·�����������λ������.
	if( m_ptrParScene->m_bUseTrack )
	{
		g_ptrTrackMgr->get_aniVec( m_iTrackId,_etime,t_vec3 );
		m_vec3FinalOffset = t_vec3 + m_vec3Offset;
	}
	else
	{
		m_vec3FinalOffset = m_vec3Offset;
	}
	


	// ��ÿһ��emission control�ڲ����Ƶ�ǰemission���������.
	for( int t_i=0;t_i<m_ptrParScene->m_iEmissionCtlNum;t_i ++ )
	{
		//
		// �����ǰ��emissionControl Instance��������״̬������Ҫ��Ⱦ��
		if( m_arrEmissonCtlIns[t_i].is_alive() )
			t_bAct = true;
		else
			continue;

		m_arrEmissonCtlIns[t_i].frameMove_eci( m_vec3FinalOffset,_etime,_cam );
	}	

	if( (!t_bAct) && _canDeleteCurIns )
		delete_parSceIns();

	unguard;
}


/** \brief
*  ��Ⱦ��ǰ��particleSceneInstance.
*
*  ��ÿһ��emissionControl���д���
*  
*  \param _etime ��һ֡��ȥ��ʱ�䡣
*  
*/
bool os_particleSceneIns::render_currentIns( 
	    osc_camera* _cam, osc_middlePipe* _mpipe )
{
	guard;

	osVec3D      t_vec3( 0.0f,0.0f,0.0f );

	osassert( this->m_ptrParScene );

	// River mod @ 2010-6-8:��������������������Ч����Ⱦ
	if( m_bTopLayer )
	{
		frameMove_curIns( sg_timer::Instance()->get_lastelatime(),_cam,true );
		if( m_ptrParScene == NULL )
			return true; 
	}

# if 1

	// ��ÿһ��emission control�ڲ����Ƶ�ǰemission���������.
	for( int t_i=0;t_i<m_ptrParScene->m_iEmissionCtlNum;t_i ++ )
	{
		//
		// �����ǰ��emissionControl Instance��������״̬������Ҫ��Ⱦ��
		if( !m_arrEmissonCtlIns[t_i].is_alive() )
			continue;

		//! River mod @ 2009-2-11:������ڷ�����״̬����Ⱦ.
		if( !m_bHide )
		{
			m_arrEmissonCtlIns[t_i].
				render_emissionControlIns( _mpipe,m_vec3FinalOffset );
		}

	}
# endif 
		
	return true;

	unguard;
}

//! ���õ�ǰ���ӳ���ʵ���Ĳ���ʱ�䡣
void os_particleSceneIns::set_parPlayTime( float _timeScale )
{
	guard;

	osassert( _timeScale > 0.0f );

	m_fTimeScale =_timeScale;

	unguard;
}

//! �����Զ������ӵķ�ɢ�߶η�Χ
void os_particleSceneIns::set_parGetLineSeg( osVec3D& _start,osVec3D& _end )
{
	osassert( m_ptrParScene->m_iEmissionCtlNum > 0 );

	if( m_arrEmissonCtlIns[0].is_lineSegPar() )
	{
		m_vec3Offset = osVec3D( 0.0f,0.0f,0.0f );
		for( int t_i=0;t_i<m_ptrParScene->m_iEmissionCtlNum;t_i ++ )
			m_arrEmissonCtlIns[t_i].set_parGetLineSeg( _start,_end );

		frameMove_curIns( 0.0f,g_ptrCamera );
	}
}


//! ��ǰ�����ӳ����Ƿ���ѭ�����ŵ����ӳ���
bool os_particleSceneIns::is_loopPlayParticle( void )
{
	guard;

	for( int t_i=0;t_i<MAX_EMISSION;t_i ++ )
	{
		if( m_arrEmissonCtlIns[t_i].is_alive() )
		{
			if( m_arrEmissonCtlIns[t_i].is_loopPlayEmitter() )
				return true;
		}
	}

	return false;

	unguard;
}


//! �õ���ǰ���ӳ���ʵ���Ĳ���ʱ�䡣
float os_particleSceneIns::get_parPlayTime( void )
{
	guard;

	float   t_fRes = 0.0f;

	// 
	// ���ѭ�����ŵ����ӣ��򷵻ظ�ֵ���ϲ�����Ƿ��Ǹ�ֵ��
	// ��ȷ���Ƿ���ѭ�����ŵ����ӡ���
	for( int t_i=0;t_i<MAX_EMISSION;t_i ++ )
	{
		if( m_arrEmissonCtlIns[t_i].is_alive() )
		{
			// 
			// �������ѭ�����ŵ����ӣ��򷵻ظ���ʱ�䲥��ֵ
			if( m_arrEmissonCtlIns[t_i].get_emitterList()->m_bLoopCycle )
			{
				t_fRes = -1.0f;
				break;
			}
			else
			{
				if( m_arrEmissonCtlIns[t_i].get_emitterList()->m_fCycleLength > t_fRes )
					t_fRes = m_arrEmissonCtlIns[t_i].get_emitterList()->m_fCycleLength;
			}
		}
	}

	return t_fRes;

	unguard;
}

//! ɾ����ǰparticle scene instance�õ�������.
void os_particleSceneIns::delete_parSceIns( bool _finalRelease/* = false*/ ) 
{
	
	m_bInuse = false;
	m_bTopLayer = false; 
	
	if( m_ptrParScene == NULL )
		return;

	for( int t_i=0;t_i<m_ptrParScene->m_iEmissionCtlNum;t_i ++ )
		m_arrEmissonCtlIns[t_i].reset_emssionIns();

	this->m_fTimeScale = 1.0f;

	if( m_ptrParScene->m_bUseTrack )
	{
		if( m_iTrackId >= 0 )
			g_ptrTrackMgr->delete_track( m_iTrackId );
		m_iTrackId = -1;
	}

	// 
	// ATTNENTION TO FIX:ɾ����ǰparticle �����Ķ�Ӧ����.

	reset_ins();

	//! River added @ 2010-3-20:�����ͷ�����
	if( _finalRelease )
	{
		m_ptrParScene->release_parScene();
		
		// �ͷ�spaceMgr.
		CSpaceMgr<os_particleScene>::NODE t_ptrNode;
		for( t_ptrNode = g_ptrParMgr->m_vecParticleScene.begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = g_ptrParMgr->m_vecParticleScene.next_validNode( &t_ptrNode ) )
		{
			if( t_ptrNode.p == m_ptrParScene )
				g_ptrParMgr->m_vecParticleScene.release_node( t_ptrNode.idx );
		}
	}


	m_ptrParScene = NULL;
}



//! ���ӹ��������õĶ��������������
int osc_particleMgr::m_iVbId = -1;
int osc_particleMgr::m_iIbId = -1;



osc_particleMgr::osc_particleMgr()
{

	m_ptrMPipe = NULL;

	m_vecParticleScene.resize( MAX_PARTICLEFILE );
	m_vecParSceIns.resize( MAX_PARTICLEINS );

	g_ptrParMgr = this;

}

osc_particleMgr::~osc_particleMgr()
{

	//
	// �ͷŹ����������е����ӳ��������ӳ���ʵ��.

}


/** \brief
 *  ��ʼ����ǰ��particle��������
 *
 */
void osc_particleMgr::init_particleMgr( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );
	m_ptrMPipe = _pipe;
	
	// �õ���ǰvertex buffer��ָ�롣
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	// River @ 2011-2-16:�Ӵ����ӵ�dynamic buff����ʹ�ö�����buff.
	osc_particleMgr::m_iVbId = _pipe->create_dynamicVB( t_dwFvf,4096*384,true );
	osassert( m_iVbId>=0 );	
	//
	// ��ʼ����ǰindex buffer,��Ϊ�������ӿ���ʹ����ͬ��index buffer.
	// ����Ⱦ��ʱ�򣬸���ÿһ��emssion������Ŀ�Ķ���ֵ����������Ŀ��
	WORD*  t_ptrW;
	int t_iGBufIdx = -1;
	t_ptrW = (WORD*)START_USEGBUF( t_iGBufIdx );
	for( int t_i=0;t_i<MAX_PARTICLENUM;t_i++ )
	{
		t_ptrW[t_i*6+0] = t_i*4;
		t_ptrW[t_i*6+1] = t_i*4+1;
		t_ptrW[t_i*6+2] = t_i*4+2;

		t_ptrW[t_i*6+3] = t_i*4 ;
		t_ptrW[t_i*6+4] = t_i*4+2;
		t_ptrW[t_i*6+5] = t_i*4+3;

	}

	osc_particleMgr::m_iIbId = _pipe->create_staticIB( t_ptrW,MAX_PARTICLENUM*6 );
	osassert( osc_particleMgr::m_iIbId>=0 );
	END_USEGBUF( t_iGBufIdx );

	//! River @ 2011-2-16:һ���Եĵ������е�particle�ļ���Դ��������Ϸ�ڵ�par������


	return;

	unguard;
}


//! ��ParticleMgr�е�particleInstance����FrameMove,��Ⱦ�ʹ���ֿ�.
void osc_particleMgr::framemove_particle( osc_camera* _cam )
{
	guard;


	// River @ 2011-1-30:ɾ��particle�ĺ�������������ɾ�������Դ˴�ȥ��CS.
	// ȷ�Ͻ�����˳��ٽ���.
	//::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	// Ϊÿһ�������ļ�����lru.
	CSpaceMgr<os_particleScene>::NODE t_ptrNode;
	for( t_ptrNode = m_vecParticleScene.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecParticleScene.next_validNode( &t_ptrNode ) )
	{
		t_ptrNode.p->add_nu();
	}


	float       t_fEtime = sg_timer::Instance()->get_lastelatime();

	CSpaceMgr<os_particleSceneIns>::NODE t_ptrN;
	for( t_ptrN = m_vecParSceIns.begin_usedNode();
		t_ptrN.p != NULL;
		t_ptrN = m_vecParSceIns.next_validNode( &t_ptrN ) )
	{
		if( t_ptrN.p->get_insInit() )
		{
			// River @ 2011-1-30:���û����ʹ���У�������һ�����Ż��˴��룬�п��ܳ������������
			if( !t_ptrN.p->is_inUse() )
				continue;

			// River mod @ 2010-6-8:��Ⱦ����������Ч����
			if( t_ptrN.p->get_topLayer() )
				continue;

			t_ptrN.p->frameMove_curIns( t_fEtime,_cam,true );
			if( !t_ptrN.p->is_inUse() )
				m_vecParSceIns.release_node( t_ptrN.idx );
		}
	}

	// �뿪�ٽ���
	//::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	return;

	unguard;
}



//! ������Ⱦparticle.
bool osc_particleMgr::render_particle( 
		osc_camera* _cam,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	guard;

	// 
	// ����Alpha Test��ֵΪ��С��
	m_ptrMPipe->set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
	m_ptrMPipe->set_renderState( D3DRS_ALPHAREF, GLOBAL_MINALPHAREF );
	m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
	m_ptrMPipe->set_renderState( D3DRS_CULLMODE,D3DCULL_NONE );

	// River @ 2011-1-30:ɾ��particle�ĺ�������������ɾ�������Դ˴�ȥ��CS.
	// ȷ�Ͻ�����˳��ٽ���.
	//::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	CSpaceMgr<os_particleSceneIns>::NODE t_ptrN;

	for( t_ptrN = m_vecParSceIns.begin_usedNode();
		t_ptrN.p != NULL;
		t_ptrN = m_vecParSceIns.next_validNode( &t_ptrN ) )
	{
		if( t_ptrN.p->get_insInit() )
		{
			// River @ 2011-1-30:���û����ʹ���У�������һ�����Ż��˴��룬�п��ܳ������������
			if( !t_ptrN.p->is_inUse() )
				continue;

			//@{
			// River added @ 2010-6-8:�����ڽ�������Ⱦ����������Ч
			if( OS_RENDER_TOPLAYER == _rtype )
			{
				if( !t_ptrN.p->get_topLayer() )
					continue;
				// River @ 2010-6-9:ȷ��������Чÿ��ֻ��Ⱦһ��particle.
				if( t_ptrN.p->m_bRenderedInCurFrame )
					continue;
			}
			if( OS_RENDER_LAYER == _rtype )
				if( t_ptrN.p->get_topLayer() )
					continue;
			//@}

			t_ptrN.p->render_currentIns( _cam,m_ptrMPipe );

			t_ptrN.p->m_bRenderedInCurFrame = true;
		}
	}

	// �뿪�ٽ���
	//::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );


	return true;
	unguard;
}


//! �ͷ��ʱ�䲻ʹ��particle.
void osc_particleMgr::discard_lruParticleScene( void )
{
	guard;

	int   t_iNu = 0;
	int   t_iIdx = -1;
	CSpaceMgr<os_particleScene>::NODE t_ptrNode;
	for( t_ptrNode = m_vecParticleScene.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecParticleScene.next_validNode( &t_ptrNode ) )
	{
		if( !t_ptrNode.p->is_sceneInuse() )
			continue;

		if( t_ptrNode.p->get_nu() > t_iNu )
		{
			t_iNu = t_ptrNode.p->get_nu();
			t_iIdx = t_ptrNode.idx;
		}
	}

	//! �ͷŴ�particle
	if( t_iIdx >= 0 )
	{
		m_vecParticleScene[t_iIdx].release_parScene();
		m_vecParticleScene.release_node( t_iIdx );
	}

	unguard;
}


/** \bief
*  ����һ�����ӳ���ʵ��.
* 
*  ������һ�γ���,�����滻�㷨,ʹ��������Զ����
*  �����������е����ӳ���.
* 
*  \return int �����˵�ǰparticle��id.
*/
int osc_particleMgr::create_parSceIns( os_particleCreate& _pcreate,
				BOOL _inScene/* = FALSE*/,bool _topLayer/* = false*/  )
{
	guard;

	int      t_idx,t_id = -1;
	WORD     t_wCreateVal = 0;

	osassert( _pcreate.m_szParticleName != "" );

	//
	// �Ƿ�ǰҪ�����ĳ����ڹ��������Ѿ�����.
	t_idx = -1;
	DWORD   t_dwHash = string_hash( _pcreate.m_szParticleName );
	CSpaceMgr<os_particleScene>::NODE t_ptrNode;
	for( t_ptrNode = m_vecParticleScene.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecParticleScene.next_validNode( &t_ptrNode ) )
	{
		if( !t_ptrNode.p->is_sceneInuse() )
			continue;

		if( t_ptrNode.p->is_needScene( t_dwHash ) )
		{
			t_idx = t_ptrNode.idx;
			break;
		}
	}

	//
	// �Ҳ���Ҫ�����ĳ����Ļ�,�����µĳ���.
	if( t_idx == -1 )
	{
		// River @ 2011-1-30:�˴�����CS,���̵߳õ�������Դ��
		::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );
		t_idx = m_vecParticleScene.get_freeNode();
		::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );

		if( !m_vecParticleScene[t_idx].load_parSceneFromFile( 
			_pcreate.m_szParticleName ) )
			osassert( false );
	}

	// �������Ҫ�������ӵ�ʵ��,����.
	if( !_pcreate.m_bCreateIns )
	{
		m_vecParticleScene[t_idx].set_swap( false );
		t_id = -1;
	}
	else
	{
		//
		// ������ǰ�����ӳ���ʵ��
		// River @ 2011-1-30:�˴�����CS,���̵߳õ�������Դ��
		::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );
		t_id = m_vecParSceIns.get_freeNode();
		::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );

		t_wCreateVal = m_vecParSceIns[t_id].init_particleIns( 
			&m_vecParticleScene[t_idx],_pcreate.m_vec3Offset,
			_pcreate.m_fSizeScale,_inScene,_topLayer );
	}

	return osn_mathFunc::syn_dword( t_wCreateVal,(WORD)t_id );

	unguard;
}


const char* osc_particleMgr::get_parNameFromId( int _id )
{
	guard;

	WORD   t_wId = WORD( _id );
	static char   t_sz[4];

	if( m_vecParSceIns.validate_id( t_wId ) )
	{
		if( m_vecParSceIns[t_wId].is_inUse() )
			if( m_vecParSceIns[t_wId].validate_cval( _id ) )
				return m_vecParSceIns[t_wId].m_ptrParScene->get_sceneName();
	}

	t_sz[0] = NULL;
	return t_sz;

	unguard;
}


/** \brief
 *  ɾ��һ��particle.
 *
 *  ATTENTION:  ��Ҫɾ������ǰ��ins���������������.
 *  
 */
void osc_particleMgr::delete_particle( int _idx,bool _finalRelease/* = false*/ )
{
	guard;

	WORD   t_wId = WORD( _idx );
	
	// River @ 2011-1-30:ɾ��particle�ĺ�������������ɾ�������Դ˴�ȥ��CS.
	// ȷ�Ͻ�����˳��ٽ���.
	//::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );


	//
	///�����ȷ��ɾ����������������
	// �����ǰ��particle����ʹ���У���ɾ����ǰ��particle.
	if( m_vecParSceIns.validate_id( t_wId ) )
	{
		if( m_vecParSceIns[t_wId].is_inUse() )
		{
			if( m_vecParSceIns[t_wId].validate_cval( _idx ) )
			{
				// River @ 2010-3-20:���������ͷ����ӵĲ���
				if( _finalRelease )
				{
					m_vecParSceIns[t_wId].delete_parSceIns( true );
					m_vecParSceIns.release_node( t_wId );
				}
				else
				{
					// River mod @ 2008-7-18: Ϊ�������ӵ���ʧЧ������ƽ��,
					//  ��ֱ��ɾ������,ֹͣ���Ӳ���,��������������ɺ��Զ���ʧ.
					m_vecParSceIns[t_wId].stop_parSceIns();
				}
			}
		}
	}

	//::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	unguard;
}

//! �������ӳ����Ĳ���ʱ�䡣
void osc_particleMgr::set_parPlayTime( int _id,float _timeScale )
{
	guard;

	WORD   t_wId = WORD( _id );

	osassert( t_wId >= 0 );
	osassert( m_vecParSceIns.validate_id( t_wId ) );
	osassert( m_vecParSceIns[t_wId].is_inUse() );

	if( m_vecParSceIns[t_wId].validate_cval( _id ) )
		m_vecParSceIns[t_wId].set_parPlayTime( _timeScale );

	unguard;
}

//! �������ӳ����Ĳ���ʱ�䡣
float osc_particleMgr::get_parPlayTime( int _id )
{
	guard;

	WORD   t_wId = WORD( _id );

	osassert( t_wId >= 0 );
	osassert( m_vecParSceIns.validate_id( t_wId ) );
	osassert( m_vecParSceIns[t_wId].is_inUse() );

	return m_vecParSceIns[t_wId].get_parPlayTime();

	unguard;
}

//! �Ƿ���ѭ�����ŵĹ����
bool osc_particleMgr::is_loopPlayParticle( int _id )
{
	guard;

	WORD   t_wId = WORD( _id );

	return m_vecParSceIns[t_wId].is_loopPlayParticle();

	unguard;
}


void osc_particleMgr::set_particlePos( osVec3D& _pos,int _idx,bool _forceUpdate/* = false*/ )
{
	guard;

	WORD   t_wId = WORD( _idx );

	osassert( _idx >= 0 );
	if( m_vecParSceIns.validate_id( t_wId ) )
	{
		osassert( m_vecParSceIns[t_wId].is_inUse() );

		// ������ϲ��������߶β��������������ӣ���ȴ������߶�ʱ����
		if( m_vecParSceIns[t_wId].is_lineSegParticle() )
			return;

		if( m_vecParSceIns[t_wId].validate_cval( _idx ) )
		{
			m_vecParSceIns[t_wId].set_curInsPos( _pos );
			if( _forceUpdate && (!m_vecParSceIns[t_wId].get_topLayer() ) )
				m_vecParSceIns[t_wId].frameMove_curIns( 0.0f,g_ptrCamera );
		}
	}

	unguard;
}

/** \brief
*  �������id��Ӧ���ӵ���Ч��.
*
*  �����Ӧ�����Ӵ�������״̬������ǰ
*  ������id.
*  \return bool �����Ч������true.
*/
bool osc_particleMgr::validate_parId( int _id )
{
	guard;

	WORD   t_wId = WORD( _id );

	bool   t_bRes;

	// River @ 2011-1-30:ɾ��particle�ĺ�������������ɾ�������Դ˴�ȥ��CS.
	// ȷ�Ͻ�����˳��ٽ���.
	//::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	if( !m_vecParSceIns.validate_id( t_wId ) )
	{
			t_bRes = false;
	}
	else
	{
		if( m_vecParSceIns[t_wId].is_inUse() )
		{
			if( m_vecParSceIns[t_wId].validate_cval( _id ) )
				t_bRes = true;
			else 
				t_bRes = false;
		}
		else
			t_bRes = false;	
	}
	
	//::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );
	return t_bRes;

	unguard;
}



