//
// Created by Administrator on 2019/6/11.
//

#include <jni.h>
#include <stdio.h>
//va_list, vsnprintf
#include <stdarg.h>
#include <errno.h>
#include <cstring>
//socket, bind, getsockname, listen, accept, recv, send, connect
#include <sys/types.h>
#include <sys/socket.h>
//sockaddr_un
#include <sys/un.h>
//htons, sockaddr_in
#include <netinet/in.h>
//inet_ntop
#include <arpa/inet.h>
//close, unlink
#include <unistd.h>
//offsetof
#include <stddef.h>
#include "../wrapper_log.h"

#define MAX_LOG_MESSAGE_LENGTH 256
#define MAX_BUFFER_SIZE 80

/**
 *将消息记录到应用
 */
static void LogMessage(JNIEnv *env, jobject obj, const char *format, ...) {
    static jmethodID methodId = NULL;//缓存方法Id
    if (NULL == methodId) {
        LOG_INFO("log -- find LogMessage");
        jclass clazz = env->GetObjectClass(obj);
        methodId = env->GetMethodID(clazz, "logMessage", "(Ljava/lang/String;)V");
        env->DeleteLocalRef(clazz);//回收局部变量
    }
    if (NULL != methodId) {
        char buffer[MAX_LOG_MESSAGE_LENGTH];
        va_list ap;
        va_start(ap, format);
        vsnprintf(buffer, MAX_LOG_MESSAGE_LENGTH, format, ap);
        va_end(ap);
        jstring logStr = env->NewStringUTF(buffer);
        if (NULL != logStr) {
            env->CallVoidMethod(obj, methodId, logStr);
            env->DeleteLocalRef(logStr);
        }
    }
}

/**
 *抛出给定异常
 */
static void ThrowException(JNIEnv *env, const char *clazzName, const char *message) {
    jclass clazz = env->FindClass(clazzName);
    if (NULL != clazz) {
        LOG_ERROR("throw exception -- %s", message);
        env->ThrowNew(clazz, message);
        env->DeleteLocalRef(clazz);
    }
}

/**
 *抛出异常，基于错误号
 */
static void ThrowErrnoException(JNIEnv *env, const char *clazzName, int errnum) {
    char buffer[MAX_LOG_MESSAGE_LENGTH];
    //获取错误号消息
    if (-1 == strerror_r(errnum, buffer, MAX_LOG_MESSAGE_LENGTH)) {
        strerror_r(errno, buffer, MAX_LOG_MESSAGE_LENGTH);
    }
    ThrowException(env, clazzName, buffer);
}

//jint JNI_OnLoad(JavaVM *vm, void *reserved) {
//    LOG_INFO("echo OnLoad 自动调用");
//    return JNI_VERSION_1_6;
//}

static int NewTcpSocket(JNIEnv *env, jobject obj) {
    LogMessage(env, obj, "Constructing a new TCP socket...");
    //构造Socket, 流
    int tcpSocket = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    LOG_INFO("tcp socket: %i", tcpSocket);
    if (-1 == tcpSocket)
        ThrowErrnoException(env, "java/io/IOException", errno);
    return tcpSocket;
}

static int NewUdpSocket(JNIEnv *env, jobject obj) {
    LogMessage(env, obj, "Constructing a new UDP socket...");
    int udpSocket = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);//数据报
    LOG_INFO("udp socket: %i", udpSocket);
    if (-1 == udpSocket)
        ThrowErrnoException(env, "java/io/IOException", errno);
    return udpSocket;
}

/**
 * 将socket 绑定到某一端口上
 *P196
 */
static void BindSocketToPort(JNIEnv *env, jobject obj, int sd, unsigned short port) {
    struct sockaddr_in address;
    //绑定socket地址
    memset(&address, 0, sizeof(address));
    address.sin_family = PF_INET;
    //绑定到所有地址
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //将端口转换为网络字节顺序
    address.sin_port = htons(port);
    LogMessage(env, obj, "Binding to port %hu.", port);
    //绑定socket
    if (-1 == bind(sd, (struct sockaddr *) &address, sizeof(address))) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
}

/**
 *获取绑定的端口号
 * htons将 unsigned short 从主机字节排序转换到网络字节排序
 * ntohs将 unsigned short 从网络字节排序转换到主机字节排序
 * htonl将 unsigned integer 从主机字节排序转换到网络字节排序
 * ntohl将 unsigned integer 从网络字节排序转换到主机字节排序
 * 具体见P198
 */
static unsigned short GetSocketPort(JNIEnv *env, jobject obj, int sd) {
    unsigned short port = 0;
    struct sockaddr_in address;
    socklen_t addrLength = sizeof(address);
    //获取socket地址
    if (-1 == getsockname(sd, (struct sockaddr *) &address, &addrLength)) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        //将端口转换为主机字节顺序
        port = ntohs(address.sin_port);
        LogMessage(env, obj, "Binded to random port %hu.", port);
    }
    return port;
}

