package org.xmagnusson.iot.smart.blinds.mqtt;

import org.springframework.integration.mqtt.support.MqttHeaders;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageHandler;

public class MqttSubscriber implements MessageHandler {

    @Override
    public void handleMessage(Message<?> message) {
        String topic = message.getHeaders().get(MqttHeaders.RECEIVED_TOPIC, String.class);
        String payload = (String) message.getPayload();

        System.out.println("[MQTT] Topic: " + topic);
        System.out.println("[MQTT] Payload: " + payload);
        System.out.println("----------------------------------");
    }
}
