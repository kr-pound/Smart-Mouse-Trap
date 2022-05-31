package com.example.smartmousetrap;

public class ImageReceiver {

    private static ImageReceiver uniqueInstance;
    private String receivedString = "";

    private ImageReceiver() {
    }

    public static synchronized ImageReceiver getInstance() {
        if (uniqueInstance == null) {
            uniqueInstance = new ImageReceiver();
        }
        return uniqueInstance;
    }

    public String getStringImage() {
        return receivedString;
    }

    public void appendString(String string) {
        receivedString += string;
    }

    public void clearString() {
        receivedString = "";
    }

}
