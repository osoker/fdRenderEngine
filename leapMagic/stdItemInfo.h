//===================================================================
/**\file  
 *  Filename: StdItemInfo.h
 *  Desc:     ��׼��Ʒ�ṹ
 *      
 *  His:      wzg create @   2005-10-19   3:47
 */ 
//===================================================================
#ifndef __FLY_STF_ITEMINFO_DATATYPE__
#define __FLY_STF_ITEMINFO_DATATYPE__

//��Ʒ���Ƶĳ���
#define ITEMNAME_LENGTH			21		
//��Ʒ�ı�׼�����������
#define ITEMPROPERTY_MAXCNT			20
//ħ�����ֵĳ���
#define MAGICNAME_LENGTH			13
//ħ���ȼ�����
#define MAGICLVCNT					16

/************************************************************************/
/*��׼��Ʒ����Ϣ�ṹ�ͱ�׼��Ʒ�Ĺ�����                                  */
/************************************************************************/
//��Ʒ�ķ���
struct TItemTypeRecord { 
	//��Ʒ����ID
	unsigned char	btItemTypeID;
	//��Ʒ��������
	char			cItemTypeName[ITEMNAME_LENGTH];
};
//��Ʒ�����ඨ��
struct TItemSubTypeRecord{
	//��Ʒ������ID
	unsigned char	btItemSubTypeID;
	//��Ʒ����������
	char			cItemSubTypeName[ITEMNAME_LENGTH];
};


//��ͼ��Ʒʹ������(���������32��)
struct TItemMapCodeRecord {
	//��ͼ���ƴ���(���������32��)
	//ȡֵ��Χ�͹淶0x00000001,0x00000002,0x00000004,0x00000008,0x00000010.....0x80000000
	unsigned int	dwMapCode;
	//��ͼ���ƴ��������
	char			cMapCodeName[ITEMNAME_LENGTH];
};

#define EXT_FIELD	7 + MAX_PROPERTY_STRUCTNUM*2

#define MAX_PROPERTY_STRUCTNUM 6
#define MAX_PROPERTY_STRING 64
#define PROPERTY_TYPE_STRING      1    //�ַ�������
#define PROPERTY_TYPE_NUM_VALUE   2    //��ֵ����
#define PROPERTY_TYPE_NUM_PERCENT 3    //��ֵ���ͣ��ٷ���
#define PROPERTY_TYPE_NUM_MILLE   4    //��ֵ���ͣ�ǧ����
#define PROPERTY_TYPE_NUM_MOD     5    //��ֵ���ͣ���������

//�����Ʒ����˵���е����ṹ
struct TPropertyMemoType{
	//˵������磺���֣���ֵ��
	unsigned char   btPropertyType;
	//����ʱ�洢�ַ���
	char			cPropertyStr[MAX_PROPERTY_STRING ];
};

//��Ʒ��չ���Խṹ
struct TItemPropertyTypeRecord {
	//��Ʒ����ID
	unsigned char		btPropertyType;
	//��Ʒ��������
	char				cPropertyName[ITEMNAME_LENGTH];
	//! ������ɫ
	unsigned char		cPropertyColor;
	//! �Ƿ���ʾ
	unsigned int		btShow;
	//���ṹ������
	unsigned char		btPropertyStructNum;
	//�ṹ����
	TPropertyMemoType	tMemoType[MAX_PROPERTY_STRUCTNUM];
};

////��Ʒ��չ���Խṹ
//struct TItemPropertyTypeRecord {
//	//��Ʒ����ID
//	unsigned char	btPropertyType;
//	//��Ʒ��������
//	char			cPropertyName[ITEMNAME_LENGTH];
//	//����˵��
//	char			cPropertyMemo[200];
//	//! ������ɫ
//	unsigned char	cPropertyColor;
//	//! �Ƿ���ʾ
//	unsigned int	btShow;
//};

//��Ʒ�����Լ�¼�ṹ
struct TItemPropertyRecord
{
	//��������(ȡ��TItemPropertyTypeRecord.btPropertyType)
	unsigned char	btPropertyType;
	//����ֵ1
	int				iValue1;
	//����ֵ2
	int				iValue2;
};
//��Ʒ���������Խṹ(�Ƿ�ɽ���\�Ƿ��ĥ��ȵ�)
struct TItemProperty{
	//��Ʒ����������
	//ȡֵ��Χ�͹淶0x00000001,0x00000002,0x00000004,0x00000008,0x00000010.....0x80000000
	unsigned int	dwProperty;
	//��Ʒ��������������
	char			PropertyName[ITEMNAME_LENGTH];
};

