package com.god.seep.jni_test.media

import android.opengl.GLSurfaceView
import android.os.Bundle
import com.god.seep.jni_test.R
import kotlinx.android.synthetic.main.activity_open_glplayer.*
import java.util.concurrent.atomic.AtomicBoolean
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class OpenGLPlayerActivity : BasePlayerActivity() {

    private val isPlaying = AtomicBoolean()
    private var instance: Long = 0//原生渲染器

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_open_glplayer)
        gl_surface_view.setRenderer(renderer)
        gl_surface_view.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
    }

    override fun onStart() {
        super.onStart()
        instance = init(avi)
    }

    override fun onResume() {
        super.onResume()
        gl_surface_view.onResume()
    }

    override fun onPause() {
        super.onPause()
        gl_surface_view.onPause()
    }

    override fun onStop() {
        super.onStop()
        free(instance)
        instance = 0
    }

    val player = Runnable {
        //使用帧速计算延迟
        val frameDelay = (1000 / getFrameRate(avi)).toLong()
        //开始播放
        while (isPlaying.get()) {
            //请求渲染
            gl_surface_view.requestRender()
            try {
                Thread.sleep(frameDelay)
            } catch (e: InterruptedException) {
                break
            }
        }
    }
    val renderer = object : GLSurfaceView.Renderer {
        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {

        }

        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            //初始化 surface
            initSurface(instance, avi)
            //开始播放
            isPlaying.set(true)
            Thread(player).start()
        }

        override fun onDrawFrame(gl: GL10?) {
            //渲染下一帧
            if (!render(instance, avi)) {
                isPlaying.set(false)
            }
        }
    }

    companion object {
        /**
         * 初始化原生渲染器
         */
        @JvmStatic
        private external fun init(avi: Long): Long

        /**
         * 初始化 OpenGL surface
         */
        @JvmStatic
        private external fun initSurface(instance: Long, avi: Long)

        /**
         * 用给定文件进行渲染
         */
        @JvmStatic
        private external fun render(instance: Long, avi: Long): Boolean

        /**
         *释放原生渲染器
         */
        @JvmStatic
        private external fun free(instance: Long)
    }
}
