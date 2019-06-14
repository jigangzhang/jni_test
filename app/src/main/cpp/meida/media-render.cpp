//
// Created by Administrator on 2019/6/14.
//
extern "C" {
#include "../../../../../../../android-sdk/ndk-bundle/sources/transcode-1.1.7/avilib/avilib.h"
}

#include "Common.h"
#include "media-render.h"

jboolean Java_com_god_seep_jni_1test_media_BitmapPlayerActivity_render(JNIEnv *env, jclass clazz,
                                                                       jlong avi, jobject bitmap) {
    jboolean isFrameRead = JNI_FALSE;
    char *frameBuffer = 0;
    long frameSize = 0;
    int keyFrame = 0;
    //锁定bitmap 并得到 raw byte
    if (0 > AndroidBitmap_lockPixels(env, bitmap, (void **) &frameBuffer)) {
        ThrowException(env, "java/io/IOException", "Unable to lock pixels. in media render");
        goto exit;
    }
    //将AVI 帧 byte 读到 bitmap中
    frameSize = AVI_read_frame((avi_t *) avi, frameBuffer, &keyFrame);
    //解锁 bitmap
    if (0 > AndroidBitmap_unlockPixels(env, bitmap)) {
        ThrowException(env, "java/io/IOException", "Unable to unlock pixels. in media render");
        goto exit;
    }
    //检查帧是否读取成功
    if (0 < frameSize) {
        isFrameRead = JNI_TRUE;
    }
    exit:
    return isFrameRead;
}
