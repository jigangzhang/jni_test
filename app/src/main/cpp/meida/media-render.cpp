//
// Created by Administrator on 2019/6/14.
//
extern "C" {
#include "../../../../../../../android-sdk/ndk-bundle/sources/transcode-1.1.7/avilib/avilib.h"
}

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <malloc.h>

#include <android/native_window_jni.h>
#include <android/native_window.h>

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

/*----------*/
struct Instance {
    char *buffer;
    GLuint texture;

    Instance() :
            buffer(0), texture(0) {}
};

jlong Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_init(JNIEnv *env, jclass clazz, jlong avi) {
    Instance *instance = 0;
    long frameSize = AVI_frame_size((avi_t *) avi, 0);
    if (0 >= frameSize) {
        ThrowException(env, "java/io/RuntimeException", "获取不到 frame size.");
        goto exit;
    }
    instance = new Instance();
    if (0 == instance) {
        ThrowException(env, "java/io/RuntimeException", "不能 allocate instance");
        goto exit;
    }
    instance->buffer = (char *) malloc(frameSize);
    if (0 == instance->buffer) {
        ThrowException(env, "java/io/RuntimeException", "不能 allocate buffer");
        goto exit;
    }
    exit:
    return (jlong) instance;
}

void Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_initSurface(JNIEnv *env, jclass clazz, jlong inst,
                                                                        jlong avi) {
    Instance *instance = (Instance *) inst;
    //启用纹理
    glEnable(GL_TEXTURE_2D);
    //生成一个纹理对象
    glGenTextures(1, &instance->texture);
    //绑定到生成的纹理上
    glBindTexture(GL_TEXTURE_2D, instance->texture);
    int frameWidth = AVI_video_width((avi_t *) avi);
    int frameHeight = AVI_video_height((avi_t *) avi);
    //剪切纹理矩形
    GLint rect[] = {0, frameHeight, frameWidth, -frameHeight};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
    //填充颜色
    glColor4f(1.0, 1, 1, 1);
    //生成一个空的纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
}

jboolean
Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_render(JNIEnv *env, jclass clazz, jlong inst, jlong avi) {
    Instance *instance = (Instance *) inst;
    jboolean isFrameRead = JNI_FALSE;
    int keyFrame = 0;
    //读取 AVI帧字节至 bitmap？
    long frameSize = AVI_read_frame((avi_t *) avi, instance->buffer, &keyFrame);
    if (0 >= frameSize) {
        goto exit;
    }
    isFrameRead = JNI_TRUE;
    //使用新帧更新纹理
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, AVI_video_width((avi_t *) avi),
                    AVI_video_height((avi_t *) avi), GL_RGB, GL_UNSIGNED_SHORT_5_6_5, instance->buffer);
    //绘制纹理
    glDrawTexiOES(0, 0, 0, AVI_video_width((avi_t *) avi),
                  AVI_video_height((avi_t *) avi));
    exit:
    return isFrameRead;
}

void Java_com_god_seep_jni_1test_media_OpenGLPlayerActivity_free(JNIEnv *env, jclass clazz, jlong inst) {
    Instance *instance = (Instance *) inst;
    if (0 != instance) {
        free(instance->buffer);
        delete instance;
    }
}

/*-----------*/

void Java_com_god_seep_jni_1test_media_NativeWindowActivity_init(JNIEnv *env, jclass clazz,
                                                                 jlong avi, jobject surface) {
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow) {
        ThrowException(env, "java/io/RuntimeException", "获取不到 native window from surface");
        goto exit;
    }
    //设置Buffer 大小为AVI 视频帧的分辨率
    //如果和window的物理大小不一致， buffer会被缩放来匹配这个大小
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow, AVI_video_width((avi_t *) avi),
                                             AVI_video_height((avi_t *) avi), WINDOW_FORMAT_RGB_565)) {
        ThrowException(env, "java/io/RuntimeException", "不能设置 buffers geometry");
    }
    ANativeWindow_release(nativeWindow);
    nativeWindow = 0;
    exit:
    return;
}

jboolean Java_com_god_seep_jni_1test_media_NativeWindowActivity_render(JNIEnv *env, jclass clazz,
                                                                       jlong avi, jobject surface) {
    jboolean isFrameRead = JNI_FALSE;
    long frameSize = 0;
    int keyFrame = 0;
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow) {
        ThrowException(env, "java/io/RuntimeException", "获取不到 native window from surface");
        goto exit;
    }
    //锁定原生window 并访问原始 Buffer
    ANativeWindow_Buffer windowBuffer;
    if (0 > ANativeWindow_lock(nativeWindow, &windowBuffer, 0)) {
        ThrowException(env, "java/io/RuntimeException", "获取不到 native window from surface");
        goto release;
    }
    //将AVI 帧的比特流读至原始缓冲区
    frameSize = AVI_read_frame((avi_t *) avi, (char *) windowBuffer.bits, &keyFrame);
    if (frameSize > 0) {
        isFrameRead = JNI_TRUE;
    }
    //解锁并且输出缓冲区来显示
    if (ANativeWindow_unlockAndPost(nativeWindow) < 0) {
        ThrowException(env, "java/io/RuntimeException", "不能 unlock and post to native window");
        goto release;
    }
    release:
    ANativeWindow_release(nativeWindow);
    nativeWindow = 0;
    exit:
    return isFrameRead;
}