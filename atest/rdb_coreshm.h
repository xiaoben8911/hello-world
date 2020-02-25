/*************************************************************************
* Copyright (c) 2019, 深圳技威时代科技有限公司
* All rights reserved.

**************************************************************************/

/*!	\file
	\brief		设备/用户关系链核心共享内存定义
	
	\author		ChenXP
	\version	1.0
*/

#pragma once

//#include "cloudlinks_typedef.h"

#include "XRevHashShm.hpp"
#include "ovector.hpp"
//#include "platform_type_def.h"

//typedef uint64_t	termid_t;

#define REVISE_NOTICE_QUE_SIZE           100001     //!< 版本号通知队列大小


/*!
	\brief	权限信息,与技威数据库中的权限标志位匹配
*/
union term_auth_t
{
	uint64_t			val;				//!<permission
		
	struct {
		uint64_t		owner:1;			//!< 主人权限标志位
		uint64_t		family:1;			//!< 分享权限标志位
		uint64_t		resv:62;			//!< 预留
	};
};

/*!
	\brief	关系链信息
*/
struct term_relation_t
{
	uint64_t				termid;				//!< 终端ID
	term_auth_t				auth;				//!< 权限信息
};

/*!
	\brief userinfo表的信息
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
	uint16_t res1;					//!<预留字段
};

/*！
	\brief npcinfo表的信息
*/
struct npc_info_t
{
	uint32_t iP2PVerifyCode1;
	uint32_t iP2PVerifyCode2;
	char iStatus;
	struct{
		uint8_t tencentFlag:2;		//!<是否是腾讯服务器,0:默认，1；技威，2：腾讯
		uint8_t res1:6;				//!<预留字段
	};
	uint16_t rseverve;				//!<预留字段
};

/*!
	\brief	设备-用户 关系链信息 用于根据设备ID，查询相关用户列表
			
			访客列表不定长，内存空间动态分配于共享内存池; 存储结构体为term_relation_t
			约定访客列表中的访客，按ID有序排列;
*/
struct tdu_relation_t
{
	npc_info_t				npcInfo;
	uint64_t				owner_id;			//!< 主人ID

	offset_ptr_arrhead_t	visitors;			//!< 访客列表
};

/*!
	\brief	用户--设备 关系链信息 用于根据用户ID查询设备列表
	
			设备列表不定长，内存空间动态分配于共享内存池;存储结构体为term_relation_t
			约定设备列表中的设备，按ID有序排列;
*/
struct tud_relation_t
{
	user_info_t 			userInfo;
	offset_ptr_arrhead_t	devices;			//!< 设备列表
};
/*!
	\brief 共享内存头部预留部分使用定义
*/
struct HashHeadResvUsr
{
	boost::atomic<uint32_t> Pub_db_comm_init_succ;			//!<发布端的加载文件标志
	uint8_t resPub[44];

	boost::atomic<uint32_t> sub_db_full_file_trans_status; //!<订阅端的同步全量文件标志
	uint8_t resSub[124];
};


/*!
	\brief	用户--设备 关系链映射哈希表
*/
class UsrDevRelationHash: public XReviseHashShm<uint64_t,tud_relation_t>
{
public:
	/*!
		\brief	构造函数
	*/
	UsrDevRelationHash();

	/*!
		\brief	析构函数
	*/
	virtual ~UsrDevRelationHash();

	/*!
		\brief	初始化
		\param	是否使能修订记录功能,仅在DB同步根节点需要使能修订功能
		\return	成功返回0，否则返回错误号
	*/
	int init( bool revise_ena = false );

	/*!
		\brief	获取用户的设备列表
		\param	user_id		用户ID
		\param	vr			输出设备列表
		\return	返回用户的设备数,返回-1表示用户不存在
	*/
	int  get_related_devices( uint64_t user_id, std::vector<term_relation_t>& vr );

	/*!
		\brief  获取用户的基本信息
		\param  user_id		用户ID
		\param  user_info	用户基本信息
		\return 返回值,返回-1表示用户不存在
	*/
	int get_user_information(uint64_t user_id, user_info_t& usr_info);

	/*!
		\brief	设置用户的设备列表信息
		\param	user_id		用户ID
		\param	vr			设备列表信息
		\return	成功返回0，否则返回错误号
	*/
	int  set_related_devices( uint64_t user_id, const std::vector<term_relation_t>& vr );

	/*!
		\brief	查询用户与设备之间的关联关系
		\return	返回关联关系
	*/
	term_auth_t get_related_auth( uint64_t user_id, uint64_t dev_id );

