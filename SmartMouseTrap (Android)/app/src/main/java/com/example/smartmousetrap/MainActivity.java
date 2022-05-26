package com.example.smartmousetrap;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    BrokerConnector brokerConnector;
    MqttClientHandler handler;

    TextView subText;
    Button pubButton;
    ImageView imageViewCamera;

    private Intent settingsIntent;

    /* Menu Option */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.menuSettings) {
            handler.disconnect();
            startActivity(settingsIntent);
            overridePendingTransition(R.anim.slide_in_right, R.anim.slide_out_left);
        }
        return super.onOptionsItemSelected(item);
    }

    /* =================================================================================== */

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initSetup();
    }

    private void initSetup() {
        // Setup the subscribe text
        subText = findViewById(R.id.subText);
        imageViewCamera = findViewById(R.id.imageViewCamera);
        // Button
        pubButton = findViewById(R.id.pubButton);

        brokerConnector = BrokerConnector.getInstance();

        settingsIntent = new Intent(MainActivity.this, SettingActivity.class);

        //handler = new MqttClientHandler(this, "tcp://192.168.162.205:1883", subText, imageViewCamera);
        //handler = new MqttClientHandler(this, "tcp://projecttech.thddns.net:5050", subText, imageViewCamera, pubButton);
        //handler = new MqttClientHandler(this, "tcp://10.0.2.2:1883", subText, imageViewCamera, pubButton);
        handler = new MqttClientHandler(this, "tcp://" + brokerConnector.brokerIp + ":" + brokerConnector.brokerPort, subText, imageViewCamera, pubButton);

        // onClick Listener
        pubButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                handler.pub();
            }
        });
    }

}