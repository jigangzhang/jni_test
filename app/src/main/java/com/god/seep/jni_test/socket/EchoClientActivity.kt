package com.god.seep.jni_test.socket

import com.god.seep.jni_test.R
import kotlinx.android.synthetic.main.activity_echo_client.*

class EchoClientActivity : BaseEchoActivity() {
    override fun getLayoutId(): Int {
        return R.layout.activity_echo_client
    }

    override fun initView() {
        logView = log_view
        logScroll = log_scroll
        startButton = start_button
        portEdit = port_edit
    }

    override fun onStartButtonClicked() {
        val ip = ip_edit.text.toString()
        val port = getPort()
        val msg = message_edit.text.toString()
        if (ip.isNotEmpty() && msg.isNotEmpty()) {
            ClientTask(ip, port, msg).start()
        }
    }

    private external fun nativeStartTcpClient(ip: String, port: Int, msg: String)

    private inner class ClientTask(val ip: String, val port: Int, val msg: String) : AbsEchoTask() {
        override fun onBackground() {
            logMessage("Starting client.")
            nativeStartTcpClient(ip, port, msg)
            logMessage("Client terminated.")
        }
    }
}
