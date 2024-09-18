//---------------------------------------------------------------------------------
/**  \file
 *   Filename: getILib.h
 *   Desc:     �����н����Ľӿں�����
 *   His:      syq @ 2004-11-15.
 *
 *             ��������룬����Ϊ����������
 */
//---------------------------------------------------------------------------------
# pragma once

#include "..\interface\osInterface.h"
#include "..\common\com_include.h"

#include <queue>
#include <vector>

#pragma pack(push,4) 
#pragma warning( disable: 4311 )
#pragma warning( disable: 4312 )
#pragma warning( disable: 4239 )
#pragma warning( disable: 4244 )
#pragma warning( disable: 4715 )
#pragma warning( disable: 4702 )
#pragma warning( disable: 4018 )
#pragma warning( disable: 4389 )
#pragma warning( disable: 4267 ) 
#pragma warning( disable: 4706 ) 

#define MSGIO_FLAG 0x80000000     //�ⲿ��Ϣ����ʼID
#define MSG_FLAG   0x40000000     //�����������Դ������Ϣ

#define IS_IO_MSG(id)  ((id&MSGIO_FLAG) ? true : false)
#define IS_OI_MSG(id)  ((id&MSGO_FLAG) ? false:true)
#define IS_MSG(id)     ((id&MSG_FLAG) ? true:false)

//ԭʼmsg id
typedef enum MSG_ID
{
	//��->����Ϣ-----------------------------------------------
	MSGOI_LBUTTONDOWN = 0,
	MSGOI_LBUTTONUP,
	MSGOI_LBUTTONDBLCLK,
	MSGOI_RBUTTONDOWN,
	MSGOI_RBUTTONUP,
	MSGOI_RBUTTONDBLCLK,
	MSGOI_MOUSEMOVE,
	MSGOI_MOUSEWHEEL,
	MSGOI_KEYDOWN,
	MSGOI_CHAR,
	MSGOI_KEYUP,
	MSGOI_INPUTLANGCHANGEREQUEST,
	MSGOI_IME_STARTCOMPOSITION,
	MSGOI_IME_ENDCOMPOSITION,
	MSGOI_INPUTLANGCHANGE,
	MSGOI_IME_COMPOSITION,
    MSGOI_IME_NOTIFY,
	MSGOI_ACTIVATE,

	//��->����Ϣ-----------------------------------------------

	//{@ ��ʱû��
	MSGIO_MESSAGEBOX_OK = MSGIO_FLAG,  //�����Ի�����ȷ�������,  wParamΪ�Ի���id,lParamΪ��ťid
	MSGIO_MESSAGEBOX_CANCEL,           //�����Ի�����cancel�����,wParamΪ�Ի���id,lParamΪ��ťid
	MSGIO_MESSAGEBOX_APPLY,            //�����Ի�����apply�����, wParamΪ�Ի���id,lParamΪ��ťid
	//}@

	MSGIO_DIALOG_CLOSE,                //�Ի���رհ�ť�����,wParam:�Ի���id
	MSGIO_BUTTON_EVENT,                //��ť�����up, wParam: parent id, lParam: myself id.
	MSGIO_CHECKBUTTON_LBUTTONDOWN,     //��ѡ��lbutton down  wParam: this,   lParam: pid, id;
	MSGIO_MOUSEMOVE_INBUTTON,          //mouse�ڰ�ť���ƶ�
	MSGIO_MOUSEMOVE_INCHECKBOX,        //mouse��checkbox���ƶ�
	MSGIO_MOUSEMOVE_NOTIP,             //�ر���ʾ��
	MSGIO_EDIT_RETURN,                 //�༭�򱻻س�ȷ��
	MSGIO_NOTEBOOK_RETURN,             //���±��ؼ����س�ȷ��
	MSGIO_ICONLIST_MOUSEMOVE,          //ICONLIST MOUSE MOVE
	MSGIO_ICONLIST_LBDOWN,             //ͼ���б���
	MSGIO_ICONLIST_LDBCLICK,           //ͼ���б�˫��
	MSGIO_COLORLIST_LBDOWN,            //��ɫѡ���б���
	MSGIO_COLORLIST_MOUSEMOVE,         //��ɫѡ���б� MOUSE MOVE
	MSGIO_STRINGLIST_LBDOWN,           //�ַ��б�����
	MSGIO_STRINGLIST_LDBCLICK,         //�ַ��б�˫��
	MSGIO_STRINGLIST_MOUSEMOVE,        //�ַ��б�mouse move
	MSGIO_ICONSTRINGLIST_LBDOWN,       //ͼ���ַ��б�����
	MSGIO_ICONSTRINGLIST_LDBCLICK,     //ͼ���ַ��б�˫��
    MSGIO_ICONSTRINGLIST_RBDOWN,       //ͼ���ַ��б��һ�
	
	//ͼ���϶��¼�, wParam: HIWORD: fromListID LOWORD: fromListIconIdx
	//              lParam: HIWORD: toListID   LOWORD: toListFrameID
	MSGIO_ICONMOVE,                    //�Ƶ�ͼ���б���
	MSGIO_ICONMOVETO_DESKTOP,          //ͼ���Ƶ�������
	MSGIO_TEXTBAR_LBDOWN,              //���textbar����Ϣ, wParamΪ�ز�id, lParamΪ��id
	MSGIO_BEFOREDIALOGRESIZE_GET_MAXDIALOGSIZE, //�ڸı�Ի����С֮ǰ���û��������ĸı�ߴ�
	MSGIO_DIALOG_RESIZE,               //�Ի����С�ı���Ϣ
	MSGIO_DIALOGTITLE_DBCLICK,              //�ڶԻ����������˫��(�ޱ���Ի����������Ǳ���)
	MSGIO_MOUSE_CHANGE,                //mouse��۱仯, wParam: UI_MOUSE_TYPE����
	MSGIO_DIALOG_RETURN_KEYDOWN,       //�Ի���ļ��̻س������¼�,
	MSGIO_ICONEFFECT_PLAYEND,          //ͼ����Ч������� wParam: ͼ���б��ָ��, lParam:ͼ�������
	MSGIO_MAPICON_MOUSEMOVE,           //mouse��С��ͼ��������ʾ��ͼ���� wParam:fdc_mapIconָ��
	MSGIO_MAPICON_LBUTTON,             //mouse���С��ͼ��������ʾ��ͼ��
	MSGIO_MAPICON_DBLCLK,              //˫��С��ͼͼ��
	MSGIO_MAP_DBCLICK,                 //��С��ͼ��˫��, wParam:
	MSGIO_MAP_MOUSEMOVE,               //��С��ͼ���ƶ�,
	MSGIO_MAP_RDBCLICK,                //��С��ͼ����˫��
	MSGIO_MAP_LRCLICK,                 //mouse��С��ͼ�����ҽ�ͬʱ����
	MSGIO_LAYERGROUPITEM_MOUSEMOVE,    //mouse��������Ŀ���ƶ�
	MSGIO_LAYERGROUPITEM_LBDOWN,       //mouse��������Ŀ�ϵ���
	MSGIO_LAYERGROUPITEM_LDBCLICK,     //mouse��������Ŀ��˫��
	MSGIO_LAYERGROUPITEM_LBUP,         //mouse��������Ŀ�ϵ���̧��
	MSGIO_LAYERGROUPITEM_RBDOWN,       //mouse��������Ŀ���Ұ���
	MSGIO_ACTIVE_CHAREDIT,             //��������ļ�����¼�
	MSGIO_USER_CLOSE_WND,              //�û����Թرմ��壬����ϵͳ��Ϣ���¼�
	MSGIO_HOTLINK_LBDOWN,              //������ı��ؼ�

	//�ڲ����ѵ���Ϣ-----------------------------------------------
	MSG_SCROLL_UPBTN_LUP = MSG_FLAG,   //�������ϰ�ţ̌��
	MSG_SCROLL_UPBTN_LDOWN,            //�������ϰ�ť����
	MSG_SCROLL_DOWNBTN_LUP,            //�������°�ţ̌��
	MSG_SCROLL_DOWNBTN_LDOWN,          //�������°�ť����
	MSG_SCROLL_BLOCK_LDOWN,            //�������п鱻����
	MSG_SCROLL_UPDATEPOS,              //�������򸸴��巢�Ͷ�λ������Ϣ
	MSG_TAB,                           //TAB��Ϣ���򸸴��巢��
	MSG_RETURN_KEYDOWN,                //�ڲ�����س��¼�
	MSG_PUCKER,                        //�Ի����۵�
	MSG_UPDATE_DRAG,                   //�Ի�����϶������
	MSG_DIALOG_RESIZE,                 //�Ի����С�ı���Ϣ,�ڲ�����
	MSG_COMBOBOX_DROPBTN_LDOWN,        //����ѡ���İ����¼�
	MSG_COMBOBOX_STRINGLIST_ITEM_SELECT, //����ѡ�����ַ��б���Ŀ������ѡ��
	MSG_MAPICON_ACTIVE,                //С��ͼ��ͼ�꼤����Ϣ
	MSG_LAYERBUTTON_LUP,               //��ΰ�ť����ͷ� wParam:this lParam: idx
	MSG_ICONLIST_UPDATEMOUSEMOVE,      //ͼ���б��Ϸź����tooltip
	MSG_LIMIT_DIALOGSIZE_FROM_CHILD,   //ĳ�ؼ�Ҫ�������丸(�Ի���)�Ĵ�С
	MSG_HOTLINK_BROTHER_SYNC_WNDSTATE, //�������ֵܿؼ���Ϣͬ��(����״̬)
	MSG_HOTLINK_BROTHER_SYNC_VISITED,  //�������ֵܿؼ���Ϣͬ��(�Ƿ����״̬)



	
	MSG_END                            //��β��Ϣ��
} MSG_ID;

//�ڲ���Ϣ�ṹ
typedef struct UIMSG 
{
	MSG_ID	 msg_id;
    UINT     wParam;
    UINT     lParam;
    DWORD    time;
    POINT    pt;

	void make_msg( MSG_ID _msg_id, UINT _wParam, UINT _lParam, POINT _pt )
	{
		msg_id = _msg_id;
		wParam = _wParam;
		lParam = _lParam;
		pt = _pt;
	}

	void make_msg( MSG_ID _msg_id, UINT _wParam, UINT _lParam, int _pt_x = 0, int _pt_y = 0 )
	{
		msg_id = _msg_id;
		wParam = _wParam;
		lParam = _lParam;
		pt.x = _pt_x;
		pt.y = _pt_y;
	}

} UIMSG;

//! ����������Ϣ�ص�����ԭ��
typedef int ( *UI_MSGCALLBACKIMM)(UIMSG&);

//! ����Ĭ�ϱ����ļ���
#define UISAVENAME "user\\default.uis"

//! ÿһ���ؼ�����ʹ�õ����ֵ���󳤶ȡ�
#define MAX_CRLNAMELEN      64

