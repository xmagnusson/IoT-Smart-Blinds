package org.xmagnusson.iot.smart.blinds.models;

import com.fasterxml.jackson.annotation.JsonProperty;

public class BlindsStateDTO {
    private String mode;
    private String state;
    @JsonProperty("currentPos")
    private int currentPosition;
    @JsonProperty("targetPos")
    private int targetPosition;
    @JsonProperty("attached")
    private boolean isAttached;
    @JsonProperty("light")
    private int lightLevel;

    public String getMode() {
        return mode;
    }

    public void setMode(String mode) {
        this.mode = mode;
    }

    public String getState() {
        return state;
    }

    public void setState(String state) {
        this.state = state;
    }

    public int getCurrentPosition() {
        return currentPosition;
    }

    public void setCurrentPosition(int currentPosition) {
        this.currentPosition = currentPosition;
    }

    public int getTargetPosition() {
        return targetPosition;
    }

    public void setTargetPosition(int targetPosition) {
        this.targetPosition = targetPosition;
    }

    public boolean getIsAttached() {
        return isAttached;
    }

    public void setIsAttached(boolean isAttached) {
        this.isAttached = isAttached;
    }

    public int getLightLevel() {
        return lightLevel;
    }

    public void setLightLevel(int lightLevel) {
        this.lightLevel = lightLevel;
    }
}
