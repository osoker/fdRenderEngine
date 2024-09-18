
#ifndef _FLYDREAM_PROTCOL
#define _FLYDREAM_PROTCOL

#define DEFBLOCKSIZE			12
#define HOWMANYMAGICS			31
#define USERITEMMAX				46
#define STORAGEITEMMAX			50


// For Login Process
#define CM_PROTOCOL				2000
#define CM_IDPASSWORD			2001
#define CM_ADDNEWUSER			2002
#define CM_CHANGEPASSWORD		2003
#define CM_UPDATEUSER			2004
#define CM_SELSVR				2005

#define SM_LOGIN_SUCCESS		500
#define SM_LOGIN_FAIL			501
#define SM_NEWID_SUCCESS        502
#define SM_NEWID_FAIL           503
#define SM_SELSVR_SUCCESS		504
#define SM_SELSVR_FAIL			505
#define SM_CHANGEPWD_SUCCESS	506
#define SM_CHANGEPWD_FAIL		507
// For Select Character Process
#define CM_QUERYCHR				100
#define CM_NEWCHR				101
#define CM_DELCHR				102
#define CM_SELCHR				103

#define	SM_NOTLOGIN				520
#define	SM_QUERYCHR_SUCCESS		521
#define SM_QUERYCHR_FAIL		522
#define	SM_NEWCHR_SUCCESS		523
#define	SM_NEWCHR_FAIL			524
#define	SM_DELCHR_SUCCESS		525
#define	SM_DELCHR_FAIL			526
#define	SM_STARTPLAY			527
#define	SM_STARTFAIL			528


// For Game Gate
#define GM_OPEN					1
#define GM_CLOSE				2
#define	GM_CHECKSERVER			3			
#define GM_CHECKCLIENT			4			
#define GM_DATA					5
#define GM_SERVERUSERINDEX		6
#define GM_RECEIVE_OK			7
#define GM_TEST					20

// For game process
// Client To Server Commands
#define CM_LOGIN_GAME			500
#define CM_PASSNOTICE			501
#define CM_QUERYUSERNAME        80
#define CM_QUERYBAGITEMS        81

#define CM_DROPITEM             1000
#define CM_PICKUP               1001
#define CM_TAKEONITEM			1003
#define CM_TAKEOFFITEM          1004
#define CM_BUTCH                1007
#define CM_MAGICKEYCHANGE		1008
#define CM_EAT                  1006
//add by blueboy for shop
#define	CM_TALKWIDTHNPC			1010
#define	CM_SELECTNPCSAY			1011
#define CM_GETSALEITEMPRICE		1012
#define CM_USERSALEITEM			1013
#define CM_USERBUYITEM			1014
#define CM_USERGETITEM			1015
#define CM_GETREPAIRITEMPRICE	1016
#define CM_USERREPAIRITEM		1017
#define CM_SAVEITEM				1018
#define CM_GETBACKITEM			1019

#define CM_TURN                 3010
#define CM_WALK                 3011
#define CM_SITDOWN              3012
#define CM_RUN                  3013
#define CM_HIT                  3014
#define CM_HEAVYHIT             3015
#define CM_BIGHIT               3016
#define CM_SPELL                3017
#define CM_POWERHIT             3018
#define CM_LONGHIT              3019
#define CM_WIDEHIT              3024
#define CM_FIREHIT              3025
#define CM_SAY                  3030
#define CM_RIDE					3031

// Server to Client Commands
#define SM_GOOD					1
#define SM_FAIL					2
#define SM_ALLOW_SKILL			3
#define SM_NOTALLOW_SKILL		4
#define SM_RUSH					6
#define SM_FIREHIT              8
#define SM_BACKSTEP				9
#define SM_TURN					10
#define SM_WALK					11
#define SM_SITDOWN              12
#define SM_RUN					13
#define SM_HIT					14
#define SM_SPELL				17
#define SM_POWERHIT             18
#define SM_LONGHIT              19
#define SM_DIGUP                20
#define SM_DIGDOWN              21
#define SM_FLYAXE               22
#define SM_LIGHTING             23
#define SM_WIDEHIT              24
#define SM_DISAPPEAR            30
#define SM_STRUCK				31
#define SM_DEATH                32
#define SM_SKELETON             33
#define	SM_NOWDEATH             34
#define SM_USERAPPEAR			35