//! �����봰��߿��Ĭ�Ͼ���(pixel)
#define TEXT_SPACE   3   

typedef enum GAMESTEP
{
	GS_LOGIN,			//! ��¼ʱ
	GS_SELS,			//! ѡ�������
	GS_CREC,			//! ��������
	GS_SELC,            //! ѡ������
	GS_LOADING,         //
	GS_RUN,				//! ����
	GS_PUBLIC,          //! ����

} GAMESTEP;


//! �ı��Ķ��뷽ʽ
enum FDCTEXT_DISFORMAT
{
	TEXT_LEFT = 0,  //��
	TEXT_CENTER,    //��
	TEXT_RIGHT,     //��
	TEXT_TOPLEFT,   //����
	TEXT_TOPCENTER, //����
	TEXT_TOPRIGHT,  //����

	TEXT_USERPOS, //�û�ָ�������ô�����Ҫָ���ı���λ��
};

//! ������пɸı��mouse��״
enum UI_MOUSE_TYPE
{
	UMT_DEFAULT,    //Ĭ�ϵ�
	UMT_HORIZONTAL, //ˮƽ��
	UMT_VERTICAL,   //��ֱ��
	UMT_CROSS,      //ʮ�ֵ�
	UMT_SIZENWSE,   //���������Ϸ���ĸı��С
	UMT_BUTTONHAND, //����button�ϱ�Ϊ����
	UMT_MAP_MOVE,   //�ڵ�ͼ���ƶ��������״
};

//! ��������������ؼ������͡�
enum FDCCTL_TYPE 
{ 
	CTL_ROOT = 0,          //! ������,һ��������ֻ����һ��
	CTL_WND,               //! ��ͨ���塡
	CTL_STATIC,            //! ��̬�ؼ�
	CTL_BUTTON,            //! ��ť
	CTL_PUCKERBUTTON,      //! �۵���ť
	CTL_CHECKBOX,          //! ��ѡ��
	CTL_EDITBOX,           //! �༭�ؼ�
	CTL_NOTEBOOK,          //! ���±��ؼ�(���б༭��)
	CTL_DIALOG,            //! �Ի���
	CTL_PIC,               //! ͼƬ�ؼ�
	CTL_CONTAINER,         //! ͼ�������ؼ�(�ڲ�ʹ��)
	CTL_ANIMPIC,           //! ����ͼƬ�ؼ�
	CTL_SCROLLBAR,         //! ������
	CTL_PROGRESS,          //! ������
	CTL_CURSOR,            //! ���
	CTL_ICON_LIST,         //! ͼ���б�
	CTL_STRING_LIST,       //! �ַ����б�
	CTL_COLOR_LIST,        //! ��ɫѡ���б�
	CTL_ICONSTRING_LIST,   //! ͼ���ַ����б�
	CTL_TOOLTIP,           //! ������ʾ
	CTL_MAP,               //! ��ͼ�ؼ�
	CTL_MAPICON,           //! ��ͼ��ͼ��ؼ�
	CTL_TEXTBAR,           //! �ı�������
	CTL_TEXTBAR_V,         //! �ı�������_��
	CTL_RESIZEBAR,         //! �Ի�����ק���ؼ�
	CTL_COMBOBOX,          //! ����ѡ���
	CTL_QQ,                //! QQ�ؼ�
	CTL_LAYER,             //! ��ΰ�ť
 	CTL_LAYERGROUP,        //! ��ΰ�ť��
	CTL_LAYERGROUPITEM,    //! ��ΰ�ť����Ŀ
	CTL_HOTLINK,           //! ������	

	CTL_SCREENEFFECT_FOLDER,                //! ��Ļ��Ч�ļ��й����ã�����һ�㲻��ʾ
	CTL_SCREENEFFECT_COMBO_ELEMENT_FOLDER,  //! ��Ļ��Ч��2���ļ���(������Ч�ļ���)
	CTL_SCREENEFFECT_ELEMENT,               //! ��Ļ��ЧԪ��

	CTL_ICONEFFECT_FOLDER,                  //! ͼ����Ч�ļ���,������ͼ���ϣ�������ʹ��
	CTL_ICONEFFECT_PATCH,                   //! ͼ����ЧԪ��

	CTL_LIST_ACTIVEWND,                     //! �б�򼤻��

	CTL_STRINGLIST_GROUP,                   //! �ַ��б�����

	//�������Ϳؼ�Ҫͨ���������ɾ��
	CTL_LIBICON,           //! ��ͼ��


	CTL_ERROR              //! ������
};

//! С��ͼ����ʾ��ͼ������
typedef enum MAPICONTYPE
{
	MIT_HERO,       //����
	MIT_DIR,        //����
	MIT_TARGET,     //Ŀ��
	MIT_NPC,        //NPC
	MIT_MONSTER,    //����
	MIT_TEAMMEMBER, //��Ա
	MIT_TEAMLEADER, //�ӳ�
	MIT_GATE,       //�ŵ�
	MIT_USER,       //�û������

} MAPICONTYPE;

//! ���ִ��ڵ�״̬
typedef enum WND_STATE 
{
  WND_DEFAULT = 0,
  WND_OVER =1,
  WND_DOWN =2,
  WND_ACTIVE = 2,
  WND_GRAY =3,
} WND_STATE;

typedef enum VHSTYLE 
{
  V_STYLE,//��
  H_STYLE,//��
} VHSTYLE;

//! �б����ʽ
#define COLSTYLE_NAMELEN 10
#define MAX_COL 30
typedef struct LIST_STYLE{
	int border;                               //! ���޿�
	int border_x;                             //! x����
	int border_y;                             //! y����
	char tex_border[256];                     //! border����
	int scrollbar;                            //! �Ƿ��л�������1:���� 0:û��
	int maxrow;                               //! ������������,�߼���
	int rowcount;                             //! ����
	int colcount;                             //! ����
	int rowheight;                            //! �и�pix
	int colwidth[MAX_COL];                    //! �п�pix
	char colstyle[MAX_COL][COLSTYLE_NAMELEN]; //! ����ʽ,ͬһ��ֻ��һ�ֿؼ�����
	int rowspace;                             //! �пհ�pix
	int colspace;                             //! �пհ�pix
	char tex_style[256];                      //! �ں��ؼ���������
	char tex_active[256];                     //! ����̬������

	osVec2D texuv_active[2];           //! ����̬��������
	osVec2D texuv_border_up[2];        //! �߿�����
	osVec2D texuv_border_bottom[2];    //! �߿�����
	osVec2D texuv_border_left[2];      //! �߿�����
	osVec2D texuv_border_right[2];     //! �߿�����

	osVec2D texuv_style[MAX_COL][2];          
	DWORD bgActiveColor;                      //! mouse over��������ɫ
	DWORD bgClickActiveColor;                 //! mouse click��������ɫ

	int showText;                             //! �Ƿ����������
	DWORD textColor;                          //! �������ɫ
	int textOff_x;                            //! �ı�xƫ��
	int textOff_y;                            //! �ı�yƫ��

	int dataMode;                             //! �洢ģʽ��ͼ���б� 0:��Ʒ�б���ʽ(�޼��),1:�������ʽ(����λ��)
	                                          //            �ַ��б� 0:���ϵ���(����ģʽ)�� 1:���µ���(�����ģʽ)  
	                                          //            ��ɫ�б�:0:��Ʒ�б���ʽ(�޼��),1:�������ʽ(����λ��) 
} LIST_STYLE;

//! ��ɫ�б���ʽ
typedef struct COLORLIST_STYLE
{
	char bindTexure[64];    // �����

	int border;
	int border_x;
	int border_y;

	int rowcount;
	int colcount;
	int rowheight;
	int colwidth;
	int rowspace;
	int colspace;

	int colorCount;
	int colorList[ 20 ];

} COLORLIST_STYLE;

//! ͼ���б�����ݴ洢��ʽ
typedef enum ICONLIST_STYLE
{
	//! �������ʽ,�����м����λ������
	ILS_SHORTCUR,

	//! ��Ʒ����ʽ,onebyone����
	ILS_ONEBYONE,

} ICONLIST_STYLE;

//! �ַ����б�����ݴ洢��ʽ
typedef enum STRINGLIST_STYLE
{
	//! �������£�����
	SLS_ONEBYONE,

	//! �������ϣ������б�, ����������ʾ��
	SLS_PUSH,

} STRINGLIST_STYLE;

//! �ַ��б��ı�������ģʽ
typedef enum STRINGLIST_HIGHLIGHTMODE
{
	//! �ƶ�����
	SHL_MOVEHIGHLIGHT,

	//! �������
	SHL_CLICKHIGHLIGHT,

	//! �޸���
	SHL_NOHIGHLIGHT,

} STRINGLIST_HIGHLIGHTMODE;


//! ����������ʽ
#define SCROLLBAR_WIDTH 12
typedef struct SCROLLBAR_STYLE {
	int scb_style;         //! 0��,1��
	int scb_range;         //! ��Χ���ں�����ʱʹ��
	int scb_unitLen;       //! ��������Ԫ�صĸ߶ȣ��᣺����Ԫ�صĿ��
	int scb_default_blockLen;//! Ĭ�Ͽ�ĳ���
	char scb_tex[128];     //! ������
	osVec2D scb_Btnlt_uv[2];   //! ���ϰ�ť����������
	osVec2D scb_Btnrb_uv[2];   //! ���°�ť����������
	osVec2D scb_Btnlt_uv2[2];   //! ���ϰ�ť����������2
	osVec2D scb_Btnrb_uv2[2];   //! ���°�ť����������2

	osVec2D scb_Blocklt_uv[2]; //! �������ϲ�����������
	osVec2D scb_Blockrb_uv[2]; //! �������²�����������
	osVec2D scb_Blockm_uv[2];  //! �����в�����������
	osVec2D scb_Blockm_uv2[2];  //! �����в�����������
} SCROLLBAR_STYLE;

//! ��ѡ�����ʽ
typedef enum CHECKBUTTON_TEXTMODE
{
	CBTM_RIGHT,           // �ڵ�ͼ���Ҳ�
	
	CBTM_OVERPIC_LEFT,    // �ڵ�ͼ֮�������д����
	CBTM_OVERPIC_MIDDLE,   // �ڵ�ͼ֮���ж���д����
	CBTM_OVERPIC_RIGHT   // �ڵ�ͼ֮���Ҷ���д����

} CHECKBUTTON_TEXTMODE;

//! �����ı�������ʽ(�D)
typedef struct TEXTBAR_STYLE
{
	float  move_speed;       // �ƶ����ٶ�(����/��)
	int    space_node_cout;  // ���δ���Ŀհ׽����� ( 6pix/�� )
} TEXTBAR_STYLE;

