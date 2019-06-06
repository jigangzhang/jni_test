#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include "wrapper_log.h"

const char *TAG = "native-tag";

void readFile();

void doShell();

void findSystemProperty(char *name);

extern "C" JNIEXPORT jstring JNICALL
Java_com_god_seep_jni_1test_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject obj,
        jstring jString) {
    __android_log_write(ANDROID_LOG_INFO, TAG, "native -- test");
    std::string hello = "Hello from C++";
    jboolean isCopy;
    const char *str = env->GetStringUTFChars(jString, &isCopy);//获取 C字符串
    isCopy = 0;//上面赋值不起作用？
    __android_log_print(ANDROID_LOG_INFO, TAG, "jString -- str = %s", str);
    if (isCopy) {
        hello = hello;
    } else {
        hello = str + hello;
    }
    env->ReleaseStringUTFChars(jString, str);//释放 C字符串地址

    jintArray jArray = env->NewIntArray(10);//创建java 数组
    jint nativeArray[10];
    env->GetIntArrayRegion(jArray, 0, 10, nativeArray);//将java 数组 复制到 c 数组 对副本进行操作
    nativeArray[0] = 0;
    nativeArray[1] = 1;
    LOG_INFO("jint[] [%i, %i]", nativeArray[0], nativeArray[1]);
    env->SetIntArrayRegion(jArray, 0, 2, nativeArray);//将对副本（c数组）的操作复制回java 数组
    jint *jint1 = env->GetIntArrayElements(jArray, nullptr);//对java 数组 直接指针操作
    LOG_INFO("jArray [%i, %i]", jint1[0], jint1[1]);
    jint1[3] = 3;
    env->ReleaseIntArrayElements(jArray, jint1, 0);//释放指针地址，0为释放模式，有3个选项:JNI_COMMIT,JNI_ABORT,0
    printf("printf -- 3");

    jclass clazz = env->GetObjectClass(obj);//获取给定java实例的java class 对象
    jfieldID fieldId = env->GetFieldID(clazz, "name", "Ljava/lang/String;");//获取域Id，java对象实例中的变量，域描述符
    jstring name = (jstring) env->GetObjectField(obj, fieldId);//根据域Id 获取域，即native 获取java
    const char *strName = env->GetStringUTFChars(name, 0);
    hello = hello + strName;
    env->ReleaseStringUTFChars(name, strName);

    jmethodID methodId = env->GetMethodID(clazz, "testA", "()Ljava/lang/String;");//获取方法Id，后面参数为方法描述符（方法签名）
    jstring methodStr = (jstring) env->CallObjectMethod(obj, methodId);//执行java方法,java和原生代码之间转换代价较大，如非必要不建议这样使用
    jthrowable ex = env->ExceptionOccurred();//若java方法有异常抛出，需用此方法显示地做异常处理（查询VM中是否有异常挂起）
    if (0 != ex) {
        env->ExceptionClear();//显示地清除异常
        /*其他操作*/
        //抛出异常
        jclass nullEx = env->FindClass("java/lang/NullPointerException");//查找异常类，返回的是局部引用，原生方法返回，局部引用自动释放
        if (0 != nullEx)
            env->ThrowNew(nullEx, "null exception");//初始化并抛出异常，手动释放原生资源 返回等？
        env->DeleteLocalRef(nullEx);//显示释放局部引用（局部引用过多时应手动释放（允许最多16个局部引用？）），局部引用可初始化为全局/弱全局引用
    }
    const char *strMethod = env->GetStringUTFChars(methodStr, nullptr);
    hello = hello + strMethod;
    env->ReleaseStringUTFChars(methodStr, strMethod);

    readFile();
    doShell();
    return env->NewStringUTF(hello.c_str());
}

