

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  os_particleEmitter

//! 从一个tvalue得到当前emitter发射出的粒子的方向.
inline void os_particleEmitter::get_emitDirection( osVec3D& _dir,float _tval )
{
	osassert( (_tval>=0.0f)&&(_tval<=1.0f) );
	int  t_idx = int(_tval*100);
	_dir = m_vec3Dir[t_idx];
}

//! 从一个tvalue得到当前emitter发射出粒子的velocity.
inline void os_particleEmitter::get_emitRandVelocity( osVec2D& _randVel,float _tval )
{
	osassert( (_tval>=0.0f)&&(_tval<=1.0f) );
	int  t_idx = int(_tval*100);
	_randVel = this->m_vec2Rand[t_idx];
}


//! 从一个tvalue得到当前emitter发射粒子的速度.
inline void os_particleEmitter::get_emissionRate( osVec2D& _erate,float _tval )
{
	osassert( (_tval>=0.0f)&&(_tval<=1.0f) );
	int  t_idx = int(_tval*100);	
	_erate = m_vec2EmissionRate[t_idx];
	_erate.x *= m_fGenSpeedMulti;
	_erate.y *= m_fGenSpeedMulti;
}


//! 从一个tvalue得到当前emitter发射的粒子在世界中的生命长度.
inline void os_particleEmitter::get_emitParticleLifeTime( osVec2D& _ltime,float _tval )
{
	osassert( (_tval>=0.0f)&&(_tval<=1.0f) );
	int  t_idx = int(_tval*100);
	_ltime = m_vec2EleLife[t_idx];
}





//////////////////////////////////////////////////////////////////////////////////////////////////////
// os_emssionCtlIns
// 场景中粒子系统的inline函数文件.
//! 粒子产生器是否生命终结,处理.
inline void os_emssionCtlIns::emission_cycleEnd( void )
{

	// 如果时间超过了emitter的生命周期,
	if( m_fTimeFromLS > m_ptrEmitter->m_fCycleLength )
	{
		if( m_bFirstFrameGenPar )
		{
			m_bFirstFrameGenPar = FALSE;
			m_fTimeFromLS = m_ptrEmitter->m_fCycleLength;
		}
		else 
		{
			if( m_ptrEmitter->m_bLoopCycle )
			{
				float t_fLastFrametime = m_fTimeLastFrame;
				emitter_cycleStart();
				if( t_fLastFrametime > m_ptrEmitter->m_fCycleLength )
					m_fTimeLastFrame = m_ptrEmitter->m_fCycleLength - 0.00001f;
			}
			else
				m_particleEmtIns.m_bAlive = false;
		}
	}


}



//! 重新开始一个emitter的cycle.
inline void os_emssionCtlIns::emitter_cycleStart( void )
{
	m_fTimeFromLS = 0.0f;
	m_fTimeLastFrame = 0.0f;

	m_particleEmtIns.m_bAlive = true;
	m_particleEmtIns.m_fTVal = 0.0f;

	framemove_emission( 0.0f,osVec3D( 0.0f,0.0f,0.0f ) );

}


//! 得到emitterInstantce在当前时刻的数据.
inline void os_emssionCtlIns::get_emitterInsData( void )
{

	float   t_f;

	t_f = m_fTimeFromLS/m_ptrEmitter->m_fCycleLength;

	// 如果周期过短，则需要执行如下的代码。
	osassert( (t_f>=0)&&(t_f<=1.0f) );

	m_particleEmtIns.m_fTVal = t_f;
	
	//
	// 
	m_ptrEmitter->get_emitDirection( m_particleEmtIns.m_vec3Dir,t_f );
	m_ptrEmitter->get_emitRandVelocity( m_particleEmtIns.m_vec2RandVelocity,t_f );
	m_ptrEmitter->get_emissionRate( m_particleEmtIns.m_vec2EmissionRate,t_f );
	m_ptrEmitter->get_emitParticleLifeTime( m_particleEmtIns.m_vec2ParLifeTime,t_f );

}



//! 得到粒子初始化的velocity.
inline void os_emssionCtlIns::get_ptlInitVelocity( int _idx )
{
	float   t_f;
	osVec3D  t_vec3,t_scope;

	t_f = getrand_fromfloatscope( m_particleEmtIns.m_vec2RandVelocity.x,
		         m_particleEmtIns.m_vec2RandVelocity.y );
	t_vec3 = m_particleEmtIns.m_vec3Dir*t_f;

	t_scope = m_ptrEmitter->m_vec3EmissionSpeedScope;
	t_vec3.x += ::getrand_fromfloatscope( -t_scope.x,t_scope.x );
	t_vec3.y += ::getrand_fromfloatscope( -t_scope.y,t_scope.y );
	t_vec3.z += ::getrand_fromfloatscope( -t_scope.z,t_scope.z );

	m_vecParticle[_idx].m_vec3Velocity = t_vec3;

	return;
}

