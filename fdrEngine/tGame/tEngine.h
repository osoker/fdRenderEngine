//=======================================================================================================
/**  \file
 *   Filename: tengine.h
 *
 *   His:      River created @ 2003-12-23
 *
 *   Des:      用于测试渲染引擎，调入新的地图格式，把人物和物品加入到引擎中去。
 *
 */
//=======================================================================================================
# pragma once

# include "stdafx.h"

# include "network/endecode.h"
# include "network/protocol.h"


# define  MUSIC_TEST  1


//! 全局的设备管理类。
extern I_deviceManager*   g_3dMgr;
//! 全局的输入管理类.
extern I_inputMgr*       g_inputMgr;
//! 全局的资源管理类.
extern I_resource*       g_resourceMgr;

//! 全局的相机类.
extern I_camera*         g_camera;

//! 全局的场景类.
extern I_fdScene*        g_sceneMgr;

//! 全局的socketMgr.
extern I_socketMgr*      g_ptrSocket;

# if MUSIC_TEST
extern I_soundManager*   g_ptrSMgr;
# endif 



extern os_deviceinit     g_devinit;
extern int              g_iTexid;




//
extern int              g_iShadowId;

extern void               rot_osa( void );
//! Console的输入状态.
extern bool                g_bConsoleIO;

//! 当前帧是否截图.
extern bool                g_bCatchScr;

//! 使用人物box
extern bool                g_bChrBox;


//! 是否处下调整地图高度信息的模式。
extern bool                g_bCHMod;


//! 远剪切面的矩离。
extern float               g_fTerrFar;

//@{
//! 是否全屏.
extern bool                g_bFullScr;
extern int                 g_iScrWidth;
extern int                 g_iScrHeight;
//@}

void                       mouse_rotCam( WPARAM _wParam,int _nX,int _nY );


//  Read the device init struct from the ini file.
bool                read_ini( os_deviceinit& _devinit );
void                t_renderStr( void );
void                t_render( void );
void                t_initRender( HINSTANCE _hInst,HWND _hwnd );
void                t_initScene( char* _filename );
void                t_releaseRes( void );
void                t_toggle( void );
void                t_processInput( void );

extern void               hero_areaMove( void );

/** \brief
 *  拉近相机与地面的距离.
 */
void                t_moveClose( float _length );

void                on_mouseWheel( WPARAM _wParam,int _nX,int _nY );

void                move_pitchTest( void );

void                light_test( void );
void                delete_light( void );


void                t_camera( void );

//! 测试人物的刀光设置
void                test_herog( void );

//! 测试人物武器附加的特效。
void                test_wEff( void );

//! 测试输入法的速度
void                t_addStr( void);

/** \brief
 *  旋转相机.
 */
extern bool          rotate_cam( float _rad );
/** \brief
 *  Picth相机.
 */
extern void          picth_cam( float _rad );

//! 相机所在的位置是否合法.
extern bool          camera_posLegal( void );

//! 调整相机.
extern bool          readj_cam( float _rad );


/** \brief
 *  得到当前选择的地图块的函数.
 *  输入值是屏幕的二维坐标.
 */
void                t_getTGidx( int _x,int _y );



//! 测试使用动作名调用动作。
void                play_actWithName( char* _name );

//! 测试人物的一次性播放动作。
void                t_action( void );

//! 调入装备文件
void                load_equFile( void );

//! 测试画人物手部的box.
void                draw_handBox( void );


//! 测试调入的人物或是怪物。
I_skinMeshObj*                load_tNpc( osVec3D& _vec3Pos = osVec3D( 0,0,0 ) );

//! 测试细节格子.
void                test_detailTile( void );

//! 是否点下右键。
extern bool          g_bRbtnDown;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//  以下为移动相关的函数
//
//
//! 每0.2秒一步，即0.75米/0.2秒.
extern  float       MOVE_SPEED;


//! 当前是否处理移动状态。
extern bool                g_bMoveStatus;

//! 测试用全局的npc.
extern I_skinMeshObj*       g_ptrNpcSkin;

extern void delete_sword( void );


extern int          g_iCursorXPos;
extern int          g_iCursorYPos;

//! 是否显示帧速度等信息。
extern bool         g_bDisInfo;


/** \brief
 *  使用当前帧过去的时间计算从源点到目标点可以移动的距离。
 */
void               t_moveInterpolate( float _etime,osVec3D* _moveVec,osVec3D* _resVec );

//! 人物要移到的目标和开始顶点。
extern osVec3D      g_vec3Start;
extern osVec3D      g_vec3End;


//! 每一帧去执行的函数，如果当前处于可移动状态的话，处理人物的移动。
void               t_moveHero( void );
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////


void               rot_character( float _tf );


//@{
//! 特效测试相关的函数.
void               play_effect1( void );
void               play_effect2( void );
void               play_effect3( void );
void               play_effect4( void );
//@} 

void change_action( void );


//! 测试天气变化.
void               change_weather( void );

//! test color.
void               test_color( void );