#define SM_HEAR					40
#define SM_FEATURECHANGED       41
#define SM_USERNAME             42
#define SM_WINEXP               44
#define SM_LEVELUP              45
#define SM_DAYCHANGING          46
#define SM_LOGIN				50
#define SM_NEWMAP               51
#define SM_ABILITY				52
#define SM_HEALTHSPELLCHANGED   53
#define SM_MAPDESCRIPTION       54
#define SM_SPELL2               117

#define SM_SYSMESSAGE           100
#define SM_GROUPMESSAGE         101
#define SM_CRY                  102
#define SM_WHISPER              103
#define SM_GUILDMESSAGE         104
#define SM_NOTICE				105

#define SM_ADDITEM				200
#define SM_BAGITEMS				201
#define SM_ADDMAGIC             210
#define SM_MAGICS				211
#define SM_EQUIPITEMS	        621

#define SM_DROPITEM_SUCCESS     600
#define SM_DROPITEM_FAIL        601

#define SM_ITEMSHOW             610
#define SM_ITEMHIDE             611
#define SM_DOOROPEN				612
#define SM_TAKEON_OK            615
#define SM_TAKEON_FAIL          616
#define SM_TAKEOFF_OK           619
#define SM_TAKEOFF_FAIL         620
#define SM_WEIGHTCHANGED        622
#define SM_CLEAROBJECTS         633
#define SM_CHANGEMAP            634
#define SM_EAT_OK               635
#define SM_EAT_FAIL             636
#define SM_BUTCH                637
#define SM_MAGICFIRE			638
#define SM_MAGIC_LVEXP          640
#define SM_DURACHANGE           642
#define SM_MERCHANTSAY          643
#define SM_MERCHANTCLOSE        644
#define SM_GOODSLIST			645
#define SM_GETITEMLISTSUCCESS	646
#define SM_GETITEMLISTFAIL		647
#define SM_CANSALEITEM			648
#define SM_CANREPAIRITEM		649
#define SM_BUYITEMSUCCESS		650
#define SM_BUYITEMFAIL			651
#define SM_ITEMCOUNTCHANGED		652
#define SM_GOLDCHANGED          653
#define SM_CHANGELIGHT          654
#define SM_CHANGENAMECOLOR      656
#define SM_CHARSTATUSCHANGED	657
#define SM_SALEITEMSUCCESS		658
#define SM_SALEITEMFAIL			659
#define SM_SALEITEMPRICE		660
#define SM_REPAIRITEMPRICE		661
#define SM_REPAIRSUCCESS		662
#define SM_REPAIRFAIL			663
#define SM_CANSAVEITEM			664
#define SM_SAVEITEMLIST			665
#define SM_SAVEITEMSUCCESS		666
#define SM_SAVEITEMFAIL			667
#define SM_GETBACKITEMSUCCESS	668
#define SM_GETBACKITEMFAIL		669


#define SM_SUBABILITY           752

#define SM_CHANGESERVER			802

#define SM_SHOWEVENT            804
#define SM_HIDEEVENT            805

#define SM_OPENHEALTH           1100
#define SM_CLOSEHEALTH          1101
#define SM_CHANGEFACE           1104

#define SM_ITEMUPDATE			1500
#define SM_MONSTERSAY			1501

// Server to Server Commands
#define RM_USERAPPEAR			10000
#define RM_TURN					10001
#define RM_WALK					10002
#define RM_RUN					10003
#define RM_HIT					10004
#define RM_SPELL				10007
#define RM_SPELL2				10008
#define RM_POWERHIT             10009
#define RM_LONGHIT              10011
#define RM_WIDEHIT              10012
#define RM_PUSH					10013
#define RM_FIREHIT              10014
#define RM_RUSH					10015
#define RM_STRUCK				10020
#define RM_DEATH                10021
#define RM_DISAPPEAR			10022
#define RM_MAGSTRUCK            10025
#define RM_STRUCK_MAG           10027
#define RM_MAGSTRUCK_MINE       10028
#define RM_MAGHEALING           10026
#define RM_HEAR					10030
#define RM_WHISPER              10031
#define RM_CRY                  10032
#define RM_RIDE					10033
#define RM_WINEXP               10044
#define RM_USERNAME				10043
#define RM_LEVELUP				10045
#define RM_CHANGENAMECOLOR      10046

