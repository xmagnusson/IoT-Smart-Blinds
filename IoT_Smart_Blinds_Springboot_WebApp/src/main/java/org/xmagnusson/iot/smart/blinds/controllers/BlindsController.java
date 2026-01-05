package org.xmagnusson.iot.smart.blinds.controllers;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.xmagnusson.iot.smart.blinds.services.MqttPublisher;

@RestController
@RequestMapping("/api/device/blinds1")
public class BlindsController {

    @Autowired
    private MqttPublisher publisher;

    @Value("${mqtt.topic.cmd-mode}")
    private String cmdModeTopic;

    @Value("${mqtt.topic.cmd-position}")
    private String cmdPositionTopic;

    @PostMapping("/mode")
    public ResponseEntity<String> setMode(@RequestParam String mode) {
        publisher.sendCommand(cmdModeTopic, mode.toUpperCase());
        return ResponseEntity.ok("Mode command sent: " + mode);
    }

    @PostMapping("/position")
    public ResponseEntity<String> setPosition(@RequestParam int position) {
        publisher.sendCommand(cmdPositionTopic, String.valueOf(position));
        return ResponseEntity.ok("Position command sent: " + position);
    }
}
