//---------------------------------------------------------------------------------
/**  \file
 *   Filename: getILib.h
 *   Desc:     引擎中界面库的接口函数。
 *   His:      syq @ 2004-11-15.
 *
 *             生活的理想，就是为了理想的生活。
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

#define MSGIO_FLAG 0x80000000     //外部消息的起始ID
#define MSG_FLAG   0x40000000     //界面库自身可以处理的消息

#define IS_IO_MSG(id)  ((id&MSGIO_FLAG) ? true : false)
#define IS_OI_MSG(id)  ((id&MSGO_FLAG) ? false:true)
#define IS_MSG(id)     ((id&MSG_FLAG) ? true:false)

//原始msg id
typedef enum MSG_ID
{
	//外->内消息-----------------------------------------------
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

	//内->外消息-----------------------------------------------

	//{@ 暂时没用
	MSGIO_MESSAGEBOX_OK = MSGIO_FLAG,  //弹出对话框中确定被点击,  wParam为对话框id,lParam为按钮id
	MSGIO_MESSAGEBOX_CANCEL,           //弹出对话框中cancel被点击,wParam为对话框id,lParam为按钮id
	MSGIO_MESSAGEBOX_APPLY,            //弹出对话框中apply被点击, wParam为对话框id,lParam为按钮id
	//}@

	MSGIO_DIALOG_CLOSE,                //对话框关闭按钮被点击,wParam:对话框id
	MSGIO_BUTTON_EVENT,                //按钮被点击up, wParam: parent id, lParam: myself id.
	MSGIO_CHECKBUTTON_LBUTTONDOWN,     //复选框lbutton down  wParam: this,   lParam: pid, id;
	MSGIO_MOUSEMOVE_INBUTTON,          //mouse在按钮上移动
	MSGIO_MOUSEMOVE_INCHECKBOX,        //mouse在checkbox上移动
	MSGIO_MOUSEMOVE_NOTIP,             //关闭提示框
	MSGIO_EDIT_RETURN,                 //编辑框被回车确认
	MSGIO_NOTEBOOK_RETURN,             //记事本控件被回车确认
	MSGIO_ICONLIST_MOUSEMOVE,          //ICONLIST MOUSE MOVE
	MSGIO_ICONLIST_LBDOWN,             //图标列表单击
	MSGIO_ICONLIST_LDBCLICK,           //图标列表双击
	MSGIO_COLORLIST_LBDOWN,            //颜色选择列表单击
	MSGIO_COLORLIST_MOUSEMOVE,         //颜色选择列表 MOUSE MOVE
	MSGIO_STRINGLIST_LBDOWN,           //字符列表被单击
	MSGIO_STRINGLIST_LDBCLICK,         //字符列表被双击
	MSGIO_STRINGLIST_MOUSEMOVE,        //字符列表mouse move
	MSGIO_ICONSTRINGLIST_LBDOWN,       //图标字符列表被单击
	MSGIO_ICONSTRINGLIST_LDBCLICK,     //图标字符列表被双击
    MSGIO_ICONSTRINGLIST_RBDOWN,       //图标字符列表被右击
	
	//图标拖动事件, wParam: HIWORD: fromListID LOWORD: fromListIconIdx
	//              lParam: HIWORD: toListID   LOWORD: toListFrameID
	MSGIO_ICONMOVE,                    //移到图标列表内
	MSGIO_ICONMOVETO_DESKTOP,          //图标移到桌面上
	MSGIO_TEXTBAR_LBDOWN,              //点击textbar的消息, wParam为控伯id, lParam为串id
	MSGIO_BEFOREDIALOGRESIZE_GET_MAXDIALOGSIZE, //在改变对话框大小之前，用户设置最大的改变尺寸
	MSGIO_DIALOG_RESIZE,               //对话框大小改变消息
	MSGIO_DIALOGTITLE_DBCLICK,              //在对话框标题栏上双击(无标题对话框整个都是标题)
	MSGIO_MOUSE_CHANGE,                //mouse外观变化, wParam: UI_MOUSE_TYPE类型
	MSGIO_DIALOG_RETURN_KEYDOWN,       //对话框的键盘回车按下事件,
	MSGIO_ICONEFFECT_PLAYEND,          //图标特效播放完毕 wParam: 图标列表的指针, lParam:图标的索引
	MSGIO_MAPICON_MOUSEMOVE,           //mouse在小地图内正在显示的图标内 wParam:fdc_mapIcon指针
	MSGIO_MAPICON_LBUTTON,             //mouse点击小地图内正在显示的图标
	MSGIO_MAPICON_DBLCLK,              //双击小地图图标
	MSGIO_MAP_DBCLICK,                 //在小地图中双击, wParam:
	MSGIO_MAP_MOUSEMOVE,               //在小地图中移动,
	MSGIO_MAP_RDBCLICK,                //在小地图中右双击
	MSGIO_MAP_LRCLICK,                 //mouse在小地图上左右健同时按下
	MSGIO_LAYERGROUPITEM_MOUSEMOVE,    //mouse在人物条目上移动
	MSGIO_LAYERGROUPITEM_LBDOWN,       //mouse在人物条目上单击
	MSGIO_LAYERGROUPITEM_LDBCLICK,     //mouse在人物条目上双击
	MSGIO_LAYERGROUPITEM_LBUP,         //mouse在人物条目上单击抬起
	MSGIO_LAYERGROUPITEM_RBDOWN,       //mouse在人物条目上右按下
	MSGIO_ACTIVE_CHAREDIT,             //激活聊天的激活框事件
	MSGIO_USER_CLOSE_WND,              //用户可以关闭窗体，处理系统消息后事件
	MSGIO_HOTLINK_LBDOWN,              //点击超文本控件

	//内部自已的消息-----------------------------------------------
	MSG_SCROLL_UPBTN_LUP = MSG_FLAG,   //滑动条上按钮抬起
	MSG_SCROLL_UPBTN_LDOWN,            //滑动条上按钮按下
	MSG_SCROLL_DOWNBTN_LUP,            //滑动条下按钮抬起
	MSG_SCROLL_DOWNBTN_LDOWN,          //滑动条下按钮按下
	MSG_SCROLL_BLOCK_LDOWN,            //滑动条中块被点下
	MSG_SCROLL_UPDATEPOS,              //滑动条向父窗体发送定位更新消息
	MSG_TAB,                           //TAB消息，向父窗体发送
	MSG_RETURN_KEYDOWN,                //内部处理回车事件
	MSG_PUCKER,                        //对话框折叠
	MSG_UPDATE_DRAG,                   //对话框的拖动后更新
	MSG_DIALOG_RESIZE,                 //对话框大小改变消息,内部处理
	MSG_COMBOBOX_DROPBTN_LDOWN,        //复合选择框的按下事件
	MSG_COMBOBOX_STRINGLIST_ITEM_SELECT, //复合选择框的字符列表条目被单击选择
	MSG_MAPICON_ACTIVE,                //小地图上图标激活消息
	MSG_LAYERBUTTON_LUP,               //层次按钮点击释放 wParam:this lParam: idx
	MSG_ICONLIST_UPDATEMOUSEMOVE,      //图标列表拖放后更新tooltip
	MSG_LIMIT_DIALOGSIZE_FROM_CHILD,   //某控件要求限制其父(对话框)的大小
	MSG_HOTLINK_BROTHER_SYNC_WNDSTATE, //超链接兄弟控件消息同步(窗体状态)
	MSG_HOTLINK_BROTHER_SYNC_VISITED,  //超链接兄弟控件消息同步(是否访问状态)



	
	MSG_END                            //结尾消息符
} MSG_ID;

//内部消息结构
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

//! 立即处理消息回调函数原型
typedef int ( *UI_MSGCALLBACKIMM)(UIMSG&);

//! 界面默认保存文件名
#define UISAVENAME "user\\default.uis"

//! 每一个控件可以使用的名字的最大长度。
#define MAX_CRLNAMELEN      64

//! 文字与窗体边框的默认距离(pixel)
#define TEXT_SPACE   3   

typedef enum GAMESTEP
{
	GS_LOGIN,			//! 登录时
	GS_SELS,			//! 选择服务器
	GS_CREC,			//! 创建人物
	GS_SELC,            //! 选择人物
	GS_LOADING,         //
	GS_RUN,				//! 运行
	GS_PUBLIC,          //! 公用

} GAMESTEP;


//! 文本的对齐方式
enum FDCTEXT_DISFORMAT
{
	TEXT_LEFT = 0,  //左
	TEXT_CENTER,    //中
	TEXT_RIGHT,     //右
	TEXT_TOPLEFT,   //左上
	TEXT_TOPCENTER, //中上
	TEXT_TOPRIGHT,  //右上

	TEXT_USERPOS, //用户指定，设置此类型要指定文本的位置
};

//! 界面库中可改变的mouse形状
enum UI_MOUSE_TYPE
{
	UMT_DEFAULT,    //默认的
	UMT_HORIZONTAL, //水平的
	UMT_VERTICAL,   //垂直的
	UMT_CROSS,      //十字的
	UMT_SIZENWSE,   //西北到东南方向的改变大小
	UMT_BUTTONHAND, //放在button上变为手形
	UMT_MAP_MOVE,   //在地图上移动的鼠标形状
};

//! 界面库中所包含控件的类型。
enum FDCCTL_TYPE 
{ 
	CTL_ROOT = 0,          //! 根窗体,一个界面组只能有一个
	CTL_WND,               //! 普通窗体　
	CTL_STATIC,            //! 静态控件
	CTL_BUTTON,            //! 按钮
	CTL_PUCKERBUTTON,      //! 折叠按钮
	CTL_CHECKBOX,          //! 复选框
	CTL_EDITBOX,           //! 编辑控件
	CTL_NOTEBOOK,          //! 记事本控件(多行编辑框)
	CTL_DIALOG,            //! 对话框
	CTL_PIC,               //! 图片控件
	CTL_CONTAINER,         //! 图标容器控件(内部使用)
	CTL_ANIMPIC,           //! 动画图片控件
	CTL_SCROLLBAR,         //! 滑动条
	CTL_PROGRESS,          //! 进度条
	CTL_CURSOR,            //! 光标
	CTL_ICON_LIST,         //! 图标列表
	CTL_STRING_LIST,       //! 字符串列表
	CTL_COLOR_LIST,        //! 颜色选择列表
	CTL_ICONSTRING_LIST,   //! 图标字符串列表
	CTL_TOOLTIP,           //! 工具提示
	CTL_MAP,               //! 地图控件
	CTL_MAPICON,           //! 地图上图标控件
	CTL_TEXTBAR,           //! 文本公告条
	CTL_TEXTBAR_V,         //! 文本公告条_竖
	CTL_RESIZEBAR,         //! 对话框拖拽条控件
	CTL_COMBOBOX,          //! 复合选择框
	CTL_QQ,                //! QQ控件
	CTL_LAYER,             //! 层次按钮
 	CTL_LAYERGROUP,        //! 层次按钮组
	CTL_LAYERGROUPITEM,    //! 层次按钮组条目
	CTL_HOTLINK,           //! 热链接	

	CTL_SCREENEFFECT_FOLDER,                //! 屏幕特效文件夹管理用，本身一般不显示
	CTL_SCREENEFFECT_COMBO_ELEMENT_FOLDER,  //! 屏幕特效第2级文件夹(复合特效文件夹)
	CTL_SCREENEFFECT_ELEMENT,               //! 屏幕特效元素

	CTL_ICONEFFECT_FOLDER,                  //! 图标特效文件夹,附属于图标上，不独立使用
	CTL_ICONEFFECT_PATCH,                   //! 图标特效元素

	CTL_LIST_ACTIVEWND,                     //! 列表框激活窗体

	CTL_STRINGLIST_GROUP,                   //! 字符列表分组层

	//以下类型控件要通过其管理器删除
	CTL_LIBICON,           //! 库图标


	CTL_ERROR              //! 无类型
};

//! 小地图上显示的图标类型
typedef enum MAPICONTYPE
{
	MIT_HERO,       //主角
	MIT_DIR,        //方向
	MIT_TARGET,     //目标
	MIT_NPC,        //NPC
	MIT_MONSTER,    //怪物
	MIT_TEAMMEMBER, //队员
	MIT_TEAMLEADER, //队长
	MIT_GATE,       //门点
	MIT_USER,       //用户定义的

} MAPICONTYPE;

//! 几种窗口的状态
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
  V_STYLE,//纵
  H_STYLE,//横
} VHSTYLE;

//! 列表的样式
#define COLSTYLE_NAMELEN 10
#define MAX_COL 30
typedef struct LIST_STYLE{
	int border;                               //! 有无框　
	int border_x;                             //! x留白
	int border_y;                             //! y留白
	char tex_border[256];                     //! border纹理
	int scrollbar;                            //! 是否有滑动条，1:在右 0:没有
	int maxrow;                               //! 最大的容量行数,逻辑行
	int rowcount;                             //! 行数
	int colcount;                             //! 列数
	int rowheight;                            //! 行高pix
	int colwidth[MAX_COL];                    //! 列宽pix
	char colstyle[MAX_COL][COLSTYLE_NAMELEN]; //! 列样式,同一列只有一种控件类型
	int rowspace;                             //! 行空白pix
	int colspace;                             //! 列空白pix
	char tex_style[256];                      //! 内含控件用纹理名
	char tex_active[256];                     //! 激活态纹理名

	osVec2D texuv_active[2];           //! 激活态纹理坐标
	osVec2D texuv_border_up[2];        //! 边框坐标
	osVec2D texuv_border_bottom[2];    //! 边框坐标
	osVec2D texuv_border_left[2];      //! 边框坐标
	osVec2D texuv_border_right[2];     //! 边框坐标

	osVec2D texuv_style[MAX_COL][2];          
	DWORD bgActiveColor;                      //! mouse over背景激活色
	DWORD bgClickActiveColor;                 //! mouse click背景激活色

	int showText;                             //! 是否有文字序号
	DWORD textColor;                          //! 序号文字色
	int textOff_x;                            //! 文本x偏移
	int textOff_y;                            //! 文本y偏移

	int dataMode;                             //! 存储模式：图标列表 0:物品列表样式(无间隔),1:快捷栏样式(任意位置)
	                                          //            字符列表 0:由上到下(文章模式)， 1:由下到上(聊天框模式)  
	                                          //            颜色列表:0:物品列表样式(无间隔),1:快捷栏样式(任意位置) 
} LIST_STYLE;

//! 颜色列表样式
typedef struct COLORLIST_STYLE
{
	char bindTexure[64];    // 邦定纹理

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

//! 图标列表的数据存储样式
typedef enum ICONLIST_STYLE
{
	//! 快捷栏样式,可以有间隔，位置随意
	ILS_SHORTCUR,

	//! 物品栏样式,onebyone依次
	ILS_ONEBYONE,

} ICONLIST_STYLE;

//! 字符串列表的数据存储样式
typedef enum STRINGLIST_STYLE
{
	//! 从上向下，依次
	SLS_ONEBYONE,

	//! 从下向上，推入列表, 用于聊天显示框。
	SLS_PUSH,

} STRINGLIST_STYLE;

//! 字符列表框的背景高亮模式
typedef enum STRINGLIST_HIGHLIGHTMODE
{
	//! 移动高亮
	SHL_MOVEHIGHLIGHT,

	//! 点击高亮
	SHL_CLICKHIGHLIGHT,

	//! 无高亮
	SHL_NOHIGHLIGHT,

} STRINGLIST_HIGHLIGHTMODE;


//! 滑动条的样式
#define SCROLLBAR_WIDTH 12
typedef struct SCROLLBAR_STYLE {
	int scb_style;         //! 0竖,1横
	int scb_range;         //! 范围，在横向滑条时使用
	int scb_unitLen;       //! 竖：代表元素的高度，横：代表元素的宽度
	int scb_default_blockLen;//! 默认块的长度
	char scb_tex[128];     //! 纹理名
	osVec2D scb_Btnlt_uv[2];   //! 左上按钮的纹理坐标
	osVec2D scb_Btnrb_uv[2];   //! 右下按钮的纹理坐标
	osVec2D scb_Btnlt_uv2[2];   //! 左上按钮的纹理坐标2
	osVec2D scb_Btnrb_uv2[2];   //! 右下按钮的纹理坐标2

	osVec2D scb_Blocklt_uv[2]; //! 滑块左上部的纹理坐标
	osVec2D scb_Blockrb_uv[2]; //! 滑块右下部的纹理坐标
	osVec2D scb_Blockm_uv[2];  //! 滑块中部的纹理坐标
	osVec2D scb_Blockm_uv2[2];  //! 滑块中部的纹理坐标
} SCROLLBAR_STYLE;

//! 复选框的样式
typedef enum CHECKBUTTON_TEXTMODE
{
	CBTM_RIGHT,           // 在底图的右侧
	
	CBTM_OVERPIC_LEFT,    // 在底图之上左对齐写文字
	CBTM_OVERPIC_MIDDLE,   // 在底图之上中对齐写文字
	CBTM_OVERPIC_RIGHT   // 在底图之上右对齐写文字

} CHECKBUTTON_TEXTMODE;

//! 滚动文本条的样式(楧)
typedef struct TEXTBAR_STYLE
{
	float  move_speed;       // 移动的速度(象素/秒)
	int    space_node_cout;  // 两段串间的空白结点个数 ( 6pix/个 )
} TEXTBAR_STYLE;

//! 枚举拖动控件在对话框位置
typedef enum RESIZEBAR_POS
{
	RP_TOP          = 0,   //上边缘
	RP_BOTTOM       = 1,   //下边缘 
	RP_LEFT         = 2,   //左边缘
	RP_RIGHT        = 3,   //右边缘
	RP_RBCORNER     = 4,   //右下角

	RP_NUM,
};

//! 改变对话框大小控件的样式
typedef struct RESIZEBAR_STYLE
{
	RESIZEBAR_POS	posType;
} RESIZEBAR_STYLE;

//! 复合选择框的样式
typedef struct COMBOBOX_STYLE
{
	char              tex[128];         //纹理名,包括stringlist使用的纹理
	int               dropListMaxRow;   //下拉列表的最大显示行数
	FDCTEXT_DISFORMAT textDisFormat;    //内容文本的对齐方式

	//内置控件类型
	LIST_STYLE        stringListStyle;  //字符列表样式
	SCROLLBAR_STYLE   scrollBarStyle;   //滑动条样式
} COMBOBOX_STYLE;

//! 最多的动画关键帧数
#define    MAX_ANIM_FRAME        20

//! 动画图标样式
typedef struct ANIMICONCTL_STYLE {

	//! 动画使用的纹理文件名
	char tex[256];

	//! 第一帧纹理坐标
    osVec2D texuv[2];

	//! 帧数
	int m_MaxFrameCount;

	//! 每帧占用的单位帧个数
	int m_FrameLen[ MAX_ANIM_FRAME ];

	//! 播放速度,(帧/每秒)
	int m_FrameCountPerSec;

} ANIMICONCTL_STYLE;

//! 滚动纹理控件(暂时没有用)
typedef struct SCROLLTEXTURE_STYLE 
{
	char name[64];     //名称
	char texname[64];  //纹理名称
	osVec2D uv[2];     //纹理坐标
	SIZE size;         //大小
	int  movedir;      //移动方向, 0:不可移动 1:向上 2:向下 3:向左 4:向右

} SCROLLTEXTURE_STYLE;

//! 竖直滚动文本
typedef struct TEXTBARV_STYLE
{
	float m_speed;
	int   m_space;
} TEXTBARV_STYLE;

//! 层次按钮条目
typedef struct LAYERGROUUPITEM
{
	char key[ 32 ];         //索引key
	int idInIconLib;        //在线的图标ID,在图标库中 (16*16)
	int idInIconLibOffline; //离线的图标ID,在图标库中 (16*16)
	char text[ 64 ];        //文本

	DWORD m_textItemColorDefault;
	DWORD m_textItemColorDown;
	DWORD m_textItemColorOver;

} LAYERGROUUPITEM;

//! 图标状态
typedef enum LAYERGROUPITEMSTATE
{
	LGIS_ONLINE,   //在线
	LGIS_OFFLINE,  //离线
	LGIS_UPLINE,   //上线闪烁
	LGIS_HIDE,     //隐身
} LAYERGROUPITEMSTATE;

//! QQ人物状态
typedef struct QQHUMANSTATE
{
	LAYERGROUPITEMSTATE iconState;
	bool bMsg;

} QQHUMANSTATE;

//! 层叠按钮组的样式
typedef struct LAYERGROUPSTYLE
{
	char tex[64];               // 纹理名
	osVec2D uvTitleButtonBg[2]; // 按钮背景坐标

	osVec2D uvLeft[2];          // 标题按钮的皮肤纹理坐标-左
	osVec2D uvRight[2];         // 标题按钮的皮肤纹理坐标-右
	osVec2D uvMiddle[2];	    // 标题按钮的皮肤纹理坐标-中

	osVec2D uvLeftDown[2];          // 标题按钮的皮肤纹理坐标-左(按下)
	osVec2D uvRightDown[2];         // 标题按钮的皮肤纹理坐标-右(按下)
	osVec2D uvMiddleDown[2];	    // 标题按钮的皮肤纹理坐标-中(按下)

	int textFormat;             // 文本对齐方式 0:left 1:middle 2:right
	int itemSpace;              // 条目间距

	DWORD m_textTitleColorDefault;
	DWORD m_textTitleColorDown;
	DWORD m_textTitleColorOver;



} LAYERGROUPSTYLE;

//! 层叠按钮的样式
typedef struct LAYERBUTTONSTYLE
{
	char tex[64];     // bind texture
	osVec2D uvBg[2];  // 背景纹理坐标
	//int itemSpace;    // 条目间距,改为编辑器控制，此处不再使用

} LAYERBUTTONSTYLE;

//! 一个页
typedef struct LAYER_PAGE
{
	char name[64];        //页的名字

	char tex[64];         //checkBox纹理名
	osVec2D uvCheck1[2];  //checkBox 1态
	osVec2D uvCheck2[2];  //checkBox 2态

	LAYERBUTTONSTYLE layerStyle; //层次按钮的样式

} LAYER_PAGE;

//! QQ的样式
typedef struct QQ_STYLE
{
	char tex[64];     // qq使用的纹理
	osVec2D uvBg[2];  // 背景纹理坐标
	int itemSpace;    // 条目间距

	DWORD textTitleColorDefault;
	DWORD textTitleColorDown;
	DWORD textTitleColorOver;

	DWORD textItemColorDefault;
	DWORD textItemColorDown;
	DWORD textItemColorOver;

	//滚动条样式
	SCROLLBAR_STYLE scb_style;

} QQ_STYLE;

//! 按钮样式
typedef struct BUTTONSTYLE
{
	int bSkin;        // 0:普通按钮, 1:蒙皮按钮

	char skinTex[32]; //蒙皮纹理与激活态共用
	int btnState;     //几态按钮 0:2态, 1:3态

	// 公用
	//----------------------------------------
	// 按钮
	DWORD colorDefault;
	DWORD colorDown;
	DWORD colorGray;
	BYTE  alphaDefault;
	BYTE  alphaDown;
	BYTE  alphaGray;

	// 文本
	DWORD textColorDefault;
	DWORD textColorDown;
	DWORD textColorGray;

	DWORD textColorOver;

	BYTE  textAlphaDefault;
	BYTE  textAlphaDown;
	BYTE  textAlphaGray;



	// 普通按钮数据
	//------------------------------------------
	// 坐标
	osVec2D uvDefault[2];
	osVec2D uvDown[2];

	//加入编辑器指定的第3态(MouseMove)
	int   bForceMoveState;
	char  buttonMoveStateTex[64];
	osVec2D uvMouseMove[2];

	// 蒙皮数据
	//------------------------------------------
	osVec2D uvDefaultLeft[2];
	osVec2D uvDefaultRight[2];
	osVec2D uvDefaultMiddle[2];

	osVec2D uvDownLeft[2];
	osVec2D uvDownRight[2];
	osVec2D uvDownMiddle[2];
	int unitLen;


} BUTTONSTYLE;

//! 提示工具条的样式
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

//! 进度条的状态
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

//! 矩形类
typedef enum HOTLINKEVENTTYPE
{
	HLE_HELPSYSTEMADDR,    //帮助系统跳转
	HLE_USEREVENT,         //返回给上层用户的事件

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
	RECT        rect;                        //占用的大小

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

//! 类前置声名
class fdc_baseCtl;
class fdc_iconCtl;
class fdc_scrollBar;

//! 磁性窗体
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

//! 窗口基类
class fdc_baseCtl
{
public:
	fdc_baseCtl();
	virtual ~fdc_baseCtl();

private: //attribute
	char  m_szCtlName[MAX_CRLNAMELEN];      //! 窗口的名字,当索引用
	DWORD m_dwID;                           //! 窗口的ID,索引动态创建的窗口对象
	bool  m_bVisible;	                    //! 当前控件是否可见。
	bool  m_bAcceptTab;                     //! 是否接受TAB键消息。
	int   m_iTabOrder;                      //! TAB键的顺序值。
	os_stringDisp m_text;                   //! 窗口的标题

	osVec2D m_textOffset;                   //! 文本的相对父的坐标
	FDCTEXT_DISFORMAT m_disFormat;          //! 标题对齐的方式
	FDCCTL_TYPE  m_ctlType;                 //! 当前控件的类型。解决动态时获得类型，
	fdc_baseCtl * m_pParent;                //! 当前控件父控件的指针
	osVec2D m_scrOffsetPos;                 //! 屏幕坐标系 
	osRect  m_rectCtlArea;                  //! 控件对应的区域,每一个控件都必须占用一个矩形的区域。其成员top,left为左上顶点的偏移值  	//l:offset_x, t:offset_y, r:width+l, b:height+t  
	bool m_bCanDrag;                        //! 当前窗口是否可以拖动。
	osRect m_CanDragRect;                   //! 能使窗口拖动的有效矩形区
	osRect m_textRect;                      //! 显示文字矩形区
	int m_textShadow;                       //! 是否有文本的阴影
public:
	os_screenPicDisp  m_screenPicDisp; 	    //! 显示控件的数据，顶点，纹理, 正常顶点用于update()
	std::vector<fdc_baseCtl*> m_vecChilds;  //! 控件对应的子控件列表。
	int m_row_space;                        //! 临时，行间距

public: //method
	DWORD get_ID();                         //! 获得窗口的ID
	void set_tabEnable(bool _b);            //! 打开TAB
	bool get_tabEnable();                   //! 得到接受TAB键消息。
	void set_tabOrder(int _i);              //! 设置Tab序号
	int  get_tabOrder();                    //! 得到TAB键的顺序。
	FDCCTL_TYPE get_ctlType();              //! 取得控件的类型。
	void  set_name(const char* _wnd_name);  //! 设置窗口的名字。
	const char* get_name() const;           //! 得到窗口的名字。
	virtual void set_visible(bool _b);              //! 设置是否可见(是否渲染)
	virtual bool get_visible();                     //! 得到控件是否可见。
	bool IsShow();                          //! 当前是否显示渲染
	void set_wndHeight( int _height );      //! 设置窗口的高度
	void set_wndWidth( int _width );        //! 设置窗口的宽度
	int get_wndHeight();                    //! 得到窗口的高度
	int get_wndWidth();                     //! 得到窗口的宽度
	virtual void   set_text( const char * _text,
		FDCTEXT_DISFORMAT _format = TEXT_CENTER, 
		int _user_x=0, int _user_y=0);      //! 设置窗口的标题文本本
	virtual char * get_text();              //! 返回窗口的标题文本
	virtual os_stringDisp& get_stringDisp();//! 返回整个文本显示结构
	virtual void   set_color(DWORD _color); //! 设置窗口颜色
	virtual DWORD  get_color();             //! 获得窗口颜色
	virtual void   set_textColor(DWORD _textColor); //! 设置窗口文本的顡色
	virtual DWORD  get_textColor();                 //! 返回窗口文本的顡色
	void set_textRect(osRect& _textRect);           //! l,t,为0,显示文本区的大小
	osRect& get_textRect();                         //! 获得文本区的矩形
	virtual void   set_alpha(BYTE _a);              //! 设置窗口的透明度 0-255
	virtual BYTE   get_alpha();                     //! 获得窗口的透明度
	void set_textAlpha(BYTE _a);                    //! 设置文本的透明度
	BYTE get_textAlpha();                           //! 获取文本的透明度
	void set_offset( int _x, int _y );              //! 相对于父窗体的偏移
	void set_offset( osVec2D& _offset );            //! 相对于父窗体的偏移
	osVec2D get_offset();                           //! 获取相对于父窗体的偏移
	void set_scrOffset(int _x, int _y);             //! 设置相对于屏幕的偏移
	void set_scrOffset(osVec2D& _scrOffset);        //! 设置相对于屏幕的偏移
	osVec2D& get_scrOffset();                       //! 获取相对于屏幕的偏移
	void set_canDrag(bool _bCanDrag);               //! 设置是否可拖动
	bool get_canDrag();                             //! 获得是否可拖动
	void set_textShadow( int _bShadow );            //! 设置文字是否有阴影显示, 1,显示 0.无
	bool get_textShadow();                          //! 是否显示文本的阴影
	void set_textOffset( osVec2D& _textOffset );    //! 设置文本的偏移
	osVec2D& get_textOffset();                      //! 获取文本的偏移
	void move_wnd( osVec2D& _offset );              //! 移动窗体
	void set_rect(osRect& _rect);                   //! 设置窗体的矩形
	osRect& get_rect();                             //! 获得窗体的矩形大小
	osRect get_scrRect();                           //! 获得窗体的绝对矩形区
	void    set_canDragRect(osRect& _rect);         //! 设置可拖动的矩形区
	osRect& get_DragRect();                         //! 获得可拖动的矩形区
	void    set_parent(fdc_baseCtl* _pParent);      //! 设置父窗体的指针
	fdc_baseCtl * get_parent();                     //! 获得父窗体的指针
	void set_textFormat( FDCTEXT_DISFORMAT _disFormat, 
		int _userx = 0, int _usery = 0 );           //! 设定文字格式
	FDCTEXT_DISFORMAT get_textFormat();             //! 获得文字格式
	void set_vertexInfo(osRect& _rect,              //! 设定顶点数据、id、uv
		int _texID = -1, osVec2D* _uv = 0 );        //! uv数组大小2个,0:左上 1:右下
	void set_vertexInfo2(osRect& _rect);            //! 设定顶点数据     
	void set_vertexInfo(osVec2D* _uv);              //! 设定顶点uv
	void set_vertexInfo( int _texID, osVec2D* _uv=0 );//! 设置顶点的纹理id,uv
	int get_texID();                                //! 获得纹理坐标的id

	virtual  void set_default();                    //! 点击恢复窗体的默认状态
	virtual  void set_default_parent();             //! 设定父为默认状态（comboBox ）
	virtual  void release_capture();                //! 释放mouse capture
	virtual  void set_defaultMoveEvent();           //! 移动出恢复默认状态
	virtual  void set_preFocusClose();              //! 处理前一窗体的附属对话框关闭事件

	virtual void msgProc( UIMSG& _uiMsg );          //! 窗口的消息处理函数

	//! 窗体的初始化
	virtual void init( FDCCTL_TYPE _ctl_type, char*_wnd_name, osRect& _wnd_rect, 
                       DWORD _color, bool _canDrag, osRect& _canDrag_rect, fdc_baseCtl * _pParent, 
                       bool _bVisible = true, FDCTEXT_DISFORMAT _disFormat = TEXT_CENTER );
	virtual void   update();                        //! 更新窗体坐标等状态
	virtual void   pre_render();                    //! 预渲染，推入管道,稍后处理
	virtual void   pre_renderFont();                //! 渲染字体
	virtual void   resize( int _newWidth, int _newHeight ); //! 改变控件大小
	virtual void   stop_update(); //停止更新
	virtual void   clear_data();  //清空控件的数据

	//! 窗体的查找
	//@{
	//! 按名字查找
	fdc_baseCtl * find_wnd( char * _wnd_name, fdc_baseCtl *  _findRoot );
	bool          find_wnds( char * _wnd_name, fdc_baseCtl *  _findRoot );
	//! 按坐标点查找
	fdc_baseCtl * find_wnd( POINT& _pt, fdc_baseCtl* _findRoot );
	void          find_wnds( POINT& _pt, fdc_baseCtl* _findRoot );
	//! 按id查找
	fdc_baseCtl * find_wnd( DWORD _id, fdc_baseCtl* _findRoot );
	bool          find_wnds( DWORD _id, fdc_baseCtl* _findRoot );
	//@}

	//! 查找指定的窗体是否为其子
	bool isMyChild( fdc_baseCtl* _pTarget );

	//! 获得文字最小包围矩形
	osRect get_textBoundRect(const  char * _text);

	//磁性窗体
	//@{
	CStickyDragManager m_dragManager;
	void    init_sticky();
	osRect& move_sticky();
	//@}

	//! 调试打印输出
	void printNode( fdc_baseCtl* _findRoot );
};

//! 静态控件接口类
struct I_staticCtl : public fdc_baseCtl
{
	//! 设置显示的文本
	virtual void   set_dispText( const char* _text,
		           FDCTEXT_DISFORMAT _format = TEXT_USERPOS, int _user_x =0 , int _user_y = 0 ) = 0;

	//! 得到当前静态控件上显示的文字。
	virtual const char*  get_dispText( void ) = 0;

	//! 设置文本的颜色
	virtual void set_dispTextColor( DWORD _textColor ) = 0;

	//! 文本的反色
	virtual void reverseColor() = 0;

	//! 设置多彩的文本
	virtual void set_multiColorText( char* _text ) = 0;

	//! 获得多彩的文本
	virtual char* get_multiColorText() = 0;
};

//! 按钮接口
struct I_buttonCtl : public fdc_baseCtl
{

	//! 设置按钮文本
	virtual void   set_dispText( const char* _text,
		           FDCTEXT_DISFORMAT _format = TEXT_CENTER, int _user_x =0 , int _user_y = 0  ) = 0;

	//! 得到当前按钮上显示的文字。
	virtual char*  get_dispText( void ) = 0;

	//! 设定按钮状态
	virtual void set_ButtonState( WND_STATE _bs ) = 0;

	//! 获得按钮的状态
	virtual WND_STATE get_ButtonState() = 0;

	//! 获得按钮的提示文本
	virtual char * get_tipText() = 0;

	//! 设定按钮的颜色
	virtual void set_buttonColor( DWORD _default, DWORD _down, DWORD _gray, 
			  			  BYTE _alphaDefault = 0xff, BYTE _alphaDown = 0xff, BYTE _alphaGray = 0x80 ) = 0;

	//! 设置按钮的弹出绑定对话框
	virtual void set_popBindDlg( fdc_baseCtl * _pDlg ) = 0;

	//! 再次更新tooltip和按钮的状态
	virtual void update_stateAgain() = 0;

	// 闪烁状态, _interval：间隔(秒)
	virtual void set_flash( bool _b, float _interval ) = 0;
};

//! 按钮的实现类。
class fdc_buttonCtl : public I_buttonCtl
{
public:
	fdc_buttonCtl();
    virtual ~fdc_buttonCtl();

	//! 设定样式
	void set_style( BUTTONSTYLE _style );

	//! 设定按钮的颜色
	void set_buttonColor( DWORD _default, DWORD _down, DWORD _gray, 
									 BYTE _alphaDefault = 0xff, BYTE _alphaDown = 0xff, BYTE _alphaGray = 0x80 );
	//! 初始化
	void init( FDCCTL_TYPE _ctl_type, char*_wnd_name, osRect& _wnd_rect, 
                       DWORD _color, bool _canDrag, osRect& _canDrag_rect, fdc_baseCtl * _pParent, 
                       bool _bVisible = true, FDCTEXT_DISFORMAT _disFormat = TEXT_LEFT );

	void fdc_buttonCtl::update();

	//! 按钮消息处理
    void msgProc( UIMSG& _uiMsg );

	//! 处理自身产生的消息
    void process_myEvent( UIMSG& _uiMsg );

	void set_text( const char * _text, 
						   FDCTEXT_DISFORMAT _format = TEXT_CENTER, int _user_x = 0, int _user_y = 0 );

	//! 设置显示的文本
    void set_dispText( const char* _text,FDCTEXT_DISFORMAT _format = TEXT_CENTER,
	                   int _user_x =0 , int _user_y = 0  );
    
	//! 获取显示的文本
    char* get_dispText( void );

	//! 增加按钮up的事件
	void add_event( UIMSG _cmd );

	//! 增加按钮down的事件
	void add_DownEvent( UIMSG _cmd );

	//! 设置按钮的按下状态
	void set_downState();

	//! 设置按钮为默认状态
    void set_default();

	//! 处理前一窗体的附属对话框关闭事件
	void set_preFocusClose();

    //! 切换按钮的状态
    virtual void set_ButtonState( WND_STATE _bs );

	//! 获得按钮的状态
	WND_STATE get_ButtonState();

	//! 设置按钮的提示文本
	void set_tipText( char * _str );

	//! 获得按钮的提示文本
	char * get_tipText();

	//! 释放mouse capture
	void  release_capture();

	//! 设回默认态
	void set_defaultMoveEvent();

	//! 设定统一的移动高亮纹理
	void set_moveHightLightTexture( char * _texName );

	//设按钮的按下文本偏移量
	void set_buttonDownTextOffset( int _x, int _y );

	//! 设置mouse的形状
	void set_mouseShape( UI_MOUSE_TYPE _mt );

	//! 设置按钮的弹出绑定对话框
	void set_popBindDlg( fdc_baseCtl * _pDlg );

	//! 再次更新tooltip和按钮的状态
	void update_stateAgain();

	//! 闪烁状态, _interval：间隔(秒)
	void set_flash( bool _b, float _interval );

private:

	//! 关联的命令，按钮只处理单击事件,以后可能会加入更新其它相关联的界面的指令？
	std::vector<UIMSG> m_vecCmd; //up event
	std::vector<UIMSG> m_vecDown;//down event

	//! 当前的状态
	WND_STATE m_state;

    //! 状态更新锁定，为防止mouse移动时多次存储状态，状态被覆盖的情况。
    bool m_bLock; 

	//! 是否被mouse捕获
	bool m_bCapture;

	//! 提示文本
	char m_tipText[256];

	//! 激活图片
	fdc_iconCtl * m_pActive;

	//! mouse move纹理名
	char m_moveHightLightTexture[64];

	//! 按下时文本偏移
	int	m_textOffsetX;
	int	m_textOffsetY;

	int m_buttonDefaultTextOffsetX;
	int m_buttonDefaultTextOffsetY;
	
	int	m_buttonDownTextOffsetX;
	int	m_buttonDownTextOffsetY;

	//! 按下弹出绑定对话框
	fdc_baseCtl * m_pPopBindDlg;

	//! 当时闪烁状态
	bool m_bFlashState;

	//! 闪烁状态下的间隔时间
	float m_flashInterval;

	//! 闪烁时间记录
	float m_flashStartTime;

public:
	
	//WND_STATE m_oldState;
	BUTTONSTYLE m_style;

	//! 统一的按钮样式
	static BUTTONSTYLE m_publicStyle;
	static char moveHightLightTexture[64];

	//! 支持编辑器指定的MouseMove态的纹理ID
	int m_mouseMoveStateTexID;
};

//! 复选框
struct I_checkButtonCtl : public fdc_baseCtl
{
	//! 获得复选框是否为选中状态
	virtual bool get_check() = 0;

	//! 设置复选框
	virtual void set_check( bool _check ) = 0;

	//! 得到复选框文本
	virtual char * get_checkBtnText() = 0;

	//! 设置1态的文本
	virtual void set_defaultStateText( char * _text ) = 0;

	//! 设定2态check文本
	virtual void set_checkStateText( char * _text ) = 0;

	//! 得到2态的复选框文本
	virtual char * get_checkStateText() = 0;

	//! 设置复选框文本模式
	virtual void set_textMode( CHECKBUTTON_TEXTMODE _cbtm ) = 0;
	
	//! 得到checkbox的默认态纹理坐标
	virtual WINTEXINFO get_defaultStateUv() = 0;

	//! 用纹理坐标数据查询是否为当前checkbox
	virtual bool is_me( WINTEXINFO& _info ) = 0;

	//! 获得提示文本
	virtual const char * get_tipText() const = 0 ;

	//! 设置check color...
	virtual void set_checkButtonColor( DWORD _defaultColor, BYTE _defaultAlpha,
			   				           DWORD _checkColor,   BYTE _checkAlpha ) = 0;
};

//! 编辑控件
struct I_editCtl : public fdc_baseCtl
{
	//! 设置文本
	virtual void   set_dispText( const char* _text ) = 0;

	//!  得到当前编辑框内的字串。
	virtual char* get_dispText( void ) = 0;

	//! 得到编辑框内全部字串
	virtual const  char* get_allText() = 0;

	//! 激活或者失激活控件
	virtual void set_editActive(bool _b) = 0;

	//! 查看当前的编辑控件是否为激活状态
	virtual bool get_editActive() = 0;

	//! 最多容纳的英文字符数
	virtual void set_maxTotalNum( int _max ) = 0;
	
	//! 获得最多可容纳的英文字符个数
	virtual int get_maxTotalNum() = 0;

	//! 编辑控件是否是立即渲染的
	virtual void set_renderImm( bool _bRenderImm ) = 0;
};

//! 多行编辑框(记事本控件)
struct I_notebookCtl : public fdc_baseCtl
{
	//! 获得文本
	virtual const char* get_notebookText() = 0;

	//! 设置记事本中的文本
	virtual void set_notebookText( char* _text ) = 0;

	//! 设置记事本文本的颜色
	virtual void set_notebookTextColor( DWORD _color ) = 0;

	//! 设置激活与否
	virtual void set_notebookActive( bool _b ) = 0;

	//! 初始化时设置最大的可容纳字符数
	virtual void set_maxCharCount( int _maxCharCount ) = 0;
};

//! 滑动条接口, 横向
struct I_scrollBar : public fdc_baseCtl
{
	//! 设置滑动条的范围
	virtual void set_scbRange(int _range) = 0;

	//! 获得滑动条的最大范围
	virtual int get_scbRange() = 0;

	//! 设置滑动条的位置
	virtual void set_scbPos(int _pos) = 0;

	//! 获得滑动条的位置
	virtual int get_scbPos() = 0;
};

//! 图标类
struct I_iconCtl : public fdc_baseCtl
{
	//! 设置图标的颜色
	virtual void set_iconColor( DWORD _color ) = 0;

	//! 设置图标的透明度
	virtual void set_iconAlpha( BYTE _a ) = 0;
	
	//! 右下设置标号
	virtual void set_number( int _n, DWORD _color, FDCTEXT_DISFORMAT _format = TEXT_RIGHT ) = 0;
	
	//! 清空右下角的数字标号
	virtual void clear_number() = 0;

	//! 播放时间特效
	virtual bool play_timeEffect( float _totalTime, DWORD _color = 0xffffffff, float _flashTime = 1.f, float _startTime = 0.0f ) = 0;

	//! 删除时间特效
	virtual bool del_timeEffect( ) = 0;

	//! 查看是否在播时间特效, -1:索引无效, 0: 没有，1:正在播放
	virtual int is_playTimeEffect( ) = 0;

	//! 更改显示的数据
	virtual int set_icon( const char* _textureName, float _u1, float _v1, float _u3, float _v3 ) = 0;

	//! 改变控件大小
	virtual void resize( int _newWidth, int _newHeight ) = 0;

};

//! 容器类
struct I_containerCtl : public fdc_baseCtl
{
	//! 增加图标
	virtual void add_icon( int _idInLib ) = 0;

	//! 删除图标
	virtual void del_icon() = 0;
	
	//! 获得图标
	virtual I_iconCtl * get_icon() = 0;
};

//! 列表基类。可带右侧滚动条
class fdc_ListCtl : public fdc_baseCtl
{
public:
	fdc_ListCtl();
	virtual ~fdc_ListCtl();

public:

	//! 消息处理
    virtual void msgProc( UIMSG& _uiMsg );

	//! 设置list样式
	void set_style( LIST_STYLE& _style );

	//! 设置滑动条的样式
	void set_scrollbarStyle( SCROLLBAR_STYLE& _scrollbarStyle );

	//! 获取list样式
	LIST_STYLE& get_style();
	
	//! 设置最大的行数
	virtual void set_maxrow( int _maxrow );

	//! 初始化
	void init( FDCCTL_TYPE _ctl_type, char*_wnd_name, osRect& _wnd_rect, 
                       DWORD _color, bool _canDrag, osRect& _canDrag_rect, fdc_baseCtl * _pParent, 
                       bool _bVisible = true, FDCTEXT_DISFORMAT _disFormat = TEXT_CENTER );

	//! 查看列表控件是否有滚动条
	bool has_scrollBar();

	//! 设置滚动条的可视性(要修改列表控件内部各条目的大小，保证控件的大小不变)
	void set_scbVisible( bool _visible );

	//! 列表样式
	LIST_STYLE   m_style;

	//! 显示的起始行
	int m_curRow;	
	
	//! 列表滑动条样式                   
	SCROLLBAR_STYLE m_scrollbar_style;

	//! 选中激活框
	fdc_baseCtl * m_pActive;
	
	//! 滑动条
	fdc_scrollBar * m_pScrollBar;
};

//! 图标列表接口
struct I_iconListCtl : public fdc_ListCtl
{
	//! 加入要显示的图标。
	virtual bool add_icontoList( int _idInLib, int _pos = -1, int _iconKey = -1, int _iconWidth = 32, int _iconHeight = 32 ) = 0;

	//! 删除图标列表框内的某一个图标。
	virtual void delete_dispIcon(  int _idx  ) = 0;

	//! 得到当前图标列表框可以显示图标的列数。
	virtual int get_dispIconCol( void ) = 0;

	//! 得到当前图标列表框内选中的图标。返回选中图标的Index.
	virtual int get_selIcon( void ) = 0;

	//! 获得指定idx的图标idInLib
	virtual int get_idInLib( int _idx ) = 0;

	//! 列表框图标位置交换
	virtual void exchange_iconPos( int _FromPos, int _ToPos ) = 0;

	//! 自身内列表框图标位置改换－＞插入方式, 成功返true,操作未成功返false
	virtual bool move_iconPosByInsert( int _from, int _insertto ) = 0;

	//! 转换框id->图标的idx
	virtual int convertFrameToIdx( int _FrameID ) = 0;

	//! 转换图标的idx->框id
	virtual int convertIdxToFrame( int _idx ) = 0;

	//! 图标在列表内的存储模式: _mode:0 物品栏的模式  _mode:1 快捷栏的模式
	virtual void set_dataMode(ICONLIST_STYLE _mode) = 0;

	//! 获取控件的存储模式
	virtual ICONLIST_STYLE get_dataMode() = 0;

	//! 取消图标列表框的激活框
	virtual void cancel_active() = 0;

	//! 通过位置获得图标接口，如果物品栏样式，图标位置动态变化不准确
	virtual I_iconCtl * get_iconCtl( int _idx ) = 0;

	//! 通过唯一索引获得一个图标(准确的动态查找)
	virtual I_iconCtl* get_iconByKey( int _iconKey ) = 0;

	//! 通过用户指定的唯一id删除一个图标
	virtual void del_iconByKey( int _iconKey ) = 0;

	//! 设置滑动条的位置
	virtual void set_scbPos( int _pos ) = 0;

	//! 设置起始行
	virtual void set_startRow( int _pos ) = 0;

	//! 设置是否显示激活框
	virtual void set_showActiveBorder( bool _show ) = 0;

    //! 设置是否图标可拖动
	virtual void set_iconCanDrag( bool _bCanDrag ) = 0;

	//! 设置无图标位置，是否自动隐藏底框
	virtual void set_autoHideFrame( bool _bAutoHide ) = 0;

	//! 清除所有的图标
	virtual void del_allIcon() = 0;

	//! 获得有效的图标数量
	virtual int get_iconCount() = 0;
};

//! 颜色选择列表
struct I_colorListCtl : public fdc_ListCtl
{
	//! 修改选色板内颜色
	virtual void set_color( int _idx, DWORD _color ) = 0;

	//! 获得选色板指定索引的颜色　
	virtual DWORD get_color( int _pos ) = 0;

	//! 得到选色板颜色个数
	virtual int get_colorCount() = 0;
};

//! 字符列表接口
struct I_stringListCtl : public fdc_ListCtl
{
	//! 设置字串列表的样式
	virtual void set_dataMode( STRINGLIST_STYLE _sls ) = 0;

	//! 设置文字列表框中文件的对齐方式
	virtual void set_strDispFormat( FDCTEXT_DISFORMAT _format ) = 0;

	//! 插入单行字串,超出cut
	virtual int add_string( const char* _str, DWORD _textColor= 0xffffffff, bool _bMultiColor = false, 
		            FDCTEXT_DISFORMAT _textFormat = TEXT_LEFT ) = 0;

	//!加入在列表中显示的文字,超出写入下行, 返回新加入的字串折分的行数
	virtual int add_dispString( const char* _str, DWORD _textColor= 0xffffffff ) = 0;
	
	//! 向字符表内增一个多彩文本串.
	virtual int add_dispMultiColorString( const char* _str ) = 0;

	//! 设置列表中可以显示的最大字串行数。
	virtual void set_maxDispStringNum( int _maxNum ) = 0;

	//! 删除一行列表框内的文字。使用字串内容进行删除。
	virtual int delete_stringInList( const char* _str ) = 0;

	//! 删除一行列表框内的文字。使用字串索引进行删除。
	virtual int delete_stringInList( int _idx ) = 0;

	//! 删除所有列表框内的文字
	virtual void delete_all() = 0;

	//! 得到鼠标单击或是双击的字串的内容。
	virtual char* get_lbClkString( void ) = 0;
	
	//! 设置滑动条到最后
	virtual void set_scbEnd() = 0;

	//! 查看是否滑块位置状态，0: 最上,1:中间, 2:最后
	virtual int get_scbPosState() = 0;

	//! 高亮模式
	virtual void set_highLightMode( STRINGLIST_HIGHLIGHTMODE _mode ) = 0;

	//! 设置条目高亮.
	virtual void set_bgHighLight( int _row ) = 0;

	//! 动态改变控件的的大小
	virtual void resize( int _newWidth, int _newHeight ) = 0;

	//! 获取指定索引的串
	virtual char* get_string( int _idx ) = 0;

	//! 是否支持文本的高亮变化
	virtual void set_highLightText( bool _b ) = 0;

	//! 设置mouse move时文本的高亮色,限制条件：该行为纯色文本
	virtual void set_highLightTextColor( DWORD _color ) = 0;

	//! 是否背景高亮
	virtual void set_highLightBackGround( bool _b ) = 0;
};

//! 图标字符串列表条目
typedef struct ICONSTRINGLISTITEM
{
	int  iconIdInLib;       //! 在图标库中的图标ID
	char str0[128];         //! 字符串0
	DWORD str0Color;        //! 字符串0的颜色
	char str1[128];         //! 字符串1
	DWORD str1Color;        //! 字符串1的颜色

} ICONSTRINGLISTITEM;

//! 图标字符串列表
struct I_IconStringCtl : public fdc_ListCtl
{
	//! 增加一个条目
	virtual bool add_item(  ICONSTRINGLISTITEM& _item ) = 0;

	//! 按索引,删除一个条目
	virtual void del_itemByIdx( int _idx ) = 0;

	//! 按图标id,删除一个条目
	virtual void del_itemByIconId( int _iconIdInLib ) = 0;

	//! 删除所有条目
	virtual void del_allItem() = 0;

	//! 获得当前选中条目数据,无选中条目时，返回false
	virtual bool get_selectItem( ICONSTRINGLISTITEM& _item ) = 0;

	//! 获得指定(框索引)的条目数据.
	virtual bool get_item( int _idx , ICONSTRINGLISTITEM& _item ) = 0;
	
	//! 获得指定(真实位置索引)的条目数据. _idx: index
	virtual bool get_itemInAll( int _idx, ICONSTRINGLISTITEM& _item ) = 0;

	//! 获得图标
	virtual I_iconCtl * get_iconCtl( int _idx ) = 0;
};

//! 进度条接口
struct I_progressCtl : public fdc_baseCtl
{
	//! 设置进度条位置, 范图0-100
	virtual void set_progressPos(int _nPos) = 0;

	//! 得到进度条的位置, 范图0-100
	virtual int  get_progressPos() = 0;

	//! 设置进度条上的文本
	virtual void set_progressText( char *_str ) = 0;

	//! 设置闪烁状态
	void init_flashState( DWORD _color, BYTE _alpha, DWORD _textColor, float _cycleTime = 1.0f );

	//! 设置当前状态
	virtual void set_state( PROGRESS_STATE _state ) = 0;
};

//! 工具提示条接口类
struct I_tooltipCtl : public fdc_baseCtl
{
	//! 设置toolTip的位置。
	virtual void      set_ttPos( int _x,int _y ) = 0;

	//! 得到Tooltip的位置。
	virtual void      get_ttPos( int& _x,int& _y ) = 0;

	//! 设置toolTip的背景颜色。
	virtual void      set_ttBackColor( DWORD  _color ) = 0;

	//! 设置ToolTop上显示的文字内容。
	virtual void      set_ttText( const char* _text ) = 0;

	//! 设置ToolTip上显示的文字内容。
	virtual void   set_ttMultiColorText( const char* _text ) = 0;

	//! 设置样式
	virtual void set_style( TOOLTIP_STYLE& _ttStyle ) = 0;

	//! 得到窗体大小
	virtual osRect& get_ttRect() = 0;
};

//! 小地图图标数据
typedef struct MAPICONDATA
{
	//! 角色
	POINT   HeroPt;
	float   HeroAngle;

	//! 方向指示
	bool    bDir;
	float   dirAngle;

	//! 目标点
	int targetCount;
	POINT * pTargetPt;

	//! npc
	int     npcCount;
	POINT * pNpcPt;
	char ** ppNpcName;

	//! monster 
	int monsterCount;
	POINT * pMonsterPt;

	//! 队员
	int teamMemberCount;
	POINT * pTeamMemberPt;

	//! 队长
	int teamLeaderCount;
	POINT * pTeamLeaderPt;

	//! 门点　
	int gateCount;
	POINT * pGatePt;

} MAPICONDATA;

//! 小地图上的图标接口
struct I_mapIcon : public fdc_baseCtl
{
	//! 获得小地图标的类型
	virtual MAPICONTYPE get_iconType() = 0;

	//! 设小地图上小图标的文本(提示文本)
	virtual void  set_iconName( char* _iconName ) = 0;

	//! 获得小图的提示文本
	virtual char* get_iconName() = 0;

	//! 设小图图标的状态
	virtual void set_state( WND_STATE _state ) = 0;

	//! 获得小图标的状态
	virtual WND_STATE get_state() = 0;

	//! 修改显示的图片
	virtual void set_texUv( WINTEXINFO& _winTexInfo ) = 0;

	//! 获得当前显示的标签信息
	virtual WINTEXINFO& get_texUv() = 0;
};

//! 地图控件接口类
struct I_mapCtl : public fdc_baseCtl
{
	//! 装载地图图形文件
	virtual bool load_map( char * _mapIni ) = 0;
	
	//! 获得地图的大小
	virtual SIZE& get_mapSize() = 0;

	//! 获得当前缩放后的地图尺寸
	virtual SIZE get_mapSizeScale() = 0;

	//! 设置视口中心点，左上角为地图的原点
	virtual void set_viewport( osVec2D& _pos ) = 0;

	//! 设置地图是否可拖拽
	virtual void set_DragMap( bool _CanDrag ) = 0;

	//! 设置地图图标显示数据
	virtual void set_data( MAPICONDATA& _mapIconData ) = 0;

	//! 设置地图上的小图标文本信息
	virtual void set_iconTextInfo( MAPICONTYPE _iconType, int _idx, char* _text ) = 0;

	//! 增加自助图标
	virtual I_mapIcon* add_userIcon( char * _iconName, char * _tooltipText, POINT _pt,char * _texName, SIZE _size, osVec2D* uv ) = 0;

	//! 通过名称,删除一个自助图标
	virtual void del_userIcon( const char* _iconName ) = 0;

	//! 获得自助图标
	virtual I_mapIcon* get_userIcon( char * _iconName ) = 0;

	//! 设定图标闪烁(激活状态)
	virtual void set_state( const char* _iconName, WND_STATE _state ) = 0;

	//! 返回图标的状态(是否闪烁)
	virtual bool get_state( const char* _iconName ) = 0;

	//! 设置寻路线条数据
	virtual void set_line( std::vector<POINT>& _vecPoint ) = 0;

	//! 是否显示线条
	virtual void set_lineVisible( bool _b ) = 0;
	
	//! 是否显示寻路的起始及结尾标志图片
	virtual void set_lineStartAndEndIcon( bool _b ) = 0;

	//! 设置显示寻路的起始及结尾标志图片位置
	virtual void set_lineStartAndEndPos( POINT _startPt, POINT _endPt ) = 0;

	//! 设定地图的颜色
	virtual void set_mapColor( DWORD _color ) = 0;
};

//! 滚动文本条
struct I_textBarCtl : public fdc_baseCtl
{
	//! 增加串,返回索引
	virtual int add_str( char* _str ) = 0;

	//! 按索引,获得串
	virtual const char* get_str( int _idx ) = 0;

	//! 删除指定索引的串
	virtual bool del_str( int _idx ) = 0;

	//! 删除所有串,返回删除的个数
	virtual int del_allStr() = 0;

	//! 设置当前索引,立即显示指定的串
	virtual void set_curIdx( int _idx ) = 0;
};

//! 拖动控件
#define RESIZEBAR_SIZE   16   //
struct I_resizeBarCtl : public fdc_baseCtl
{
	
};

//! 复合选择框
struct I_comboBoxCtl : public fdc_baseCtl
{

};

//! 树控件
struct I_treeCtl : public fdc_baseCtl
{

};

//! 对话框的接口类
struct I_gameDlg : public fdc_baseCtl
{
	//! 设置gameDlg是否可以移动。
	virtual void     set_moveEnable( BOOL _move ) = 0;

	//! 设置当前对话框的位置。
	virtual void     set_dlgPos( int _x,int _y ) = 0;

	//! 得到当前对话框占用的区域。
	virtual void     get_dlgRect( osRect& _rect ) = 0;

	//! 根据一个控件的名字得到一个控件的指针。
	virtual fdc_baseCtl*  get_ctlFromName( const char* _ctlname ) = 0;

	//! 向可自动大小对话框插入一个空的图片容器元素,每个元素自动从左自右，从上至下排列
	virtual I_containerCtl * add_picContainer( int _x, int _y ) = 0;
	
	//! 是否为容器对话框
	virtual bool IsContainerDlg() = 0;

	//! 设置对话框的标题,位置固定在左上角
	virtual void set_dlgTitle( char * _title ) = 0;

	//! 设置对话框的显示与隐藏, 处理对话框的折叠状态 
	virtual void set_dlgVisible(bool _b) = 0;

	//! 获取对话框的是否可见
	virtual bool get_dlgVisible() = 0;

	//! 获取对话框当前的状态, return 0:不可见 1:正常可见 2:折叠可见
	virtual int get_state() = 0;

	//! 添加公用库中的对话框,(组队)
	virtual fdc_baseCtl* add_templateDlg( char * _CommIniName, char * _CommdlgName, osVec2D& _offset ) = 0;
	
	//! 在对诂框子中，删除一个指定名称的公用对话,true:成功,false:没找到此对诂框
	virtual bool del_templateDlg( char * _CommdlgName ) = 0;

	//! 在对诂框子中，删除一个指定指针的公用对话,true:成功,false:没找到此对诂框
	virtual bool del_templateDlg( fdc_baseCtl * _pDlg ) = 0;
	
	//! 改变对话框的大小
	virtual void dlg_resize( osRect _rect, bool _bKeepOffset = true ) = 0;

	//! 设置对话框的小限制
	virtual void set_minSize( SIZE _minSize ) = 0;

	//! 设置对话框的大限制
	virtual void set_maxSize( SIZE _maxSize ) = 0;

	//! 设置对话框当前最大可拖拉大小
	virtual void set_maxResize( int _max_w, int _max_h ) = 0;

	//! 获得对话框当前最大可拖拉大小
	virtual SIZE get_maxResize() = 0;

	//! 给对话框的增加拖拽条,只能初始化调用一次
	virtual void init_resizeBar( bool _up, bool _bottom, bool _left, bool _right, bool _rbcorner,
								 int _up_h = 16, int _bottom_h = 16, int _left_w = 16, int _right_w = 16, 
								 int _rbcorner_w = 16, int _rbcorner_h = 16 ) = 0;
	
	//! 获得resizeBar控件
	virtual fdc_baseCtl* get_resizeBar( RESIZEBAR_POS _posType ) = 0;
	
	//! 获得设计时大小
	virtual SIZE get_designSize() = 0;

	//! 是否处理鼠标消息
	virtual void set_processEvent( bool _b ) = 0;

	//! 是否支持alpha渐变
	virtual void set_supportAlphaGradual( bool _b ) = 0;

	//! alpha渐变, _targetAlpha:目标alpha, _speed:alpha/秒
	virtual void set_alphaGradual( BYTE _targetAlpha, float _speed ) = 0;
};


//! 循环播放类型
enum LOOP_TYPE
{
	LT_NO,            //播放一次
	LT_LOOP,          //循环播放
	LT_LOOP_REVERSE   //来回循环
};

//! 一个最小单位的特效参数
struct SCREEN_EFFECT
{
	char name[64];               //特效元素名称
	LOOP_TYPE     loopType;      //循环类型
	float CycleTime;             //周期

	POINT startPt,    endPt;     //位置(左上角)
	SIZE  startSize,  endSize;   //大小

	float fAngle;                //旋转周数

	DWORD startColor, endColor;  //颜色
	BYTE  startAlpha, endAlpha;  //alpha

	char texName[128];           //所用的纹理名
	osVec2D  texUv[2];           //纹理坐标

	bool isAnim;                 //是否有动画数据,  如有后接 ANIMICONCTL_STYLE
	ANIMICONCTL_STYLE*  pAnim;   //动画数据

	bool bTextureText;           //是否是纹理字
	SIZE textureTextSize;        //单个纹理字大小
};



//! 定义构造弹出对话框的样式(暂未使用)
#define UIMB_OK					(1<<0)
#define UIMB_CANCEL				(1<<1)
#define UIMB_APPLY				(1<<2)

#define UIMB_ICON_QUESTION		(1<<6)    // ?
#define UIMB_ICON_WARNING	    (1<<7)    // !
#define UIMB_ICON_ERROR			(1<<8)    // x



//! 界面管理器
struct I_gameDlgMgr
{
	//! 设置桌面的大小
	virtual void set_desktopSize(int _w,int _h) = 0;

	//! 获得桌面的大小
	virtual SIZE get_desktopSize() = 0;

	//! 获得当前场景的根窗体指针
	virtual fdc_baseCtl* get_pCurRoot() = 0;

	//! 查找某一时期的根窗体, _iniName : 配置文件名
	virtual fdc_baseCtl* find_rootWnd( char * _iniName ) = 0;

	//! 初始化对话框管理器
	virtual bool init_gameDlgMgr(I_fdScene* _pScene, I_deviceManager* _dev, HWND _hWnd, int loadModel = 0 ) = 0;

	//! 释放对话框管理器
	virtual void release_gameDlgMgr() = 0;

	//! 改变整个界面
	virtual bool change_ui( char* _uiName ) = 0;

	//! 更新界面系统
	virtual void update() = 0;

	//! 渲染界面系统
	virtual void pre_render() = 0;

	//! 按id,获得对话框的指针
	virtual fdc_baseCtl* get_gameDlgFromId( int _id ) = 0;

	//! 按名字查找窗体
	virtual fdc_baseCtl* find_wnd(char * _wnd_name ) = 0;

	//! 按id查找窗体
	virtual fdc_baseCtl* find_wnd(DWORD _id) = 0;

	//! 查找某一时期场景的窗体
	virtual fdc_baseCtl* find_wnd( char * _iniName, char * _wnd_name ) = 0;

	//! 是否点击在界面的2级窗体上
	virtual bool IsPtInUI( POINT _pt ) = 0;

	//! 获得工具提示条的指针
	virtual I_tooltipCtl* get_tooltipCtl() = 0;

	//! 界面系统的消息处理
	virtual void msgProc( MSG_ID uMsg, WPARAM wParam, LPARAM lParam ) = 0;

	//! 上层要处理的消息队列是否为空
	virtual bool  cmd_empty() = 0;

	//! 弹出一个上层要处理消息
	virtual UIMSG pop_cmd() = 0;

	//! 弹出消息框
	virtual I_gameDlg* messagebox( bool _model,  bool _CanDrag, 
				char * _msgStr, DWORD _btnAndIconType, int  code = -1) = 0;

	//! 按名称，弹出通用窗体
	virtual I_gameDlg* showCommonDlg( char * _CommIniName, char * _dlgName ) = 0;

	//! 关闭弹出的通用窗体。
	virtual void close_CommonDlg( int _id ) = 0;

	//! 图标库操作接口
	//@{
	//! 提供给用户使用的增加库图标,返回在库中的id
	virtual int   add_icon_byUser( char* _filename, float _startu, float _startv, float _endu, float _endv,
								   int _iconWidth = 32, int _iconHeight = 32 ) = 0;

	//! 获得图标库总个数
	virtual int get_iconMaxCount() = 0;
	//@}

	//! 设置当前游戏的用户名，界面为保存其相对应文件
	virtual void  set_uisaveName( char* _userName ) = 0;

	//! 获得当前的界面用户名
	virtual char * get_uisaveName() = 0;

	//! 保存控件的位置，状态
	virtual bool ui_save( char* _uiSaveName ) = 0;

	//! 从文件中恢复界面的状态
	virtual bool ui_load( char* _stepName, char* _uiSaveName ) = 0;

	//! 保存运行时根指针
	virtual void save_runRoot() = 0;

	//! 将窗体移至前端显示，降低其渲染级别
	virtual void BringLevel2WndTop( fdc_baseCtl* _pTarget ) = 0;

	//! 当前的焦点窗口是否在桌面(根)上
	virtual bool IsFocusInDesktop() = 0;
	
	//! 设置焦点窗体
	virtual void set_focus( fdc_baseCtl* _pWnd ) = 0;

	//! 设置当前场景的桌面为焦点窗体
	virtual void set_focus_wnd_desktop( ) = 0;

	//! 复位焦点,有些窗体的点击事件后的动作会删除控件窗体操作，如果不进行复位，引起焦点的野指针错误
	virtual void reset_focus() = 0;

	//! 释放某一时期的桌面纹理...
	virtual bool release_desktopTexture( char* _step ) = 0;

	//! 装入一个时期的界面,2进制模式
	virtual bool Load_step_bin( GAMESTEP _gs, char* _stepName, bool _bShow = true ) = 0;

	//! 设置模式对话框背景的名称
	virtual fdc_baseCtl * init_modelbg( char * _stepName, char* _name ) = 0;

	//!	处理模式对话框的可见性...
	virtual void process_modeldlg_bg() = 0;

	//! 注册立即处理的消息回调函数
	virtual void register_msgCallBackFunc( UI_MSGCALLBACKIMM _pFunc ) = 0;

	//! 注册用户的设定焦点的回调函数
	virtual void register_afterDlgCloseFocusCallBackFunc( UI_MSGCALLBACKIMM _pFunc ) = 0;

	//! 上层生成一个无需进行原始处理的命令，直接返回上层处理
	virtual void send_simulate_cmd( UIMSG& _uiMsg ) = 0;

	//! 将控件加入最先渲染的元素
	virtual void add_renderImm( I_editCtl * _pCtl ) = 0;

	//! 立即开始最先渲染的元素
	virtual void renderImm() = 0;

	//! 播放一个屏幕特效(*.uie)
	virtual bool play_effect( char* _effectName ) = 0;
	
	//! 用结构数据数组播放一组特效
	virtual bool play_effect( char* _effectName, SCREEN_EFFECT* _pse, int _effcount, SIZE _size/*design size*/ ) = 0;

	//! 停止一个屏幕特效
	virtual bool stop_effect( char* _effectName ) = 0;

	//! 判断当前的焦点窗体是否存在于一个窗体指针之内
	virtual bool is_focusInCtl( fdc_baseCtl * _p ) = 0;

	//! 播放纹理字
	virtual void play_screenText( char * _name, osVec2D _pos, osVec2D _dir, 
		                          SIZE _textureTextSize,    /*单个字的开始大小*/
		                          float _totalTime, char * _text, DWORD  _textColor = 0xffffffff ) = 0;

	//! 更新纹理字的位置
	virtual void update_screenText( char * _name, POINT _pos ) = 0;

	//! 获得通用的白色纹理在图标库中的id
	virtual int get_whiteTexIdInLib( ) = 0;

	//! 注册首先需要回车处理的对话框
	virtual void register_returnKeyDlg( fdc_baseCtl * _pDlg ) = 0;

	//! 查询当前焦点类型
	virtual FDCCTL_TYPE get_curFocusCtlType() = 0;

	//! 当前的mouse是否在指定的控件矩形区
	virtual bool is_mouseMoveInCtlRect( fdc_baseCtl * _p ) = 0;

	//! 通过sound id获得一个声音文件名
	virtual const char* get_soundName( int _id ) = 0;

	//! 获得图标库中指定图标的声音类型
	virtual int get_soundType( int _iconID ) = 0;

};

