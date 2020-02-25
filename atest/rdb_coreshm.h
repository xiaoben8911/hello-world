/*************************************************************************
* Copyright (c) 2019, ���ڼ���ʱ���Ƽ����޹�˾
* All rights reserved.

**************************************************************************/

/*!	\file
	\brief		�豸/�û���ϵ�����Ĺ����ڴ涨��
	
	\author		ChenXP
	\version	1.0
*/

#pragma once

//#include "cloudlinks_typedef.h"

#include "XRevHashShm.hpp"
#include "ovector.hpp"
//#include "platform_type_def.h"

//typedef uint64_t	termid_t;

#define REVISE_NOTICE_QUE_SIZE           100001     //!< �汾��֪ͨ���д�С


/*!
	\brief	Ȩ����Ϣ,�뼼�����ݿ��е�Ȩ�ޱ�־λƥ��
*/
union term_auth_t
{
	uint64_t			val;				//!<permission
		
	struct {
		uint64_t		owner:1;			//!< ����Ȩ�ޱ�־λ
		uint64_t		family:1;			//!< ����Ȩ�ޱ�־λ
		uint64_t		resv:62;			//!< Ԥ��
	};
};

/*!
	\brief	��ϵ����Ϣ
*/
struct term_relation_t
{
	uint64_t				termid;				//!< �ն�ID
	term_auth_t				auth;				//!< Ȩ����Ϣ
};

/*!
	\brief userinfo�����Ϣ
*/
struct user_info_t
{
	uint32_t iP2PVerifyCode1;
	uint32_t iP2PVerifyCode2;
	uint64_t iPhoneNO;
	uint32_t iAppVersion;
	uint32_t iSessionID;
	uint32_t iReserve2;
	char iAppOS;
	char iStatus;
	uint16_t res1;					//!<Ԥ���ֶ�
};

/*��
	\brief npcinfo�����Ϣ
*/
struct npc_info_t
{
	uint32_t iP2PVerifyCode1;
	uint32_t iP2PVerifyCode2;
	char iStatus;
	struct{
		uint8_t tencentFlag:2;		//!<�Ƿ�����Ѷ������,0:Ĭ�ϣ�1��������2����Ѷ
		uint8_t res1:6;				//!<Ԥ���ֶ�
	};
	uint16_t rseverve;				//!<Ԥ���ֶ�
};

/*!
	\brief	�豸-�û� ��ϵ����Ϣ ���ڸ����豸ID����ѯ����û��б�
			
			�ÿ��б��������ڴ�ռ䶯̬�����ڹ����ڴ��; �洢�ṹ��Ϊterm_relation_t
			Լ���ÿ��б��еķÿͣ���ID��������;
*/
struct tdu_relation_t
{
	npc_info_t				npcInfo;
	uint64_t				owner_id;			//!< ����ID

	offset_ptr_arrhead_t	visitors;			//!< �ÿ��б�
};

/*!
	\brief	�û�--�豸 ��ϵ����Ϣ ���ڸ����û�ID��ѯ�豸�б�
	
			�豸�б��������ڴ�ռ䶯̬�����ڹ����ڴ��;�洢�ṹ��Ϊterm_relation_t
			Լ���豸�б��е��豸����ID��������;
*/
struct tud_relation_t
{
	user_info_t 			userInfo;
	offset_ptr_arrhead_t	devices;			//!< �豸�б�
};
/*!
	\brief �����ڴ�ͷ��Ԥ������ʹ�ö���
*/
struct HashHeadResvUsr
{
	boost::atomic<uint32_t> Pub_db_comm_init_succ;			//!<�����˵ļ����ļ���־
	uint8_t resPub[44];

	boost::atomic<uint32_t> sub_db_full_file_trans_status; //!<���Ķ˵�ͬ��ȫ���ļ���־
	uint8_t resSub[124];
};


/*!
	\brief	�û�--�豸 ��ϵ��ӳ���ϣ��
*/
class UsrDevRelationHash: public XReviseHashShm<uint64_t,tud_relation_t>
{
public:
	/*!
		\brief	���캯��
	*/
	UsrDevRelationHash();

	/*!
		\brief	��������
	*/
	virtual ~UsrDevRelationHash();

	/*!
		\brief	��ʼ��
		\param	�Ƿ�ʹ���޶���¼����,����DBͬ�����ڵ���Ҫʹ���޶�����
		\return	�ɹ�����0�����򷵻ش����
	*/
	int init( bool revise_ena = false );

	/*!
		\brief	��ȡ�û����豸�б�
		\param	user_id		�û�ID
		\param	vr			����豸�б�
		\return	�����û����豸��,����-1��ʾ�û�������
	*/
	int  get_related_devices( uint64_t user_id, std::vector<term_relation_t>& vr );

	/*!
		\brief  ��ȡ�û��Ļ�����Ϣ
		\param  user_id		�û�ID
		\param  user_info	�û�������Ϣ
		\return ����ֵ,����-1��ʾ�û�������
	*/
	int get_user_information(uint64_t user_id, user_info_t& usr_info);

	/*!
		\brief	�����û����豸�б���Ϣ
		\param	user_id		�û�ID
		\param	vr			�豸�б���Ϣ
		\return	�ɹ�����0�����򷵻ش����
	*/
	int  set_related_devices( uint64_t user_id, const std::vector<term_relation_t>& vr );

	/*!
		\brief	��ѯ�û����豸֮��Ĺ�����ϵ
		\return	���ع�����ϵ
	*/
	term_auth_t get_related_auth( uint64_t user_id, uint64_t dev_id );