//! ö���϶��ؼ��ڶԻ���λ��
typedef enum RESIZEBAR_POS
{
	RP_TOP          = 0,   //�ϱ�Ե
	RP_BOTTOM       = 1,   //�±�Ե 
	RP_LEFT         = 2,   //���Ե
	RP_RIGHT        = 3,   //�ұ�Ե
	RP_RBCORNER     = 4,   //���½�

	RP_NUM,
};

//! �ı�Ի����С�ؼ�����ʽ
typedef struct RESIZEBAR_STYLE
{
	RESIZEBAR_POS	posType;
} RESIZEBAR_STYLE;

//! ����ѡ������ʽ
typedef struct COMBOBOX_STYLE
{
	char              tex[128];         //������,����stringlistʹ�õ�����
	int               dropListMaxRow;   //�����б�������ʾ����
	FDCTEXT_DISFORMAT textDisFormat;    //�����ı��Ķ��뷽ʽ

	//���ÿؼ�����
	LIST_STYLE        stringListStyle;  //�ַ��б���ʽ
	SCROLLBAR_STYLE   scrollBarStyle;   //��������ʽ
} COMBOBOX_STYLE;

//! ���Ķ����ؼ�֡��
#define    MAX_ANIM_FRAME        20

//! ����ͼ����ʽ
typedef struct ANIMICONCTL_STYLE {

	//! ����ʹ�õ������ļ���
	char tex[256];

	//! ��һ֡��������
    osVec2D texuv[2];

	//! ֡��
	int m_MaxFrameCount;

	//! ÿ֡ռ�õĵ�λ֡����
	int m_FrameLen[ MAX_ANIM_FRAME ];

	//! �����ٶ�,(֡/ÿ��)
	int m_FrameCountPerSec;

} ANIMICONCTL_STYLE;

//! ��������ؼ�(��ʱû����)
typedef struct SCROLLTEXTURE_STYLE 
{
	char name[64];     //����
	char texname[64];  //��������
	osVec2D uv[2];     //��������
	SIZE size;         //��С
	int  movedir;      //�ƶ�����, 0:�����ƶ� 1:���� 2:���� 3:���� 4:����

} SCROLLTEXTURE_STYLE;

//! ��ֱ�����ı�
typedef struct TEXTBARV_STYLE
{
	float m_speed;
	int   m_space;
} TEXTBARV_STYLE;

//! ��ΰ�ť��Ŀ
typedef struct LAYERGROUUPITEM
{
	char key[ 32 ];         //����key
	int idInIconLib;        //���ߵ�ͼ��ID,��ͼ����� (16*16)
	int idInIconLibOffline; //���ߵ�ͼ��ID,��ͼ����� (16*16)
	char text[ 64 ];        //�ı�

	DWORD m_textItemColorDefault;
	DWORD m_textItemColorDown;
	DWORD m_textItemColorOver;

} LAYERGROUUPITEM;

//! ͼ��״̬
typedef enum LAYERGROUPITEMSTATE
{
	LGIS_ONLINE,   //����
	LGIS_OFFLINE,  //����
	LGIS_UPLINE,   //������˸
	LGIS_HIDE,     //����
} LAYERGROUPITEMSTATE;

//! QQ����״̬
typedef struct QQHUMANSTATE
{
	LAYERGROUPITEMSTATE iconState;
	bool bMsg;

} QQHUMANSTATE;

//! �����ť�����ʽ
typedef struct LAYERGROUPSTYLE
{
	char tex[64];               // ������
	osVec2D uvTitleButtonBg[2]; // ��ť��������

	osVec2D uvLeft[2];          // ���ⰴť��Ƥ����������-��
	osVec2D uvRight[2];         // ���ⰴť��Ƥ����������-��
	osVec2D uvMiddle[2];	    // ���ⰴť��Ƥ����������-��

	osVec2D uvLeftDown[2];          // ���ⰴť��Ƥ����������-��(����)
	osVec2D uvRightDown[2];         // ���ⰴť��Ƥ����������-��(����)
	osVec2D uvMiddleDown[2];	    // ���ⰴť��Ƥ����������-��(����)

	int textFormat;             // �ı����뷽ʽ 0:left 1:middle 2:right
	int itemSpace;              // ��Ŀ���

	DWORD m_textTitleColorDefault;
	DWORD m_textTitleColorDown;
	DWORD m_textTitleColorOver;



} LAYERGROUPSTYLE;

//! �����ť����ʽ
typedef struct LAYERBUTTONSTYLE
{
	char tex[64];     // bind texture
	osVec2D uvBg[2];  // ������������
	//int itemSpace;    // ��Ŀ���,��Ϊ�༭�����ƣ��˴�����ʹ��

} LAYERBUTTONSTYLE;

//! һ��ҳ
typedef struct LAYER_PAGE
{
	char name[64];        //ҳ������

	char tex[64];         //checkBox������
	osVec2D uvCheck1[2];  //checkBox 1̬
	osVec2D uvCheck2[2];  //checkBox 2̬

	LAYERBUTTONSTYLE layerStyle; //��ΰ�ť����ʽ

} LAYER_PAGE;

//! QQ����ʽ
typedef struct QQ_STYLE
{
	char tex[64];     // qqʹ�õ�����
	osVec2D uvBg[2];  // ������������
	int itemSpace;    // ��Ŀ���

	DWORD textTitleColorDefault;
	DWORD textTitleColorDown;
	DWORD textTitleColorOver;

	DWORD textItemColorDefault;
	DWORD textItemColorDown;
	DWORD textItemColorOver;

	//��������ʽ
	SCROLLBAR_STYLE scb_style;

} QQ_STYLE;

//! ��ť��ʽ
typedef struct BUTTONSTYLE
{
	int bSkin;        // 0:��ͨ��ť, 1:��Ƥ��ť

	char skinTex[32]; //��Ƥ�����뼤��̬����
	int btnState;     //��̬��ť 0:2̬, 1:3̬

	// ����
	//----------------------------------------
	// ��ť
	DWORD colorDefault;
	DWORD colorDown;
	DWORD colorGray;
	BYTE  alphaDefault;
	BYTE  alphaDown;
	BYTE  alphaGray;

	// �ı�
	DWORD textColorDefault;
	DWORD textColorDown;
	DWORD textColorGray;

	DWORD textColorOver;

	BYTE  textAlphaDefault;
	BYTE  textAlphaDown;
	BYTE  textAlphaGray;



	// ��ͨ��ť����
	//------------------------------------------
	// ����
	osVec2D uvDefault[2];
	osVec2D uvDown[2];

	//����༭��ָ���ĵ�3̬(MouseMove)
	int   bForceMoveState;
	char  buttonMoveStateTex[64];
	osVec2D uvMouseMove[2];

	// ��Ƥ����
	//------------------------------------------
	osVec2D uvDefaultLeft[2];
	osVec2D uvDefaultRight[2];
	osVec2D uvDefaultMiddle[2];

	osVec2D uvDownLeft[2];
	osVec2D uvDownRight[2];
	osVec2D uvDownMiddle[2];
	int unitLen;


} BUTTONSTYLE;

//! ��ʾ����������ʽ
typedef struct TOOLTIP_STYLE 
{
	int spacex;
	int spacey;
	int border;
	char tex_border[256];
	DWORD color;
	DWORD textColor;
	FDCTEXT_DISFORMAT format;
	int userx;
	int usery;
	
} TOOLTIP_STYLE;

//! ��������״̬
typedef enum PROGRESS_STATE
{
	PS_DEFAULT,
	PS_FLASH,

} PROGRESS_STATE;

typedef struct WINTEXINFO
{
	char texname[64];
	osVec2D uv[2];

} WINTEXINFO;

//! ������
typedef enum HOTLINKEVENTTYPE
{
	HLE_HELPSYSTEMADDR,    //����ϵͳ��ת
	HLE_USEREVENT,         //���ظ��ϲ��û����¼�

} HOTLINKEVENTTYPE;

typedef struct HOTLINKSTYLE
{
	bool m_bUnderLine;
	DWORD m_defaultColor;
	DWORD m_hotColor;
	DWORD m_visitedColor;

} HOTLINKSTYLE;

typedef struct PARSER_LINE_SECTION
{
	FDCCTL_TYPE type;
	std::string sectionStr;
	int         param;                       //type == CTL_HOTLINK, is link id
	RECT        rect;                        //ռ�õĴ�С

} PARSER_LINE_SECTION;

typedef struct PARSER_LINE
{
	std::vector< PARSER_LINE_SECTION > m_vecSection;

} PARSER_LINE;

class osRect
{
public:
	osRect():l(0),t(0),r(0),b(0){}
	osRect(int _l,int _t,int _r,int _b){
		l=_l;t=_t;r=_r;b=_b;
	}
	osRect(RECT& _rect)	{
		l=_rect.left;t=_rect.top;r=_rect.right;b=_rect.bottom;
	}
	osRect(osRect& _osRect)	{
		l=_osRect.l;t=_osRect.t;r=_osRect.r;b=_osRect.b;
	}
	~osRect(){}

	void set_rect(int _l,int _t,int _r,int _b){
		l=_l;t=_t;r=_r;b=_b;
	}
	int get_height(){
		return b-t;
	}
	int get_width(){
		return r-l;
	}
	BOOL ptInRect(POINT _pt){
		RECT r1;
		r1.left=(LONG)l;
		r1.top=(LONG)t;
		r1.right=(LONG)r;
		r1.bottom=(LONG)b;
        return PtInRect(&r1,_pt);
	}
	void OffsetRect(int _offx,int _offy)
	{
		l+=_offx;
		r+=_offx;
		t+=_offy;
		b+=_offy;
	}
	void Inflate_Rect( int _x, int _y )
	{
		RECT a;
		a.left=l; a.top=t; a.right=r; a.bottom=b;
		InflateRect( &a, _x, _y );
		l=a.left; t=a.top; r=a.right; b=a.bottom;
	}
	BOOL EqualRect(const osRect& _r)
	{
		if(_r.l==l && _r.t==t && _r.r==r && _r.b==b)
			return TRUE;
		return FALSE;
	}
	BOOL IntersectRect(osRect& _r, osRect* _outRect)
	{
		BOOL ret;
		RECT r1,r2,out;
		r1.left=_r.l;
		r1.top=_r.t;
		r1.right=_r.r;
		r1.bottom=_r.b;
		r2.left=l;
		r2.top=t;
		r2.right=r;
		r2.bottom=b;
		ret = ::IntersectRect(&out, &r1, &r2);
		_outRect->l = out.left;
		_outRect->t = out.top;
		_outRect->r = out.right;
		_outRect->b = out.bottom;
		return ret;
	}
	void SetRectEmpty()
	{
		l=t=r=b=0;
	}
	void operator =( const osRect& _srcRect )
	{
		l=_srcRect.l;
		t=_srcRect.t;
		r=_srcRect.r;
		b=_srcRect.b;
	}
	BOOL operator ==( const osRect& _srcRect )
	{
		return this->EqualRect(_srcRect);
	}

	int l,t,r,b;
};

