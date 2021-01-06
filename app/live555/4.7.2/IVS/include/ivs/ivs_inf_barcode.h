#ifndef __IVS_INF_BARCODE_H__
#define __IVS_INF_BARCODE_H__


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

//#include <wchar.h>

/**
 * 条形码识别
 * */

typedef enum {
    UNRECOGNIZED        =0, /** 未识别的条形码类别,或不支持的格式*/
    CODABAR             = 2,
    CODE_39             = 3,
    CODE_128            = 5,
    EAN_13              = 8,
 } BarCodeType;


typedef struct {
    char placeholder;
} barcode_param_input_t;

typedef struct {
    BarCodeType type;      /** 条形码的类型*/
    //wchar_t * content;    /** 条形码的内容*/
    char * content;  /** 条形码的内容*/
    int64_t timeStamp;      /** 时间戳 */
} barcode_output_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __IVS_INF_BARCODE_H__ */
