//
// Created by Administrator on 2019/6/6.
//
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "native-thread.h"
#include "wrapper_log.h"

struct NativeWorkerArgs {
    jint id;
    jint iterations;
} WorkerArgs;

static JavaVM *gVm = NULL;//java虚拟机接口指针，全局变量，作缓存，供多线程使用
static jobject gObj = nullptr;//java对象的全局引用（activity）
static jmethodID gOnNativeMessage = NULL;//全局变量，缓存
static pthread_mutex_t mutex;
static sem_t sem;//信号量

extern "C" void Java_com_god_seep_jni_1test_ThreadActivity_nativeInit(JNIEnv *env, jobject obj) {
    if (NULL == gObj) {
        gObj = env->NewGlobalRef(obj);//为Java对象创建全局引用
        if (NULL == gObj)
            goto exit;
    }
    if (NULL == gOnNativeMessage) {
        jclass clazz = env->GetObjectClass(obj);
        gOnNativeMessage = env->GetMethodID(clazz, "onNativeMessage", "(Ljava/lang/String;)V");
        if (NULL == gOnNativeMessage) {
            jclass rtClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(rtClazz, "未找到方法 onNativeMessage");
            goto exit;
        }
    }
    //初始化互斥锁
    if (0 != pthread_mutex_init(&mutex, NULL)) {
        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(rtClazz, "unable to initialize mutex.");
        goto exit;
    }
    //初始化信号量
    if (0 != sem_init(&sem, 0, 1)) {
        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(rtClazz, "unable to initialize sem.");
        goto exit;
    }
    exit:
    return;
}

extern "C" void Java_com_god_seep_jni_1test_ThreadActivity_nativeFree(JNIEnv *env, jobject obj) {
    gOnNativeMessage = nullptr;
    if (NULL != gObj) {
        env->DeleteGlobalRef(gObj);//删除全局引用，否则会发生内存泄漏
        gObj = nullptr;
    }
    //销毁互斥锁，成功返回0，否则返回错误代码
    if (0 != pthread_mutex_destroy(&mutex)) {
        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(rtClazz, "unable to destroy mutex.");
    }
    //销毁信号量
    if (0 != sem_destroy(&sem)) {
        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(rtClazz, "unable to destroy sem.");
    }
}

extern "C" void Java_com_god_seep_jni_1test_ThreadActivity_nativeWorker(JNIEnv *env, jobject obj,
                                                                        jint id, jint iterations) {
    //锁定互斥锁
//    if (0 != pthread_mutex_lock(&mutex)) {
//        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
//        env->ThrowNew(rtClazz, "unable to lock mutex.");
//        goto exit;
//    }
    //锁定信号量
    if (0 != sem_wait(&sem)) {
        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(rtClazz, "unable to lock sem.");
        goto exit;
    }
    for (int i = 0; i < iterations; ++i) {
        //准备消息
        char message[26];
        sprintf(message, "Worker %d: Iteration %d", id, i);
        //创建java字符串（来自C字符串）
        jstring msgStr = env->NewStringUTF(message);
        //调用java回调方法
        env->CallVoidMethod(obj, gOnNativeMessage, msgStr);
        //检查是否产生异常
        if (NULL != env->ExceptionOccurred())
            break;
        //睡眠1秒
        sleep(1);
    }
    //解锁互斥锁
//    if (0 != pthread_mutex_unlock(&mutex)) {
//        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
//        env->ThrowNew(rtClazz, "unable to unlock mutex.");
//        goto exit;
//    }
    //解锁信号量
    if (0 != sem_post(&sem)) {
        jclass rtClazz = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(rtClazz, "unable to unlock sem.");
        goto exit;
    }
    exit:
    return;
}

/**
 *将POSIX线程附着到虚拟机上，已获得一个有效的 JNIEnv 接口指针
 */
static void *nativeWorkerThread(void *args) {
    JNIEnv *env = nullptr;
    //将当前线程附着到JavaVM上
    if (0 == gVm->AttachCurrentThread(&env, NULL)) {
        NativeWorkerArgs *workerArgs = (NativeWorkerArgs *) args;
        //执行上面的Worker程序
        Java_com_god_seep_jni_1test_ThreadActivity_nativeWorker(env, gObj, workerArgs->id, workerArgs->iterations);
        //释放
        delete workerArgs;
        //从JavaVM 中分离当前线程
        gVm->DetachCurrentThread();
    }
    return (void *) 1;
}

extern "C" void
Java_com_god_seep_jni_1test_ThreadActivity_posixThreads(JNIEnv *env, jobject obj,
                                                        jint threads, jint iterations) {
    //线程句柄
    pthread_t *handles = new pthread_t[threads];
    for (int i = 0; i < threads; ++i) {
        NativeWorkerArgs *args = new NativeWorkerArgs();
        args->id = i;
        args->iterations = iterations;
        //创建线程
        int result = pthread_create(&handles[i], NULL, nativeWorkerThread, args);

        if (0 != result) {
            jclass exClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(exClazz, "create pthread failed");
            goto exit;
        }
    }
    //等待线程终止
    for (int i = 0; i < threads; ++i) {
        void *result = NULL;
        //pthread_join 连接每个线程句柄，等待线程终止（将挂起UI线程，直到创建的线程终止，先显示下面消息，后显示迭代消息）
        if (0 != pthread_join(handles[i], &result)) {
            jclass rtClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(rtClazz, "native join thread failed");
        } else {
            char message[26];
            sprintf(message, "Worker %d returned %d", i, result);
            jstring msgStr = env->NewStringUTF(message);
            env->CallVoidMethod(obj, gOnNativeMessage, msgStr);
            if (NULL != env->ExceptionOccurred())
                goto exit;
        }
    }
    exit:
    return;
}

/**
 * java虚拟机不能识别 POSIX线程，需要将POSIX附着到虚拟机上（需要 JavaVM接口）
 *当共享库开始加载时虚拟机自动调用该函数
 */
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOG_INFO("JNI OnLoad 自动调用");
    gVm = vm;
    return JNI_VERSION_1_6;
}
/**
 *POSIX线程同步：
 *      互斥锁：Mutexes，特定代码部分不同时执行，具体看P177
 *      信号量：Semaphores，控制对特定数目资源的访问，如果没有可用资源，调用线程在信号量涉及的资源上等待，直到资源可用
 *              信号量的值大于0， 可上锁，并且值递减，如果信号量的值为0，调用线程挂起，直到解锁。具体见P180
 */