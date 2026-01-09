document.addEventListener("DOMContentLoaded", () => {
    loadInitialState("B1");

    // websockets
    const socket = new SockJS('/ws');
    const stompClient = Stomp.over(socket);

    stompClient.connect({}, function () {
        stompClient.subscribe('/home/bedroom/blinds/B1/availability', function (message) {
            const topic = message.headers.destination;
            const data = JSON.parse(message.body);;
            const status = data.availability;

            const deviceId = topic.split("/")[4];

            updateUIAvailability(deviceId, status);
        });

        stompClient.subscribe('/home/bedroom/blinds/B1/state', function (message) {
            const topic = message.headers.destination;
            const data = JSON.parse(message.body);;
            const deviceId = topic.split("/")[4];

            updateUIState(deviceId, data);
        });
    });
});

// Load Initial state
async function loadInitialState(deviceId) {
    const availabilityRes = await fetch(`/api/${deviceId}/availability`);
    const stateRes = await fetch(`/api/${deviceId}/state`);

    if (availabilityRes.ok) {
        const availability = await availabilityRes.json();
        const status = availability.availability;

        updateUIAvailability(deviceId, status);
    }

    if (stateRes.ok) {
        const state = await stateRes.json();

        updateUIState(deviceId, state);
        updateTiltSlider(state.currentPos);
    }
}

// Send Commands
function sendCommand(deviceId, command, value) {
    fetch(`/api/blinds/${deviceId}/${command}?${command}=${value}`, {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        }
    })
    .then(response => {
        if (response.ok) return response.text();
        throw new Error("Network response was not OK");
    })
    .catch(error => {
        console.error("Error sending command:", error);
    });
}

// Update UI
function updateUIAvailability(deviceId, status){
    const statusDot = document.getElementById("status-" + deviceId + "-dot");
    const SectionForDevice = document.getElementById(deviceId + "-device");

    document.getElementById("status-" + deviceId).textContent = (status === "online")? "connected":"disconnected";
    statusDot.classList.remove((status === "online")? "offline":"online");
    statusDot.classList.add((status === "online")? "online":"offline");

    if (status === "online") {
        SectionForDevice.classList.remove("opacity-offline");
        document.querySelectorAll("#set-mode-toggle .btn-check").forEach(e => e.classList.remove("disable-mode-button"));
    }else{
        SectionForDevice.classList.add("opacity-offline");
        document.querySelectorAll("#set-mode-toggle .btn-check").forEach(e => e.classList.add("disable-mode-button"));
        disableTiltSet();
        setTelemetryData(deviceId, "-", "-", "-");
    }
}

function updateUIState(deviceId, state){
    setTelemetryData(deviceId, state.mode, state.state, state.currentPos);

    if(state.mode === "MANUAL"){
        document.querySelector(`label[for="${manualButton.id}"]`).textContent = "MANUAL";
        enableTiltSet();
        if (!manualButton.checked)
            manualButton.checked = true;

        updateTiltSlider(state.targetPos);
    }
    if(state.mode === "AUTO"){
        document.querySelector(`label[for="${autoButton.id}"]`).textContent = "AUTO";
        if (!autoButton.checked)
            autoButton.checked = true;

        updateTiltSlider(state.currentPos);
    }
}

function disableTiltSet(){
    document.querySelectorAll("#manual-tilt-set div").forEach(e => e.classList.add("text-muted"));
    document.getElementById("custom-tilt").disabled = true;
    document.querySelector("#manual-tilt-set button").disabled = true;
}

function enableTiltSet(){
    document.querySelectorAll("#manual-tilt-set div").forEach(e => e.classList.remove("text-muted"));
    document.getElementById("custom-tilt").disabled = false;
    document.querySelector("#manual-tilt-set button").disabled = false;
}

function setTelemetryData(deviceId, mode, state, tilt){
    document.getElementById("state-mode-" + deviceId).textContent = mode;
    document.getElementById("state-now-" + deviceId).textContent = state;
    document.getElementById("state-tilt-" + deviceId).textContent = tilt;
}

function updateTiltSlider(tilt){
    const tiltValue = Number(tilt);
    document.getElementById("custom-tilt-value").textContent = tilt;
    document.getElementById("custom-tilt").value = tilt;
}
