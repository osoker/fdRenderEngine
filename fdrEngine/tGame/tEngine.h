//=======================================================================================================
/**  \file
 *   Filename: tengine.h
 *
 *   His:      River created @ 2003-12-23
 *
 *   Des:      ���ڲ�����Ⱦ���棬�����µĵ�ͼ��ʽ�����������Ʒ���뵽������ȥ��
 *
 */
//=======================================================================================================
# pragma once

# include "stdafx.h"

# include "network/endecode.h"
# include "network/protocol.h"


# define  MUSIC_TEST  1


//! ȫ�ֵ��豸�����ࡣ
extern I_deviceManager*   g_3dMgr;
//! ȫ�ֵ����������.
extern I_inputMgr*       g_inputMgr;
//! ȫ�ֵ���Դ������.
extern I_resource*       g_resourceMgr;

//! ȫ�ֵ������.
extern I_camera*         g_camera;

//! ȫ�ֵĳ�����.
extern I_fdScene*        g_sceneMgr;

//! ȫ�ֵ�socketMgr.
extern I_socketMgr*      g_ptrSocket;

# if MUSIC_TEST
extern I_soundManager*   g_ptrSMgr;
# endif 



extern os_deviceinit     g_devinit;
extern int              g_iTexid;




//
extern int              g_iShadowId;

extern void               rot_osa( void );
//! Console������״̬.
extern bool                g_bConsoleIO;

//! ��ǰ֡�Ƿ��ͼ.
extern bool                g_bCatchScr;

//! ʹ������box
extern bool                g_bChrBox;


//! �Ƿ��µ�����ͼ�߶���Ϣ��ģʽ��
extern bool                g_bCHMod;


//! Զ������ľ��롣
extern float               g_fTerrFar;

//@{
//! �Ƿ�ȫ��.
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
 *  ������������ľ���.
 */
void                t_moveClose( float _length );

void                on_mouseWheel( WPARAM _wParam,int _nX,int _nY );

void                move_pitchTest( void );

void                light_test( void );
void                delete_light( void );


void                t_camera( void );

//! ��������ĵ�������
void                test_herog( void );

//! ���������������ӵ���Ч��
void                test_wEff( void );

//! �������뷨���ٶ�
void                t_addStr( void);

/** \brief
 *  ��ת���.
 */
extern bool          rotate_cam( float _rad );
/** \brief
 *  Picth���.
 */
extern void          picth_cam( float _rad );

//! ������ڵ�λ���Ƿ�Ϸ�.
extern bool          camera_posLegal( void );

//! �������.
extern bool          readj_cam( float _rad );


/** \brief
 *  �õ���ǰѡ��ĵ�ͼ��ĺ���.
 *  ����ֵ����Ļ�Ķ�ά����.
 */
void                t_getTGidx( int _x,int _y );



//! ����ʹ�ö��������ö�����
void                play_actWithName( char* _name );

//! ���������һ���Բ��Ŷ�����
void                t_action( void );

//! ����װ���ļ�
void                load_equFile( void );

//! ���Ի������ֲ���box.
void                draw_handBox( void );


//! ���Ե����������ǹ��
I_skinMeshObj*                load_tNpc( osVec3D& _vec3Pos = osVec3D( 0,0,0 ) );

//! ����ϸ�ڸ���.
void                test_detailTile( void );

//! �Ƿ�����Ҽ���
extern bool          g_bRbtnDown;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//  ����Ϊ�ƶ���صĺ���
//
//
//! ÿ0.2��һ������0.75��/0.2��.
extern  float       MOVE_SPEED;


//! ��ǰ�Ƿ����ƶ�״̬��
extern bool                g_bMoveStatus;

//! ������ȫ�ֵ�npc.
extern I_skinMeshObj*       g_ptrNpcSkin;

extern void delete_sword( void );


extern int          g_iCursorXPos;
extern int          g_iCursorYPos;

//! �Ƿ���ʾ֡�ٶȵ���Ϣ��
extern bool         g_bDisInfo;


/** \brief
 *  ʹ�õ�ǰ֡��ȥ��ʱ������Դ�㵽Ŀ�������ƶ��ľ��롣
 */
void               t_moveInterpolate( float _etime,osVec3D* _moveVec,osVec3D* _resVec );

//! ����Ҫ�Ƶ���Ŀ��Ϳ�ʼ���㡣
extern osVec3D      g_vec3Start;
extern osVec3D      g_vec3End;


//! ÿһ֡ȥִ�еĺ����������ǰ���ڿ��ƶ�״̬�Ļ�������������ƶ���
void               t_moveHero( void );
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////


void               rot_character( float _tf );


//@{
//! ��Ч������صĺ���.
void               play_effect1( void );
void               play_effect2( void );
void               play_effect3( void );
void               play_effect4( void );
//@} 