//! ��ǰ������
class fdc_baseCtl;
class fdc_iconCtl;
class fdc_scrollBar;

//! ���Դ���
class CStickyDragManager
{
public:
	CStickyDragManager(bool isDocker = false);
	~CStickyDragManager();

	void Init( fdc_baseCtl * pWnd = 0 );
	osRect& SnapMoveRect( osRect& r);
	inline void AddWindow(fdc_baseCtl * pWnd) {	m_windows.push_back(pWnd);	}
	inline bool IsCloseTo(int ms, int ss) {	return ((ms > (ss - m_slack)) && (ms < (ss + m_slack))) ? true : false;	}
	
	static int      m_slack;
	static osVec2D  m_initOffset;

protected:
	bool   m_stickFlags[4];
	int    m_stickPos[4];
	fdc_baseCtl *   m_activeWindow;
	std::vector<fdc_baseCtl *> m_windows;
	osRect m_offsetRect;
};

//! ���ڻ���
class fdc_baseCtl
{
public:
	fdc_baseCtl();
	virtual ~fdc_baseCtl();

private: //attribute
	char  m_szCtlName[MAX_CRLNAMELEN];      //! ���ڵ�����,��������
	DWORD m_dwID;                           //! ���ڵ�ID,������̬�����Ĵ��ڶ���
	bool  m_bVisible;	                    //! ��ǰ�ؼ��Ƿ�ɼ���
	bool  m_bAcceptTab;                     //! �Ƿ����TAB����Ϣ��
	int   m_iTabOrder;                      //! TAB����˳��ֵ��
	os_stringDisp m_text;                   //! ���ڵı���

	osVec2D m_textOffset;                   //! �ı�����Ը�������
	FDCTEXT_DISFORMAT m_disFormat;          //! �������ķ�ʽ
	FDCCTL_TYPE  m_ctlType;                 //! ��ǰ�ؼ������͡������̬ʱ������ͣ�
	fdc_baseCtl * m_pParent;                //! ��ǰ�ؼ����ؼ���ָ��
	osVec2D m_scrOffsetPos;                 //! ��Ļ����ϵ 
	osRect  m_rectCtlArea;                  //! �ؼ���Ӧ������,ÿһ���ؼ�������ռ��һ�����ε��������Աtop,leftΪ���϶����ƫ��ֵ  	//l:offset_x, t:offset_y, r:width+l, b:height+t  
	bool m_bCanDrag;                        //! ��ǰ�����Ƿ�����϶���
	osRect m_CanDragRect;                   //! ��ʹ�����϶�����Ч������
	osRect m_textRect;                      //! ��ʾ���־�����
	int m_textShadow;                       //! �Ƿ����ı�����Ӱ
public:
	os_screenPicDisp  m_screenPicDisp; 	    //! ��ʾ�ؼ������ݣ����㣬����, ������������update()
	std::vector<fdc_baseCtl*> m_vecChilds;  //! �ؼ���Ӧ���ӿؼ��б�
	int m_row_space;                        //! ��ʱ���м��

public: //method
	DWORD get_ID();                         //! ��ô��ڵ�ID
	void set_tabEnable(bool _b);            //! ��TAB
	bool get_tabEnable();                   //! �õ�����TAB����Ϣ��
	void set_tabOrder(int _i);              //! ����Tab���
	int  get_tabOrder();                    //! �õ�TAB����˳��
	FDCCTL_TYPE get_ctlType();              //! ȡ�ÿؼ������͡�
	void  set_name(const char* _wnd_name);  //! ���ô��ڵ����֡�
	const char* get_name() const;           //! �õ����ڵ����֡�
	virtual void set_visible(bool _b);              //! �����Ƿ�ɼ�(�Ƿ���Ⱦ)
	virtual bool get_visible();                     //! �õ��ؼ��Ƿ�ɼ���
	bool IsShow();                          //! ��ǰ�Ƿ���ʾ��Ⱦ
	void set_wndHeight( int _height );      //! ���ô��ڵĸ߶�
	void set_wndWidth( int _width );        //! ���ô��ڵĿ��
	int get_wndHeight();                    //! �õ����ڵĸ߶�
	int get_wndWidth();                     //! �õ����ڵĿ��
	virtual void   set_text( const char * _text,
		FDCTEXT_DISFORMAT _format = TEXT_CENTER, 
		int _user_x=0, int _user_y=0);      //! ���ô��ڵı����ı���
	virtual char * get_text();              //! ���ش��ڵı����ı�
	virtual os_stringDisp& get_stringDisp();//! ���������ı���ʾ�ṹ
	virtual void   set_color(DWORD _color); //! ���ô�����ɫ
	virtual DWORD  get_color();             //! ��ô�����ɫ
	virtual void   set_textColor(DWORD _textColor); //! ���ô����ı����ɫ
	virtual DWORD  get_textColor();                 //! ���ش����ı����ɫ
	void set_textRect(osRect& _textRect);           //! l,t,Ϊ0,��ʾ�ı����Ĵ�С
	osRect& get_textRect();                         //! ����ı����ľ���
	virtual void   set_alpha(BYTE _a);              //! ���ô��ڵ�͸���� 0-255
	virtual BYTE   get_alpha();                     //! ��ô��ڵ�͸����
	void set_textAlpha(BYTE _a);                    //! �����ı���͸����
	BYTE get_textAlpha();                           //! ��ȡ�ı���͸����
	void set_offset( int _x, int _y );              //! ����ڸ������ƫ��
	void set_offset( osVec2D& _offset );            //! ����ڸ������ƫ��
	osVec2D get_offset();                           //! ��ȡ����ڸ������ƫ��
	void set_scrOffset(int _x, int _y);             //! �����������Ļ��ƫ��
	void set_scrOffset(osVec2D& _scrOffset);        //! �����������Ļ��ƫ��
	osVec2D& get_scrOffset();                       //! ��ȡ�������Ļ��ƫ��
	void set_canDrag(bool _bCanDrag);               //! �����Ƿ���϶�
	bool get_canDrag();                             //! ����Ƿ���϶�
	void set_textShadow( int _bShadow );            //! ���������Ƿ�����Ӱ��ʾ, 1,��ʾ 0.��
	bool get_textShadow();                          //! �Ƿ���ʾ�ı�����Ӱ
	void set_textOffset( osVec2D& _textOffset );    //! �����ı���ƫ��
	osVec2D& get_textOffset();                      //! ��ȡ�ı���ƫ��
	void move_wnd( osVec2D& _offset );              //! �ƶ�����
	void set_rect(osRect& _rect);                   //! ���ô���ľ���
	osRect& get_rect();                             //! ��ô���ľ��δ�С
	osRect get_scrRect();                           //! ��ô���ľ��Ծ�����
	void    set_canDragRect(osRect& _rect);         //! ���ÿ��϶��ľ�����
	osRect& get_DragRect();                         //! ��ÿ��϶��ľ�����
	void    set_parent(fdc_baseCtl* _pParent);      //! ���ø������ָ��
	fdc_baseCtl * get_parent();                     //! ��ø������ָ��
	void set_textFormat( FDCTEXT_DISFORMAT _disFormat, 
		int _userx = 0, int _usery = 0 );           //! �趨���ָ�ʽ
	FDCTEXT_DISFORMAT get_textFormat();             //! ������ָ�ʽ
	void set_vertexInfo(osRect& _rect,              //! �趨�������ݡ�id��uv
		int _texID = -1, osVec2D* _uv = 0 );        //! uv�����С2��,0:���� 1:����
	void set_vertexInfo2(osRect& _rect);            //! �趨��������     
	void set_vertexInfo(osVec2D* _uv);              //! �趨����uv
	void set_vertexInfo( int _texID, osVec2D* _uv=0 );//! ���ö��������id,uv
	int get_texID();                                //! ������������id

	virtual  void set_default();                    //! ����ָ������Ĭ��״̬
	virtual  void set_default_parent();             //! �趨��ΪĬ��״̬��comboBox ��
	virtual  void release_capture();                //! �ͷ�mouse capture
	virtual  void set_defaultMoveEvent();           //! �ƶ����ָ�Ĭ��״̬
	virtual  void set_preFocusClose();              //! ����ǰһ����ĸ����Ի���ر��¼�

	virtual void msgProc( UIMSG& _uiMsg );          //! ���ڵ���Ϣ������

	//! ����ĳ�ʼ��
	virtual void init( FDCCTL_TYPE _ctl_type, char*_wnd_name, osRect& _wnd_rect, 
                       DWORD _color, bool _canDrag, osRect& _canDrag_rect, fdc_baseCtl * _pParent, 
                       bool _bVisible = true, FDCTEXT_DISFORMAT _disFormat = TEXT_CENTER );
	virtual void   update();                        //! ���´��������״̬
	virtual void   pre_render();                    //! Ԥ��Ⱦ������ܵ�,�Ժ���
	virtual void   pre_renderFont();                //! ��Ⱦ����
	virtual void   resize( int _newWidth, int _newHeight ); //! �ı�ؼ���С
	virtual void   stop_update(); //ֹͣ����
	virtual void   clear_data();  //��տؼ�������

	//! ����Ĳ���
	//@{
	//! �����ֲ���
	fdc_baseCtl * find_wnd( char * _wnd_name, fdc_baseCtl *  _findRoot );
	bool          find_wnds( char * _wnd_name, fdc_baseCtl *  _findRoot );
	//! ����������
	fdc_baseCtl * find_wnd( POINT& _pt, fdc_baseCtl* _findRoot );
	void          find_wnds( POINT& _pt, fdc_baseCtl* _findRoot );
	//! ��id����
	fdc_baseCtl * find_wnd( DWORD _id, fdc_baseCtl* _findRoot );
	bool          find_wnds( DWORD _id, fdc_baseCtl* _findRoot );
	//@}

	//! ����ָ���Ĵ����Ƿ�Ϊ����
	bool isMyChild( fdc_baseCtl* _pTarget );

	//! ���������С��Χ����
	osRect get_textBoundRect(const  char * _text);

	//���Դ���
	//@{
	CStickyDragManager m_dragManager;
	void    init_sticky();
	osRect& move_sticky();
	//@}

	//! ���Դ�ӡ���
	void printNode( fdc_baseCtl* _findRoot );
};

//! ��̬�ؼ��ӿ���
struct I_staticCtl : public fdc_baseCtl
{
	//! ������ʾ���ı�
	virtual void   set_dispText( const char* _text,
		           FDCTEXT_DISFORMAT _format = TEXT_USERPOS, int _user_x =0 , int _user_y = 0 ) = 0;

	//! �õ���ǰ��̬�ؼ�����ʾ�����֡�
	virtual const char*  get_dispText( void ) = 0;

	//! �����ı�����ɫ
	virtual void set_dispTextColor( DWORD _textColor ) = 0;

	//! �ı��ķ�ɫ
	virtual void reverseColor() = 0;