//! 可移动的纹理字
struct I_scrollTexture : public fdc_baseCtl
{
	
};

//! 文本滚动条（竖）
struct I_textBarVCtl : public fdc_baseCtl
{
	//! 设定文本
	virtual void set_textBarText( char * _text, DWORD _textColor = 0xffffffff ) = 0;

	//! 设定彩色文本
	virtual void set_textBarMultiColorText( char * _colorText ) = 0;

	//! 设定行间距
	virtual void set_rowSpace( int _rowSpace ) = 0;

	//! 设定速度 (像素/秒)
	virtual void set_moveSpeed( int _pixelPerSec ) = 0;

	//! 播放、停止
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

//! 热链接
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

	//! 更新mouse的状态
	void update_mouseState();
	bool icon_visible();

public:
	int m_idInIconLib;
	fdc_baseCtl * m_pPic;
	int m_fromListID;      //拖拽的图标所在list的id
	int m_fromIconIdx;     //图标的在list中的总索引

	POINT m_pos;

	bool lb_down;
	bool mb_down;
	bool rb_down;

	bool lr_AllDown; //左右键同时按下
};

//! 全局的对话框管理器。
extern I_gameDlgMgr&      get_gameDlgMgr();
extern I_fdScene*         g_ptrScene;
extern I_deviceManager*   g_3dMgrUI;
extern HWND               g_hWndUI;
extern MOUSE_STATE        g_MouseState;