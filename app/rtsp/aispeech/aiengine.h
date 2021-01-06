#ifndef AIENGINE_H_
#define AIENGINE_H_

#define AIENGINE_VERSION "2.6.2"

#if (!(defined AIENGINE_CALL) || !(defined AIENGINE_IMPORT_OR_EXPORT))
#    if defined __WIN32__ || defined _WIN32 || defined _WIN64
#       define AIENGINE_CALL __stdcall
#       ifdef  AIENGINE_IMPLEMENTION
#           define AIENGINE_IMPORT_OR_EXPORT __declspec(dllexport)
#       else
#           define AIENGINE_IMPORT_OR_EXPORT __declspec(dllimport)
#       endif
#    elif defined __ANDROID__
#       define AIENGINE_CALL
#       define AIENGINE_IMPORT_OR_EXPORT
#    else
#       define AIENGINE_CALL
#       define AIENGINE_IMPORT_OR_EXPORT __attribute ((visibility("default")))
#    endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    AIENGINE_MESSAGE_TYPE_JSON = 1,
    AIENGINE_MESSAGE_TYPE_BIN
};

struct aiengine;

typedef int (AIENGINE_CALL *aiengine_callback)(const void *usrdata, const char *id, int type, const void *message, int size);
AIENGINE_IMPORT_OR_EXPORT struct aiengine * AIENGINE_CALL aiengine_new(const char *cfg);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_delete(struct aiengine *engine);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_start(struct aiengine *engine, const char *param, char id[64], aiengine_callback callback, const void *usrdata);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_feed(struct aiengine *engine, const void *data, int size);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_stop(struct aiengine *engine);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_cancel(struct aiengine *engine);

/*
 *int opt : 1:获取aiengine版本号；2: 设置wifi状态；3: 设置设备ID;
 *          4:获取aiengine生成的recordId; 5: 发送JSON结构的日志； 6: 获取设备ID;
 *          7:获取vad数据 8: 9:资源更新
 * */
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_opt(struct aiengine *engine, int opt, char *data, int size);

int aiengine_server_startup();
int aiengine_server_stop();

#ifdef __cplusplus
}
#endif
#endif