	//! ���ö�ʵ��ı�
	virtual void set_multiColorText( char* _text ) = 0;

	//! ��ö�ʵ��ı�
	virtual char* get_multiColorText() = 0;
};

//! ��ť�ӿ�
struct I_buttonCtl : public fdc_baseCtl
{

	//! ���ð�ť�ı�
	virtual void   set_dispText( const char* _text,
		           FDCTEXT_DISFORMAT _format = TEXT_CENTER, int _user_x =0 , int _user_y = 0  ) = 0;

	//! �õ���ǰ��ť����ʾ�����֡�
	virtual char*  get_dispText( void ) = 0;

	//! �趨��ť״̬
	virtual void set_ButtonState( WND_STATE _bs ) = 0;

	//! ��ð�ť��״̬
	virtual WND_STATE get_ButtonState() = 0;

	//! ��ð�ť����ʾ�ı�
	virtual char * get_tipText() = 0;

	//! �趨��ť����ɫ
	virtual void set_buttonColor( DWORD _default, DWORD _down, DWORD _gray, 
			  			  BYTE _alphaDefault = 0xff, BYTE _alphaDown = 0xff, BYTE _alphaGray = 0x80 ) = 0;

	//! ���ð�ť�ĵ����󶨶Ի���
	virtual void set_popBindDlg( fdc_baseCtl * _pDlg ) = 0;

	//! �ٴθ���tooltip�Ͱ�ť��״̬
	virtual void update_stateAgain() = 0;

	// ��˸״̬, _interval�����(��)
	virtual void set_flash( bool _b, float _interval ) = 0;
};

//! ��ť��ʵ���ࡣ
class fdc_buttonCtl : public I_buttonCtl
{
public:
	fdc_buttonCtl();
    virtual ~fdc_buttonCtl();

	//! �趨��ʽ
	void set_style( BUTTONSTYLE _style );

	//! �趨��ť����ɫ
	void set_buttonColor( DWORD _default, DWORD _down, DWORD _gray, 
									 BYTE _alphaDefault = 0xff, BYTE _alphaDown = 0xff, BYTE _alphaGray = 0x80 );
	//! ��ʼ��
	void init( FDCCTL_TYPE _ctl_type, char*_wnd_name, osRect& _wnd_rect, 
                       DWORD _color, bool _canDrag, osRect& _canDrag_rect, fdc_baseCtl * _pParent, 
                       bool _bVisible = true, FDCTEXT_DISFORMAT _disFormat = TEXT_LEFT );

	void fdc_buttonCtl::update();

	//! ��ť��Ϣ����
    void msgProc( UIMSG& _uiMsg );

	//! ���������������Ϣ
    void process_myEvent( UIMSG& _uiMsg );

	void set_text( const char * _text, 
						   FDCTEXT_DISFORMAT _format = TEXT_CENTER, int _user_x = 0, int _user_y = 0 );

	//! ������ʾ���ı�
    void set_dispText( const char* _text,FDCTEXT_DISFORMAT _format = TEXT_CENTER,
	                   int _user_x =0 , int _user_y = 0  );
    
	//! ��ȡ��ʾ���ı�
    char* get_dispText( void );

	//! ���Ӱ�ťup���¼�
	void add_event( UIMSG _cmd );

	//! ���Ӱ�ťdown���¼�
	void add_DownEvent( UIMSG _cmd );

	//! ���ð�ť�İ���״̬
	void set_downState();

	//! ���ð�ťΪĬ��״̬
    void set_default();

	//! ����ǰһ����ĸ����Ի���ر��¼�
	void set_preFocusClose();

    //! �л���ť��״̬
    virtual void set_ButtonState( WND_STATE _bs );

	//! ��ð�ť��״̬
	WND_STATE get_ButtonState();

	//! ���ð�ť����ʾ�ı�
	void set_tipText( char * _str );

	//! ��ð�ť����ʾ�ı�
	char * get_tipText();

	//! �ͷ�mouse capture
	void  release_capture();

	//! ���Ĭ��̬
	void set_defaultMoveEvent();

	//! �趨ͳһ���ƶ���������
	void set_moveHightLightTexture( char * _texName );

	//�谴ť�İ����ı�ƫ����
	void set_buttonDownTextOffset( int _x, int _y );

	//! ����mouse����״
	void set_mouseShape( UI_MOUSE_TYPE _mt );

	//! ���ð�ť�ĵ����󶨶Ի���
	void set_popBindDlg( fdc_baseCtl * _pDlg );

	//! �ٴθ���tooltip�Ͱ�ť��״̬
	void update_stateAgain();

	//! ��˸״̬, _interval�����(��)
	void set_flash( bool _b, float _interval );

private:

	//! �����������ťֻ�������¼�,�Ժ���ܻ�����������������Ľ����ָ�
	std::vector<UIMSG> m_vecCmd; //up event
	std::vector<UIMSG> m_vecDown;//down event

	//! ��ǰ��״̬
	WND_STATE m_state;

    //! ״̬����������Ϊ��ֹmouse�ƶ�ʱ��δ洢״̬��״̬�����ǵ������
    bool m_bLock; 

	//! �Ƿ�mouse����
	bool m_bCapture;

	//! ��ʾ�ı�
	char m_tipText[256];

	//! ����ͼƬ
	fdc_iconCtl * m_pActive;

	//! mouse move������
	char m_moveHightLightTexture[64];

	//! ����ʱ�ı�ƫ��
	int	m_textOffsetX;
	int	m_textOffsetY;

	int m_buttonDefaultTextOffsetX;
	int m_buttonDefaultTextOffsetY;
	
	int	m_buttonDownTextOffsetX;
	int	m_buttonDownTextOffsetY;

	//! ���µ����󶨶Ի���
	fdc_baseCtl * m_pPopBindDlg;

	//! ��ʱ��˸״̬
	bool m_bFlashState;

	//! ��˸״̬�µļ��ʱ��
	float m_flashInterval;

	//! ��˸ʱ���¼
	float m_flashStartTime;

public:
	
	//WND_STATE m_oldState;
	BUTTONSTYLE m_style;

	//! ͳһ�İ�ť��ʽ
	static BUTTONSTYLE m_publicStyle;
	static char moveHightLightTexture[64];

	//! ֧�ֱ༭��ָ����MouseMove̬������ID
	int m_mouseMoveStateTexID;
};

//! ��ѡ��
struct I_checkButtonCtl : public fdc_baseCtl
{
	//! ��ø�ѡ���Ƿ�Ϊѡ��״̬
	virtual bool get_check() = 0;

	//! ���ø�ѡ��
	virtual void set_check( bool _check ) = 0;

	//! �õ���ѡ���ı�
	virtual char * get_checkBtnText() = 0;

	//! ����1̬���ı�
	virtual void set_defaultStateText( char * _text ) = 0;

	//! �趨2̬check�ı�
	virtual void set_checkStateText( char * _text ) = 0;

	//! �õ�2̬�ĸ�ѡ���ı�
	virtual char * get_checkStateText() = 0;

	//! ���ø�ѡ���ı�ģʽ
	virtual void set_textMode( CHECKBUTTON_TEXTMODE _cbtm ) = 0;
	
	//! �õ�checkbox��Ĭ��̬��������
	virtual WINTEXINFO get_defaultStateUv() = 0;

	//! �������������ݲ�ѯ�Ƿ�Ϊ��ǰcheckbox
	virtual bool is_me( WINTEXINFO& _info ) = 0;

	//! �����ʾ�ı�
	virtual const char * get_tipText() const = 0 ;

	//! ����check color...
	virtual void set_checkButtonColor( DWORD _defaultColor, BYTE _defaultAlpha,
			   				           DWORD _checkColor,   BYTE _checkAlpha ) = 0;
};

//! �༭�ؼ�
struct I_editCtl : public fdc_baseCtl
{
	//! �����ı�
	virtual void   set_dispText( const char* _text ) = 0;

	//!  �õ���ǰ�༭���ڵ��ִ���
	virtual char* get_dispText( void ) = 0;

	//! �õ��༭����ȫ���ִ�
	virtual const  char* get_allText() = 0;

	//! �������ʧ����ؼ�
	virtual void set_editActive(bool _b) = 0;

	//! �鿴��ǰ�ı༭�ؼ��Ƿ�Ϊ����״̬
	virtual bool get_editActive() = 0;

	//! ������ɵ�Ӣ���ַ���
	virtual void set_maxTotalNum( int _max ) = 0;
	
	//! ����������ɵ�Ӣ���ַ�����
	virtual int get_maxTotalNum() = 0;

	//! �༭�ؼ��Ƿ���������Ⱦ��
	virtual void set_renderImm( bool _bRenderImm ) = 0;
};

//! ���б༭��(���±��ؼ�)
struct I_notebookCtl : public fdc_baseCtl
{
	//! ����ı�
	virtual const char* get_notebookText() = 0;

	//! ���ü��±��е��ı�
	virtual void set_notebookText( char* _text ) = 0;

	//! ���ü��±��ı�����ɫ
	virtual void set_notebookTextColor( DWORD _color ) = 0;

	//! ���ü������
	virtual void set_notebookActive( bool _b ) = 0;

	//! ��ʼ��ʱ�������Ŀ������ַ���
	virtual void set_maxCharCount( int _maxCharCount ) = 0;
};

//! �������ӿ�, ����
struct I_scrollBar : public fdc_baseCtl
{
	//! ���û������ķ�Χ
	virtual void set_scbRange(int _range) = 0;

	//! ��û����������Χ
	virtual int get_scbRange() = 0;

	//! ���û�������λ��
	virtual void set_scbPos(int _pos) = 0;

	//! ��û�������λ��
	virtual int get_scbPos() = 0;
};

//! ͼ����
struct I_iconCtl : public fdc_baseCtl
{
	//! ����ͼ�����ɫ
	virtual void set_iconColor( DWORD _color ) = 0;

	//! ����ͼ���͸����
	virtual void set_iconAlpha( BYTE _a ) = 0;
	
	//! �������ñ��
	virtual void set_number( int _n, DWORD _color, FDCTEXT_DISFORMAT _format = TEXT_RIGHT ) = 0;
	
	//! ������½ǵ����ֱ��
	virtual void clear_number() = 0;

	//! ����ʱ����Ч
	virtual bool play_timeEffect( float _totalTime, DWORD _color = 0xffffffff, float _flashTime = 1.f, float _startTime = 0.0f ) = 0;

	//! ɾ��ʱ����Ч
	virtual bool del_timeEffect( ) = 0;

	//! �鿴�Ƿ��ڲ�ʱ����Ч, -1:������Ч, 0: û�У�1:���ڲ���
	virtual int is_playTimeEffect( ) = 0;

	//! ������ʾ������
	virtual int set_icon( const char* _textureName, float _u1, float _v1, float _u3, float _v3 ) = 0;

	//! �ı�ؼ���С
	virtual void resize( int _newWidth, int _newHeight ) = 0;

};

