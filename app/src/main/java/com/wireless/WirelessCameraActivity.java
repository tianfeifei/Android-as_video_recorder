//package com.wireless;
//
//import android.content.Intent;
//import android.net.Uri;
//import android.os.Bundle;
//import android.os.Handler;
//import android.os.Looper;
//import android.os.Message;
//import android.provider.Settings;
//import android.support.annotation.NonNull;
//import android.support.v7.app.AppCompatActivity;
//import android.util.Log;
//import android.view.View;
//import android.widget.Toast;
//
//import com.deepglint.dgliberosdk.DGLiberoManager;
//import com.deepglint.dgliberosdk.callback.DLCallback;
//import com.deepglint.dgliberosdk.struct.DLCommon;
//import com.deepglint.dgliberosdk.struct.DLDeviceConn;
//import com.deepglint.dgliberosdk.struct.DLInterfaceType;
//
//import media.ushow.webrtcdemo.R;
//
//
//public class WirelessCameraActivity extends AppCompatActivity {
//    private static final String TAG = "WirelessCameraActivity";
//    private Handler mHandler;
//    private DLCallback dwCallback;
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_wireless_camera);
//        dwCallback = new DLCallback();
//        dwCallback.setDLEventCallback(new DLCallback.OnEventCB() {
//            @Override
//            public void onData(DLCommon.DLEvent event) {
//                Message msg = Message.obtain();
//                msg.what = event.getDlEventType().getValue();
//                msg.obj = event.getContent();
//                mHandler.sendMessage(msg);
//            }
//        });
//
//        dwCallback.setDLFrameCallback(new DLCallback.OnFrameCB() {
//            @Override
//            public void onData(DLCommon.DLFrame frame, DLCommon.DLCustom custom) {
////                Log.e(TAG, ((byte[])frame.getData()).length+"    "+frame.getMedia_type().getValue());
//                switch (frame.getMedia_type().getValue()) {
//                    case 0:
//                        Log.e(TAG, ((byte[]) frame.getData()).length + "    " + frame.getPts());
//
//                        break;
//                    case 1:
//                        break;
//                    default:
//                        break;
//                }
//            }
//        });
//        DGLiberoManager.getInstance().create(dwCallback);
//
//        mHandler = new Handler(Looper.getMainLooper()) {
//            @Override
//            public void handleMessage(@NonNull Message msg) {
//                super.handleMessage(msg);
//                switch (msg.what) {
//                    case 9001:
//                        Toast.makeText(WirelessCameraActivity.this, "连接成功", Toast.LENGTH_SHORT).show();
//                        break;
//                    case 9002:
//                    case 9003:
//                        Toast.makeText(WirelessCameraActivity.this, "连接断开", Toast.LENGTH_SHORT).show();
////                        DGLiberoManager.getInstance().disconnect();
//                        break;
//                    case 9004:
//                    case 9005:
//
////                        DGLiberoManager.getInstance().mediaDisconnect();
//                        break;
//                    default:
//                        Log.e(TAG, msg.what + "  " + msg.obj);
//                        break;
//                }
//            }
//        };
//
//    }
//
//
//    public void onClick(View view) {
//        DLDeviceConn param = new DLDeviceConn();
//        param.setIp("192.137.10.1");
//        DGLiberoManager.getInstance().connect(DLInterfaceType.IF_NET, param);
//    }
//}