// 当前不使用DInput做为输入法，
# define USE_DINPUT 0


class  I_Hero;
extern I_Hero	g_cHero	;

extern HINSTANCE hInst;

class CCameraCtrl;
extern CCameraCtrl  g_cCamera;

class  Pathmanage;
extern	Pathmanage  g_PathManage;

class CNpcManager;
extern  CNpcManager  g_cNpcManager;

class  CEquipmentManager;
extern  CEquipmentManager g_EquipManager;

extern void test();


//! 主角人物插入地图的位置。　
extern int         g_iInsertX;
extern int         g_iInsertZ;


//! 下雨测试。
extern void   rain_test( void );


//! 人物的shun移效果。
extern void   character_move( void );

//! 标题栏文字。
extern char   g_titleStr[128];

//! 场景退出前的处理。
extern void   process_beforeExit( void );

//! 右键点击鼠标时播放的动作的名称。
extern char   g_strRBtnName[10][256];
extern int    g_iRBtnActIdx;

//! 更换人物的装备。
extern void   change_weapon( void );


//! 相机移动状态。
extern bool   g_bCameraMove;


//! 测试
extern bool       g_bRenderBtn;


//! 测试阴影相关。
extern void   test_shadow( void );

//! 每一个npc人物对应的结构.
struct npc_struct
{
	char       m_szDir[64];
	int        m_iInsertX;
	int        m_iInsertZ;

	osVec3D    m_vec3Offset;

	float      m_fRotAgl;

	char       m_szActName[3][32];

	//! 当前人物对应相机的名字。
	char       m_szCamName[32];


	I_skinMeshObj*  m_ptrSm;

	npc_struct()
	{
		m_ptrSm = NULL;
		m_vec3Offset = osVec3D( 0.0f,0.0f,0.0f );
	}
};


//! 从ini文件中读入相应的装备切换数据。
extern void  equip_change( void );


//! 用于调整场景中使用的Npc人物的类.
class npc_adjust
{
private:
	npc_struct  m_sNpcStruct[6];

	//! 当前可以调整的npc人物的索引.
	int        m_iCurActiveNpc;


public:
	npc_adjust();

	//! 从ini文件中调入我们的npc
	bool       load_npcFromIni( char* _iniName );

	//! 用于调整当前的npc人物.

	//! 使下一个人物变为可调的人物.
	void        active_nextNpc( void );



	//! 旋转当前的npc.
	void        rotate_curNpc( float _radian );

	//! 在y方向上移动npc.
	void        move_npcY( float _dis );

	//! 在X方向上移动npc.
	void        move_npcX( float _dis );

	// ! 在Z方向上移动npc.

	void        move_npcZ( float _dis );

	//! 写入npc人物的ini文件. 
	void       save_npcChrToFile( char* _iniName );

	//! 测试人物的第二三套动作.
	void       play_selectAction( void );

	//! 重设人物的动作。
	void       reset_npcAction( void );

	//! 设置当前人物相关联的相机。
	void       set_camera( bool _ani );


};

//! 全局的人物调整类.
extern npc_adjust   g_npcAdjust;;

extern void  test_bolt1( void );

//! 播放音乐。
extern void  play_music( char* _filename );
extern void  play_sound( char* _sname );
extern void  adj_volume( int _mm, OS_SOUNDTYPE type = SOUNDTYPE_UI );




//! 测试地图的瞬移接口。 
extern void move_to_fmna( void );


extern void test_actionSuspend( void );
extern void next_actPose( void );
extern void bef_actPose( void );

extern void spacemove_test( void );


# if 0
//! 用于在两点间移动的类。
class gc_PtMove
{
private:
	//! 开始点。
	osVec3D    m_vec3SPt;
	//! 结束点。
	osVec3D    m_vec3EPt;

	//! 从结束点到开始点的向量。
	osVec3D    m_vec3Dir;

	//! 移动速度。
	float     m_fMoveSpeed;

	//! 从结束点到开始点的矩离。
	float      m_fTotalDis;

	//! 当前的已经过去的矩离。
	float      m_fMoveDis;

public:
	gc_PtMove();


	//! 使用一个开始点和一个结束点初始化当前的类。
	void      init_curPtMove( osVec3D& _s,osVec3D& _e,float _moveSpeed );

	/** \brief
	 *  传入一个帧间流逝时间，得到当前应该到达的位置
	 *
	 *  \return bool 如果到达顶点，返回真，否则返回假。
	 */
	bool      get_curMovePt( float _etime,osVec3D& _pos );


};
# endif 


extern VEC_string   g_equFile;
extern s_string     g_szChrName;



// TEST CODE:
void   test_chunk( void );

struct sc_BBox
{
	D3DXVECTOR3     vecmax;
	D3DXVECTOR3     vecmin;

	//! 当前bounding box在世界空间中的位置。
	D3DXVECTOR3     m_vec3BoxPos;

	//! 当前Bounding Box的旋转角度。
	float           m_fBoxRot;
};

extern void   test_snapBbox( void );
