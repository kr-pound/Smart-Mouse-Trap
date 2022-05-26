package com.example.smartmousetrap;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class SettingActivity extends AppCompatActivity {

    EditText editTxtIP, editTxtPort;
    Button buttonSave;

    private Intent mainIntent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting);

        initSetup();
        setupEditText();
    }

    private void initSetup() {
        editTxtIP = findViewById(R.id.editTxtIP);
        editTxtPort = findViewById(R.id.editTxtPort);
        buttonSave = findViewById(R.id.buttonSave);

        mainIntent = new Intent(SettingActivity.this, MainActivity.class);

        buttonSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                BrokerConnector.getInstance().brokerIp = editTxtIP.getText().toString();
                BrokerConnector.getInstance().brokerPort = editTxtPort.getText().toString();
                startActivity(mainIntent);
                overridePendingTransition(R.anim.slide_in_left, R.anim.slide_out_right);
            }
        });
    }

    public void setupEditText() {
        editTxtIP.setText(BrokerConnector.getInstance().brokerIp);
        editTxtPort.setText(BrokerConnector.getInstance().brokerPort);
    }

}