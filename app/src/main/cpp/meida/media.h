//
// Created by Administrator on 2019/6/14.
//

#ifndef JNI_TEST_MEDIA_H
#define JNI_TEST_MEDIA_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_god_seep_jni_1test_media_MediaAviActivity_play(JNIEnv *, jclass, jstring);

#ifdef __cplusplus
}
#endif
#endif //JNI_TEST_MEDIA_H
