//===================================================================
/**\file  
 *  Filename: StdItemInfo.h
 *  Desc:     标准物品结构
 *      
 *  His:      wzg create @   2005-10-19   3:47
 */ 
//===================================================================
#ifndef __FLY_STF_ITEMINFO_DATATYPE__
#define __FLY_STF_ITEMINFO_DATATYPE__

//物品名称的长度
#define ITEMNAME_LENGTH			21		
//物品的标准属性最大数量
#define ITEMPROPERTY_MAXCNT			20
//魔法名字的长度
#define MAGICNAME_LENGTH			13
//魔法等级个数
#define MAGICLVCNT					16

/************************************************************************/
/*标准物品的信息结构和标准物品的管理器                                  */
/************************************************************************/
//物品的分类
struct TItemTypeRecord { 
	//物品类型ID
	unsigned char	btItemTypeID;
	//物品类型名称
	char			cItemTypeName[ITEMNAME_LENGTH];
};
//物品的子类定义
struct TItemSubTypeRecord{
	//物品子类型ID
	unsigned char	btItemSubTypeID;
	//物品子类型名称
	char			cItemSubTypeName[ITEMNAME_LENGTH];
};


//地图物品使用限制(最大允许有32个)
struct TItemMapCodeRecord {
	//地图限制代码(最大允许有32个)
	//取值范围和规范0x00000001,0x00000002,0x00000004,0x00000008,0x00000010.....0x80000000
	unsigned int	dwMapCode;
	//地图限制代码的名字
	char			cMapCodeName[ITEMNAME_LENGTH];
};

#define EXT_FIELD	7 + MAX_PROPERTY_STRUCTNUM*2

#define MAX_PROPERTY_STRUCTNUM 6
#define MAX_PROPERTY_STRING 64
#define PROPERTY_TYPE_STRING      1    //字符串类型
#define PROPERTY_TYPE_NUM_VALUE   2    //数值类型
#define PROPERTY_TYPE_NUM_PERCENT 3    //数值类型，百分数
#define PROPERTY_TYPE_NUM_MILLE   4    //数值类型，千分数
#define PROPERTY_TYPE_NUM_MOD     5    //数值类型，四舍五入

//组成物品属性说明中的类别结构
struct TPropertyMemoType{
	//说明类别，如：文字，数值等
	unsigned char   btPropertyType;
	//文字时存储字符串
	char			cPropertyStr[MAX_PROPERTY_STRING ];
};

//物品扩展属性结构
struct TItemPropertyTypeRecord {
	//物品属性ID
	unsigned char		btPropertyType;
	//物品属性名称
	char				cPropertyName[ITEMNAME_LENGTH];
	//! 名称颜色
	unsigned char		cPropertyColor;
	//! 是否显示
	unsigned int		btShow;
	//类别结构的数量
	unsigned char		btPropertyStructNum;
	//结构数组
	TPropertyMemoType	tMemoType[MAX_PROPERTY_STRUCTNUM];
};

////物品扩展属性结构
//struct TItemPropertyTypeRecord {
//	//物品属性ID
//	unsigned char	btPropertyType;
//	//物品属性名称
//	char			cPropertyName[ITEMNAME_LENGTH];
//	//属性说明
//	char			cPropertyMemo[200];
//	//! 名称颜色
//	unsigned char	cPropertyColor;
//	//! 是否显示
//	unsigned int	btShow;
//};

//物品的属性记录结构
struct TItemPropertyRecord
{
	//属性类型(取子TItemPropertyTypeRecord.btPropertyType)
	unsigned char	btPropertyType;
	//属性值1
	int				iValue1;
	//属性值2
	int				iValue2;
};
//物品的其他属性结构(是否可交易\是否会磨损等等)
struct TItemProperty{
	//物品的其他属性
	//取值范围和规范0x00000001,0x00000002,0x00000004,0x00000008,0x00000010.....0x80000000
	unsigned int	dwProperty;
	//物品的其他属性名称
	char			PropertyName[ITEMNAME_LENGTH];
};

//标准物品资料
struct TStdItemInfo {
	//标准物品的索引号
	int				iStdIdx;
	//标准物品的名字
	char			cItemStdName[ITEMNAME_LENGTH];
	//物品的外形
	int				iShape;
	//物品的类型
	unsigned char	btItemType;
	//物品的子类型(装备位置)
	unsigned char	btItemSubType;
	//物品的价格
	unsigned short	wPrice;
	//物品的持久
	unsigned short	wMaxDura;
	//物品适合使用的职业
	unsigned int	dwJob;
	//物品适合使用的性别
	unsigned char	btGender;
	//物品使用地图的限制位(TItemMapCodeRecord.dwMapCode多个相异或的结果)
	unsigned int	dwMapCode;
	//物品等级
	unsigned char	btItemLv;
	//物品装备使用需要的需求类型
	unsigned char	btNeedType;
	//物品使用装备需要的等级
	unsigned char	btNeedValue;
	//是否可叠加
	unsigned char	btOverLay;
	//最大叠加数量
	unsigned char	btOverMaxCnt;
	//物品公共cd时间
	unsigned int	dwPublicCdTime;
	//物品的CD分组ID
	unsigned char	btCDGroupId;
	//物品分组CD时间
	unsigned int	dwCDGroupTime;
	//物品使用间隔
	unsigned int	dwItemCdTime;
	//物品的其他属性(TItemProperty.dwProperty多个异或的结果)
	unsigned int	dwItemProperty;
	//套装ID
	unsigned int	dwGroupID;
	//套装状态位(使用异或的方式得到套装的具体含义)
	unsigned int	dwGroupInfo;

