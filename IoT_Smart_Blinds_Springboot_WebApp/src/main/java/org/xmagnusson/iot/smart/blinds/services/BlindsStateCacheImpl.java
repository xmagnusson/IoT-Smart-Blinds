package org.xmagnusson.iot.smart.blinds.services;

import org.springframework.stereotype.Service;
import org.xmagnusson.iot.smart.blinds.models.AvailabilityDTO;
import org.xmagnusson.iot.smart.blinds.models.BlindsStateDTO;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

@Service
public class BlindsStateCacheImpl implements BlindsStateCache{
    private final Map<String, BlindsStateDTO> stateByDevice = new ConcurrentHashMap<>();
    private final Map<String, AvailabilityDTO> availabilityByDevice = new ConcurrentHashMap<>();


    @Override
    public void updateAvailability(String deviceId, AvailabilityDTO availability) {
        availabilityByDevice.put(deviceId, availability);
    }

    @Override
    public void updateState(String deviceId, BlindsStateDTO state) {
        stateByDevice.put(deviceId, state);
    }

    @Override
    public AvailabilityDTO getAvailability(String deviceId) {
        return availabilityByDevice.get(deviceId);
    }

    @Override
    public BlindsStateDTO getState(String deviceId) {
        return stateByDevice.get(deviceId);
    }
}
