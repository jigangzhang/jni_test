//
// Created by Administrator on 2019/6/14.
//
extern "C" {
#include "../../../../../../../android-sdk/ndk-bundle/sources/transcode-1.1.7/avilib/avilib.h"
}

#include "Common.h"
#include "media-avi.h"

jlong Java_com_god_seep_jni_1test_media_BasePlayerActivity_open(JNIEnv *env, jclass clazz, jstring fileName) {
    avi_t *avi = 0;
    const char *cFileName = env->GetStringUTFChars(fileName, 0);
    if (0 == cFileName)
        goto exit;
    //打开AVI文件
    avi = AVI_open_input_file(cFileName, 1);
    env->ReleaseStringUTFChars(fileName, cFileName);
    if (0 == avi)
        ThrowException(env, "java/io/IOException", AVI_strerror());
    exit:
    return (jlong) avi;
}

jint Java_com_god_seep_jni_1test_media_BasePlayerActivity_getWidth(JNIEnv *env, jclass clazz, jlong avi) {
    return AVI_video_width((avi_t *) avi);
}

jint Java_com_god_seep_jni_1test_media_BasePlayerActivity_getHeight(JNIEnv *env, jclass clazz, jlong avi) {
    return AVI_video_height((avi_t *) avi);
}

jdouble Java_com_god_seep_jni_1test_media_BasePlayerActivity_getFrameRate(JNIEnv *env, jclass clazz, jlong avi) {
    return AVI_frame_rate((avi_t *) avi);
}

void Java_com_god_seep_jni_1test_media_BasePlayerActivity_close(JNIEnv *env, jclass clazz, jlong avi) {
    AVI_close((avi_t *) avi);
}
