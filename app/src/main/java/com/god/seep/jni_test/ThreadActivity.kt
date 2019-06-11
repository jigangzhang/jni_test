package com.god.seep.jni_test

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_thread.*

class ThreadActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_thread)
        //初始化原生代码
        nativeInit()
        start_button.setOnClickListener {
            val threads = threads_edit.text.toString().toInt()
            val iterations = iterations_edit.text.toString().toInt()
            if (threads > 0 && iterations > 0)
                startThreads(threads, iterations)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        nativeFree()
    }

    /**
     *原生消息回调
     */
    private fun onNativeMessage(message: String) {
        runOnUiThread {
            log_view.append(message)
            log_view.append("\n")
        }
    }

    private fun startThreads(threads: Int, iterations: Int) {
//        javaThreads(threads, iterations)
        posixThreads(threads, iterations)
    }

    /**
     *初始化原生代码
     */
    private external fun nativeInit()

    /**
     *释放原生资源
     */
    private external fun nativeFree()

    /**
     *原生 Worker
     */
    private external fun nativeWorker(id: Int, iterations: Int)

    private external fun posixThreads(threads: Int, iterations: Int)

    private fun javaThreads(threads: Int, iterations: Int) {
        for (i in 1..threads) {
            Thread { nativeWorker(i, iterations) }
                .start()
        }
    }
}