//! ������
struct I_containerCtl : public fdc_baseCtl
{
	//! ����ͼ��
	virtual void add_icon( int _idInLib ) = 0;

	//! ɾ��ͼ��
	virtual void del_icon() = 0;
	
	//! ���ͼ��
	virtual I_iconCtl * get_icon() = 0;
};

//! �б���ࡣ�ɴ��Ҳ������
class fdc_ListCtl : public fdc_baseCtl
{
public:
	fdc_ListCtl();
	virtual ~fdc_ListCtl();

public:

	//! ��Ϣ����
    virtual void msgProc( UIMSG& _uiMsg );

	//! ����list��ʽ
	void set_style( LIST_STYLE& _style );

	//! ���û���������ʽ
	void set_scrollbarStyle( SCROLLBAR_STYLE& _scrollbarStyle );

	//! ��ȡlist��ʽ
	LIST_STYLE& get_style();
	
	//! ������������
	virtual void set_maxrow( int _maxrow );

	//! ��ʼ��
	void init( FDCCTL_TYPE _ctl_type, char*_wnd_name, osRect& _wnd_rect, 
                       DWORD _color, bool _canDrag, osRect& _canDrag_rect, fdc_baseCtl * _pParent, 
                       bool _bVisible = true, FDCTEXT_DISFORMAT _disFormat = TEXT_CENTER );

	//! �鿴�б�ؼ��Ƿ��й�����
	bool has_scrollBar();

	//! ���ù������Ŀ�����(Ҫ�޸��б�ؼ��ڲ�����Ŀ�Ĵ�С����֤�ؼ��Ĵ�С����)
	void set_scbVisible( bool _visible );

	//! �б���ʽ
	LIST_STYLE   m_style;

	//! ��ʾ����ʼ��
	int m_curRow;	
	
	//! �б�������ʽ                   
	SCROLLBAR_STYLE m_scrollbar_style;

	//! ѡ�м����
	fdc_baseCtl * m_pActive;
	
	//! ������
	fdc_scrollBar * m_pScrollBar;
};

//! ͼ���б�ӿ�
struct I_iconListCtl : public fdc_ListCtl
{
	//! ����Ҫ��ʾ��ͼ�ꡣ
	virtual bool add_icontoList( int _idInLib, int _pos = -1, int _iconKey = -1, int _iconWidth = 32, int _iconHeight = 32 ) = 0;

	//! ɾ��ͼ���б���ڵ�ĳһ��ͼ�ꡣ
	virtual void delete_dispIcon(  int _idx  ) = 0;

	//! �õ���ǰͼ���б�������ʾͼ���������
	virtual int get_dispIconCol( void ) = 0;

	//! �õ���ǰͼ���б����ѡ�е�ͼ�ꡣ����ѡ��ͼ���Index.
	virtual int get_selIcon( void ) = 0;

	//! ���ָ��idx��ͼ��idInLib
	virtual int get_idInLib( int _idx ) = 0;

	//! �б��ͼ��λ�ý���
	virtual void exchange_iconPos( int _FromPos, int _ToPos ) = 0;

	//! �������б��ͼ��λ�øĻ��������뷽ʽ, �ɹ���true,����δ�ɹ���false
	virtual bool move_iconPosByInsert( int _from, int _insertto ) = 0;

	//! ת����id->ͼ���idx
	virtual int convertFrameToIdx( int _FrameID ) = 0;

	//! ת��ͼ���idx->��id
	virtual int convertIdxToFrame( int _idx ) = 0;

	//! ͼ�����б��ڵĴ洢ģʽ: _mode:0 ��Ʒ����ģʽ  _mode:1 �������ģʽ
	virtual void set_dataMode(ICONLIST_STYLE _mode) = 0;

	//! ��ȡ�ؼ��Ĵ洢ģʽ
	virtual ICONLIST_STYLE get_dataMode() = 0;

	//! ȡ��ͼ���б��ļ����
	virtual void cancel_active() = 0;

	//! ͨ��λ�û��ͼ��ӿڣ������Ʒ����ʽ��ͼ��λ�ö�̬�仯��׼ȷ
	virtual I_iconCtl * get_iconCtl( int _idx ) = 0;

	//! ͨ��Ψһ�������һ��ͼ��(׼ȷ�Ķ�̬����)
	virtual I_iconCtl* get_iconByKey( int _iconKey ) = 0;

	//! ͨ���û�ָ����Ψһidɾ��һ��ͼ��
	virtual void del_iconByKey( int _iconKey ) = 0;

	//! ���û�������λ��
	virtual void set_scbPos( int _pos ) = 0;

	//! ������ʼ��
	virtual void set_startRow( int _pos ) = 0;

	//! �����Ƿ���ʾ�����
	virtual void set_showActiveBorder( bool _show ) = 0;

    //! �����Ƿ�ͼ����϶�
	virtual void set_iconCanDrag( bool _bCanDrag ) = 0;

	//! ������ͼ��λ�ã��Ƿ��Զ����ص׿�
	virtual void set_autoHideFrame( bool _bAutoHide ) = 0;

	//! ������е�ͼ��
	virtual void del_allIcon() = 0;

	//! �����Ч��ͼ������
	virtual int get_iconCount() = 0;
};

//! ��ɫѡ���б�
struct I_colorListCtl : public fdc_ListCtl
{
	//! �޸�ѡɫ������ɫ
	virtual void set_color( int _idx, DWORD _color ) = 0;

	//! ���ѡɫ��ָ����������ɫ��
	virtual DWORD get_color( int _pos ) = 0;

	//! �õ�ѡɫ����ɫ����
	virtual int get_colorCount() = 0;
};

//! �ַ��б�ӿ�
struct I_stringListCtl : public fdc_ListCtl
{
	//! �����ִ��б����ʽ
	virtual void set_dataMode( STRINGLIST_STYLE _sls ) = 0;

	//! ���������б�����ļ��Ķ��뷽ʽ
	virtual void set_strDispFormat( FDCTEXT_DISFORMAT _format ) = 0;

	//! ���뵥���ִ�,����cut
	virtual int add_string( const char* _str, DWORD _textColor= 0xffffffff, bool _bMultiColor = false, 
		            FDCTEXT_DISFORMAT _textFormat = TEXT_LEFT ) = 0;

	//!�������б�����ʾ������,����д������, �����¼�����ִ��۷ֵ�����
	virtual int add_dispString( const char* _str, DWORD _textColor= 0xffffffff ) = 0;
	
	//! ���ַ�������һ������ı���.
	virtual int add_dispMultiColorString( const char* _str ) = 0;

	//! �����б��п�����ʾ������ִ�������
	virtual void set_maxDispStringNum( int _maxNum ) = 0;

	//! ɾ��һ���б���ڵ����֡�ʹ���ִ����ݽ���ɾ����
	virtual int delete_stringInList( const char* _str ) = 0;

	//! ɾ��һ���б���ڵ����֡�ʹ���ִ���������ɾ����
	virtual int delete_stringInList( int _idx ) = 0;

	//! ɾ�������б���ڵ�����
	virtual void delete_all() = 0;

	//! �õ���굥������˫�����ִ������ݡ�
	virtual char* get_lbClkString( void ) = 0;
	
	//! ���û����������
	virtual void set_scbEnd() = 0;

	//! �鿴�Ƿ񻬿�λ��״̬��0: ����,1:�м�, 2:���
	virtual int get_scbPosState() = 0;

	//! ����ģʽ
	virtual void set_highLightMode( STRINGLIST_HIGHLIGHTMODE _mode ) = 0;

	//! ������Ŀ����.
	virtual void set_bgHighLight( int _row ) = 0;

	//! ��̬�ı�ؼ��ĵĴ�С
	virtual void resize( int _newWidth, int _newHeight ) = 0;

	//! ��ȡָ�������Ĵ�
	virtual char* get_string( int _idx ) = 0;

	//! �Ƿ�֧���ı��ĸ����仯
	virtual void set_highLightText( bool _b ) = 0;

	//! ����mouse moveʱ�ı��ĸ���ɫ,��������������Ϊ��ɫ�ı�
	virtual void set_highLightTextColor( DWORD _color ) = 0;

	//! �Ƿ񱳾�����
	virtual void set_highLightBackGround( bool _b ) = 0;
};

//! ͼ���ַ����б���Ŀ
typedef struct ICONSTRINGLISTITEM
{
	int  iconIdInLib;       //! ��ͼ����е�ͼ��ID
	char str0[128];         //! �ַ���0
	DWORD str0Color;        //! �ַ���0����ɫ
	char str1[128];         //! �ַ���1
	DWORD str1Color;        //! �ַ���1����ɫ

} ICONSTRINGLISTITEM;

//! ͼ���ַ����б�
struct I_IconStringCtl : public fdc_ListCtl
{
	//! ����һ����Ŀ
	virtual bool add_item(  ICONSTRINGLISTITEM& _item ) = 0;

	//! ������,ɾ��һ����Ŀ
	virtual void del_itemByIdx( int _idx ) = 0;

	//! ��ͼ��id,ɾ��һ����Ŀ
	virtual void del_itemByIconId( int _iconIdInLib ) = 0;

	//! ɾ��������Ŀ
	virtual void del_allItem() = 0;

	//! ��õ�ǰѡ����Ŀ����,��ѡ����Ŀʱ������false
	virtual bool get_selectItem( ICONSTRINGLISTITEM& _item ) = 0;

	//! ���ָ��(������)����Ŀ����.
	virtual bool get_item( int _idx , ICONSTRINGLISTITEM& _item ) = 0;
	
	//! ���ָ��(��ʵλ������)����Ŀ����. _idx: index
	virtual bool get_itemInAll( int _idx, ICONSTRINGLISTITEM& _item ) = 0;

	//! ���ͼ��
	virtual I_iconCtl * get_iconCtl( int _idx ) = 0;
};

//! �������ӿ�
struct I_progressCtl : public fdc_baseCtl
{
	//! ���ý�����λ��, ��ͼ0-100
	virtual void set_progressPos(int _nPos) = 0;

	//! �õ���������λ��, ��ͼ0-100
	virtual int  get_progressPos() = 0;

	//! ���ý������ϵ��ı�
	virtual void set_progressText( char *_str ) = 0;

	//! ������˸״̬
	void init_flashState( DWORD _color, BYTE _alpha, DWORD _textColor, float _cycleTime = 1.0f );

	//! ���õ�ǰ״̬
	virtual void set_state( PROGRESS_STATE _state ) = 0;
};

//! ������ʾ���ӿ���
struct I_tooltipCtl : public fdc_baseCtl
{
	//! ����toolTip��λ�á�
	virtual void      set_ttPos( int _x,int _y ) = 0;

	//! �õ�Tooltip��λ�á�
	virtual void      get_ttPos( int& _x,int& _y ) = 0;

