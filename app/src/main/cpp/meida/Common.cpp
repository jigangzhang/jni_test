//
// Created by Administrator on 2019/6/14.
//

#include "Common.h"
#include "../wrapper_log.h"

/**
 *抛出给定异常
 */
void ThrowException(JNIEnv *env, const char *clazzName, const char *message) {
    jclass clazz = env->FindClass(clazzName);
    if (NULL != clazz) {
        LOG_ERROR("throw exception -- %s", message);
        env->ThrowNew(clazz, message);
        env->DeleteLocalRef(clazz);
    }
}