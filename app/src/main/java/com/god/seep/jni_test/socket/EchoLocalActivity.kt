package com.god.seep.jni_test.socket

import android.net.LocalSocket
import android.net.LocalSocketAddress
import com.god.seep.jni_test.R
import kotlinx.android.synthetic.main.activity_echo_local.*
import java.io.File

class EchoLocalActivity : BaseEchoActivity() {
    override fun getLayoutId(): Int {
        return R.layout.activity_echo_local
    }

    override fun initView() {
        logView = log_view
        logScroll = log_scroll
        startButton = start_button
        portEdit = port_edit
    }

    override fun onStartButtonClicked() {
        val msg = message_edit.text.toString()
        val port = portEdit.text.toString()
        if (port.isNotEmpty() && msg.isNotEmpty()) {
            val socketName = if (isFileSystemSocket(port)) {
                File(filesDir, port).absolutePath
            } else
                port
            ServerTask(port).start()
            ClientTask(port, msg).start()
        }
    }

    /**
     *  文件系统权限限制不可用（read-only）
     */
    private fun isFileSystemSocket(name: String): Boolean {
        return name.startsWith("/")
    }

    private external fun nativeStartLocalServer(name: String)

    private fun startLocalClient(name: String, msg: String) {
        val clientSocket = LocalSocket()
        var nameSpace =
            if (isFileSystemSocket(name))
                LocalSocketAddress.Namespace.FILESYSTEM
            else
                LocalSocketAddress.Namespace.ABSTRACT
        val address = LocalSocketAddress(name, nameSpace)
        logMessage("Connecting to $name")
        clientSocket.connect(address)
        logMessage("Connected.")
        val byteArray = msg.toByteArray()
        logMessage("Sending to the socket...")
        val outputStream = clientSocket.outputStream
        outputStream.write(byteArray)
        logMessage("Sent ${byteArray.size} bytes: $msg")
        logMessage("Receiving from the socket...")
        val inputStream = clientSocket.inputStream
        val readSize = inputStream.read(byteArray)
        logMessage("Received $readSize bytes: $byteArray")
        logMessage(
            "Received $readSize bytes: ${String(byteArray, 0, readSize)}"
        )
        outputStream.close()
        inputStream.close()
        clientSocket.close()
    }

    private inner class ServerTask(val portName: String) : AbsEchoTask() {
        override fun onBackground() {
            logMessage("Staring server.")
            nativeStartLocalServer(portName)
            logMessage("Server terminated.")
        }
    }

    private inner class ClientTask(val portName: String, val msg: String) : AbsEchoTask() {
        override fun onBackground() {
            logMessage("Staring client.")
            startLocalClient(portName, msg)
            logMessage("Client terminated.")
        }
    }
}