	//! ����toolTip�ı�����ɫ��
	virtual void      set_ttBackColor( DWORD  _color ) = 0;

	//! ����ToolTop����ʾ���������ݡ�
	virtual void      set_ttText( const char* _text ) = 0;

	//! ����ToolTip����ʾ���������ݡ�
	virtual void   set_ttMultiColorText( const char* _text ) = 0;

	//! ������ʽ
	virtual void set_style( TOOLTIP_STYLE& _ttStyle ) = 0;

	//! �õ������С
	virtual osRect& get_ttRect() = 0;
};

//! С��ͼͼ������
typedef struct MAPICONDATA
{
	//! ��ɫ
	POINT   HeroPt;
	float   HeroAngle;

	//! ����ָʾ
	bool    bDir;
	float   dirAngle;

	//! Ŀ���
	int targetCount;
	POINT * pTargetPt;

	//! npc
	int     npcCount;
	POINT * pNpcPt;
	char ** ppNpcName;

	//! monster 
	int monsterCount;
	POINT * pMonsterPt;

	//! ��Ա
	int teamMemberCount;
	POINT * pTeamMemberPt;

	//! �ӳ�
	int teamLeaderCount;
	POINT * pTeamLeaderPt;

	//! �ŵ㡡
	int gateCount;
	POINT * pGatePt;

} MAPICONDATA;

//! С��ͼ�ϵ�ͼ��ӿ�
struct I_mapIcon : public fdc_baseCtl
{
	//! ���С��ͼ�������
	virtual MAPICONTYPE get_iconType() = 0;

	//! ��С��ͼ��Сͼ����ı�(��ʾ�ı�)
	virtual void  set_iconName( char* _iconName ) = 0;

	//! ���Сͼ����ʾ�ı�
	virtual char* get_iconName() = 0;

	//! ��Сͼͼ���״̬
	virtual void set_state( WND_STATE _state ) = 0;

	//! ���Сͼ���״̬
	virtual WND_STATE get_state() = 0;

	//! �޸���ʾ��ͼƬ
	virtual void set_texUv( WINTEXINFO& _winTexInfo ) = 0;

	//! ��õ�ǰ��ʾ�ı�ǩ��Ϣ
	virtual WINTEXINFO& get_texUv() = 0;
};

//! ��ͼ�ؼ��ӿ���
struct I_mapCtl : public fdc_baseCtl
{
	//! װ�ص�ͼͼ���ļ�
	virtual bool load_map( char * _mapIni ) = 0;
	
	//! ��õ�ͼ�Ĵ�С
	virtual SIZE& get_mapSize() = 0;

	//! ��õ�ǰ���ź�ĵ�ͼ�ߴ�
	virtual SIZE get_mapSizeScale() = 0;

	//! �����ӿ����ĵ㣬���Ͻ�Ϊ��ͼ��ԭ��
	virtual void set_viewport( osVec2D& _pos ) = 0;

	//! ���õ�ͼ�Ƿ����ק
	virtual void set_DragMap( bool _CanDrag ) = 0;

	//! ���õ�ͼͼ����ʾ����
	virtual void set_data( MAPICONDATA& _mapIconData ) = 0;

	//! ���õ�ͼ�ϵ�Сͼ���ı���Ϣ
	virtual void set_iconTextInfo( MAPICONTYPE _iconType, int _idx, char* _text ) = 0;

	//! ��������ͼ��
	virtual I_mapIcon* add_userIcon( char * _iconName, char * _tooltipText, POINT _pt,char * _texName, SIZE _size, osVec2D* uv ) = 0;

	//! ͨ������,ɾ��һ������ͼ��
	virtual void del_userIcon( const char* _iconName ) = 0;

	//! �������ͼ��
	virtual I_mapIcon* get_userIcon( char * _iconName ) = 0;

	//! �趨ͼ����˸(����״̬)
	virtual void set_state( const char* _iconName, WND_STATE _state ) = 0;

	//! ����ͼ���״̬(�Ƿ���˸)
	virtual bool get_state( const char* _iconName ) = 0;

	//! ����Ѱ·��������
	virtual void set_line( std::vector<POINT>& _vecPoint ) = 0;

	//! �Ƿ���ʾ����
	virtual void set_lineVisible( bool _b ) = 0;
	
	//! �Ƿ���ʾѰ·����ʼ����β��־ͼƬ
	virtual void set_lineStartAndEndIcon( bool _b ) = 0;

	//! ������ʾѰ·����ʼ����β��־ͼƬλ��
	virtual void set_lineStartAndEndPos( POINT _startPt, POINT _endPt ) = 0;

	//! �趨��ͼ����ɫ
	virtual void set_mapColor( DWORD _color ) = 0;
};

//! �����ı���
struct I_textBarCtl : public fdc_baseCtl
{
	//! ���Ӵ�,��������
	virtual int add_str( char* _str ) = 0;

	//! ������,��ô�
	virtual const char* get_str( int _idx ) = 0;

	//! ɾ��ָ�������Ĵ�
	virtual bool del_str( int _idx ) = 0;

	//! ɾ�����д�,����ɾ���ĸ���
	virtual int del_allStr() = 0;

	//! ���õ�ǰ����,������ʾָ���Ĵ�
	virtual void set_curIdx( int _idx ) = 0;
};

//! �϶��ؼ�
#define RESIZEBAR_SIZE   16   //
struct I_resizeBarCtl : public fdc_baseCtl
{
	
};

//! ����ѡ���
struct I_comboBoxCtl : public fdc_baseCtl
{

};

//! ���ؼ�
struct I_treeCtl : public fdc_baseCtl
{

};

//! �Ի���Ľӿ���
struct I_gameDlg : public fdc_baseCtl
{
	//! ����gameDlg�Ƿ�����ƶ���
	virtual void     set_moveEnable( BOOL _move ) = 0;

	//! ���õ�ǰ�Ի����λ�á�
	virtual void     set_dlgPos( int _x,int _y ) = 0;

	//! �õ���ǰ�Ի���ռ�õ�����
	virtual void     get_dlgRect( osRect& _rect ) = 0;

	//! ����һ���ؼ������ֵõ�һ���ؼ���ָ�롣
	virtual fdc_baseCtl*  get_ctlFromName( const char* _ctlname ) = 0;

	//! ����Զ���С�Ի������һ���յ�ͼƬ����Ԫ��,ÿ��Ԫ���Զ��������ң�������������
	virtual I_containerCtl * add_picContainer( int _x, int _y ) = 0;
	
	//! �Ƿ�Ϊ�����Ի���
	virtual bool IsContainerDlg() = 0;

	//! ���öԻ���ı���,λ�ù̶������Ͻ�
	virtual void set_dlgTitle( char * _title ) = 0;

	//! ���öԻ������ʾ������, ����Ի�����۵�״̬ 
	virtual void set_dlgVisible(bool _b) = 0;

	//! ��ȡ�Ի�����Ƿ�ɼ�
	virtual bool get_dlgVisible() = 0;

	//! ��ȡ�Ի���ǰ��״̬, return 0:���ɼ� 1:�����ɼ� 2:�۵��ɼ�
	virtual int get_state() = 0;

	//! ��ӹ��ÿ��еĶԻ���,(���)
	virtual fdc_baseCtl* add_templateDlg( char * _CommIniName, char * _CommdlgName, osVec2D& _offset ) = 0;
	
	//! �ڶ�ڬ�����У�ɾ��һ��ָ�����ƵĹ��öԻ�,true:�ɹ�,false:û�ҵ��˶�ڬ��
	virtual bool del_templateDlg( char * _CommdlgName ) = 0;

	//! �ڶ�ڬ�����У�ɾ��һ��ָ��ָ��Ĺ��öԻ�,true:�ɹ�,false:û�ҵ��˶�ڬ��
	virtual bool del_templateDlg( fdc_baseCtl * _pDlg ) = 0;
	
	//! �ı�Ի���Ĵ�С
	virtual void dlg_resize( osRect _rect, bool _bKeepOffset = true ) = 0;

	//! ���öԻ����С����
	virtual void set_minSize( SIZE _minSize ) = 0;

	//! ���öԻ���Ĵ�����
	virtual void set_maxSize( SIZE _maxSize ) = 0;

	//! ���öԻ���ǰ����������С
	virtual void set_maxResize( int _max_w, int _max_h ) = 0;

	//! ��öԻ���ǰ����������С
	virtual SIZE get_maxResize() = 0;

	//! ���Ի����������ק��,ֻ�ܳ�ʼ������һ��
	virtual void init_resizeBar( bool _up, bool _bottom, bool _left, bool _right, bool _rbcorner,
								 int _up_h = 16, int _bottom_h = 16, int _left_w = 16, int _right_w = 16, 
								 int _rbcorner_w = 16, int _rbcorner_h = 16 ) = 0;
	
	//! ���resizeBar�ؼ�
	virtual fdc_baseCtl* get_resizeBar( RESIZEBAR_POS _posType ) = 0;
	
	//! ������ʱ��С
	virtual SIZE get_designSize() = 0;

	//! �Ƿ��������Ϣ
	virtual void set_processEvent( bool _b ) = 0;

	//! �Ƿ�֧��alpha����
	virtual void set_supportAlphaGradual( bool _b ) = 0;

	//! alpha����, _targetAlpha:Ŀ��alpha, _speed:alpha/��
	virtual void set_alphaGradual( BYTE _targetAlpha, float _speed ) = 0;
};


//! ѭ����������
enum LOOP_TYPE
{
	LT_NO,            //����һ��
	LT_LOOP,          //ѭ������
	LT_LOOP_REVERSE   //����ѭ��
};

//! һ����С��λ����Ч����
struct SCREEN_EFFECT
{
	char name[64];               //��ЧԪ������
	LOOP_TYPE     loopType;      //ѭ������
	float CycleTime;             //����

	POINT startPt,    endPt;     //λ��(���Ͻ�)
	SIZE  startSize,  endSize;   //��С

	float fAngle;                //��ת����

	DWORD startColor, endColor;  //��ɫ
	BYTE  startAlpha, endAlpha;  //alpha

	char texName[128];           //���õ�������
	osVec2D  texUv[2];           //��������

	bool isAnim;                 //�Ƿ��ж�������,  ���к�� ANIMICONCTL_STYLE
	ANIMICONCTL_STYLE*  pAnim;   //��������

	bool bTextureText;           //�Ƿ���������
	SIZE textureTextSize;        //���������ִ�С
};



//! ���幹�쵯���Ի������ʽ(��δʹ��)
#define UIMB_OK					(1<<0)
#define UIMB_CANCEL				(1<<1)
#define UIMB_APPLY				(1<<2)

#define UIMB_ICON_QUESTION		(1<<6)    // ?
#define UIMB_ICON_WARNING	    (1<<7)    // !
#define UIMB_ICON_ERROR			(1<<8)    // x



//! ���������
struct I_gameDlgMgr
{
	//! ��������Ĵ�С
	virtual void set_desktopSize(int _w,int _h) = 0;

