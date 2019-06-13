//
// Created by Administrator on 2019/6/13.
//

#include "native-local.h"
#include "echo.cpp"

static int NewLocalSocket(JNIEnv *env, jobject obj) {
    LogMessage(env, obj, "Constructing a new local UNIX socket...");
    int localSocket = socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (-1 == localSocket)
        ThrowErrnoException(env, "java/io/IOException", errno);
    return localSocket;
}

static void BindLocalSocketToName(JNIEnv *env, jobject obj, int sd, const char *name) {
    struct sockaddr_un address;
    const size_t nameLength = strlen(name);
    size_t pathLength = nameLength;
    //以'/'开头 在文件系统中 否则 为抽象命名空间
    bool absNamespace = ('/' != name[0]);
    //抽象命名空间要求目录的第一个字节是0字节， 更新路径长度包括0字节
    if (absNamespace)
        pathLength++;
    if (pathLength > sizeof(address.sun_path))
        ThrowException(env, "java/io/IOException", "Name is too big.");
    else {
        //清除地址字节
        memset(&address, 0, sizeof(address));
        address.sun_family = PF_LOCAL;
        char *sunPath = address.sun_path;
        //第一字节为0
        if (absNamespace)
            *sunPath++ = NULL;
        //追加本地名字
        strcpy(sunPath, name);
        //地址长度
        socklen_t addressLength = (offsetof(struct sockaddr_un, sun_path)) + pathLength;
        //如果socket名已经绑定，取消连接
        unlink(address.sun_path);
        LogMessage(env, obj, "Binding to local name %s%s.", absNamespace ? "(null)" : "", name);
        //绑定socket
        if (-1 == bind(sd, (struct sockaddr *) &address, addressLength)) {
            ThrowErrnoException(env, "java/io/IOException", errno);
        }
    }
}

static int AcceptOnLocalSocket(JNIEnv *env, jobject obj, int sd) {
    LogMessage(env, obj, "Waiting for a client connection...");
    int clientSocket = accept(sd, NULL, NULL);
    if (-1 == clientSocket) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
    return clientSocket;
}

extern "C" JNIEXPORT void JNICALL
Java_com_god_seep_jni_1test_socket_EchoLocalActivity_nativeStartLocalServer(JNIEnv *env, jobject obj, jstring port) {
    int serverSocket = NewLocalSocket(env, obj);
    if (NULL == env->ExceptionOccurred()) {
        const char *nameText = env->GetStringUTFChars(port, NULL);
        if (NULL == nameText)
            goto exit;
        BindLocalSocketToName(env, obj, serverSocket, nameText);
        env->ReleaseStringUTFChars(port, nameText);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        ListenOnSocket(env, obj, serverSocket, 5);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        int clientSocket = AcceptOnLocalSocket(env, obj, serverSocket);
        if (NULL != env->ExceptionOccurred())
            goto exit;
        char buffer[MAX_BUFFER_SIZE];
        ssize_t recvSize;
        ssize_t sentSize;
        while (true) {
            recvSize = ReceiveFromSocket(env, obj, clientSocket, buffer, MAX_BUFFER_SIZE);
            if (0 == recvSize || NULL != env->ExceptionOccurred())
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