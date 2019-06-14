## JNI

#### CMakeLists.txt 文件详解 
[点击这里](https://blog.csdn.net/u012528526/article/details/80647537)
[参考](https://www.cnblogs.com/chenxibobo/p/7678389.html)

#### 工具

    控制台打印STDOUT、STDERR：
        adb shell stop
        adb shell setprop log.redirect-stdio true
        adb shell start
        
    javah -- 生成头文件
    swig -- 自动生成JNI代码
    ndk-gdb -- 调试
    
    ndk-stack -- 堆栈跟踪分析
        adb logcat | ndk-stack -sym obj/local/x86 (logcat打印堆栈跟踪)
    
    CheckJNI(对JNI的扩展检查):
        adb shell setprop debug.checkjni 1
    内存分析：
        libc调试模式
            adb shell setprop libc.debug.malloc 1
            adb shell stop
            adb shell start
    strace(监控系统调用):
        adb  shell strace -v -p pid
        (模拟器带有strace功能)
        
#### 库概念

[参考](https://blog.csdn.net/koibiki/article/details/80952367)
    
    最后是所谓的动态库和静态库的概念。
    所谓动态库是程序运行时动态加载进进程内的，
    它的好处是如果程序中有很多独立的模块都需要使用同样的一个动态库，
    那么只需要在内存中加载一次就可以了。
    而静态库，是在模块编译的时候，在链接的过程中，
    将程序库中所需要的代码“拷贝”到模块内部实现的，
    它的坏处就是同样的代码会在各个模块中都存在，浪费内存和磁盘存储空间，
    甚至不同模块间的库函数代码会相互干扰，出现诡异的行为。
            