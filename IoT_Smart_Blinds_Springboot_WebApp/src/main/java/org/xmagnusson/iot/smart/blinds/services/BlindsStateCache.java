package org.xmagnusson.iot.smart.blinds.services;

import org.xmagnusson.iot.smart.blinds.models.AvailabilityDTO;
import org.xmagnusson.iot.smart.blinds.models.BlindsStateDTO;

public interface BlindsStateCache {
    void updateAvailability(String deviceId, AvailabilityDTO availability);
    void updateState(String deviceId, BlindsStateDTO state);
    AvailabilityDTO getAvailability(String deviceId);
    BlindsStateDTO getState(String deviceId);
}
