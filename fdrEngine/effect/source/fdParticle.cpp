//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdParticle.cpp
 *
 *  Desc:     调入场景中粒子特效相关数据的实现类。
 *
 *  His:      River created @ 2004-5-20.
 *
 *  "命运是我们行动的半个主宰，另一半归我们自己来支配。
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdParticle.h"
# include "../../backPipe/include/osCamera.h"
# include "../../Terrain/include/fdFieldMgr.h"

static osc_particleMgr* g_ptrParMgr = NULL;

//! 渲染相关的函数。
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
// emissionControlInstance相关的函数。
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
 *  初始化当前的粒子控制。
 *
 *  需要填充当前粒子控制需要的polygon数据。
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
	//  初始化其它的数据.
	m_fTimeLastFrame = 0.0f;
	m_fTimeFromLS = 0.0f;

	m_fAccumulate = 0.0f;

	m_iParVertexNum = 0;

	// 开始了emission的周期。
	emitter_cycleStart();

	// 填充要渲染的多边形的数据.
	m_sPolygon.m_bUseStaticVB = true;
	m_sPolygon.m_sBufData.m_iIbufId = osc_particleMgr::m_iIbId;
	m_sPolygon.m_sBufData.m_iVbufId = osc_particleMgr::m_iVbId;

	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_billBoardVertex );
	m_sPolygon.m_iShaderId = m_ptrParticleRunCtl->m_iShaderId;
	osassert( osc_particleMgr::m_iIbId>= 0 );

	// 初始化粒子发生器的位置
	m_vec3LastEmitPos = osVec3D( 0.0f,0.0f,0.0f );;

	m_bFirstFrameGenPar = TRUE;

	return;

	unguard;
}


//! 重设当前的emssionInstance.
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
*   初始化一个粒子.
*/
void os_emssionCtlIns::init_particle( int _idx,osVec3D& _offset )
{
	guard;

	osassert( _idx >= 0 );

	m_vecParticle[_idx].m_bAlive = true;
	m_vecParticle[_idx].m_fElapseTime = 0.0f;
	m_vecParticle[_idx].m_fTval = 0.0f;

	//
	// 得到纹理相关.
	m_vecParticle[_idx].m_iTexFrame = 0;
	m_vecParticle[_idx].m_iTexSetsIdx = 
		getrand_fromintscope( 0,m_ptrParticleRunCtl->m_iFrameSets - 1 );

	// 当前产生的粒子的生命周期.
	m_vecParticle[_idx].m_fLifeTime = getrand_fromfloatscope( 
		m_particleEmtIns.m_vec2ParLifeTime.x,m_particleEmtIns.m_vec2ParLifeTime.y );

	//
	// 得到粒子的简单显示属性.
	get_parSize( _idx );
	get_parAlpha( _idx );
	get_parColor( _idx );
	get_parRotation( _idx );

	//
	// 得到粒子初始化的velocity.
	get_ptlInitVelocity( _idx );

	//
	// 得到粒子的租力系数.
	get_particleResistance( _idx );

	//
	// 得到粒子的位置.
	get_ptlInitPosition( _idx );



	// 
	// 如果是发散到世界空间,则应该加入粒子在世界空间的位移量.
	// 如果是发散到粒子空间，则需要每次处理粒子的世界矩阵
	if( m_ptrParScene->m_iEmitterTo == 0 )
		m_vecParticle[_idx].m_vec3ParticlePos += _offset;


	unguard;
}

