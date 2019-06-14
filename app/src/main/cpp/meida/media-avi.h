//
// Created by Administrator on 2019/6/14.
//

#ifndef JNI_TEST_MEDIA_AVI_H
#define JNI_TEST_MEDIA_AVI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL Java_com_god_seep_jni_1test_media_BasePlayerActivity_open(JNIEnv *, jclass, jstring);

JNIEXPORT jint JNICALL Java_com_god_seep_jni_1test_media_BasePlayerActivity_getWidth(JNIEnv *, jclass, jlong);

JNIEXPORT jint JNICALL Java_com_god_seep_jni_1test_media_BasePlayerActivity_getHeight(JNIEnv *, jclass, jlong);

JNIEXPORT jdouble JNICALL Java_com_god_seep_jni_1test_media_BasePlayerActivity_getFrameRate(JNIEnv *, jclass, jlong);

JNIEXPORT void JNICALL Java_com_god_seep_jni_1test_media_BasePlayerActivity_close(JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif //JNI_TEST_MEDIA_AVI_H
