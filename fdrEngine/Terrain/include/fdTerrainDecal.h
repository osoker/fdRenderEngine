//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTerrainDecal.h
 *
 *  His:      River created @ 2006-3-6
 *
 *  Des:      �����ڵ�ǰ�ĵر��ϴ���Decal.
 *   
 * ��������ߵ����̵ı�־�ǣ�����������ȫ�෴���뷨��ͷ���У�����Ϊ˿������Ӱ���������
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once


//! ��������Ľӿ�ͷ�ļ�
# include "../../interface/osInterface.h"
# include "../../mfpipe/include/osPolygon.h"

class     osc_TGManager;


/*
Decal�ǽ����ر���Ⱦ�Ķ���Σ���Ҫ���ݵر����εĸߵ����������Ӧ�ĵ�����
Decal����������Ч����
1: ����ļ���Ӱ��
2: �����ڵر�Ľ�ӡ��
3: ħ����Ч�Եر���ƻ�����ɫ�ۼ�

Decal��Ϊ�з�����޷�������������з���ĸ����޷����decal���ٶȸ��졣
�޷���ı����������Ӱ��������Բ�ġ��з���ı�������Ľ�ӡ��ħ���ۼ���

Decal����Ҫ��ϸ��Ϊ��Ҫ������Decal��һֱ��ʾֱ��ɾ����Decal.�����ӡ�͵���ر��
���ֵĹ����Ҫ�����������������Ӱ����Ҫһֱ��ʾ��

Decal��λ�úʹ�С������ʱ������λ�úʹ�С������ȾDecal�Ĺ����У����Բ��ϵĵ���
Decal��λ�úʹ�С(������Ⱦ����ļ���Ӱ,��������С�Ľӿڲ���Ҫ���ϲ㿪��)��


DecalӦ����һ�����������������������FrameMove��draw

1: �����decal��x,z�����ϵ�boundSquare��С��
2: ���Ӱ��decal�ĸ��ӡ�
3: ����decalӰ��ĸ��ӣ����г���decal���������Щ������ɵ������Ρ�
4: ����ÿһ��decal�����Ӧ��uv����(Decal��uv���궼�Ǵ�0��1)��


ʹ��ͬһ��Shader��Decalʹ��ͬһ��DIP������

�ر��CursorҲʹ��Decal��ͳһ�����ѵ���ر���ֹ��Ĵ���ȥ����

���е�decalʹ�ö�̬���Ķ��㻺����??����������Ҳʹ�ö�̬������.


*/


//! ����ʹ��
typedef std::vector<os_billBoardVertex>   VEC_decalVer;

class   osc_decalMgr;

//! һ��decal���Ӱ���tile��Ŀ��
# define MAX_TILE_PERDECAL  256

/** \brief
 *  �ر��Ͽ�����ʾ��decal���ݽṹ.
 *
 *  ����Decal�ڵĶ��㶼������ռ�Ķ��㡣
 */
class osc_decal : public os_createVal
{
	friend class   osc_decalMgr;
private:
	//! ����������м�ܵ���ָ��.
	static osc_TGManager*   m_ptrTGMgr;
	static osc_middlePipe*  m_ptrMPipe;

	//! ����decalʱ�����decal����tile���ӵ��ĸ��档
	static osPlane         m_arrDecalPlane[4];


	//! Decal�õ��Ķ������Ⱦ����
	VEC_decalVer       m_vec3DecalVer;
	VEC_word           m_vecDecalIdx;
	int               m_iDecalIdxnum;

	//! ��ǰdecal��textureId.
	DWORD             m_dwDecalColor;

	bool              m_bAniDecal;
	//! �ӿ�ʼ��ʾdecal��Ŀǰ��ȥ��ʱ�䡣
	float             m_fEtime;
	//! decal��ʧ��ʱ��
	float             m_fVanishTime;

	//! �Ƿ�����ת��Decal
	float             m_fRotAgl;

	//! Decal��λ�úʹ�С.λ��λ��decal�����ĵ㡣��С�ǰ뾶����decal��ȵ�һ�롣
	osVec3D           m_vec3DecalPos;
	float             m_fDecalSize;

	//! ��ת���u��v source Plane,ÿһ�����㵽����ľ��룬����decalSize��Ϊu.
	static osPlane     m_sUSourcePlane;      
	static osPlane     m_sVSourcePlane;

	//! ��Ⱦ��ǰ��Decalʹ�õ�������
	os_polygon         m_sRenderData;

	//! ��ǰDecal��offsetֵ
	float              m_fDecalOffset;

	//! ��ǰDecal����������ص�����
	WORD               m_wXFrameNum;
	WORD               m_wYFrameNum;
	float              m_fFrameTime;

	//! ��ǰ����ʱ��̣���Decal�������ڵ�����
	WORD               m_iCurXFrame;
	WORD               m_iCurYFrame;
	float              m_fAcctFrameTime;

	//! ��ǰ��decal�Ƿ�����
	bool               m_bHide;

