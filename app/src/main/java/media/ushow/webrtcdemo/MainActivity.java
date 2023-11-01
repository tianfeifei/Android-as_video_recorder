package media.ushow.webrtcdemo;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private Button forward_video_recorder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        requestPermissions();
        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        forward_video_recorder = (Button) findViewById(R.id.forward_video_recorder);
        forward_video_recorder.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, VideoRecorderActivity.class);
                startActivity(intent);
            }
        });
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();


    private String [] permissions = {
            Manifest.permission.CAMERA,
            Manifest.permission.INTERNET,
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.WAKE_LOCK,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
    };

    /**
     * 请求权限
     *
     * @param callback 回调
     */
    protected void requestPermissions() {
        ActivityCompat.requestPermissions(this, permissions, 100);
//
//        {int requestCode, @NonNull String[] permissions,
//            @NonNull int[] grantResults ->
//            if (code == permissionsRequestCode) {
//                if (results.all { it == PackageManager.PERMISSION_GRANTED }) {
//                    Log.i(TAG, "已获得权限")
//                    callback(true)
//                } else {
//                    Log.e(TAG, "未完全授权")
//                    callback(false)
//                }
//            }
//        }
    }
}
