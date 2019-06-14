package com.god.seep.jni_test.media

import android.graphics.Bitmap
import android.os.Bundle
import android.view.SurfaceHolder
import com.god.seep.jni_test.R
import kotlinx.android.synthetic.main.activity_bitmap_player.*
import java.util.concurrent.atomic.AtomicBoolean

class BitmapPlayerActivity : BasePlayerActivity() {

    var isPlaying = AtomicBoolean()
    lateinit var surfaceHolder: SurfaceHolder

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_bitmap_player)
        surfaceHolder = surface_view.holder
        surfaceHolder.addCallback(surfaceHolderCallBack)
    }

    private val surfaceHolderCallBack = object : SurfaceHolder.Callback {
        override fun surfaceCreated(holder: SurfaceHolder?) {
            isPlaying.set(true)
            Thread(renderer).start()
        }

        override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {

        }

        override fun surfaceDestroyed(holder: SurfaceHolder?) {
            isPlaying.set(false)
        }
    }
    val renderer = Runnable {
        //创建一个新的 bitmap 来保存所有帧
        val bitmap = Bitmap.createBitmap(getWidth(avi), getHeight(avi), Bitmap.Config.RGB_565)
        //使用帧速来计算延迟
        var frameDelay = (1000 / getFrameRate(avi)).toLong()
        while (isPlaying.get()) {
            //将帧渲染至 bitmap
            render(avi, bitmap)
            //锁定 canvas
            val canvas = surfaceHolder.lockCanvas()
            canvas.drawBitmap(bitmap, 0f, 0f, null)
            surfaceHolder.unlockCanvasAndPost(canvas)
            try {
                Thread.sleep(frameDelay)
            } catch (ex: InterruptedException) {
                break
            }
        }
    }

    companion object {
        @JvmStatic
        external fun render(avi: Long, bitmap: Bitmap): Boolean
    }
}
