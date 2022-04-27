package com.AMD.many_spheres;

// default supplied packages by android SDK
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

//my packages
import android.view.Window;						//for Window.FEATURE_NO_TITLE
import android.view.WindowManager;				//for WindowManager.layoutParams.FLAG_FULLSCREEN
import android.content.pm.ActivityInfo;			//for ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
import android.graphics.Color;
import android.view.View;

public class MainActivity extends AppCompatActivity 
{
    //private MyView myView;
       private GLESView glesView;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        // this is done to get rid of ActionBar
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);

		//Remove Title Bar
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        getSupportActionBar().hide();
        this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        
        // this is done to make Fullscreen
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);
        
        // force activity window orientation to Landscape
        MainActivity.this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

//        myView=new MyView(this);
	glesView = new GLESView(this);
        
        // set view as content view of the activity
        setContentView(glesView);
    }
    
    @Override
    protected void onPause()
    {
        super.onPause();
    }
    
    @Override
    protected void onResume()
    {
        super.onResume();
    }
}