//��׼��Ʒ����
struct TStdItemInfo {
	//��׼��Ʒ��������
	int				iStdIdx;
	//��׼��Ʒ������
	char			cItemStdName[ITEMNAME_LENGTH];
	//��Ʒ������
	int				iShape;
	//��Ʒ������
	unsigned char	btItemType;
	//��Ʒ��������(װ��λ��)
	unsigned char	btItemSubType;
	//��Ʒ�ļ۸�
	unsigned short	wPrice;
	//��Ʒ�ĳ־�
	unsigned short	wMaxDura;
	//��Ʒ�ʺ�ʹ�õ�ְҵ
	unsigned int	dwJob;
	//��Ʒ�ʺ�ʹ�õ��Ա�
	unsigned char	btGender;
	//��Ʒʹ�õ�ͼ������λ(TItemMapCodeRecord.dwMapCode��������Ľ��)
	unsigned int	dwMapCode;
	//��Ʒ�ȼ�
	unsigned char	btItemLv;
	//��Ʒװ��ʹ����Ҫ����������
	unsigned char	btNeedType;
	//��Ʒʹ��װ����Ҫ�ĵȼ�
	unsigned char	btNeedValue;
	//�Ƿ�ɵ���
	unsigned char	btOverLay;
	//����������
	unsigned char	btOverMaxCnt;
	//��Ʒ����cdʱ��
	unsigned int	dwPublicCdTime;
	//��Ʒ��CD����ID
	unsigned char	btCDGroupId;
	//��Ʒ����CDʱ��
	unsigned int	dwCDGroupTime;
	//��Ʒʹ�ü��
	unsigned int	dwItemCdTime;
	//��Ʒ����������(TItemProperty.dwProperty������Ľ��)
	unsigned int	dwItemProperty;
	//��װID
	unsigned int	dwGroupID;
	//��װ״̬λ(ʹ�����ķ�ʽ�õ���װ�ľ��庬��)
	unsigned int	dwGroupInfo;

	// ʹ�ö�������
	unsigned char	btUsedObjType;
	// ʹ�þ���,(��λ������)
	int				btDistance;

	// wzg [11/12/2005]
	//��������ֵ
	unsigned short	wMakeExp;
	//��Ʒ����������ʼֵ
	unsigned short	wMakeStartExp;
	//��Ʒ�����������ֵ
	unsigned short	wMakeEndExp;

	//��Ʒ�����Ը���
	unsigned char	PropertyCnt;
	//��Ʒ�ı�׼����
	TItemPropertyRecord	tPtopertyS[ITEMPROPERTY_MAXCNT];

	//! �Ƿ�ɸ���
	unsigned char bReBuild;
	//! �������
	unsigned char iReBuild;
	//! ������ɫ
	unsigned char iNameColor;
	//! ע�᷶Χ
	unsigned char btInjectRange;
	char		  cItemDes[256];
};

/** \brief
*	�ϳ���Ʒ�ɷֽṹ
*/
struct SynItemElement
{
	unsigned int	dwStdItemID;	//! ����
	unsigned int	btNumber;		//! ����
};

/** \brief
*	�ϳ���Ʒ��Ϣ�ṹ
*/
struct TSynItemInfo
{
	unsigned short	wSynIndex;		//!	�ϳ���Ʒ�������±�
	unsigned int	dwStdItemID;	//! �ϳ���Ʒ��׼��ƷID
	unsigned char	btElement;		//! �ϳɳɷ�����
	SynItemElement	arrElement[10];	//! �ϳ���Ʒ�ɷ�
	unsigned int	dwNeedMoney;	//! �ϳ���Ҫ�Ľ�Ǯ
};


# define FILE_HEADER_MAGICINFO  "mci"
# define FILE_VERSION_MAGIC  5
//! ÿһ�����ܵȼ����5������
# define MAX_MAGIC_PROPNUM    5
//! ���ܵȼ�������
struct TStdMagicProp
{
	unsigned char       m_btProIdx[MAX_MAGIC_PROPNUM];
	unsigned int        m_dwProNumber[MAX_MAGIC_PROPNUM][MAX_MAGIC_PROPNUM];

	// 2006-2-28:�����е����ݶ�Ӧ����ͬ�ļ��ܵȼ���
	//����CDʱ��(����)       
	unsigned int	m_dwPublicCdTime;
	//��������ȴʱ��         
	unsigned int	m_dwCdGroupTime;
	//�����ܵ���ȴʱ��       
	unsigned int	m_dwMagicCdTime;
	//
	unsigned int    m_dwMaxRange;
	unsigned int    m_dwUseArea;

	//! ����ʱ��
	unsigned int   m_dwYinDaoTime;
};

//! �汾3�õ��ļ��ܽṹ
struct TStdMagicInfo
{
	//����ID     
	unsigned char	btMagicId;
	//�������� 0ս������ 1�������ܡ���???������.
	unsigned char	btMagicType;
	//��������      ����
	char			cMagicName[MAGICNAME_LENGTH];
	//���ܷ���ID      
	unsigned char	btMagicGroup;
	//���ܵȼ�����    
	unsigned char	btMagicLvCnt;
	//����ѧϰ���������Ǽ�����������  
	unsigned int	dwMagicLvExp[MAGICLVCNT];
	//�����ʺ�ѧϰ��ְҵ     
	unsigned int	dwJob;
	//ʹ�ü�����Ҫ��MP����   
	unsigned short	wExpendMp;
	//MP���Ĳ���             
	int				iExpendMpParam;
	//ʹ�ü�����Ҫ��HP����   
	unsigned short	wExpendHp;
	//HP���Ĳ���             
	int				iExpendHpParam;
	//������ȴʱ�����ID     
	unsigned char	btCdGroupID;
	//���ܵ�ʹ�ö���         
	unsigned char	btEffectType;

	//���ܶ�Ӧ��Ʒ���͡�����
	unsigned char	btItemType;
	//���ܶ�Ӧ��Ʒ�����͡���
	unsigned char	btItemSubType;

	//! ��ͼ��������.
	unsigned int   dwMapLimit;     


	//! ����ʱ��,
	unsigned int   dwAnimationTime;

	// �汾3��������ݽṹ
	unsigned char    m_btMPNum[MAGICLVCNT];
	TStdMagicProp    m_vecMagicProp[MAGICLVCNT];

	//! �ٴα���8���ֶΣ��Թ��Ժ���С�޸�
	unsigned int     dwReserved[8];
};
#endif
