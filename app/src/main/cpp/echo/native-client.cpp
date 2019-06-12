//
// Created by Administrator on 2019/6/12.
//

#include "native-client.h"
#include "echo.cpp"

/**
 * 连接到给定的IP地址和端口上
 */
static void ConnectToAddress(JNIEnv *env, jobject obj, int sd, const char *ip, unsigned short port) {
    LogMessage(env, obj, "Connecting to %s: %hu...", ip, port);
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = PF_INET;
    //将IP地址字符串转换为网络地址
    if (0 == inet_aton(ip, &address.sin_addr)) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        address.sin_port = htons(port);
        //与地址连接
        if (-1 == connect(sd, (const sockaddr *) &address, sizeof(address))) {
            ThrowErrnoException(env, "java/io/IOException", errno);
        } else {
            LogMessage(env, obj, "Connected.");
        }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_socket_EchoClientActivity_nativeStartTcpClient(JNIEnv *env, jobject obj, jstring ip,
                                                                           jint port, jstring msg) {
    int clientSocket = NewTcpSocket(env, obj);
    if (NULL == env->ExceptionOccurred()) {
        const char *ipAddress = env->GetStringUTFChars(ip, nullptr);
        if (NULL == ipAddress)
            goto exit;
        ConnectToAddress(env, obj, clientSocket, ipAddress, (unsigned short) port);
        env->ReleaseStringUTFChars(ip, ipAddress);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        const char *msgText = env->GetStringUTFChars(msg, NULL);
        if (NULL == msgText)
            goto exit;
        jsize msgSize = env->GetStringUTFLength(msg);
        SendToSocket(env, obj, clientSocket, msgText, msgSize);
        env->ReleaseStringUTFChars(msg, msgText);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        char buffer[MAX_BUFFER_SIZE];
        ReceiveFromSocket(env, obj, clientSocket, buffer, MAX_BUFFER_SIZE);
    }
    exit:
    if (clientSocket > -1)
        close(clientSocket);
}

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_socket_EchoClientActivity_nativeStartUdpClient(JNIEnv *env, jobject obj, jstring ip,
                                                                           jint port, jstring msg) {
    int clientSocket = NewUdpSocket(env, obj);
    if (NULL == env->ExceptionOccurred()) {
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = PF_INET;
        const char *ipAddress = env->GetStringUTFChars(ip, NULL);
        if (NULL == ipAddress)
            goto exit;
        int result = inet_aton(ipAddress, &address.sin_addr);
        env->ReleaseStringUTFChars(ip, ipAddress);
        if (0 == result) {
            ThrowErrnoException(env, "java/io/IOException", errno);
            goto exit;
        }
        address.sin_port = htons(port);
        const char *msgText = env->GetStringUTFChars(msg, NULL);
        if (NULL == msgText)
            goto exit;
        jsize msgSize = env->GetStringUTFLength(msg);
        SendDatagramToSocket(env, obj, clientSocket, &address, msgText, msgSize);
        env->ReleaseStringUTFChars(msg, msgText);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        char buffer[MAX_BUFFER_SIZE];
        //清除地址
        memset(&address, 0, sizeof(address));
        ReceiveDatagramFromSocket(env, obj, clientSocket, &address, buffer, MAX_BUFFER_SIZE);
    }
    exit:
    if (clientSocket > 0)
        close(clientSocket);
}