	//! ��ǰdecal���������ۺϷ�ʽ.
	int                m_iAlphaType;

	//! �Ƿ�fakeDecal.
	bool               m_bFake;

	//! ��ǰ��decal�Ƿ���������Ⱦ�����С�
	bool               m_bInRender;

private:
	//! ����õ�decal�õ����ĸ�������
	void             get_decalClipPlane( osPlane* _planeArray = NULL );

	/** \brief
	 *  Clipһ��Tile���������Σ������µ�Decalʹ�õ��������б�
	 *
	 *  \param osVec3D* _pos ������������������㣬˳ʱ�����С�
	 */
	void             clip_tileTriangle( const osVec3D* _pos );

	//! TriangleFan���������TriangleStrip��������������Ŀ�ı仯��
	int              triFanNum2TriStripNum( int _tfIdxNum );

	//! �����зֶ����ÿһ��decal�����uv����
	void             cal_decalVerUv( void );
	//! �������������Ķ���uv����
	void             cal_decalVerUvTexAnim( void );


	//! ���ڴ洢decalӰ����ӵ���ʱ������,�洢��tile������ռ�����꣬�ᳬ��64
	static os_tileIdx    m_arrTileIdx[MAX_TILE_PERDECAL];
	static int          m_iAffectNum;


	//! ����һ�����ĵ��һ��Size,�õ���Ӱ���Tg��������
	int              get_affectTile( osVec3D& _pos,float _size );
	int              get_affectTileWithRot( osVec3D& _pos,float _size,float _rotAgl );

	//! ���´�decal����������Ϣ
	void             frame_texAnim( float _etime );

	//! �����Ǹ�����ε���ͨdecal.
	void             move_fakeDecal( osVec3D& _pos,float _rot,float _offset = 0.0f );

public:
	osc_decal();
	~osc_decal();

	void       reset_decal( void );


	//! ��ʼ��һ��decal,����Decal�Ĵ���ֵ��ɾ��Decalʱ������ȷ��ɾ����ֵ�ʹ���ֵ��ͬ��
	WORD       init_decal( os_decalInit& _dinit,int _shaderId = -1 );

	//! ����decal����ת��λ����Ϣ
	void       move_decal( osVec3D& _pos,float _rotAgl );
	//! ����Decal��λ����Ϣ
	void       move_decal( osVec3D& _pos ) { move_decal( _pos,m_fRotAgl ); } 

	//! ��Decal����FrameMove.����decal��animation��Ϣ
	bool       frame_move( float _etime );

	//! ���õ�ǰ��decal�Ƿ�ɼ�
	void       set_hide( bool _hide ){ m_bHide = _hide; }

	//! ��ǰ��decal�Ƿ�����Ⱦ��.
	bool       is_inRender( void ) { return m_bInRender; } 

};


//! ����������scene cursorNum.
# define MAX_SCENECURSORNUM   30
/** \brief
 *  decal�Ĵ�����������decal��صĹ��ܣ���������������ڴ�����
 *
 */
class osc_decalMgr
{
private:

	//! ��䶥���õ��Ķ��㻺����id.

	//! ��������õ�������������id,����ʱ��ʹ��TriangleStrip,
	//! ʹ��Degenerate Triangle,�ѷ�ɢ�Ķ������������

	CSpaceMgr<osc_decal>      m_arrDecalIns;

	//! ��������Ĺ���õ���Decal
	osc_decal                 m_sSceneCursor[MAX_SCENECURSORNUM];

	// ��ȾDecal��meshSet.
	static os_polygonSet      m_sPolySet;

	//! fake shadow��shaderID.
	static int                m_iFakeShadowShaId;

private:
	//! ��������decalMgr����FrameMove

	//! ��Ҫ��Ⱦ��Decal�ṹָ���������.


public:
	osc_decalMgr();
	~osc_decalMgr();

	//! �ѵ�ǰ��decalMgr��tgMgr�����ϵ����.
	static void  OneTimeInit( osc_TGManager* _tgMgr,osc_middlePipe* _pipe );

	//! ΪsceneCursor����һ�������Decal,Ϊ���ǿ����ڳ�������ȷ����ʾ���
	void       create_sceneCursor( os_decalInit& _dinit );

	/** \brief
	 *  ����һ��decal
	 *
	 */
	int         create_decal( os_decalInit& _dinit );

	//! River @ 2011-2-11:����fakeShadow�������룬��һ���ݾ����̣�ʹ������ֵĸ���.
	int         create_fakeShadow( os_decalInit& _dinit );

	//! ɾ��һ��decal.
	void        delete_decal( int _id );

	//! ����һ��decal.
	void        hide_decal( int _id,bool _hide );


	//! ����һ��decal��λ��
	void        reset_decalPos( int _id,osVec3D& _pos );


	//! ��Ⱦ��ǰ��decalMgr.
	void        render_decalmgr( os_FightSceneRType _rtype );

	//! ��Ⱦ�����ڵĹ��.
	void        render_sceneCursor( void );

	//! Release All the Decal.
	void        release_allDecal( void );


};