#define RM_LOGIN				10050
#define RM_ABILITY				10051
#define RM_HEALTHSPELLCHANGED   10052
#define RM_DAYCHANGING          10053

#define RM_SYSMESSAGE			10100
#define RM_GROUPMESSAGE         10102
#define RM_SYSMESSAGE2			10103
#define RM_GUILDMESSAGE         10104
#define RM_ITEMSHOW             10110
#define RM_ITEMHIDE             10111
#define RM_DOOROPEN				10112
#define RM_SENDUSEITEMS         10114
#define RM_WEIGHTCHANGED        10115
#define RM_FEATURECHANGED       10116
#define RM_CLEAROBJECTS         10117
#define RM_CHANGEMAP            10118
#define RM_BUTCH                10119
#define RM_MAGICFIRE            10120
#define RM_SENDMYMAGIC			10122
#define RM_MAGIC_LVEXP          10123
#define RM_SKELETON             10024
#define RM_DURACHANGE           10125

#define RM_GOLDCHANGED          10136
#define RM_CHANGELIGHT          10137
#define RM_ITEMCOUNTCHANGED		10138
#define RM_CHARSTATUSCHANGED	10139
#define RM_DELAYMAGIC			10154


#define RM_DIGUP                10200
#define RM_DIGDOWN              10201
#define RM_FLYAXE               10202
#define RM_LIGHTING             10204

#define RM_SUBABILITY           10302
#define RM_TRANSPARENT          10308

#define RM_SPACEMOVE_SHOW       10331
#define RM_HIDEEVENT            10333
#define RM_SHOWEVENT            10334
#define RM_ZEN_BEE				10337

#define RM_OPENHEALTH           10410
#define RM_CLOSEHEALTH          10411
#define RM_DOOPENHEALTH         10412
#define RM_CHANGEFACE           10415

#define RM_ITEMUPDATE			11000
#define RM_MONSTERSAY			11001
#define RM_MAKESLAVE			11002
//add by blueboy for shop
#define RM_TALKWIDTHNPC			12000
#define RM_SELECTNPCSAY			12001
#define RM_USERBUYITEM			12002
#define RM_USERGETITEM			12003
#define RM_DOTASK				12006

// For DB Server
//>=100 <=200是针对数据库的指令
#define DB_LOADHUMANRCD			100
#define DB_SAVEHUMANRCD			101
#define DB_MAKEITEMRCD			150
#define DB_ITEMTHROW			151
#define DB_MAKEITEMRCD2			152
//数据库操作时返回的指令
#define DBR_LOADHUMANRCD		1100
#define DBR_LOADHUMANRCD2		1101
#define DBR_MAKEITEMRCD			1500
#define DBR_MAKEITEMRCD2		1501
#define DBR_FAIL				2000



#define U_HELMET			0		//头盔
#define U_NECKLACE			1		//项链
#define U_DRESS				2		//衣服
#define U_GLOVE				3		//手套
#define U_RINGL				4		//左戒指
#define U_RINGR				5		//右戒指
#define U_WEAPON			6		//武器
#define U_SHOES				7		//靴子
#define U_OTHER				8		//护身符，箭
#define U_RESERVE1			9		//保留1
#define U_RESERVE2			10		//保留2
#define U_RESERVE3			9		//保留3

//数据服务器使用，确定某个地图的服务器的索引
typedef struct tag_TMAPINFO
{
	char	szMapFileName[15];
	WORD	wServerIdx;
}_TMAPINFO,*_LPTMAPINFO;

//和客户端交换信息使用的一个结构
typedef struct tag_TMESSAGEBODYWL
{
	LONG	lParam1;
	LONG	lParam2;
	LONG	nTag1;
	LONG	nTag2;
} _TMESSAGEBODYWL, _LPTMESSAGEBODYWL;
//
typedef struct tag_TSHORTMSSEAGE
{
	WORD	wIdent;
	WORD	wMsg;
} _TSHORTMSSEAGE, *_LPTSHORTMSSEAGE;


