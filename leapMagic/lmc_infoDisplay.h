///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lmc_infoDisplay.h
 *
 *  Desc:     ��������Ļ����ʾ��Ϣ�ִ�,�ִ���ʾ���������ķ�Χ�ڣ����ļ��ݲ�ʹ��
 * 
 *  His:      River created @ 2006-4-29 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once


# include "stdafx.h"
# include "gc_command.h"

/** \brief
 *  ����Ļ��ʾ�ִ���Ϣ����
 */
class lmc_infoDisplay : public lm_gameEntity
{
private:

public:
	lmc_infoDisplay(void);
	~lmc_infoDisplay(void);

	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );	

};
