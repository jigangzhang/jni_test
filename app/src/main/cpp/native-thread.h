//
// Created by Administrator on 2019/6/6.
//

#ifndef JNI_TEST_NATIVE_THREAD_H
#define JNI_TEST_NATIVE_THREAD_H

#endif //JNI_TEST_NATIVE_THREAD_H

#include <jni.h>

extern "C" JNIEXPORT void JNICALL Java_com_god_seep_jni_1test_ThreadActivity_nativeInit(JNIEnv *, jobject);

extern "C" JNIEXPORT void  JNICALL Java_com_god_seep_jni_1test_ThreadActivity_nativeFree(JNIEnv *, jobject);

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_ThreadActivity_nativeWorker(JNIEnv *, jobject, jint, jint);

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_ThreadActivity_posixThreads(JNIEnv *, jobject, jint, jint);
