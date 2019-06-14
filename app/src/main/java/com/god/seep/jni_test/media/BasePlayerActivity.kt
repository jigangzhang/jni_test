package com.god.seep.jni_test.media

import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import com.god.seep.jni_test.R
import java.io.IOException

abstract class BasePlayerActivity : AppCompatActivity() {

    var avi: Long = 0
    protected var fileName: String = ""
        get() {
            field = intent?.extras?.getString(EXTRA_FILE_NAME) ?: ""
            return field
        }

    override fun onStart() {
        super.onStart()
        try {
            avi = open(fileName)
        } catch (ex: IOException) {
            AlertDialog.Builder(this)
                .setTitle(R.string.error_alert_title)
                .setMessage(ex.message)
                .show()
        }
    }

    override fun onStop() {
        super.onStop()
        if (0L != avi) {
            close(avi)
            avi = 0
        }
    }

    companion object {
        const val EXTRA_FILE_NAME = "media.extra_file_name"

        /**
         * 打开指定 AVI文件 返回一个文件描述符
         */
        @JvmStatic
        external fun open(fileName: String): Long

        /**
         *获得视频宽度
         */
        @JvmStatic
        external fun getWidth(avi: Long): Int

        /**
         *获得视频高度
         */
        @JvmStatic
        external fun getHeight(avi: Long): Int

        /**
         *获得帧速
         */
        @JvmStatic
        external fun getFrameRate(avi: Long): Double

        /**
         *关闭 AVI文件
         */
        @JvmStatic
        external fun close(avi: Long)

        init {
            System.loadLibrary("media-lib")
        }
    }
}