typedef struct tag_TDEFUSERINFO
{
	TCHAR		szLoginID[10];
	TCHAR		szPassword[10];
	TCHAR		szUserName[20];
} _TDEFUSERINFO, *_LPTUDEFUSERINFO;
//用户帐号信息结构
typedef struct tag_TADDUSERINFO1
{
	TCHAR		szZipCode[15];
	TCHAR		szAddress1[21];
	TCHAR		szAddress2[21];
	TCHAR		szSSNo[15];
	TCHAR		szPhone[14];
	TCHAR		szQuiz[20];
	TCHAR		szAnswer[12];
	TCHAR		szEmail[40];
	TCHAR		szQuiz2[20];
	TCHAR		szAnswer2[12];
	TCHAR		szBirthDay[11];
	TCHAR		szMobilePhone[15];
} _TADDUSERINFO1, *_LPTADDUSERINFO;
//用户帐号信息结构
typedef struct tag_TUSERENTRYINFO
{
	_TDEFUSERINFO	tDefUserInfo;
	_TADDUSERINFO1	tAddUserInfo;
} _TUSERENTRYINFO, *_LPTUSERENTRYINFO;

typedef struct tag_AUSERENTRYINFO
{
	char		szLoginID[11];
	char		szPassword[11];
	char		szUserName[21];
	
	char		szSSNo[19];
	char		szBirthDay[11];
	char		szZipCode[7];
	char		szAddress1[21];
	char		szAddress2[21];
	char		szPhone[15];
	char		szMobilePhone[12];
	char		szEmail[41];
	char		szQuiz[21];
	char		szAnswer[21];
	char		szQuiz2[21];
	char		szAnswer2[21];
} _AUSERENTRYINFO, *_LPAUSERENTRYINFO;

typedef struct tag_TLOADHUMAN
{
	char		szUserID[11];
	char		szCharName[21];
	char		szUserAddr[21];
	int			nCertification;
} _TLOADHUMAN, *_LPTLOADHUMAN;




