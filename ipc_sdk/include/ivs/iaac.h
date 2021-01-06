#ifndef __IAAC_H__
#define __IAAC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define IAAC_ACTIVE_SUCCESS				0x0			/**< 激活成功 */
#define ERR_IAAC_INCOMPATIBLE_CHIP			0xc0000001		/**< 芯片不兼容 */
#define ERR_IAAC_INV_CID				0xc0000002		/**< CID不存在 */
#define ERR_IAAC_INV_FID				0xc0000003		/**< FID不可用或未激活 */
#define ERR_IAAC_OVER_FID_PERMITED_NUM			0xc0000004		/**< 激活超过FID可用数量 */
#define ERR_IAAC_INV_SN					0xc0000005		/**< SN不存在 */
#define ERR_IAAC_REPEATED_SN				0xc0000006		/**< SN重复使用 */
#define ERR_IAAC_OTHER_SN_ACTIVED			0xc0000007		/**< 产品已使用其它SN激活 */
#define ERR_IAAC_OVER_ACTIVE_TIMES			0xc0000008		/**< 激活超过限定次数 */
#define ERR_IAAC_UNCONNECT_SERVER			0xc0000009		/**< 无法连接服务器 */
#define ERR_IAAC_INVALID_LICENSE			0xc000000a		/**< 授权证书无效 */
#define ERR_IAAC_STORE_LICENSE				0xc000000b		/**< 授权证书存储错误 */

typedef struct {
	char    *license_path;	/*<< 许可证License文件路径名,以此文件路径名建立的文件必须允许可读可写 */
	int     cid;		/*<< 客户编号CID,由君正提供,客户必须保密以防信息流失 */
	int     fid;		/*<< 功能编号FID,由君正提供 */
	char    *sn;		/*<< 激活序列号SN,每台产品一个,唯一不重复,可以由君正提供,也可以由客户在产品生产前统一向君正提供 */
} IAACInfo;

int IAAC_Init(IAACInfo *info);
int IAAC_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IAAC_H__ */
