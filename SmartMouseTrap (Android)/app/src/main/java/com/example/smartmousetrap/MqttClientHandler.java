package com.example.smartmousetrap;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Log;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class MqttClientHandler {

    private final MainActivity context;
    private ImageReceiver imageReceiver = ImageReceiver.getInstance();
    private ImageDecoder decoder = new ImageDecoder();

    private final MqttAndroidClient client;
    private String pubTopic = "app/request_status";
    private String subTopic1 = "esp32/status";
    private String subTopic2 = "esp32cam/image_data";

    private String currentStatus = "";
    private String requestStatus = "";

    public MqttClientHandler(MainActivity context, String uri, TextView textView,
                             ImageView imageView, Button pubButton) {

        this.context = context;

        // Generate ID & set Android Client
        String clientId = MqttClient.generateClientId();
        client = new MqttAndroidClient(context.getApplicationContext(), uri, clientId);

        // Make a Connection
        connect();

        // Set a Callback for Subscription
        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
                Log.d("Debug-callback", "Connection Lost!!");
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                if (topic.equals("esp32/status")) {                     // status received
                    if (new String(message.getPayload()).equals("0"))
                        textView.setText("Door Closed");
                    else if (new String(message.getPayload()).equals("1"))
                        textView.setText("Door Opened");
                    currentStatus = new String(message.getPayload());
                    Log.d("Debug-currentStatus", currentStatus);
                }
                else if (topic.equals("esp32cam/image_data")) {         // image received
                    // Check the data received
                    if (new String(message.getPayload()).equals(" ")) {                                          // end of image --> starting to decode

                        decoder.setEncodedString(imageReceiver.getStringImage());
                        Log.d("Debug-stringImage", decoder.encodedString);
                        Bitmap bitmap = decoder.decodeString();
                        imageView.setImageBitmap(bitmap);

                    } else if (new String(message.getPayload()).equals("data:image/jpeg;base64,")) {             // image starting point
                        imageReceiver.clearString();
                        Log.d("Debug-stringImage", "cleared string");
                    }
                    else
                        imageReceiver.appendString(new String(message.getPayload()));   // image data
                }
                Log.d("Debug-callback", "Data Received");

                // UI Label
                if (textView.getText().toString().equals("Door Closed")) {
                    pubButton.setText("Request Open");
                    textView.setTextColor(Color.RED);
                }

                else if (textView.getText().toString().equals("Door Opened")) {
                    pubButton.setText("Request Close");
                    textView.setTextColor(Color.GREEN);
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                Log.d("Debug-callback", "Publish Complete!!");
            }
        });

    }

    /* ====================================================================================== */

    private void setSubscription() {
        try {
            client.subscribe(subTopic1, 0);
            client.subscribe(subTopic2, 0);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    // Called when button gets clicked
    public void pub() {
        try {
            if (currentStatus.equals("0"))
                requestStatus = "1";
            else if (currentStatus.equals("1"))
                requestStatus = "0";
            client.publish(pubTopic, requestStatus.getBytes(), 0, false);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    /* ====================================================================================== */

    public void connect() {
        // Make a Connection
        try {
            IMqttToken token = client.connect();

            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d("Debug-connect", "Connection Success");
                    Toast.makeText(context, "Connected", Toast.LENGTH_LONG).show();
                    setSubscription();
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.d("Debug-connect", "Connection Failure");
                    Toast.makeText(context, "Failed to Connect", Toast.LENGTH_LONG).show();
                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void disconnect() {
        // Disconnect CLient
        try {
            IMqttToken token = client.disconnect();

            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d("Debug-disconnect", "Disconnected");
                    Toast.makeText(context, "Disconnected", Toast.LENGTH_LONG).show();
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.d("Debug-disconnect", "Failed on Disconnection..");
                    Toast.makeText(context, "Failed to Disconnect..", Toast.LENGTH_LONG).show();
                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

}
