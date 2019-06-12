package com.god.seep.jni_test.socket

import android.os.Bundle
import android.os.Handler
import android.text.TextUtils
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.annotation.LayoutRes
import androidx.appcompat.app.AppCompatActivity
import androidx.core.widget.NestedScrollView

abstract class BaseEchoActivity : AppCompatActivity() {
    lateinit var logView: TextView
    lateinit var logScroll: NestedScrollView
    lateinit var startButton: Button
    lateinit var portEdit: EditText

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(getLayoutId())
        initView()
        startButton.setOnClickListener {
            onStartButtonClicked()
        }
    }

    @LayoutRes
    abstract fun getLayoutId(): Int

    abstract fun initView()

    abstract fun onStartButtonClicked()

    fun getPort(): Int {
        return if (TextUtils.isEmpty(portEdit.text))
            0
        else
            portEdit.text.toString().toInt()
    }

    fun logMessage(message: String) {
        runOnUiThread { logMessageDirect(message) }
    }

    /**
     *记录消息
     */
    fun logMessageDirect(message: String) {
        logView.append(message)
        logView.append("\n")
        logScroll.fullScroll(View.FOCUS_DOWN)
    }

    companion object {
        init {
            System.loadLibrary("echo-lib")
        }
    }

    abstract inner class AbsEchoTask : Thread() {

        private var handler: Handler = Handler()//UI线程中执行的，是 mainHandler

        private fun onPreExecute() {
            startButton.isEnabled = false
            logView.text = ""
        }

        override fun start() {
            onPreExecute()
            super.start()
        }

        override fun run() {
            super.run()
            onBackground()
            handler.post { onPostExecute() }
        }

        abstract fun onBackground()

        private fun onPostExecute() {
            startButton.isEnabled = true
        }
    }
}