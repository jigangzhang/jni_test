//
// Created by Administrator on 2019/6/12.
//

#ifndef JNI_TEST_NATIVE_CLIENT_H
#define JNI_TEST_NATIVE_CLIENT_H

#endif //JNI_TEST_NATIVE_CLIENT_H

#include <jni.h>

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_socket_EchoClientActivity_nativeStartTcpClient(JNIEnv *, jobject, jstring, jint, jstring);