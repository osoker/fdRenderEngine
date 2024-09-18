# pragma once
#ifndef _CLIENTCRYPT_INTERFACE_INCLUDE_
#define _CLIENTCRYPT_INTERFACE_INCLUDE_

struct IClientCryptInterface
{
	/**brief
	 *初始化
	*/
	virtual void Init(char * _szibuf, int _ilen) = 0;


	/**brief
	 *重置加密
	*/
	virtual void	reset() = 0;



	/**brief
	 *!加密客户端发送的数据
	 *\param	_szibuf		等待加密的数据指针
	 *\param	_ilen		数据长度
	 *\param	_szobuf		输出缓冲
	 *\	return	int			输出长度
	*/
	virtual int	encode(unsigned char * _szibuf, int _ilen, unsigned char * _szobuf) = 0;
	
	/**brief
	 *!解密客户端收到的数据
	 *\param	_szibuf		等待加密的数据指针
	 *\param	_ilen		数据长度
	 *\param	_szobuf		输出缓冲
	 *\	return	int			输出长度
	*/
	virtual int	decode(unsigned char * _szibuf, int _ilen, unsigned char * _szobuf) = 0;
};

extern IClientCryptInterface *CreateClientCryptObj();
extern void FreeClientCryptObj(IClientCryptInterface * _pobj);

#endif