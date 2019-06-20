//
// Created by Administrator on 2019/6/14.
//

#include "media.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "Common.h"

extern "C" {
#include "../../../../../../../android-sdk/ndk-bundle/sources/transcode-1.1.7/avilib/wavlib.h"
}

static const char *JAVA_LANG_IOEXCEPTION = "java/lang/IOException";
static const char *JAVA_LANG_OUTOFMEMORYERROR = "java/lang/OutOfMemoryError";

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

struct PlayerContext {
    SLObjectItf engineObject;
    SLEngineItf engineEngine;
    SLObjectItf outputMixObject;
    SLObjectItf audioPlayerObject;
    SLAndroidSimpleBufferQueueItf audioPlayerBufferQueue;
    SLPlayItf audioPlayerPlay;
    WAV wav;

    unsigned char *buffer;
    size_t bufferSize;

    PlayerContext() :
            engineObject(0), engineEngine(0), outputMixObject(0),
            audioPlayerBufferQueue(0), audioPlayerPlay(0), wav(0), bufferSize(0) {}
};

static WAV OpenWaveFile(JNIEnv *env, jstring fileName) {
    WAVError error = WAV_SUCCESS;
    WAV wav = 0;
    const char *cFileName = env->GetStringUTFChars(fileName, 0);
    if (0 == cFileName) {
        goto exit;
    }
    wav = wav_open(cFileName, WAV_READ, &error);
    env->ReleaseStringUTFChars(fileName, cFileName);
    if (0 == wav) {
        ThrowException(env, JAVA_LANG_IOEXCEPTION, wav_strerror(error));
    }
    exit:
    return wav;
}

static void CloseWaveFile(WAV wav) {
    if (0 != wav)
        wav_close(wav);
}

/**
 * 将OpenSL ES 结果转换为字符串
 */
static const char *ResultToString(SLresult result) {
    const char *str = 0;
    switch (result) {
        case SL_RESULT_SUCCESS:
            str = "Success";
            break;
        case SL_RESULT_PRECONDITIONS_VIOLATED:
            str = "违反了先决条件";
            break;
        case SL_RESULT_PARAMETER_INVALID:
            str = "无效参数";
            break;
        case SL_RESULT_MEMORY_FAILURE:
            str = "Memory failure(内存错误)";
            break;
        case SL_RESULT_RESOURCE_ERROR:
            str = "Resource error(资源错误)";
            break;
        case SL_RESULT_RESOURCE_LOST:
            str = "资源丢失";
            break;
        case SL_RESULT_IO_ERROR:
            str = "IO 错误";
            break;
        case SL_RESULT_BUFFER_INSUFFICIENT:
            str = "buffer 不足";
            break;
        case SL_RESULT_CONTENT_CORRUPTED:
            str = "Success(CONTENT_CORRUPTED)";
            break;
        case SL_RESULT_CONTENT_UNSUPPORTED:
            str = "不支持的内容";
            break;
        case SL_RESULT_CONTENT_NOT_FOUND:
            str = "未找到 Content";
            break;
        case SL_RESULT_PERMISSION_DENIED:
            str = "未获得权限";
            break;
        case SL_RESULT_FEATURE_UNSUPPORTED:
            str = "不支持的 Feature";
            break;
        case SL_RESULT_INTERNAL_ERROR:
            str = "内部错误";
            break;
        case SL_RESULT_UNKNOWN_ERROR:
            str = "未知错误";
            break;
        case SL_RESULT_OPERATION_ABORTED:
            str = "Operation aborted(操作失败)";
            break;
        case SL_RESULT_CONTROL_LOST:
            str = "Control lost(丢失控制)";
            break;
        default:
            str = "未知 Code";
    }
    return str;
}

/**
 * 检查结果 抛出异常
 */
static bool CheckError(JNIEnv *env, SLresult result) {
    bool isError = false;
    if (SL_RESULT_SUCCESS != result) {
        ThrowException(env, JAVA_LANG_IOEXCEPTION, ResultToString(result));
        isError = true;
    }
    return isError;
}

/**
 * 先创建一个引擎对象，以便访问其他API
 */
