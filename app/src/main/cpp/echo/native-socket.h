//
// Created by Administrator on 2019/6/11.
//

#ifndef JNI_TEST_NATIVE_SOCKET_H
#define JNI_TEST_NATIVE_SOCKET_H

#endif //JNI_TEST_NATIVE_SOCKET_H

#include <jni.h>

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_socket_EchoServerActivity_nativeStartTcpServer(JNIEnv *, jobject, jint);

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_socket_EchoServerActivity_nativeStartUdpServer(JNIEnv *, jobject, jint);