package org.xmagnusson.iot.smart.blinds.controllers;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import org.xmagnusson.iot.smart.blinds.services.MqttPublisher;

@RestController
@RequestMapping("/api")
public class BlindsController {

    @Autowired
    private MqttPublisher publisher;

    @Value("${mqtt.topic.cmd-mode}")
    private String cmdModeTopic;

    @Value("${mqtt.topic.cmd-position}")
    private String cmdPositionTopic;

    @PostMapping("blinds/{deviceId}/mode")
    public ResponseEntity<String> setMode(@PathVariable String deviceId, @RequestParam String mode) {
        String topicWithDeviceId = cmdModeTopic.replace("DEVICE_ID", deviceId);
        System.out.println("Edited topic: " + topicWithDeviceId);
        publisher.sendCommand(topicWithDeviceId, mode.toUpperCase());
        return ResponseEntity.ok("Mode command sent: " + mode);
    }

    @PostMapping("blinds/{deviceId}/position")
    public ResponseEntity<String> setPosition(@PathVariable String deviceId, @RequestParam int position) {
        String topicWithDeviceId = cmdPositionTopic.replace("DEVICE_ID", deviceId);
        System.out.println("Edited topic: " + topicWithDeviceId);

        publisher.sendCommand(topicWithDeviceId, String.valueOf(position));
        return ResponseEntity.ok("Position command sent: " + position);
    }
}