	/*!
		\brief	���/����һ���û�/�豸������Ϣ ���ڶ�̬�����ϵ
		\return	�ɹ�����0�����򷵻ش����
	*/
	int add_relation( uint64_t user_id, const term_relation_t& r ,int64_t &i64Revesion);
	int add_related_device( uint64_t user_id, uint64_t dev_id, uint64_t auth,int64_t &i64Revesion );
	int add_user_info(uint64_t user_id, const user_info_t &userInfo,int64_t &i64Revesion);

	/*!
		\brief	ɾ���û�/�豸������Ϣ ���ڶ�̬�����ϵ
		\return	�ɹ�����0�����򷵻ش����
	*/
	int del_relation( uint64_t user_id, uint64_t dev_id,int64_t &i64Revesion );

	/*!
		\brief	��ӡ����û���ϵ����Ϣ
	*/
	void print_user_info( std::ostream& os, UsrDevRelationHash::value_type& pair );
	void print_user_info( std::ostream& os, uint64_t usr_id );	
	void print_user_info( std::ostream& os );
	/*!
		\brief д��ȫ���ļ�������ɱ�־������Ҷ�ӽڵ�ʹ��
	*/
	bool write_full_file_status(uint32_t status);
	/*!
		\brief ��ȡȫ���ļ���־
		\param status :0,δͬ����ɣ�1��ͬ�����
	*/
	bool read_full_file_status(uint32_t& status);

	/*!
		\brief д�뷢����ͨѶ���̳�ʼ�������ڴ�״̬
	*/
	bool write_pub_db_comm_init_status(uint32_t status);
	/*!
		\brief ��ȡ������ͨѶ���̳�ʼ�������ڴ�״̬
		\param status :0,δͬ����ɣ�1��ͬ�����
	*/
	bool read_pub_db_comm_init_status(uint32_t& status);
	
protected:
};

/*!
	\brief	�豸--�û� ��ϵ��ӳ���ϣ��
*/
class DevUsrRelationHash: public XReviseHashShm<uint64_t,tdu_relation_t>
{
public:
	typedef boost::interprocess::managed_shared_memory		IPCManagedShm;

	/*!
		\brief
	*/
	DevUsrRelationHash();

	/*!
		\brief
	*/
	virtual ~DevUsrRelationHash();
	
	/*!
		\brief
	*/
	int init(bool revise_ena = false);

	/*!
		\brief	��ȡ�豸����û��б�
		\param	dev_id		�豸ID
		\param	vr			����û��б�,��һ��Ԫ��Լ��Ϊ����
		\return	�����û���,����-1��ʾ�豸������
	*/
	int  get_related_users( uint64_t dev_id, std::vector<term_relation_t>& vr );

	/*!
		\brief  ��ȡ�豸�Ļ�����Ϣ
		\param  dev_id		�豸ID
		\param  npc_info    �豸��Ϣ
		\return �����û���,����-1��ʾ�豸������
	*/
	int get_device_information(uint64_t dev_id,npc_info_t &npc_info);

	/*!
		\brief	�����豸���û��б���Ϣ
		\param	dev_id		�豸ID
		\param	vr			�û��б���Ϣ,��һ��Ԫ��Լ��Ϊ����
		\return	�ɹ�����0,���򷵻ش����
	*/
	int  set_related_users( uint64_t user_id, const std::vector<term_relation_t>& vr );

	/*!
		\brief	��ѯ�û����豸֮��Ĺ�����ϵ
		\return	���ع�����ϵ
	*/
	term_auth_t get_related_auth( uint64_t user_id, uint64_t dev_id );

	/*!
		\brief	���/����һ���û�/�豸������Ϣ ���ڶ�̬�����ϵ
		\return	�ɹ�����0�����򷵻ش����
	*/
	int add_relation( uint64_t dev_id, const term_relation_t& r,int64_t &i64Revesion );

	/*!
		\brief
	*/
	int add_npc_info(uint64_t dev_id, const npc_info_t npcInfo, int64_t &i64Revesion);

	/*!
		\brief	ɾ���豸/�û�������Ϣ ���ڶ�̬�����ϵ
		\return	�ɹ�����0�����򷵻ش����
	*/
	int del_relation( uint64_t dev_id, uint64_t user_id,bool bIsOwner ,int64_t &i64Revesion);

	/*!
		\brief	��ӡ����豸��ϵ����Ϣ
	*/
	void print_device_info( std::ostream& os, DevUsrRelationHash::value_type& pair );
	void print_device_info( std::ostream& os, uint64_t dev_id );
	void print_device_info(std::ostream& os);
	/*!
		\brief д��ȫ���ļ�������ɱ�־������Ҷ�ӽڵ�ʹ��
	*/
	bool write_full_file_status(uint32_t status);
	/*!
		\brief ��ȡȫ���ļ���־
		\param status :0,δͬ����ɣ�1��ͬ�����
	*/
	bool read_full_file_status(uint32_t& status);

	/*!
		\brief д�뷢����ͨѶ���̳�ʼ�������ڴ�״̬
	*/
	bool write_pub_db_comm_init_status(uint32_t status);
	/*!
		\brief ��ȡ������ͨѶ���̳�ʼ�������ڴ�״̬
		\param status :0,δͬ����ɣ�1��ͬ�����
	*/
	bool read_pub_db_comm_init_status(uint32_t& status);
	
protected:
};


/*!
	\brief	��ϵ�����Ĺ����ڴ�
*/
class RelationDbCoreShm
{
public:
	typedef XReviseHashShm<uint64_t,uint64_t>		HwUsrAsscoHash;

protected:
	UsrDevRelationHash		_hmapUsrDev;			//!< �û�-�豸��ϵ����
	DevUsrRelationHash		_hmapDevUsr;			//!< �豸-�û���ϵ����
	
	HwUsrAsscoHash			_hmapHwUsr;				//!< ��Ϊ/�����û��ʻ�������(��ѡ��Ŀ��Ӧ��)
};