void change_action( void );


//! ���������仯.
void               change_weather( void );

//! test color.
void               test_color( void );



// ��ǰ��ʹ��DInput��Ϊ���뷨��
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


//! ������������ͼ��λ�á���
extern int         g_iInsertX;
extern int         g_iInsertZ;


//! ������ԡ�
extern void   rain_test( void );


//! �����shun��Ч����
extern void   character_move( void );

//! ���������֡�
extern char   g_titleStr[128];

//! �����˳�ǰ�Ĵ���
extern void   process_beforeExit( void );

//! �Ҽ�������ʱ���ŵĶ��������ơ�
extern char   g_strRBtnName[10][256];
extern int    g_iRBtnActIdx;

//! ���������װ����
extern void   change_weapon( void );


//! ����ƶ�״̬��
extern bool   g_bCameraMove;


//! ����
extern bool       g_bRenderBtn;


//! ������Ӱ��ء�
extern void   test_shadow( void );

//! ÿһ��npc�����Ӧ�Ľṹ.
struct npc_struct
{
	char       m_szDir[64];
	int        m_iInsertX;
	int        m_iInsertZ;

	osVec3D    m_vec3Offset;

	float      m_fRotAgl;

	char       m_szActName[3][32];

	//! ��ǰ�����Ӧ��������֡�
	char       m_szCamName[32];


	I_skinMeshObj*  m_ptrSm;

	npc_struct()
	{
		m_ptrSm = NULL;
		m_vec3Offset = osVec3D( 0.0f,0.0f,0.0f );
	}
};


//! ��ini�ļ��ж�����Ӧ��װ���л����ݡ�
extern void  equip_change( void );


//! ���ڵ���������ʹ�õ�Npc�������.
class npc_adjust
{
private:
	npc_struct  m_sNpcStruct[6];

	//! ��ǰ���Ե�����npc���������.
	int        m_iCurActiveNpc;


public:
	npc_adjust();

	//! ��ini�ļ��е������ǵ�npc
	bool       load_npcFromIni( char* _iniName );

	//! ���ڵ�����ǰ��npc����.

	//! ʹ��һ�������Ϊ�ɵ�������.
	void        active_nextNpc( void );



	//! ��ת��ǰ��npc.
	void        rotate_curNpc( float _radian );

	//! ��y�������ƶ�npc.
	void        move_npcY( float _dis );

	//! ��X�������ƶ�npc.
	void        move_npcX( float _dis );

	// ! ��Z�������ƶ�npc.

	void        move_npcZ( float _dis );

	//! д��npc�����ini�ļ�. 
	void       save_npcChrToFile( char* _iniName );

	//! ��������ĵڶ����׶���.
	void       play_selectAction( void );

	//! ��������Ķ�����
	void       reset_npcAction( void );

	//! ���õ�ǰ����������������
	void       set_camera( bool _ani );


};

//! ȫ�ֵ����������.
extern npc_adjust   g_npcAdjust;;

extern void  test_bolt1( void );

//! �������֡�
extern void  play_music( char* _filename );
extern void  play_sound( char* _sname );
extern void  adj_volume( int _mm, OS_SOUNDTYPE type = SOUNDTYPE_UI );




//! ���Ե�ͼ��˲�ƽӿڡ� 
extern void move_to_fmna( void );


extern void test_actionSuspend( void );
extern void next_actPose( void );
extern void bef_actPose( void );

extern void spacemove_test( void );


# if 0
//! ������������ƶ����ࡣ
class gc_PtMove
{
private:
	//! ��ʼ�㡣
	osVec3D    m_vec3SPt;
	//! �����㡣
	osVec3D    m_vec3EPt;

	//! �ӽ����㵽��ʼ���������
	osVec3D    m_vec3Dir;

	//! �ƶ��ٶȡ�
	float     m_fMoveSpeed;

	//! �ӽ����㵽��ʼ��ľ��롣
	float      m_fTotalDis;

	//! ��ǰ���Ѿ���ȥ�ľ��롣
	float      m_fMoveDis;

public:
	gc_PtMove();


	//! ʹ��һ����ʼ���һ���������ʼ����ǰ���ࡣ
	void      init_curPtMove( osVec3D& _s,osVec3D& _e,float _moveSpeed );

	/** \brief
	 *  ����һ��֡������ʱ�䣬�õ���ǰӦ�õ����λ��
	 *
	 *  \return bool ������ﶥ�㣬�����棬���򷵻ؼ١�
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

	//! ��ǰbounding box������ռ��е�λ�á�
	D3DXVECTOR3     m_vec3BoxPos;

	//! ��ǰBounding Box����ת�Ƕȡ�
	float           m_fBoxRot;
};

extern void   test_snapBbox( void );
