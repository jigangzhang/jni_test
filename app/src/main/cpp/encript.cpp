#include <jni.h>
#include <string>
#include "aes.c"
#include "base64.h"
#include "wrapper_log.h"

using std::string;

static int addPKCS7Padding(string &data, int length);

static int removePKCS7Padding(uint8_t *data, int length);

static char *decrypt_cbc(const uint8_t *key, const uint8_t *iv, uint8_t *buffer, int length);

static char *encrypt_cbc(const uint8_t *key, const uint8_t *iv, uint8_t *buffer, int length);

jstring charToJstring(JNIEnv *envPtr, char *src);

const char *key = "_zone_@2019.code";
const char *iv = "luanlaibuyaomofa";

jstring encode(JNIEnv *env, jclass cls, jstring res) {
    const char *str = env->GetStringUTFChars(res, 0);
    LOG_ERROR("orgin :%s, length :%d", str, strlen(str));
    char *result = encrypt_cbc((uint8_t *) key, (uint8_t *) iv, (uint8_t *) str, strlen(str));
    env->ReleaseStringUTFChars(res, str);
    LOG_ERROR("after encrypt :%s, length :%d", result, strlen(result));
    LOG_ERROR("after release :%s", str);    //注意释放内存
    return env->NewStringUTF(result);
}

jstring decode(JNIEnv *env, jclass cls, jstring res) {
    const char *str = env->GetStringUTFChars(res, 0);
    LOG_ERROR("orgin :%s, length :%d", str, strlen(str));
    char *result = decrypt_cbc((uint8_t *) key, (uint8_t *) iv, (uint8_t *) str, (int) strlen(str));
    env->ReleaseStringUTFChars(res, str);
    LOG_ERROR("after decrypt :%s", result);
    LOG_ERROR("after release :%s", result);
    return charToJstring(env, result);
}

jstring charToJstring(JNIEnv *env, char *src) {
    jsize len = strlen(src);
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("UTF-8");
    jmethodID mid = env->GetMethodID(clsstring, "<init>",
                                     "([BLjava/lang/String;)V");
    jbyteArray barr = env->NewByteArray(len);
    env->SetByteArrayRegion(barr, 0, len, (jbyte *) src);

    return (jstring) env->NewObject(clsstring, mid, barr, strencode);
}

/**
 * 输入 buffer 必须是 16字节 的倍数，只需要自己做 padding，不用分段，ECB需要分段（16字节一段）
 *  内部已进行分段加密
 *  @param length buffer长度
 */
static char *encrypt_cbc(const uint8_t *key, const uint8_t *iv, uint8_t *buffer, int length) {
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    string data = (char *) buffer;
    int outLen = addPKCS7Padding(data, length);
    LOG_ERROR("after padding :%s, length :%d", data.c_str(), data.size());
    if (outLen < 0)
        return 0;
    AES_CBC_encrypt_buffer(&ctx, (uint8_t *) data.c_str(), outLen);
    LOG_ERROR("before base64 encode :%s, length :%d", data.c_str(), data.size());
    return b64_encode((uint8_t *) data.c_str(), outLen);
}

/**
 * 直接对 buffer 解密
 */
static char *decrypt_cbc(const uint8_t *key, const uint8_t *iv, uint8_t *buffer, int length) {
    buffer = b64_decode((char *) buffer, length);
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    int size = strlen((char *) buffer);
    LOG_ERROR("after base64 decode :%s, %d", buffer, size);
    AES_CBC_decrypt_buffer(&ctx, buffer, size);
    LOG_ERROR("after decrypt :%s, %d", buffer, strlen((char *) buffer));
    int outLen = removePKCS7Padding(buffer, strlen((char *) buffer));
    LOG_ERROR("after padding :%s, %d", buffer, strlen((char *) buffer));
    if (outLen < size)
        buffer[outLen] = 0;
    return (char *) buffer;
}

/**
 *  加密前需要填充 padding
 */
static int addPKCS7Padding(string &data, int length) {
    int padding = AES_BLOCKLEN - (length % AES_BLOCKLEN);
    int padVal = padding;
    for (int i = 0; i < padding; ++i) {
        data.push_back(padVal);
    }
    return length + padVal;
}

/**
 *  密文已填充了 padding，所以需要解密后移除 padding
 */
static int removePKCS7Padding(uint8_t *data, int length) {
    unsigned char padVal = data[length - 1];
    if (padVal < 1 || padVal > AES_BLOCKLEN
        || padVal > length      /* 填充数据量不足 */
        || (length % AES_BLOCKLEN) != 0     /* 不是 16 的倍数 */
        || (padVal != data[length - padVal])) {     /* 非连续填充数据 */
        return length;
    }
    for (int i = 0; i < padVal; i++) {
        if (data[length - i - 1] != padVal)
            return length;
    }
    return length - padVal;
}