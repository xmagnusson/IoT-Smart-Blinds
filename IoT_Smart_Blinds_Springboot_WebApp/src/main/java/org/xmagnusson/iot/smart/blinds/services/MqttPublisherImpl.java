package org.xmagnusson.iot.smart.blinds.services;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.integration.mqtt.core.MqttPahoClientFactory;
import org.springframework.integration.mqtt.outbound.MqttPahoMessageHandler;
import org.springframework.integration.mqtt.support.DefaultPahoMessageConverter;
import org.springframework.integration.mqtt.support.MqttHeaders;
import org.springframework.messaging.support.MessageBuilder;
import org.springframework.stereotype.Service;

@Service
public class MqttPublisherImpl implements MqttPublisher{

    private final MqttPahoClientFactory mqttClientFactory;
    private final String clientId;

    public MqttPublisherImpl(MqttPahoClientFactory mqttClientFactory,
                             @Value("${mqtt.client-id}") String clientId) {
        this.mqttClientFactory = mqttClientFactory;
        this.clientId = clientId + "-publisher"; // separate clientId for publishing
    }

    @Override
    public void sendCommand(String topic, String payload) {
        try {
            MqttPahoMessageHandler handler = new MqttPahoMessageHandler(clientId, mqttClientFactory);
            handler.setAsync(true); // non-blocking publish
            handler.setConverter(new DefaultPahoMessageConverter());
            handler.handleMessage(MessageBuilder.withPayload(payload).setHeader(MqttHeaders.TOPIC, topic).build());
            System.out.println("[MQTT] Published: " + payload + " → " + topic);
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("[MQTT] Publish failed: " + e.getMessage());
        }
    }
}