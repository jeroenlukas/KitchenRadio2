
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    initButtons();
}



function getValuesHome()
{
    websocket.send("getValuesAudio");
    websocket.send("getValuesWeather");
}


function getValuesSystem()
{
    websocket.send("getValuesSystem");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');    

    console.log("path : " + window.location.pathname);

    if(window.location.pathname == "/")
    {
        // Home
        let interval = setInterval(() => getValuesHome(), 1000);
    }
    else if(window.location.pathname == "/settings")
    {
        // Settings
        let interval = setInterval(() => getValuesSystem(), 1000);
    }

    
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function initButtons() {
    eleButtonAudioOff = document.getElementById('button_audio_off');
    if(eleButtonAudioOff != null) eleButtonAudioOff.addEventListener('click', buttonOffPressed);

    eleButtonAudioWebradio = document.getElementById('button_audio_webradio');
    if(eleButtonAudioWebradio != null) eleButtonAudioWebradio.addEventListener('click', buttonWebradioPressed);    
    
    eleButtonAudioBluetooth = document.getElementById('button_audio_bluetooth');
    if(eleButtonAudioBluetooth != null) eleButtonAudioBluetooth.addEventListener('click', buttonBluetoothPressed);
}

function colorPickerChanged(picker)
{
    console.log("color picker changed");
    
    websocket.send('{"ledring": {"r": ' + Math.round(picker.channel('R')) + ', "g": ' + Math.round(picker.channel('G')) + ', "b": ' + Math.round(picker.channel('B')) + '}}');
}

function buttonOffPressed(){
    console.log('off pressed');
    websocket.send('buttonOffPressed');
}

function buttonWebradioPressed(){
    websocket.send('buttonWebradioPressed');
}

function buttoBluetoothPressed(){
    websocket.send('buttonBluetoothPressed');
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        var ele = document.getElementById(key);
        if(ele != null){
            ele.innerHTML = myObj[key];
        }
    }
}