static void CreateEngine(JNIEnv *env, SLObjectItf &engineObject) {
    //OpenSL ES 线程安全的，该选项请求被忽略， 但应保证源码可移植到其他平台
    SLEngineOption engineOptions[] = {{(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE}};
    //创建 OpenSL ES 引擎对象
    SLresult result = slCreateEngine(&engineObject, ARRAY_LEN(engineOptions), engineOptions, 0, 0, 0);
    CheckError(env, result);
}

/**
 * 对象已创建处于未实例化状态，对象已存在但未分配任何资源，要使用对象先实例化
 */
static void RealizeObject(JNIEnv *env, SLObjectItf object) {
    //实现引擎对象
    SLresult result = (*object)->Realize(object, SL_BOOLEAN_FALSE);
    CheckError(env, result);
}

/**
 * 销毁对象实例
 */
static void DestroyObject(SLObjectItf &object) {
    if (object != 0) {
        (*object)->Destroy(object);
        object = 0;
    }
}

/**
 * 从引擎对象中获得引擎接口以便从该引擎中创建其他对象
 */
static void GetEngineInterface(JNIEnv *env, SLObjectItf &engineObject, SLEngineItf &engineEngine) {
    //获得引擎接口
    SLresult result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    CheckError(env, result);
}

/**
 * 创建和输出混合对象
 */
static void CreateOutputMix(JNIEnv *env, SLEngineItf engineEngine, SLObjectItf &outputMixObject) {
    SLresult result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    CheckError(env, result);
}

/**
 * 释放播放器缓冲区
 */
static void FreePlayerBuffer(unsigned char *&buffers) {
    if (buffers != 0) {
        delete buffers;
        buffers = 0;
    }
}

/**
 * 初始化播放器缓冲区, 通过WAVE 音频文件头获取跟占用 缓冲区值的大小，根据输入文件大小得出
 */
static void InitPlayerBuffer(JNIEnv *env, WAV wav, unsigned char *&buffer, size_t &bufferSize) {
    //计算缓冲区大小
    bufferSize = wav_get_channels(wav) * wav_get_rate(wav) * wav_get_bits(wav);
    buffer = new unsigned char[bufferSize];
    if (buffer == 0) {
        ThrowException(env, JAVA_LANG_OUTOFMEMORYERROR, "buffer");
    }
}

/**
 * 创建缓冲区队列音频播放器
 */
static void CreateBufferQueueAudioPlayer(WAV wav, SLEngineItf engineEngine, SLObjectItf outputMixObject,
                                         SLObjectItf &audioPlayerObject) {
    //android 针对数据源的简单缓存区队列定位器
    SLDataLocator_AndroidSimpleBufferQueue dataSourceLocator = {SL_DATALOCATOR_ANDROIDBUFFERQUEUE,//定位器类型
                                                                1//缓冲区数
    };
    //PCM 数据源格式
    SLDataFormat_PCM dataSourceFormat = {
            SL_DATAFORMAT_PCM,//格式类型
            wav_get_channels(wav),//通道数
            (SLuint32) (wav_get_rate(wav) * 1000), //mHz/s 的样本数
            wav_get_bits(wav),//每个样本的位数
            wav_get_bits(wav),//容器大小
            SL_SPEAKER_FRONT_CENTER, //通道屏蔽
            SL_BYTEORDER_LITTLEENDIAN, //字节顺序
    };
    //数据源是含有PCM 格式的简单缓冲区队列
    SLDataSource dataSource = {
            &dataSourceLocator,//数据定位器
            &dataSourceFormat//数据格式
    };
    //针对数据接收器的输出混合定位器
    SLDataLocator_OutputMix dataSinkLocator = {
            SL_DATALOCATOR_OUTPUTMIX,//定位器类型
            outputMixObject//输出混合
    };
    //数据定位器是一个输出混合
    SLDataSink dataSink = {
            &dataSinkLocator,//定位器
            0//格式
    };
    //需要的接口
    SLInterfaceID interfaceIds[] = {
            SL_IID_BUFFERQUEUE
    };
    //需要的接口，
    SLboolean requiredInterfaces[] = {
            SL_BOOLEAN_TRUE//for SL_IID_BUFFERQUEUE
    };
    //创建音频播放器对象
    SLresult result = (*engineEngine)->CreateAudioPlayer(engineEngine, &audioPlayerObject, &dataSource,
                                                         &dataSink, ARRAY_LEN(interfaceIds), interfaceIds,
                                                         requiredInterfaces);
}

/**
 * 获得音频播放器缓存区队列接口
 */
static void
GetAudioPlayerBufferQueueInterface(JNIEnv *env, SLObjectItf audioPlayerObject,
                                   SLAndroidSimpleBufferQueueItf &audioPlayerBufferQueue) {
    //获得缓冲区队列接口
    SLresult result = (*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_BUFFERQUEUE,
                                                         &audioPlayerBufferQueue);
    CheckError(env, result);
}

/**
 *销毁播放器上下文，播放器停止时释放 OpenSL ES 资源和缓冲区
 */
static void DestroyContext(PlayerContext *&ctx) {
    //销毁音频播放器对象
    DestroyObject(ctx->audioPlayerObject);
    //释放播放器缓冲区
    FreePlayerBuffer(ctx->buffer);
    //销毁输出混合对象
    DestroyObject(ctx->outputMixObject);
    //销毁引擎实例
    DestroyObject(ctx->engineObject);
    //关闭WAVE 文件
    CloseWaveFile(ctx->wav);
    //释放上下文
    delete ctx;
    ctx = 0;
}

/**
 * 当一个缓冲区完成播放时获得调用 （回调函数）
 * 读取排列下一个用于播放的音频数据块，若WAVE 播放完毕，DestroyContext 释放资源
 */
static void PlayerCallback(SLAndroidSimpleBufferQueueItf audioPlayerBufferQueue, void *context) {
    PlayerContext *ctx = (PlayerContext *) context;
    ssize_t readSize = wav_read_data(ctx->wav, ctx->buffer, ctx->bufferSize);
    if (readSize > 0) {
        (*audioPlayerBufferQueue)->Enqueue(audioPlayerBufferQueue, ctx->buffer, readSize);
    } else {
        DestroyContext(ctx);
    }
}

/**
 * 注册播放器回调
 */
static void
RegisterPlayerCallback(JNIEnv *env, SLAndroidSimpleBufferQueueItf audioPlayerBufferQueue, PlayerContext *ctx) {
    //注册播放器回调
    SLresult result = (*audioPlayerBufferQueue)->RegisterCallback(audioPlayerBufferQueue, PlayerCallback, ctx);
    CheckError(env, result);
}

/**
 * 获得音频播放器播放接口
 */
static void GetAudioPlayerPlayInterface(JNIEnv *env, SLObjectItf audioPlayerObject, SLPlayItf &audioPlayerPlay) {
    //获得播放器接口
    SLresult result = (*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_PLAY, &audioPlayerPlay);
    CheckError(env, result);
}

/**
 * 启动音频播放器，把音频播放器设置为播放状态
 */
static void SetAudioPlayerStatePlaying(JNIEnv *env, SLPlayItf audioPayerPlay) {
    SLresult result = (*audioPayerPlay)->SetPlayState(audioPayerPlay, SL_PLAYSTATE_PLAYING);
    CheckError(env, result);
}

/**
 * 使用上面的辅助函数实现播放
 */
void Java_com_god_seep_jni_1test_media_MediaAviActivity_play(JNIEnv *env, jclass clazz, jstring fileName) {
    PlayerContext *ctx = new PlayerContext();

    ctx->wav = OpenWaveFile(env, fileName);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }

    //创建引擎
    CreateEngine(env, ctx->engineObject);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }

    //实现引擎对象
    RealizeObject(env, ctx->engineObject);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //获得引擎接口
    GetEngineInterface(env, ctx->engineObject, ctx->engineEngine);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //创建输出混合对象
    CreateOutputMix(env, ctx->engineEngine, ctx->outputMixObject);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //实现输出混合对象
    RealizeObject(env, ctx->outputMixObject);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //初始化缓冲区
    InitPlayerBuffer(env, ctx->wav, ctx->buffer, ctx->bufferSize);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //创建缓冲区队列音频播放器对象
    CreateBufferQueueAudioPlayer(ctx->wav, ctx->engineEngine, ctx->outputMixObject, ctx->audioPlayerObject);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //实现音频播放器对象
    RealizeObject(env, ctx->audioPlayerObject);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //获得音频播放器缓冲区队列接口
    GetAudioPlayerBufferQueueInterface(env, ctx->audioPlayerObject, ctx->audioPlayerBufferQueue);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //注册播放器回调函数
    RegisterPlayerCallback(env, ctx->audioPlayerBufferQueue, ctx);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //获得音频播放器播放接口
    GetAudioPlayerPlayInterface(env, ctx->audioPlayerObject, ctx->audioPlayerPlay);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //设置音频播放器为播放状态
    SetAudioPlayerStatePlaying(env, ctx->audioPlayerPlay);
    if (env->ExceptionOccurred() != 0) {
        goto exit;
    }
    //将第一个缓冲区入队来启动运行
    PlayerCallback(ctx->audioPlayerBufferQueue, ctx);
    exit:
    if (env->ExceptionOccurred() != 0) {
        DestroyContext(ctx);
    }
}