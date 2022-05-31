package com.example.smartmousetrap;

public class BrokerConnector {

    String brokerIp = "projecttech.thddns.net";
    String brokerPort = "5050";
    private static BrokerConnector uniqueInstance;

    private BrokerConnector() {
    }

    public static synchronized BrokerConnector getInstance() {
        if (uniqueInstance == null) {
            uniqueInstance = new BrokerConnector();
        }
        return uniqueInstance;
    }

}
