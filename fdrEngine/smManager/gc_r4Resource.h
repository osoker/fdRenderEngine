////////////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_r4resource.h
 *
 *  Desc:     资源的基类。
 *
 *  His:      raffaello created @ 2005-6-19
 */
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _R4RESOURCE_H_
#define _R4RESOURCE_H_
#include <string>
class I_r4Resource{
public:
	I_r4Resource():m_tCount(0),bFever(false){}
public:
	void touch()	{if(bFever) {m_tCount=1;return;}m_tCount++;}
	void unload()	{if(bFever) {m_tCount=1;return;}m_tCount--;}
	size_t count()	const {return m_tCount;}
	const std::string get_name()	const{return m_szName;}

	//! 从文件中读入一个资源
	virtual bool load(const char* _lpszFileName)	=	0;
	//! 销毁这个资源
	virtual bool destory()	=	0;
protected:
	size_t	m_tCount;
	std::string	m_szName;
	bool	bFever;

};
#endif//_R4RESOURCE_H_