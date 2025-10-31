package com.example.linkerextractor;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ScrollView;
import android.widget.LinearLayout;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class MainActivity extends AppCompatActivity {
    private static final int STORAGE_PERMISSION_CODE = 1;
    private TextView tvData;

    static {
        System.loadLibrary("linker_extractor");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        ScrollView scrollView = new ScrollView(this);
        LinearLayout mainLayout = new LinearLayout(this);
        mainLayout.setOrientation(LinearLayout.VERTICAL);
        mainLayout.setPadding(50, 50, 50, 50);

        tvData = new TextView(this);
        tvData.setText("Loading...");
        tvData.setTextIsSelectable(true);
        tvData.setTextSize(10);
        tvData.setTypeface(android.graphics.Typeface.MONOSPACE);
        
        Button btnRefresh = new Button(this);
        btnRefresh.setText("Refresh");
        btnRefresh.setOnClickListener(v -> refreshData());
        
        Button btnExport = new Button(this);
        btnExport.setText("Export");
        btnExport.setOnClickListener(v -> exportData());

        mainLayout.addView(btnRefresh);
        mainLayout.addView(btnExport);
        mainLayout.addView(tvData);
        scrollView.addView(mainLayout);
        setContentView(scrollView);

        refreshData();
        
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) 
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, 
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 
                    STORAGE_PERMISSION_CODE);
        }
    }

    private void refreshData() {
        new Thread(() -> {
            final String data = collectAllLinkerData();
            runOnUiThread(() -> tvData.setText(data));
        }).start();
    }

    private void exportData() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) 
                != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        new Thread(() -> {
            String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(new Date());
            String filename = "linker_data_" + timestamp + ".txt";
            File file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), filename);
            
            final String result = exportDataToFile(file.getAbsolutePath());
            runOnUiThread(() -> tvData.setText(result + "\n\n" + collectAllLinkerData()));
        }).start();
    }

    public native String collectAllLinkerData();
    public native String exportDataToFile(String path);
}