	/*!
		\brief	添加/更改一条用户/设备关联信息 用于动态变更关系
		\return	成功返回0，否则返回错误号
	*/
	int add_relation( uint64_t user_id, const term_relation_t& r ,int64_t &i64Revesion);
	int add_related_device( uint64_t user_id, uint64_t dev_id, uint64_t auth,int64_t &i64Revesion );
	int add_user_info(uint64_t user_id, const user_info_t &userInfo,int64_t &i64Revesion);

	/*!
		\brief	删除用户/设备关联信息 用于动态变更关系
		\return	成功返回0，否则返回错误号
	*/
	int del_relation( uint64_t user_id, uint64_t dev_id,int64_t &i64Revesion );

	/*!
		\brief	打印输出用户关系链信息
	*/
	void print_user_info( std::ostream& os, UsrDevRelationHash::value_type& pair );
	void print_user_info( std::ostream& os, uint64_t usr_id );	
	void print_user_info( std::ostream& os );
	/*!
		\brief 写入全量文件传输完成标志，仅限叶子节点使用
	*/
	bool write_full_file_status(uint32_t status);
	/*!
		\brief 读取全量文件标志
		\param status :0,未同步完成，1：同步完成
	*/
	bool read_full_file_status(uint32_t& status);

	/*!
		\brief 写入发布端通讯进程初始化共享内存状态
	*/
	bool write_pub_db_comm_init_status(uint32_t status);
	/*!
		\brief 读取发布端通讯进程初始化共享内存状态
		\param status :0,未同步完成，1：同步完成
	*/
	bool read_pub_db_comm_init_status(uint32_t& status);
	
protected:
};

/*!
	\brief	设备--用户 关系链映射哈希表
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
		\brief	获取设备相关用户列表
		\param	dev_id		设备ID
		\param	vr			输出用户列表,第一个元素约定为主人
		\return	返回用户数,返回-1表示设备不存在
	*/
	int  get_related_users( uint64_t dev_id, std::vector<term_relation_t>& vr );

	/*!
		\brief  获取设备的基本信息
		\param  dev_id		设备ID
		\param  npc_info    设备信息
		\return 返回用户数,返回-1表示设备不存在
	*/
	int get_device_information(uint64_t dev_id,npc_info_t &npc_info);

	/*!
		\brief	设置设备的用户列表信息
		\param	dev_id		设备ID
		\param	vr			用户列表信息,第一个元素约定为主人
		\return	成功返回0,否则返回错误号
	*/
	int  set_related_users( uint64_t user_id, const std::vector<term_relation_t>& vr );

	/*!
		\brief	查询用户与设备之间的关联关系
		\return	返回关联关系
	*/
	term_auth_t get_related_auth( uint64_t user_id, uint64_t dev_id );

	/*!
		\brief	添加/更改一条用户/设备关联信息 用于动态变更关系
		\return	成功返回0，否则返回错误号
	*/
	int add_relation( uint64_t dev_id, const term_relation_t& r,int64_t &i64Revesion );

	/*!
		\brief
	*/
	int add_npc_info(uint64_t dev_id, const npc_info_t npcInfo, int64_t &i64Revesion);

	/*!
		\brief	删除设备/用户关联信息 用于动态变更关系
		\return	成功返回0，否则返回错误号
	*/
	int del_relation( uint64_t dev_id, uint64_t user_id,bool bIsOwner ,int64_t &i64Revesion);

	/*!
		\brief	打印输出设备关系链信息
	*/
	void print_device_info( std::ostream& os, DevUsrRelationHash::value_type& pair );
	void print_device_info( std::ostream& os, uint64_t dev_id );
	void print_device_info(std::ostream& os);
	/*!
		\brief 写入全量文件传输完成标志，仅限叶子节点使用
	*/
	bool write_full_file_status(uint32_t status);
	/*!
		\brief 读取全量文件标志
		\param status :0,未同步完成，1：同步完成
	*/
	bool read_full_file_status(uint32_t& status);

	/*!
		\brief 写入发布端通讯进程初始化共享内存状态
	*/
	bool write_pub_db_comm_init_status(uint32_t status);
	/*!
		\brief 读取发布端通讯进程初始化共享内存状态
		\param status :0,未同步完成，1：同步完成
	*/
	bool read_pub_db_comm_init_status(uint32_t& status);
	
protected:
};


/*!
	\brief	关系链核心共享内存
*/
class RelationDbCoreShm
{
public:
	typedef XReviseHashShm<uint64_t,uint64_t>		HwUsrAsscoHash;

protected:
	UsrDevRelationHash		_hmapUsrDev;			//!< 用户-设备关系链表
	DevUsrRelationHash		_hmapDevUsr;			//!< 设备-用户关系链表
	
	HwUsrAsscoHash			_hmapHwUsr;				//!< 华为/技威用户帐户关联表(智选项目中应用)
};
