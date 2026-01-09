package org.xmagnusson.iot.smart.blinds.services;

public interface MqttPublisher {
    void sendCommand(String topic, String payload);
}
