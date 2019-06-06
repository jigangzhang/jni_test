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