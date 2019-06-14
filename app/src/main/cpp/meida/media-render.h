//
// Created by Administrator on 2019/6/14.
//

#ifndef JNI_TEST_MEDIA_RENDER_H
#define JNI_TEST_MEDIA_RENDER_H

#include <android/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jboolean JNICALL
Java_com_god_seep_jni_1test_media_BitmapPlayerActivity_render(JNIEnv *, jclass, jlong, jobject);

#ifdef __cplusplus
}
#endif
#endif //JNI_TEST_MEDIA_RENDER_H