	//! �������Ĵ�С
	virtual SIZE get_desktopSize() = 0;

	//! ��õ�ǰ�����ĸ�����ָ��
	virtual fdc_baseCtl* get_pCurRoot() = 0;

	//! ����ĳһʱ�ڵĸ�����, _iniName : �����ļ���
	virtual fdc_baseCtl* find_rootWnd( char * _iniName ) = 0;

	//! ��ʼ���Ի��������
	virtual bool init_gameDlgMgr(I_fdScene* _pScene, I_deviceManager* _dev, HWND _hWnd, int loadModel = 0 ) = 0;

	//! �ͷŶԻ��������
	virtual void release_gameDlgMgr() = 0;

	//! �ı���������
	virtual bool change_ui( char* _uiName ) = 0;

	//! ���½���ϵͳ
	virtual void update() = 0;

	//! ��Ⱦ����ϵͳ
	virtual void pre_render() = 0;

	//! ��id,��öԻ����ָ��
	virtual fdc_baseCtl* get_gameDlgFromId( int _id ) = 0;

	//! �����ֲ��Ҵ���
	virtual fdc_baseCtl* find_wnd(char * _wnd_name ) = 0;

	//! ��id���Ҵ���
	virtual fdc_baseCtl* find_wnd(DWORD _id) = 0;

	//! ����ĳһʱ�ڳ����Ĵ���
	virtual fdc_baseCtl* find_wnd( char * _iniName, char * _wnd_name ) = 0;

	//! �Ƿ����ڽ����2��������
	virtual bool IsPtInUI( POINT _pt ) = 0;

	//! ��ù�����ʾ����ָ��
	virtual I_tooltipCtl* get_tooltipCtl() = 0;

	//! ����ϵͳ����Ϣ����
	virtual void msgProc( MSG_ID uMsg, WPARAM wParam, LPARAM lParam ) = 0;

	//! �ϲ�Ҫ�������Ϣ�����Ƿ�Ϊ��
	virtual bool  cmd_empty() = 0;

	//! ����һ���ϲ�Ҫ������Ϣ
	virtual UIMSG pop_cmd() = 0;

	//! ������Ϣ��
	virtual I_gameDlg* messagebox( bool _model,  bool _CanDrag, 
				char * _msgStr, DWORD _btnAndIconType, int  code = -1) = 0;

	//! �����ƣ�����ͨ�ô���
	virtual I_gameDlg* showCommonDlg( char * _CommIniName, char * _dlgName ) = 0;

	//! �رյ�����ͨ�ô��塣
	virtual void close_CommonDlg( int _id ) = 0;

	//! ͼ�������ӿ�
	//@{
	//! �ṩ���û�ʹ�õ����ӿ�ͼ��,�����ڿ��е�id
	virtual int   add_icon_byUser( char* _filename, float _startu, float _startv, float _endu, float _endv,
								   int _iconWidth = 32, int _iconHeight = 32 ) = 0;

	//! ���ͼ����ܸ���
	virtual int get_iconMaxCount() = 0;
	//@}

	//! ���õ�ǰ��Ϸ���û���������Ϊ���������Ӧ�ļ�
	virtual void  set_uisaveName( char* _userName ) = 0;

	//! ��õ�ǰ�Ľ����û���
	virtual char * get_uisaveName() = 0;

	//! ����ؼ���λ�ã�״̬
	virtual bool ui_save( char* _uiSaveName ) = 0;

	//! ���ļ��лָ������״̬
	virtual bool ui_load( char* _stepName, char* _uiSaveName ) = 0;

	//! ��������ʱ��ָ��
	virtual void save_runRoot() = 0;

	//! ����������ǰ����ʾ����������Ⱦ����
	virtual void BringLevel2WndTop( fdc_baseCtl* _pTarget ) = 0;

	//! ��ǰ�Ľ��㴰���Ƿ�������(��)��
	virtual bool IsFocusInDesktop() = 0;
	
	//! ���ý��㴰��
	virtual void set_focus( fdc_baseCtl* _pWnd ) = 0;

	//! ���õ�ǰ����������Ϊ���㴰��
	virtual void set_focus_wnd_desktop( ) = 0;

	//! ��λ����,��Щ����ĵ���¼���Ķ�����ɾ���ؼ������������������и�λ�����𽹵��Ұָ�����
	virtual void reset_focus() = 0;

	//! �ͷ�ĳһʱ�ڵ���������...
	virtual bool release_desktopTexture( char* _step ) = 0;

	//! װ��һ��ʱ�ڵĽ���,2����ģʽ
	virtual bool Load_step_bin( GAMESTEP _gs, char* _stepName, bool _bShow = true ) = 0;

	//! ����ģʽ�Ի��򱳾�������
	virtual fdc_baseCtl * init_modelbg( char * _stepName, char* _name ) = 0;

	//!	����ģʽ�Ի���Ŀɼ���...
	virtual void process_modeldlg_bg() = 0;

	//! ע�������������Ϣ�ص�����
	virtual void register_msgCallBackFunc( UI_MSGCALLBACKIMM _pFunc ) = 0;

	//! ע���û����趨����Ļص�����
	virtual void register_afterDlgCloseFocusCallBackFunc( UI_MSGCALLBACKIMM _pFunc ) = 0;

	//! �ϲ�����һ���������ԭʼ��������ֱ�ӷ����ϲ㴦��
	virtual void send_simulate_cmd( UIMSG& _uiMsg ) = 0;

	//! ���ؼ�����������Ⱦ��Ԫ��
	virtual void add_renderImm( I_editCtl * _pCtl ) = 0;

	//! ������ʼ������Ⱦ��Ԫ��
	virtual void renderImm() = 0;

	//! ����һ����Ļ��Ч(*.uie)
	virtual bool play_effect( char* _effectName ) = 0;
	
	//! �ýṹ�������鲥��һ����Ч
	virtual bool play_effect( char* _effectName, SCREEN_EFFECT* _pse, int _effcount, SIZE _size/*design size*/ ) = 0;

	//! ֹͣһ����Ļ��Ч
	virtual bool stop_effect( char* _effectName ) = 0;

	//! �жϵ�ǰ�Ľ��㴰���Ƿ������һ������ָ��֮��
	virtual bool is_focusInCtl( fdc_baseCtl * _p ) = 0;

	//! ����������
	virtual void play_screenText( char * _name, osVec2D _pos, osVec2D _dir, 
		                          SIZE _textureTextSize,    /*�����ֵĿ�ʼ��С*/
		                          float _totalTime, char * _text, DWORD  _textColor = 0xffffffff ) = 0;

	//! ���������ֵ�λ��
	virtual void update_screenText( char * _name, POINT _pos ) = 0;

	//! ���ͨ�õİ�ɫ������ͼ����е�id
	virtual int get_whiteTexIdInLib( ) = 0;

	//! ע��������Ҫ�س�����ĶԻ���
	virtual void register_returnKeyDlg( fdc_baseCtl * _pDlg ) = 0;

	//! ��ѯ��ǰ��������
	virtual FDCCTL_TYPE get_curFocusCtlType() = 0;

	//! ��ǰ��mouse�Ƿ���ָ���Ŀؼ�������
	virtual bool is_mouseMoveInCtlRect( fdc_baseCtl * _p ) = 0;

	//! ͨ��sound id���һ�������ļ���
	virtual const char* get_soundName( int _id ) = 0;

	//! ���ͼ�����ָ��ͼ�����������
	virtual int get_soundType( int _iconID ) = 0;

};

//! ���ƶ���������
struct I_scrollTexture : public fdc_baseCtl
{
	
};

//! �ı�������������
struct I_textBarVCtl : public fdc_baseCtl
{
	//! �趨�ı�
	virtual void set_textBarText( char * _text, DWORD _textColor = 0xffffffff ) = 0;

	//! �趨��ɫ�ı�
	virtual void set_textBarMultiColorText( char * _colorText ) = 0;

	//! �趨�м��
	virtual void set_rowSpace( int _rowSpace ) = 0;

	//! �趨�ٶ� (����/��)
	virtual void set_moveSpeed( int _pixelPerSec ) = 0;

	//! ���š�ֹͣ
	virtual void play( bool _b ) = 0;

};

//! QQ
struct I_layerGroupItem : public fdc_buttonCtl
{
	virtual void set_state( QQHUMANSTATE _state ) = 0;
	virtual QQHUMANSTATE get_state() = 0;
	virtual const char * get_itemText() const = 0; 
};

struct I_layerGroup : public fdc_baseCtl
{
	virtual I_layerGroupItem* add_item( LAYERGROUUPITEM _item ) = 0;
	virtual I_layerGroupItem* get_item( char* _key ) = 0;
	virtual void del_item( char* _key ) = 0;
	virtual void del_allItem() = 0;
	virtual void sort_itemByOnline() = 0;
};

struct I_layerButtonCtl : public fdc_baseCtl
{
	virtual I_layerGroup* add_group( char* _groupName, char* _groupTitle, FDCTEXT_DISFORMAT _textFormat=TEXT_CENTER ) = 0;
	virtual I_layerGroup* get_group( char* _groupName ) = 0;
	virtual int get_groupCount() = 0;
	virtual void del_group( char* _key ) = 0;
	virtual void del_allGroup() = 0;
};

struct I_QQCtl : public fdc_baseCtl
{
	virtual I_layerButtonCtl* add_layer( LAYER_PAGE& _lp ) = 0;
	virtual I_layerButtonCtl* get_layer( char * _layerName ) = 0;
	virtual void set_curLayer( char * _layerName ) = 0;
	virtual void del_allLayer() = 0;
};

//! ������
struct I_hotLinkCtl : public fdc_baseCtl
{
	virtual void set_hotLinkText( char * _text ) = 0;
	virtual char* get_hotLinkText() = 0;
};

//! mouse state
class MOUSE_STATE
{
public:
	MOUSE_STATE();
	~MOUSE_STATE();



	void init_mousePic();
	bool set_tex( int _idInIconLib );
	int  get_texidInIconLib();

	void pre_render();
	void release_mouse();

	void update();
	POINT& get_pos();

	//! ����mouse��״̬
	void update_mouseState();
	bool icon_visible();

public:
	int m_idInIconLib;
	fdc_baseCtl * m_pPic;
	int m_fromListID;      //��ק��ͼ������list��id
	int m_fromIconIdx;     //ͼ�����list�е�������

	POINT m_pos;

	bool lb_down;
	bool mb_down;
	bool rb_down;

	bool lr_AllDown; //���Ҽ�ͬʱ����
};

//! ȫ�ֵĶԻ����������
extern I_gameDlgMgr&      get_gameDlgMgr();
extern I_fdScene*         g_ptrScene;
extern I_deviceManager*   g_3dMgrUI;
extern HWND               g_hWndUI;
extern MOUSE_STATE        g_MouseState;