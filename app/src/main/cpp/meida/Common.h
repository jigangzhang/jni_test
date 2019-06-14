//
// Created by Administrator on 2019/6/14.
//

#ifndef JNI_TEST_COMMON_H
#define JNI_TEST_COMMON_H

#endif //JNI_TEST_COMMON_H

#pragma once

#include <jni.h>

void ThrowException(JNIEnv *env, const char *className, const char *message);