#pragma pack(1)
#pragma pack(push,1)
//人物属性，服务器内部存放
typedef struct tag_THUMANRCD
{
	char		szUserID[11];
	char		szCharName[21];

	BYTE		btJob;
	BYTE		btGender;
	int			nTakeItem[12];
	BYTE		szLevel;	
	BYTE		szHair;
	BYTE		nDirection;
	char		szMapName[15];

	BYTE		btAttackMode;
	BYTE		fIsAdmin;

	short		nCX;
	short		nCY;
	short		nHp;
	short		nMP;

	DWORD		dwGold;
	int			nExp;
	int			nLuck;
	int			nPk;
	char		szStoragePwd[11];
} _THUMANRCD, *_LPTHUMANRCD;
//----------------------------------物品结构------------------------------
//制造物品的结构
typedef struct tag_TMAKEITEMRCD
{	
	char		szUserID[11];
	char		szChrName[21];
	//char		szStdType;
	int			nStdIndex;
	BYTE		btLevel;
	WORD		nDura;
	WORD		nDuraMax;
	WORD		wCount;
	BYTE		btValue[14];
} _TMAKEITEMRCD, *_LPTMAKEITEMRCD;
//用户物品结构，服务器内部存放
typedef struct tag_TUSERITEMRCD
{
	int			nItemIndex;
	char		szMakeDate[7];
	WORD		nStdIndex;
	BYTE		btLevel;
	WORD		wCount;
	WORD		nDura;
	WORD		nDuraMax;
	BYTE		btValue[14];
	char		szPrefixName[21];
} _TUSERITEMRCD, *_LPTUSERITEMRCD;
//标准物品，用于客户端
typedef struct tag_TSTANDARDITEM
{
	char		szName[21];			
	char		szPrefixName[21];
	char		szStdType;
	BYTE		btGender;
	BYTE		btJob;
	BYTE		btLevel;
	BYTE		btOverlay;
	BYTE		btShape;            
	BYTE		btWeight;                       
	BYTE		btNeedIdentify;     
	WORD		wLooks;             
	WORD		wDuraMax;
	WORD		wRSource;
	WORD		wAC;				
	WORD		wMAC;				
	WORD		wDC;				
	WORD		wMC;				
	WORD		wSC;				
	BYTE		btNeed;             
	BYTE		btNeedLevel;        
	UINT		nPrice;
} _TSTANDARDITEM, *_LPTSTANDARDITEM;
//客户端物品结构
typedef struct tag_TCLIENTITEMRCD
{
	int				nItemID;
	WORD			nDura;
	WORD			nDuraMax;
	WORD			wCount;
	BYTE			btEquipType;
	_TSTANDARDITEM	tStdItem;
} _TCLIENTITEMRCD, *_LPTCLIENTITEMRCD;
//--------------------------其他结构----------------------------
//返回给客户端的角色结构
typedef struct tag_TQUERYCHR
{	char		szName[21];
	BYTE		btClass;
	BYTE		btHair;
	BYTE		btGender;
	BYTE		btLevel;
} _TQUERYCHR,*_LPTQUERYCHR;
//马的结构
typedef struct tag_THORSERCD
{
	int			nHorseIndex;
	BYTE		btHorseType;
	BYTE		btHorseLv;
	WORD		wHorseHp;
} _THORSERCD, *_LPTHORSERCD;
//------------------技能-----------------------------------------
//人物技能结构
typedef struct tag_THUMANMAGICRCD
{
	BYTE		btMagicID;
	BYTE		btLevel;
	BYTE		btUseKey;
	int			nCurrTrain;
} _THUMANMAGICRCD, *_LPTHUMANMAGICRCD;
//标准技能结构，数据库
typedef struct tag_TSTANDARDMAGIC
{
	WORD	btMagicID;
	char	szName[13];
	BYTE	btEffectType;
	BYTE	btEffect;
	BYTE	sSpell;
	BYTE	btDefSpell;
	
	BYTE	sNeed[3];
	int		nTrain[3];
	
	WORD	nDelayTime;
} _TSTANDARDMAGIC, *_LPTSTANDARDMAGIC;
//客户端技能结构
typedef struct tag_TCLIENTMAGICRCD
{	
	char			btLevel;
	char			btUseKey;
	int				nCurrTrain;
	
	_TSTANDARDMAGIC	tStdMagic;
} _TCLIENTMAGICRCD, *_LPTCLIENTMAGICRCD;

//-----------------------------------------------------------
/*
 *Desc:			数据服务器中转信息的基本结构
 *His:			blueboy @ 17/03 2004
*/
//-----------------------------------------------------------
//add by blueboy @6/12 2003 10:37
//游戏服务器发送给数据服务器的中转指令
//>=300
#define GD_SERVERIDX			300
#define GD_USERLOGINOFF			301
#define GD_CHANGESVR			302
#define GD_SAY					304
	#define CT_WHISPER			0
#define GD_USERCOUNT			305

//中转服务器反馈给游戏服务器的指令
#define DG_CHANGESVR			3000
#define DG_DOCHANGESVR			3001
#define DG_WHISPER				3002
#define DG_SYSMSG				3003
#define DG_USERCOUNT			3004

//宝宝的基本结构
typedef struct tag_TSLAVEMONSTER
{	
	char	szName[41];
	BYTE	btLevel;
	WORD	wHp;
	WORD	wMp;
	int		Exp;
}_TSLAVEMONSTER;
//切换场景的基本结构
typedef struct tag_TCHANGEMAP
{
	char			szUserID[11];
	char			szCharName[21];
	int				nCert;
	char			szMapFile[15];
	BYTE			btSlaveCnt;
}_TCHANGEMAP;
//聊天信息转发
typedef struct tag_TCHATMSG
{
	char			szUserID[11];//主动者的ID，因为在数据服务器保存的资料是以ID为标准的
	char			szCharName[21];//被动者名字
	BYTE			btChatType;//聊天类型（私聊，行会，队伍，夫妻，朋友等）
}_TCHATMSG;

#pragma pack(pop,1)

#pragma pack(8)

#endif
