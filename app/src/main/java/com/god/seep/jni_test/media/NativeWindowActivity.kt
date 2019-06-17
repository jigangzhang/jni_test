package com.god.seep.jni_test.media

import android.os.Bundle
import android.view.Surface
import android.view.SurfaceHolder
import com.god.seep.jni_test.R
import kotlinx.android.synthetic.main.activity_native_window.*
import java.util.concurrent.atomic.AtomicBoolean

class NativeWindowActivity : BasePlayerActivity() {

    private val isPlaying = AtomicBoolean()
    private lateinit var surfaceHolder: SurfaceHolder

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_native_window)
        surfaceHolder = surface_view.holder
        surfaceHolder.addCallback(callBack)
    }

    private val callBack = object : SurfaceHolder.Callback {
        override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {

        }

        override fun surfaceDestroyed(holder: SurfaceHolder?) {
            isPlaying.set(false)
        }

        override fun surfaceCreated(holder: SurfaceHolder?) {
            isPlaying.set(true)
            Thread(renderer).start()
        }
    }
    private val renderer = Runnable {
        val surface = surfaceHolder.surface
        init(avi, surface)
        var frameDelay = (1000 / getFrameRate(avi)).toLong()
        while (isPlaying.get()) {
            //将帧渲染至 surface
            render(avi, surface)
            try {
                Thread.sleep(frameDelay)
            } catch (ex: InterruptedException) {
                break
            }
        }
    }

    companion object {
        @JvmStatic
        private external fun init(avi: Long, surface: Surface)

        /**
         * 将AVI文件帧渲染到 surface 上
         */
        @JvmStatic
        private external fun render(avi: Long, surface: Surface): Boolean
    }
}