//! 得到粒子的resistantance.
inline void os_emssionCtlIns::get_particleResistance( int _idx )
{
	osVec3D     t_vec3Normal,t_vec3SizeRes( 0,0,0 ),t_vec3VelRes;
	float       t_f;

	osVec3Normalize( &t_vec3Normal,&m_vecParticle[_idx].m_vec3Velocity );
	t_f = osVec3Length( &m_vecParticle[_idx].m_vec3Velocity );

	// 
	// 如果当前的速度还大于不使用租力的速度.
	if( t_f > m_ptrParticleRunCtl->m_fSpeedNoResis )
	{
		// 如果受大小影响,使用初始化值.
		if( m_ptrParticleRunCtl->m_iAreaSizeResisType == 1 )
		{
			t_f = m_vecParticle[_idx].m_fSize*
				m_ptrParticleRunCtl->m_fAreaSizeCons;
			t_vec3SizeRes = t_f*t_vec3Normal;
		}
		t_vec3SizeRes *= m_fTimeLastFrame;

		// 
		// 受当前加速度的线性影响. 
		t_vec3VelRes = m_vecParticle[_idx].m_vec3Velocity*
			m_ptrParticleRunCtl->m_fAreaSizeCons;
		t_vec3VelRes *= m_fTimeLastFrame;

		t_vec3SizeRes += t_vec3VelRes;

		m_vecParticle[_idx].m_vec3Velocity -= t_vec3SizeRes;
	}

	return;

}

//! 当前的粒子场景是否是刀光线段的粒子
inline bool os_emssionCtlIns::is_lineSegPar( void )
{
	if( m_ptrEmitter->m_iEmitterShape == 3 )
		return true;
	else
		return false;
}


//! 得到粒子的初始化位置.
inline void os_emssionCtlIns::get_ptlInitPosition( int _idx )
{
	osVec3D     t_vec3;

	// 矩形区内产生粒子.
	if( this->m_ptrEmitter->m_iEmitterShape == 0 )
	{
		t_vec3.x = getrand_fromfloatscope( 
			m_ptrEmitter->m_sbox.m_vecMin.x,m_ptrEmitter->m_sbox.m_vecMax.x );
		t_vec3.y = getrand_fromfloatscope( 
			m_ptrEmitter->m_sbox.m_vecMin.y,m_ptrEmitter->m_sbox.m_vecMax.y );
		t_vec3.z = getrand_fromfloatscope( 
			m_ptrEmitter->m_sbox.m_vecMin.z,m_ptrEmitter->m_sbox.m_vecMax.z );

		t_vec3 *= m_ptrParScene->m_fMultiSize;
		
		t_vec3 += m_ptrEmitter->m_vec3RelativePosition;
		m_vecParticle[_idx].m_vec3ParticlePos = t_vec3;
	}
	// 球形区内产生粒子.
	else if( m_ptrEmitter->m_iEmitterShape == 1 )
	{
		float   t_f = m_ptrEmitter->m_fFallOff+m_ptrEmitter->m_fHotPt;

		t_vec3.x = getrand_fromfloatscope( -t_f,t_f );
		t_vec3.y = getrand_fromfloatscope( -t_f,t_f );
		t_vec3.z = getrand_fromfloatscope( -t_f,t_f );

		t_vec3 *= m_ptrParScene->m_fMultiSize;

		t_vec3 += m_ptrEmitter->m_vec3RelativePosition;

		m_vecParticle[_idx].m_vec3ParticlePos = t_vec3;
	}
	// 自定义产生粒子的区域.
	else if( m_ptrEmitter->m_iEmitterShape == 2 )
	{
		if( m_ptrEmitter->m_iTrackId >= 0 )
		{
			osVec3D   t_vec3Pos;
			g_ptrTrackMgr->get_randomVec( m_ptrEmitter->m_iTrackId,t_vec3Pos );

			t_vec3Pos *= m_ptrParScene->m_fMultiSize;
			m_vecParticle[_idx].m_vec3ParticlePos = t_vec3Pos + m_ptrEmitter->m_vec3RelativePosition;
		}
	}
	// River added @ 2006-6-30:加入线段
	else if( m_ptrEmitter->m_iEmitterShape == 3 )
	{
		float t_fRand = getrand_fromfloatscope( 0.0f,1.0f );

		osVec3Lerp( &m_vecParticle[_idx].m_vec3ParticlePos,&m_vec3LSStart,&m_vec3LSEnd,t_fRand );

		m_vecParticle[_idx].m_vec3ParticlePos += m_ptrEmitter->m_vec3RelativePosition;;
	}
	// River added @ 2008-6-13:加入环形带产生粒子的功能.
	else if( m_ptrEmitter->m_iEmitterShape == 4 )
	{
		D3DXMATRIX  t_mat;
		D3DXMatrixRotationY( &t_mat,getrand_fromfloatscope( 0.0f,OS_PI*2.0f) );

		float t_fS = m_ptrEmitter->m_fHotPt*m_ptrParScene->m_fMultiSize;

		t_vec3.x = getrand_fromfloatscope( 
			t_fS,t_fS + m_ptrParScene->m_fMultiSize*m_ptrEmitter->m_fFallOff );
		t_vec3.y = 0.0f;
		t_vec3.z = 0.0f;
		osVec3Transform( &t_vec3,&t_vec3,&t_mat );

		t_vec3 += m_ptrEmitter->m_vec3RelativePosition;
		m_vecParticle[_idx].m_vec3ParticlePos = t_vec3;
	}

	return;
}

