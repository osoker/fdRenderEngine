# pragma once
#ifndef _CLIENTCRYPT_INTERFACE_INCLUDE_
#define _CLIENTCRYPT_INTERFACE_INCLUDE_

struct IClientCryptInterface
{
	/**brief
	 *��ʼ��
	*/
	virtual void Init(char * _szibuf, int _ilen) = 0;


	/**brief
	 *���ü���
	*/
	virtual void	reset() = 0;



	/**brief
	 *!���ܿͻ��˷��͵�����
	 *\param	_szibuf		�ȴ����ܵ�����ָ��
	 *\param	_ilen		���ݳ���
	 *\param	_szobuf		�������
	 *\	return	int			�������
	*/
	virtual int	encode(unsigned char * _szibuf, int _ilen, unsigned char * _szobuf) = 0;
	
	/**brief
	 *!���ܿͻ����յ�������
	 *\param	_szibuf		�ȴ����ܵ�����ָ��
	 *\param	_ilen		���ݳ���
	 *\param	_szobuf		�������
	 *\	return	int			�������
	*/
	virtual int	decode(unsigned char * _szibuf, int _ilen, unsigned char * _szobuf) = 0;
};

extern IClientCryptInterface *CreateClientCryptObj();
extern void FreeClientCryptObj(IClientCryptInterface * _pobj);

#endif