/**
 *监听指定待处理连接的backlog 的socket，当backlog 已满时拒绝新的连接
 * listen 函数监听输入连接只是简单地将输入连接放进一个队列里并等待程序显式地接受
 */
static void ListenOnSocket(JNIEnv *env, jobject obj, int sd, int backlog) {
    LogMessage(env, obj, "Listening on socket with a backlog of %d pending connections.", backlog);
    //监听给定backlog 的 socket
    if (-1 == listen(sd, backlog)) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
}

/**
 *记录给定地址的IP地址和端口号
 */
static void LogAddress(JNIEnv *env, jobject obj, const char *message, const struct sockaddr_in *address) {
    char ip[INET_ADDRSTRLEN];
    //将IP地址转换为字符串
    if (NULL == inet_ntop(PF_INET, &(address->sin_addr), ip, INET_ADDRSTRLEN)) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        unsigned short port = ntohs(address->sin_port);
        LogMessage(env, obj, "%s %s: %hu.", message, ip, port);
    }
}

/**
 * 阻塞等待接受client连接并返回client socket
 *  accept（是阻塞函数） 显示地将输入连接从监听队列里取出并接受
 */
static int AcceptOnSocket(JNIEnv *env, jobject obj, int sd) {
    struct sockaddr_in address;
    socklen_t addrLength = sizeof(address);
    LogMessage(env, obj, "Waiting for a client connection...");
    int clientSocket = accept(sd, (struct sockaddr *) &address, &addrLength);
    //client socket无效
    if (-1 == clientSocket) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        LogAddress(env, obj, "Client connection from ", &address);
    }
    return clientSocket;
}

/**
 *  阻塞并接收来自 socket 的数据放到缓冲区
 *  @return 接收到的数据大小
 * 从socket 接收数据：recv（阻塞函数）
 */
static ssize_t ReceiveFromSocket(JNIEnv *env, jobject obj, int sd, char *buffer, size_t bufferSize) {
    LogMessage(env, obj, "Receiving from the socket...");
    //阻塞并接收来自 socket 的数据放到缓冲区
    ssize_t recvSize = recv(sd, buffer, bufferSize - 1, 0);
    //-1 表示接收失败
    if (-1 == recvSize) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        //缓冲区以NULL 结尾，组成一个字符串
        buffer[recvSize] = NULL;
        //大于0 表示接收成功
        if (recvSize > 0) {
            LogMessage(env, obj, "Received %d bytes: %s", recvSize, buffer);
        } else {
            //recv 返回0，表示socket 连接失败（已断开连接？）
            LogMessage(env, obj, "client disconnected.");
        }
    }
    return recvSize;
}

/**
 *接收UDP 数据报
 */
static ssize_t ReceiveDatagramFromSocket(JNIEnv *env, jobject obj, int sd, struct sockaddr_in *address,
                                         char *buffer, size_t bufferSize) {
    socklen_t addressLength = sizeof(struct sockaddr_in);
    LogMessage(env, obj, "Receiving from the socket...");
    //从socket 中接收数据报
    ssize_t recvSize = recvfrom(sd, buffer, bufferSize, 0, (struct sockaddr *) address, &addressLength);
    //-1 接收失败
    if (-1 == recvSize) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        LogAddress(env, obj, "Received from", address);
        buffer[recvSize] = NULL;
        if (recvSize > 0) {
            LogMessage(env, obj, "Received %d bytes: %s", recvSize, buffer);
        }
        //返回 0 连接失败
    }
    return recvSize;
}

/**
 * 将数据缓冲区发送到 socket
 *向socket 发送数据：send（阻塞函数）
 */
static ssize_t SendToSocket(JNIEnv *env, jobject obj, int sd, const char *buffer, size_t bufferSize) {
    LogMessage(env, obj, "Sending to the socket...");
    //将数据缓冲区发送到 socket，返回已发送字节数
    ssize_t sentSize = send(sd, buffer, bufferSize, 0);
    //-1 表示发送失败
    if (-1 == sentSize) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        if (sentSize > 0) {
            LogMessage(env, obj, "Sent %d bytes: %s", sentSize, buffer);
        } else {
            //send 返回0，表示socket 连接失败（已断开连接？）
            LogMessage(env, obj, "Client disconnected.");
        }
    }
    return sentSize;
}

/**
 *向UDP 发送数据报
 */
static ssize_t SendDatagramToSocket(JNIEnv *env, jobject obj, int sd, const struct sockaddr_in *address,
                                    const char *buffer, size_t bufferSize) {
    LogAddress(env, obj, "Sending to", address);
    ssize_t sentSize = sendto(sd, buffer, bufferSize, 0, (const sockaddr *) address, sizeof(struct sockaddr_in));
    //-1 发送失败
    if (-1 == sentSize)
        ThrowErrnoException(env, "java/io/IOException", errno);
    else if (sentSize > 0) {
        LogMessage(env, obj, "Sent %d bytes: %s", sentSize, buffer);
    }
    //返回0 连接失败
    return sentSize;
}