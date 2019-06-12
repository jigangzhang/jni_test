package com.god.seep.jni_test.socket

import com.god.seep.jni_test.R
import kotlinx.android.synthetic.main.activity_echo_server.*

class EchoServerActivity : BaseEchoActivity() {
    override fun getLayoutId(): Int {
        return R.layout.activity_echo_server
    }

    override fun initView() {
        logView = log_view
        logScroll = log_scroll
        startButton = start_button
        portEdit = port_edit
    }

    override fun onStartButtonClicked() {
        val port = getPort()
        ServerTask(port).start()
    }

    //启动TCP服务器
    private external fun nativeStartTcpServer(port: Int)

    //启动UDP服务
    private external fun nativeStartUdpServer(port: Int)

    private inner class ServerTask(val port: Int) : AbsEchoTask() {
        override fun onBackground() {
            logMessage("Starting server.")
//            nativeStartTcpServer(port)
            nativeStartUdpServer(port)
            logMessage("Server terminated.")
        }
    }
}