void createBuffer(JNIEnv *env) {
    unsigned char *buffer = (unsigned char *) malloc(1024);//分配内存（标准C库函数）
    jobject directBuffer = env->NewDirectByteBuffer(buffer, 1024);//创建直接字节缓冲区（java可使用的）
//    unsigned char *bufferAddress = (unsigned char *) env->GetDirectBufferAddress(directBuffer);//获取java创建的字节缓冲区的原生字节数组内存地址
    free(buffer);//释放内存
    buffer = nullptr;
    auto *dInt = new int[16];//动态内存分配（C++支持）（调用构造函数）
    *dInt = 1;
    *dInt = 2;
    delete[] dInt;//释放内存（释放前先调用析构函数），释放数组原生使用[]
    dInt = nullptr;
}
/**内存管理：
 *      静态分配：静态和全局变量，在应用程序启动时自动发生；
 *      自动分配：函数参数和局部变量，在包含声明的复合语句被输入时发生，退出复合语句时所分配的内存自动释放；
 *      动态分配：前两者假设所需内存大小和范围是固定的，编译时被定义，动态分配则在事先
 *               不知情的情况下起作用，内存大小和范围的分配取决于运行时因素。
 * */
//全局引用可被多线程共享，局部引用不能， JNIEnv接口指针 在方法调用的线程中有效，不能被其他线程缓存或使用

//写操作--fwrite 返回值同fread, fputs 、fputc() 返回EOF 表示字符不能写入， fprintf() 正常返回写入的字符个数，出错返回负数
//刷新缓冲区-- fflush() 刷新输出缓冲区， 正常返回0， 不能写入返回 EOF
//读操作--fread, fgets() 读取以换行符结尾的字符序列 成功返回缓冲区指针 否则NULL，fgetc() 返回整数位无符号字符 EOF结尾，
//          fscanf() 读取格式数据 返回读取的项目个数 错误返回EOF
//检查文件结尾-- feof() 如果设置了文件结束指示符则使用，已到达结尾返回非0值， 还有数据返回0
void readFile() {
    FILE *file = fopen("/sdcard/shared/viewcache", "r");
    if (NULL == file)
        LOG_ERROR("文件打开失败， null");
    else {
        char *buffer = (char *) malloc(sizeof(char) * 10);
        size_t count = fread(buffer, sizeof(buffer), 10, file);//返回读取流的元素个数，个数与预期不一致表示读取完毕？
        LOG_INFO("read count %i", count);
        LOG_INFO("read data: %s", buffer);
        free(buffer);
        fclose(file);
    }
}

void doShell() {
    int result = system("mkdir /sdcard/shared/temp");//执行shell命令，与进程交互
    if (-1 == result || 127 == result)
        LOG_ERROR("shell命令执行失败");
    else
        LOG_INFO("shell result -- %i", result);
    //与子进程通信，打开一个双向通道，阻塞式--命令执行结束前一直等待
    FILE *stream = popen("ls /", "r");//在父、子进程间打开一个双向通道，向ls命令打开只读通道
    if (NULL == stream)
        LOG_ERROR("ls 命令执行失败");
    else {
        char buffer[1024];
        while (NULL != fgets(buffer, 1024, stream)) {
            LOG_INFO("read: %s", buffer);
        }
        int status = pclose(stream);//关闭通道
        LOG_INFO("pclose status: %i", status);
    }

    char value[PROP_VALUE_MAX];
    if (__system_property_get("ro.product.model", value) == 0)//通过系统属性名称获取值
        LOG_ERROR("未找到系统属性");
    else
        LOG_INFO("product model: %s", value);
    findSystemProperty("ro.product.model");
}

void findSystemProperty(char *name) {
    char *user = getlogin();
    LOG_INFO("login user: %s, uid: %u, gid: %u", user, getuid(), getgid());
    const prop_info *info = __system_property_find(name);
    if (NULL == info)
        LOG_ERROR("%s 属性未找到", name);
    else {
        char name[PROP_NAME_MAX];
        char value[PROP_VALUE_MAX];
        int result = __system_property_read(info, name, value);
        if (result == 0)
            LOG_ERROR("prop_info 未找到具体信息");
        else
            LOG_INFO("prop info, name: %s, value: %s", name, value);
    }
}