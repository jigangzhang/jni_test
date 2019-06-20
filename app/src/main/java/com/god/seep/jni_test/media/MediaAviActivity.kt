package com.god.seep.jni_test.media

import android.annotation.SuppressLint
import android.content.Intent
import android.os.AsyncTask
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import com.god.seep.jni_test.R
import kotlinx.android.synthetic.main.activity_media_avi.*
import java.io.File
import java.io.IOException
import java.lang.Exception

class MediaAviActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_media_avi)
        play_button.setOnClickListener { onPlayButtonClick() }
        wav_button.setOnClickListener {
            val file = File(Environment.getExternalStorageDirectory(), wav_name_edit.text.toString())
            PlayTask().execute(file.absolutePath)
        }
    }

    private fun onPlayButtonClick() {
        var intent: Intent = Intent()
        val radioId = player_radio_group.checkedRadioButtonId
        when (radioId) {
            R.id.bitmap_player_radio -> {
                intent.setClass(this, BitmapPlayerActivity::class.java)
            }
            R.id.open_gl_player_radio -> {
                intent.setClass(this, OpenGLPlayerActivity::class.java)
            }
            R.id.native_window_player_radio -> {
                intent.setClass(this, NativeWindowActivity::class.java)
            }
        }
        val file = File(Environment.getExternalStorageDirectory(), file_name_edit.text.toString())
        intent.putExtra(BasePlayerActivity.EXTRA_FILE_NAME, file.absolutePath)
        startActivity(intent)
    }

    companion object {
        @JvmStatic
        external fun play(fileName: String)

        init {
            System.loadLibrary("media-lib")
        }
    }
}

private class PlayTask : AsyncTask<String, Void, Exception>() {
    override fun doInBackground(vararg params: String): Exception? {
        var result: Exception? = null
        try {
            MediaAviActivity.play(params[0])
        } catch (ex: IOException) {
            result = ex
        }
        return result
    }
}