	// 使用对像类型
	unsigned char	btUsedObjType;
	// 使用距离,(单位：毫米)
	int				btDistance;

	// wzg [11/12/2005]
	//生产经验值
	unsigned short	wMakeExp;
	//物品生产经验起始值
	unsigned short	wMakeStartExp;
	//物品生产经验结束值
	unsigned short	wMakeEndExp;

	//物品的属性个数
	unsigned char	PropertyCnt;
	//物品的标准属性
	TItemPropertyRecord	tPtopertyS[ITEMPROPERTY_MAXCNT];

	//! 是否可改造
	unsigned char bReBuild;
	//! 改造次数
	unsigned char iReBuild;
	//! 名称颜色
	unsigned char iNameColor;
	//! 注册范围
	unsigned char btInjectRange;
	char		  cItemDes[256];
};

/** \brief
*	合成物品成分结构
*/
struct SynItemElement
{
	unsigned int	dwStdItemID;	//! 索引
	unsigned int	btNumber;		//! 数量
};

/** \brief
*	合成物品信息结构
*/
struct TSynItemInfo
{
	unsigned short	wSynIndex;		//!	合成物品向量的下标
	unsigned int	dwStdItemID;	//! 合成物品标准物品ID
	unsigned char	btElement;		//! 合成成分数量
	SynItemElement	arrElement[10];	//! 合成物品成分
	unsigned int	dwNeedMoney;	//! 合成需要的金钱
};


# define FILE_HEADER_MAGICINFO  "mci"
# define FILE_VERSION_MAGIC  5
//! 每一个技能等级最多5个属性
# define MAX_MAGIC_PROPNUM    5
//! 技能等级的属性
struct TStdMagicProp
{
	unsigned char       m_btProIdx[MAX_MAGIC_PROPNUM];
	unsigned int        m_dwProNumber[MAX_MAGIC_PROPNUM][MAX_MAGIC_PROPNUM];

	// 2006-2-28:把下列的数据对应到不同的技能等级中
	//公共CD时间(毫秒)       
	unsigned int	m_dwPublicCdTime;
	//技能组冷却时间         
	unsigned int	m_dwCdGroupTime;
	//本技能的冷却时间       
	unsigned int	m_dwMagicCdTime;
	//
	unsigned int    m_dwMaxRange;
	unsigned int    m_dwUseArea;

	//! 引导时间
	unsigned int   m_dwYinDaoTime;
};

//! 版本3用到的技能结构
struct TStdMagicInfo
{
	//技能ID     
	unsigned char	btMagicId;
	//技能类型 0战斗技能 1生产技能　　???下拉框.
	unsigned char	btMagicType;
	//技能名称      　　
	char			cMagicName[MAGICNAME_LENGTH];
	//技能分组ID      
	unsigned char	btMagicGroup;
	//技能等级数量    
	unsigned char	btMagicLvCnt;
	//技能学习条件或者是技能升级经验  
	unsigned int	dwMagicLvExp[MAGICLVCNT];
	//仅能适合学习的职业     
	unsigned int	dwJob;
	//使用技能需要的MP消耗   
	unsigned short	wExpendMp;
	//MP消耗参数             
	int				iExpendMpParam;
	//使用技能需要的HP消耗   
	unsigned short	wExpendHp;
	//HP消耗参数             
	int				iExpendHpParam;
	//技能冷却时间分组ID     
	unsigned char	btCdGroupID;
	//技能的使用对象         
	unsigned char	btEffectType;

	//技能对应物品类型　　　
	unsigned char	btItemType;
	//技能对应物品子类型　　
	unsigned char	btItemSubType;

	//! 地图属性限制.
	unsigned int   dwMapLimit;     


	//! 动画时间,
	unsigned int   dwAnimationTime;

	// 版本3加入的数据结构
	unsigned char    m_btMPNum[MAGICLVCNT];
	TStdMagicProp    m_vecMagicProp[MAGICLVCNT];

	//! 再次保留8个字段，以供以后做小修改
	unsigned int     dwReserved[8];
};
#endif
