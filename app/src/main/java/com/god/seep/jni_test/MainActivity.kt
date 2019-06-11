package com.god.seep.jni_test

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import androidx.annotation.RequiresApi
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    var name: String = "abc-kotlin"

    @RequiresApi(Build.VERSION_CODES.M)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val code = checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
        if (code == PackageManager.PERMISSION_DENIED)
            requestPermissions(arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE), 100)
        // Example of a call to a native method
        sample_text.text = stringFromJNI("test -- ")
        Log.e("tag", "uid -- ${Unix.getuid()}")
        settings.setOnClickListener { startActivity(Intent(this, ThreadActivity::class.java)) }
    }

    fun testA(): String {
        return "$name-java"
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(text: String): String

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