//! 得到粒子的大小.
inline void os_emssionCtlIns::get_parSize( int _idx )
{
	int   t_idx = int(m_vecParticle[_idx].m_fTval*100);
	
	m_vecParticle[_idx].m_fSize = 
		m_ptrParticleRunCtl->m_vecParSize[t_idx];

	// 对大小进行倍乘.
	m_vecParticle[_idx].m_fSize *= m_ptrParScene->m_fMultiSize;

}

//! 得到粒子的alpha
inline void os_emssionCtlIns::get_parAlpha( int _idx )
{
	int   t_idx = int(m_vecParticle[_idx].m_fTval*100);

	m_vecParticle[_idx].m_fAlpha = 
		m_ptrParticleRunCtl->m_vecAlpha[t_idx];
}

//! 得到粒子的color
inline void os_emssionCtlIns::get_parColor( int _idx )
{
	int   t_idx = int(m_vecParticle[_idx].m_fTval*100);

	m_vecParticle[_idx].m_vec3Color = 
		m_ptrParticleRunCtl->m_vecColor[t_idx];
}

//! 得到粒子的旋转.
inline void os_emssionCtlIns::get_parRotation( int _idx )
{
	int   t_idx = int(m_vecParticle[_idx].m_fTval*100);

	m_vecParticle[_idx].m_fRotation += 
		((m_ptrParticleRunCtl->m_vecRotation[t_idx])*m_fTimeLastFrame);

}

//! 得到当前粒子的tvalue
inline void os_emssionCtlIns::get_tvalue( int _idx )
{
	m_vecParticle[_idx].m_fElapseTime += m_fTimeLastFrame;
	
	if( m_vecParticle[_idx].m_fElapseTime > 
		   m_vecParticle[_idx].m_fLifeTime )
	{
		m_vecParticle[_idx].m_bAlive = false;
	}
	else
	{
		m_vecParticle[_idx].m_fTval = 
			m_vecParticle[_idx].m_fElapseTime / m_vecParticle[_idx].m_fLifeTime;
	}

	return;		
}

//! 得到当前粒子用到的纹理帧数.
inline void os_emssionCtlIns::get_textureFrame( int _idx )
{
	float    t_f;

	t_f = 1.0f/float(m_ptrParticleRunCtl->m_iFrameNum);
	t_f = m_vecParticle[_idx].m_fTval / t_f;
	m_vecParticle[_idx].m_iTexFrame = int( t_f );

	// River @ 2006-9-1:为保证每一个粒子的纹理uv坐标不能大于1.0f
	if( m_vecParticle[_idx].m_iTexFrame >= m_ptrParticleRunCtl->m_iFrameNum )
		m_vecParticle[_idx].m_iTexFrame = m_ptrParticleRunCtl->m_iFrameNum - 1;
	return;
}






//! 设置粒子产生所在的线段范围
inline void os_emssionCtlIns::set_parGetLineSeg( osVec3D& _start,osVec3D& _end )
{
	m_vec3LSStart = _start;
	m_vec3LSEnd = _end;
}






//=========================================================================================
// 



//=========================================================================================
//
// particle scene instance 用到的数据.


//! 平和的停止当前的粒子场景
inline void os_particleSceneIns::stop_parSceIns( void )
{
	for( int t_i=0;t_i<m_ptrParScene->m_iEmissionCtlNum;t_i ++ )
		m_arrEmissonCtlIns[t_i].stop_emissionIns();
}


//=========================================================================================
//
// particle Manager用到的粒子数据.
// 
//! 设置一个particle的位置数据.


//! 设置粒子的发射线段的开始和结束位置
inline void osc_particleMgr::set_parGenLineSeg( int _id,osVec3D& _start,osVec3D& _end )
{
	WORD   t_wId = WORD( _id );

	osassert( _id >= 0 );
	if( m_vecParSceIns.validate_id( t_wId ) )
	{
		osassert( m_vecParSceIns[t_wId].is_inUse() );

		if( m_vecParSceIns[t_wId].validate_cval( _id ) )
			m_vecParSceIns[t_wId].set_parGetLineSeg( _start,_end );
	}	
}


//! River @ 2010-6-9:为了人物身上的粒子渲染在界面中正确.
inline void osc_particleMgr::reset_particleInsRender( int _id )
{
	WORD  t_wId = WORD( _id );

	if( m_vecParSceIns.validate_id( t_wId ) )
		m_vecParSceIns[t_wId].m_bRenderedInCurFrame = false;

	return ;
}


//! 隐藏传入的particle id效果
inline bool osc_particleMgr::hide_particle( int _id,bool _hide )
{
	WORD  t_wId = WORD( _id );

	// River @ 2010-8-7：隐藏的时候注意此ID是否valid.
	if( !m_vecParSceIns.validate_id( t_wId ) )
		return false;

	m_vecParSceIns[t_wId].hide_particle( _hide );

	return true;
}



