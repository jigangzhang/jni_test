//
// Created by Administrator on 2019/6/14.
//

#ifndef JNI_TEST_MEDIA_RENDER_H
#define JNI_TEST_MEDIA_RENDER_H

#include <android/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 使用bitmap 渲染
 */
JNIEXPORT jboolean JNICALL
Java_com_god_seep_jni_1test_media_BitmapPlayerActivity_render(JNIEnv *, jclass, jlong, jobject);

/**
 * 使用openGL 渲染
 */
JNIEXPORT jlong JNICALL
Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_init(JNIEnv *, jclass, jlong);

JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_initSurface(JNIEnv *, jclass, jlong, jlong);

JNIEXPORT jboolean JNICALL
Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_render(JNIEnv *, jclass, jlong, jlong);

JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_free(JNIEnv *, jclass, jlong);

/**
 *  使用原生Window渲染
 */
JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_media_NativeWindowActivity_init(JNIEnv *, jclass, jlong, jobject);

JNIEXPORT jboolean JNICALL
Java_com_god_seep_jni_1test_media_NativeWindowActivity_render(JNIEnv *, jclass, jlong, jobject);

#ifdef __cplusplus
}
#endif
#endif //JNI_TEST_MEDIA_RENDER_H