/** \brief
*   从当前的粒子对列点,替代一定数目的粒子.
*/
void os_emssionCtlIns::replace_particle( int _rnum,osVec3D* _offset )
{
	guard;

	float   t_f;
	int     t_iMax,t_idx;

	osassert( _rnum > 0 );

	t_iMax = m_ptrParScene->m_iMaxParticleNum;
	// 
	// 替代最老的粒子.
	if( (m_ptrParScene->m_iNpwmr == 1) )
	{
		for( int t_i = 0;t_i<_rnum;t_i ++ )
		{
			t_f = 0;

			// 得到生命时间最长的粒子.
			for( int t_j = 0;t_j<t_iMax;t_j ++ )
			{
				if( m_vecParticle[t_j].m_fElapseTime < t_f )
					continue;
				t_f = m_vecParticle[t_j].m_fElapseTime;
				t_idx = t_j;
			}

			// 初始化这个粒子.
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
		//此处应该等于零，即等待新产生空闲粒子
		return;
	}

	return;

	unguard;
}

/** \brief
*  对粒子进行更新.
*/
void os_emssionCtlIns::update_particle( int _idx,osVec3D& _offset )
{
	guard;

	float    t_fGravity;
	osVec3D   t_vec3Offset;

	osassert( _idx >= 0 );

	//
	// 进行杂项处理.
	get_tvalue( _idx );
	get_textureFrame( _idx );
	get_particleResistance( _idx );
	get_parSize( _idx );
	get_parAlpha( _idx );
	get_parColor( _idx );
	get_parRotation( _idx );

	//
	// 处理粒子的位置.
	t_fGravity = m_ptrParticleRunCtl->m_fGravity;
	t_vec3Offset = m_vecParticle[_idx].m_vec3Velocity*m_fTimeLastFrame;
	t_vec3Offset.y -= (t_fGravity*m_fTimeLastFrame*m_fTimeLastFrame/2.0f);
	t_vec3Offset *= m_ptrParScene->m_fMultiSize;

# if 0 // River @ 2009-5-14:为了暂时兼容以前边缘的粒子特效，不然
	   // 很多的粒子效果往左或是往中间移动，效果不太对
	// River mod @ 2008-6-13:加入加速度方向为零时，粒子的加速度向粒子
	// 的原点飞行.
	//! River mod @ 2008-6-11:为了能做出往中间吸引的粒子效果
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


	// River mod @ 2008-6-13:处理Y轴的旋转信息,可以统一旋转，以进行批量计算.
	// River @ 2009-3-30:粒子不能有旋转操作，否则会产生出错误的数据......
	//                   暂时想不出更好的解决方案。
	osMatrix  t_sMat;
	osMatrixRotationY( &t_sMat,m_fTimeLastFrame*m_ptrParticleRunCtl->m_fParticleYRot );
	osVec3Transform( &m_vecParticle[_idx].m_vec3ParticlePos,
		&m_vecParticle[_idx].m_vec3ParticlePos,&t_sMat );


	return;

	unguard;
}





//! 得到当前时刻产生emitter产生粒子的数目.
int os_emssionCtlIns::get_emitterGenParNum( void )
{
	guard;

	float         t_fMax,t_fMin;

	//
	// ATTENTION TO OPP:可以不使用accumulate测试粒子产生情况.
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
	// 如果产生粒子数目的最大值和最小值相等.
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


//! 当前的emicontrolIns是否激活。
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
	// 只有粒子发射器发射完粒子，粒子数组中所有的粒子
	// 不在激活时，整个emssion系统才算死亡。
	return false;

	unguard;
}


//! 计算新产生的每一个粒子的位置
void os_emssionCtlIns::cal_newParticlePos( osVec3D* _pos,osVec3D& _offset,int _num )
{
	guard;

	osassert( _pos );

	if( _num > MAX_PARTICLENUM )
		_num = MAX_PARTICLENUM;

	m_bEmitPosChanged = FALSE;
	if( !vec3_equal( m_vec3LastEmitPos,_offset ) )
		m_bEmitPosChanged = TRUE;

	// 三分之一的粒子在新的位置产生，用于生成头部的效果
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

	//! 另外一半的粒子在当前帧位置和上帧位置之间产生，用于生成连续的效果
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
*  每一帧对当前的emissionControlInstance进行更新。
*
*  更新粒子控制内的emitterIns和particleInsArray.
* 
*/
void os_emssionCtlIns::framemove_emission( float _etime,osVec3D& _offset )
{
	guard;

	int             t_iParticleNum,t_iMax,t_iNum,t_i;
	//! 这个数组用于计算新产生的每一个粒子的位置,如果粒子发生器位置改变，产生连续位置的粒子
	static osVec3D  t_vec3NewParticlePos[MAX_PARTICLENUM];

	if( _etime <= 0.000001f )
		return;

	osassert( _etime >= 0.0f );

	// 
	// 如果是初始位置，则赋上帧位置为当前位置
	if( vec3_equal( m_vec3LastEmitPos,osVec3D( 0.0f,0.0f,0.0f ) ) )
		m_vec3LastEmitPos = _offset;

	m_fTimeFromLS += _etime;
	m_fTimeLastFrame = _etime;

	//
	// 处理emitter是否结果.
	if( m_particleEmtIns.m_bAlive )
		emission_cycleEnd();


	//
	// 得到当前时刻emitter的相关数据.
	if( m_particleEmtIns.m_bAlive )
	{
		get_emitterInsData();

		// 对粒子控制进行处理.
		t_iParticleNum = get_emitterGenParNum();
	}
	else
	{
		t_iParticleNum = 0;
	} 

	// River added @ 2007-5-10:解决一下子产生大量粒子的问题：
	if( t_iParticleNum >= m_ptrParScene->m_iMaxParticleNum )
		t_iParticleNum = m_ptrParScene->m_iMaxParticleNum;


	// 计算新产生的每一个粒子的位置。
	cal_newParticlePos( t_vec3NewParticlePos,_offset,t_iParticleNum );

	t_iMax = m_ptrParScene->m_iMaxParticleNum;

	//
	// 从非激活粒子中,产生出新的粒子.
	t_iNum = 0;

	for( t_i=0;(t_i<t_iMax)&&(t_iNum<t_iParticleNum);t_i++ )
	{
		if( m_vecParticle[t_i].m_bAlive )
			continue;

		init_particle( t_i,t_vec3NewParticlePos[t_iNum] );
		t_iNum ++;

	}

	//
	// 从当前激活的粒子中,替代产生出新的粒子.
	if( (t_i == t_iMax)&&(t_iNum<t_iParticleNum) )
	{
		replace_particle( t_iParticleNum - t_iNum,&t_vec3NewParticlePos[t_iNum] );
	}


	//
	// 对当前粒子队列中的每一个激活的粒子进行更新.
	for( int t_i=0;t_i<t_iMax;t_i ++ )
	{
		if( m_vecParticle[t_i].m_bAlive )
			update_particle( t_i,_offset );
	} 

	// 记录粒子发射器上次发射粒子所在的位置
	if( m_bEmitPosChanged ) m_vec3LastEmitPos = _offset;

	return ;

	unguard;
}

/** \brief
*  填充当前emissionControl Instance内的粒子Array到指定的数据缓冲区中。
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

	// River @ 20106-6-8:界面上的粒子渲染，使用固定的方向
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
	// 对每一个粒子,填充这个粒子的四个顶点.
	int    t_iMax = m_ptrParScene->m_iMaxParticleNum;
	for( int t_i=0;t_i<t_iMax;t_i ++ )
	{
		// 如果不是激活粒子,退出不渲染.
		if( !m_vecParticle[t_i].m_bAlive )
			continue;

		t_ptrPar = &m_vecParticle[t_i];
		t_ptrVerStart = &_verStart[m_iParVertexNum];

		// 
		// 计算uv.
		t_fUBase = t_ptrPar->m_iTexFrame*t_fULength;
		t_fVBase = t_ptrPar->m_iTexSetsIdx*t_fVLength;

		// 
		// 计算旋转相关数据.
		osMatrixRotationAxis( &t_matRot,&t_vec3Look,m_vecParticle[t_i].m_fRotation );
		osVec3Transform( &t_vec3BR,&t_vec3Right,&t_matRot );
		osVec3Transform( &t_vec3BU,&t_vec3Up,&t_matRot );

		//
		// 填充顶点.
		t_ptrVerStart[0].m_vec3Pos = 
			(-t_vec3BR-t_vec3BU)*t_ptrPar->m_fSize+t_ptrPar->m_vec3ParticlePos;
		t_ptrVerStart[1].m_vec3Pos = 
			(t_vec3BR-t_vec3BU)*t_ptrPar->m_fSize + t_ptrPar->m_vec3ParticlePos;
		t_ptrVerStart[2].m_vec3Pos = 
			(t_vec3BR+t_vec3BU)*t_ptrPar->m_fSize + t_ptrPar->m_vec3ParticlePos;
		t_ptrVerStart[3].m_vec3Pos = 
			(-t_vec3BR+t_vec3BU)*t_ptrPar->m_fSize + t_ptrPar->m_vec3ParticlePos;

		//
		// 填充uv坐标.
		t_ptrVerStart[0].m_vec2Uv = 
			osVec2D( t_fUBase,t_fVBase );
		t_ptrVerStart[1].m_vec2Uv = 
			osVec2D( t_fUBase + t_fULength,t_fVBase );
		t_ptrVerStart[2].m_vec2Uv = 
			osVec2D( t_fUBase + t_fULength,t_fVBase + t_fVLength );
		t_ptrVerStart[3].m_vec2Uv = 
			osVec2D( t_fUBase,t_fVBase + t_fVLength );


		//
		// 填充颜色值.
		t_ptrVec3 = &t_ptrPar->m_vec3Color;

		t_ptrVerStart[0].m_dwColor = 
			osColor( t_ptrVec3->x,t_ptrVec3->y,t_ptrVec3->z,t_ptrPar->m_fAlpha  );
		t_ptrVerStart[1].m_dwColor = t_ptrVerStart[2].m_dwColor = 
			t_ptrVerStart[3].m_dwColor = t_ptrVerStart[0].m_dwColor;

		//
		// 每一个激活粒子需要填充4个顶点.
		m_iParVertexNum += 4;

	}

	unguard;
}


//! 对当前的emiControlIns进行FrameMove.
void os_emssionCtlIns::frameMove_eci( 
			osVec3D& _offset,float _etime,osc_camera* _cam )
{
	guard;

	osassert( m_ptrParticleRunCtl );

	// 
	// 如果是世界空间的粒子,则使用identity的世界矩阵.
	// 然后把粒子在世界空间的位置加到粒子的顶点位置上.
	// 先frameMove当前的emissionControl,并填充粒子到内存。
	framemove_emission( _etime,_offset );

	fill_renderVerBuf( m_vecRenderVer,_cam );

	// 记录上次Emitter发射粒子所在的位置
	m_vec3LastEmitPos = _offset;


	return;

	unguard;
}


/** \brief
*  渲染当前的emissionControlInstance.
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
	// 设置世界矩阵.emitterto为1时粒子发散到世界空间
	osMatrix   t_mat,t_scaleMat;
	osMatrixScaling( &t_scaleMat,m_fSizeScale,m_fSizeScale,m_fSizeScale );

	// 发散到世界空间的的粒子，处理缩放时，直接加入粒子的位置位移即可
	if( m_ptrParScene->m_iEmitterTo == 1 )
	{
		osMatrixTranslation( &t_mat,_offset.x,_offset.y,_offset.z );
		osMatrixMultiply( &t_mat,&t_scaleMat,&t_mat );
	}
	else
	{
		// 发散到粒子空间的粒子，需要计算出粒子缩放前的位置和缩放后的位置，
		// 把两个位置的偏移量，加入到的粒子的世界缩放矩阵内，即粒子缩放后，
		// 位置也缩放了，但我们需要粒子位置没有缩放的效果。
		osVec3D   t_vec3AfterScale = _offset*m_fSizeScale;
		osVec3D   t_vec3Offset = _offset - t_vec3AfterScale;
		
		t_mat = t_scaleMat;
		t_mat._41 = t_vec3Offset.x;
		t_mat._42 = t_vec3Offset.y;
		t_mat._43 = t_vec3Offset.z;
	}

	_pipe->set_worldMatrix( t_mat );


	// 如果当前emission内有需要渲染的particle.
	if( m_iParVertexNum > 0 )
	{
		// 
		// 根据和背景混合或是和背景相加两种渲染方式，设置渲染状态。
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
		// 填充顶点数据到顶点缓冲区，并渲染当前粒子的顶点数据。
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
// Particle ins相关函数。
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
*  使用场景指针来初始化一个particleInstance.
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

	// River @ 2010-6-9:用于界面上的人物渲染
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
	// 如果当前的particle场景用到了track,则从trackMgr中创建track.
	if( m_ptrParScene->m_bUseTrack )
	{
		m_iTrackId = g_ptrTrackMgr->create_track( 
			m_ptrParScene->m_szTrackFileName,m_ptrParScene->m_fRunSpeed,true,false );

		// ATTENTION TO FIX:River mod @ 2007-4-6:修改为不出错.
		if( m_iTrackId < 0 )
			m_ptrParScene->m_bUseTrack = FALSE;

	}

	m_bInuse = true;

	return get_val();

	unguard;
}



//! 对partileIns进行framemove.
void os_particleSceneIns::frameMove_curIns( float _etime,
						osc_camera* _cam,bool _canDeleteCurIns/* = false*/ )
{
	guard;

	osassert( this->m_ptrParScene );
	m_ptrParScene->set_used();

	osVec3D    t_vec3;
	bool       t_bAct = false;

	// River mod @ 2010-6-8:界面上的粒子不需要cam.
	if( !m_bTopLayer )
		osassert( _cam );


	t_vec3.x = 0.0f;t_vec3.y = 0.0f;t_vec3.z = 0.0f;

	// 得到路径动画的相对位置数据.
	if( m_ptrParScene->m_bUseTrack )
	{
		g_ptrTrackMgr->get_aniVec( m_iTrackId,_etime,t_vec3 );
		m_vec3FinalOffset = t_vec3 + m_vec3Offset;
	}
	else
	{
		m_vec3FinalOffset = m_vec3Offset;
	}
	


	// 在每一个emission control内部控制当前emission的世界矩阵.
	for( int t_i=0;t_i<m_ptrParScene->m_iEmissionCtlNum;t_i ++ )
	{
		//
		// 如果当前的emissionControl Instance处于死亡状态，则不需要渲染。
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
*  渲染当前的particleSceneInstance.
*
*  对每一个emissionControl进行处理。
*  
*  \param _etime 上一帧过去的时间。
*  
*/
bool os_particleSceneIns::render_currentIns( 
	    osc_camera* _cam, osc_middlePipe* _mpipe )
{
	guard;

	osVec3D      t_vec3( 0.0f,0.0f,0.0f );

	osassert( this->m_ptrParScene );

	// River mod @ 2010-6-8:界面上人物武器粒子特效的渲染
	if( m_bTopLayer )
	{
		frameMove_curIns( sg_timer::Instance()->get_lastelatime(),_cam,true );
		if( m_ptrParScene == NULL )
			return true; 
	}

# if 1

	// 在每一个emission control内部控制当前emission的世界矩阵.
	for( int t_i=0;t_i<m_ptrParScene->m_iEmissionCtlNum;t_i ++ )
	{
		//
		// 如果当前的emissionControl Instance处于死亡状态，则不需要渲染。
		if( !m_arrEmissonCtlIns[t_i].is_alive() )
			continue;

		//! River mod @ 2009-2-11:如果处于非隐藏状态，渲染.
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

//! 设置当前粒子场景实例的播放时间。
void os_particleSceneIns::set_parPlayTime( float _timeScale )
{
	guard;

	osassert( _timeScale > 0.0f );

	m_fTimeScale =_timeScale;

	unguard;
}

//! 设置自定义粒子的发散线段范围
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


//! 当前的粒子场景是否是循环播放的粒子场景
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


//! 得到当前粒子场景实例的播放时间。
float os_particleSceneIns::get_parPlayTime( void )
{
	guard;

	float   t_fRes = 0.0f;

	// 
	// 如果循环播放的粒子，则返回负值，上层根据是否是负值，
	// 来确认是否是循环播放的粒子。　
	for( int t_i=0;t_i<MAX_EMISSION;t_i ++ )
	{
		if( m_arrEmissonCtlIns[t_i].is_alive() )
		{
			// 
			// 如果碰上循环播放的粒子，则返回负的时间播放值
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

//! 删除当前particle scene instance用到的数据.
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
	// ATTNENTION TO FIX:删除当前particle 场景的对应数据.

	reset_ins();

	//! River added @ 2010-3-20:彻底释放粒子
	if( _finalRelease )
	{
		m_ptrParScene->release_parScene();
		
		// 释放spaceMgr.
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



//! 粒子管理器公用的顶点和索引缓冲区
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
	// 释放管理器中所有的粒子场景和粒子场景实例.

}


/** \brief
 *  初始化当前的particle管理器。
 *
 */
void osc_particleMgr::init_particleMgr( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );
	m_ptrMPipe = _pipe;
	
	// 得到当前vertex buffer的指针。
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	// River @ 2011-2-16:加大粒子的dynamic buff，并使用独立的buff.
	osc_particleMgr::m_iVbId = _pipe->create_dynamicVB( t_dwFvf,4096*384,true );
	osassert( m_iVbId>=0 );	
	//
	// 初始化当前index buffer,因为所有粒子可以使用相同的index buffer.
	// 在渲染的时候，根据每一个emssion粒子数目的多少值入三角形数目。
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

	//! River @ 2011-2-16:一次性的调入所有的particle文件资源，加速游戏内的par创建。


	return;

	unguard;
}


//! 对ParticleMgr中的particleInstance进行FrameMove,渲染和处理分开.
void osc_particleMgr::framemove_particle( osc_camera* _cam )
{
	guard;


	// River @ 2011-1-30:删除particle的函数并非真正的删除，测试此处去掉CS.
	// 确认进入和退出临界区.
	//::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	// 为每一个粒子文件设置lru.
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
			// River @ 2011-1-30:如果没有在使用中，进入下一个，优化了代码，有可能出现这种情况。
			if( !t_ptrN.p->is_inUse() )
				continue;

			// River mod @ 2010-6-8:渲染界面武器特效粒子
			if( t_ptrN.p->get_topLayer() )
				continue;

			t_ptrN.p->frameMove_curIns( t_fEtime,_cam,true );
			if( !t_ptrN.p->is_inUse() )
				m_vecParSceIns.release_node( t_ptrN.idx );
		}
	}

	// 离开临界区
	//::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	return;

	unguard;
}



//! 测试渲染particle.
bool osc_particleMgr::render_particle( 
		osc_camera* _cam,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	guard;

	// 
	// 设置Alpha Test的值为最小。
	m_ptrMPipe->set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
	m_ptrMPipe->set_renderState( D3DRS_ALPHAREF, GLOBAL_MINALPHAREF );
	m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
	m_ptrMPipe->set_renderState( D3DRS_CULLMODE,D3DCULL_NONE );

	// River @ 2011-1-30:删除particle的函数并非真正的删除，测试此处去掉CS.
	// 确认进入和退出临界区.
	//::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	CSpaceMgr<os_particleSceneIns>::NODE t_ptrN;

	for( t_ptrN = m_vecParSceIns.begin_usedNode();
		t_ptrN.p != NULL;
		t_ptrN = m_vecParSceIns.next_validNode( &t_ptrN ) )
	{
		if( t_ptrN.p->get_insInit() )
		{
			// River @ 2011-1-30:如果没有在使用中，进入下一个，优化了代码，有可能出现这种情况。
			if( !t_ptrN.p->is_inUse() )
				continue;

			//@{
			// River added @ 2010-6-8:用于在界面上渲染人物武器特效
			if( OS_RENDER_TOPLAYER == _rtype )
			{
				if( !t_ptrN.p->get_topLayer() )
					continue;
				// River @ 2010-6-9:确保界面特效每次只渲染一个particle.
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

	// 离开临界区
	//::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );


	return true;
	unguard;
}


//! 释放最长时间不使用particle.
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

	//! 释放此particle
	if( t_iIdx >= 0 )
	{
		m_vecParticleScene[t_iIdx].release_parScene();
		m_vecParticleScene.release_node( t_iIdx );
	}

	unguard;
}


/** \bief
*  创建一个粒子场景实例.
* 
*  处理这一段程序,加入替换算法,使引擎中永远可以
*  创建正常运行的粒子场景.
* 
*  \return int 返回了当前particle的id.
*/
int osc_particleMgr::create_parSceIns( os_particleCreate& _pcreate,
				BOOL _inScene/* = FALSE*/,bool _topLayer/* = false*/  )
{
	guard;

	int      t_idx,t_id = -1;
	WORD     t_wCreateVal = 0;

	osassert( _pcreate.m_szParticleName != "" );

	//
	// 是否当前要创建的场景在管理器中已经存在.
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
	// 找不到要创建的场景的话,创建新的场景.
	if( t_idx == -1 )
	{
		// River @ 2011-1-30:此处加入CS,多线程得到可用资源。
		::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );
		t_idx = m_vecParticleScene.get_freeNode();
		::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );

		if( !m_vecParticleScene[t_idx].load_parSceneFromFile( 
			_pcreate.m_szParticleName ) )
			osassert( false );
	}

	// 如果不需要创建粒子的实例,返回.
	if( !_pcreate.m_bCreateIns )
	{
		m_vecParticleScene[t_idx].set_swap( false );
		t_id = -1;
	}
	else
	{
		//
		// 创建当前的粒子场景实例
		// River @ 2011-1-30:此处加入CS,多线程得到可用资源。
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
 *  删除一个particle.
 *
 *  ATTENTION:  需要删除跟当前的ins相关联的粒子数据.
 *  
 */
void osc_particleMgr::delete_particle( int _idx,bool _finalRelease/* = false*/ )
{
	guard;

	WORD   t_wId = WORD( _idx );
	
	// River @ 2011-1-30:删除particle的函数并非真正的删除，测试此处去掉CS.
	// 确认进入和退出临界区.
	//::EnterCriticalSection( &osc_mapLoadThread::m_sParticleCS );


	//
	///如何正确的删除？？？？？？　
	// 如果当前的particle正在使用中，则删除当前的particle.
	if( m_vecParSceIns.validate_id( t_wId ) )
	{
		if( m_vecParSceIns[t_wId].is_inUse() )
		{
			if( m_vecParSceIns[t_wId].validate_cval( _idx ) )
			{
				// River @ 2010-3-20:加入最终释放粒子的参数
				if( _finalRelease )
				{
					m_vecParSceIns[t_wId].delete_parSceIns( true );
					m_vecParSceIns.release_node( t_wId );
				}
				else
				{
					// River mod @ 2008-7-18: 为了让粒子的消失效果更加平和,
					//  不直接删除粒子,停止粒子产生,粒子生命周期完成后自动消失.
					m_vecParSceIns[t_wId].stop_parSceIns();
				}
			}
		}
	}

	//::LeaveCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	unguard;
}

//! 设置粒子场景的播放时间。
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

//! 得以粒子场景的播放时间。
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

//! 是否是循正播放的公告板
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

		// 如果是上层设置由线段产生器产生的粒子，则等待设置线段时产生
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
*  检测粒子id对应粒子的有效性.
*
*  如果对应的粒子处于死亡状态，则处理当前
*  的粒子id.
*  \return bool 如果有效，返回true.
*/
bool osc_particleMgr::validate_parId( int _id )
{
	guard;

	WORD   t_wId = WORD( _id );

	bool   t_bRes;

	// River @ 2011-1-30:删除particle的函数并非真正的删除，测试此处去掉CS.
	// 确认进入和退出临界区.
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



