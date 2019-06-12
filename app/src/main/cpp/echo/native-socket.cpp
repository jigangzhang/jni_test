//
// Created by Administrator on 2019/6/11.
//
#include "native-socket.h"
#include "echo.cpp"

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_socket_EchoServerActivity_nativeStartTcpServer(JNIEnv *env, jobject obj, jint port) {
    int serverSocket = NewTcpSocket(env, obj);
    //socket 正确创建，无异常发生
    if (NULL == env->ExceptionOccurred()) {
        //将socket 绑定到端口上
        BindSocketToPort(env, obj, serverSocket, (unsigned short) port);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        //0 表示请求随机端口号
        if (0 == port) {
            GetSocketPort(env, obj, serverSocket);
            if (NULL != env->ExceptionOccurred())
                goto exit;
        }
        //5个连接
        ListenOnSocket(env, obj, serverSocket, 5);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        int clientSocket = AcceptOnSocket(env, obj, serverSocket);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        char buffer[MAX_BUFFER_SIZE];
        ssize_t recvSize;
        ssize_t sentSize;
        while (1) {
            recvSize = ReceiveFromSocket(env, obj, clientSocket, buffer, MAX_BUFFER_SIZE);
            if (0 == recvSize || (NULL != env->ExceptionOccurred()))
                break;
            sentSize = SendToSocket(env, obj, clientSocket, buffer, (size_t) recvSize);
            if (0 == sentSize || NULL != env->ExceptionOccurred())
                break;
        }
        close(clientSocket);
    }
    exit:
    if (serverSocket > 0)
        close(serverSocket);
}