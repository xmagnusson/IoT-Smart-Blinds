package org.xmagnusson.iot.smart.blinds.mqtt;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.integration.mqtt.support.MqttHeaders;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageHandler;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Component;
import org.xmagnusson.iot.smart.blinds.models.AvailabilityDTO;
import org.xmagnusson.iot.smart.blinds.models.BlindsStateDTO;

@Component
public class MqttSubscriber implements MessageHandler {

    @Value("${mqtt.topic.state}")
    private String stateTopic;

    @Value("${mqtt.topic.availability}")
    private String availabilityTopic;

    @Autowired
    private ObjectMapper objectMapper;

    @Autowired
    private SimpMessagingTemplate messagingTemplate;

    @Override
    public void handleMessage(Message<?> message) {
        try {
            String topic = message.getHeaders().get(MqttHeaders.RECEIVED_TOPIC, String.class);
            String payload = (String) message.getPayload();

            System.out.println("[MQTT] Topic: " + topic);
            System.out.println("[MQTT] Payload: " + payload);
            System.out.println("----------------------------------");

            if (availabilityTopic.equals(topic)) {
                AvailabilityDTO status = new AvailabilityDTO();
                status.setAvailability(payload);

                // send to frontend via websocket
                messagingTemplate.convertAndSend(
                    "/" + topic,
                    status
                );
            }

            if (stateTopic.equals(topic)) {
                BlindsStateDTO state = objectMapper.readValue(payload, BlindsStateDTO.class);

                // send to frontend via websocket
                messagingTemplate.convertAndSend(
                        "/" + topic,
                        state
                );
            }


        } catch (Exception e){
            e.printStackTrace();
        }
    }
}
