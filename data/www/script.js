
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

function getValuesConfig()
{
    websocket.send("getValuesConfig");
}

function getValuesStations()
{
    websocket.send("getValuesStations");
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
    else if(window.location.pathname == "/config")
    {
        // Config (once)
        getValuesConfig();
    }

    else if(window.location.pathname == "/stations")
    {
        // Stations (once)
        getValuesStations();
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

    eleButtonConsole = document.getElementById('button_console');
    if(eleButtonConsole != null) eleButtonConsole.addEventListener('click', buttonConsolePressed);

    eleSliderVolume = document.getElementById('slider_volume');
    if(eleSliderVolume != null) eleSliderVolume.addEventListener('click', sliderVolumeChanged);

    eleButtonAudioPrev = document.getElementById('button_audio_prev');
    if(eleButtonAudioPrev != null) eleButtonAudioPrev.addEventListener('click', buttonAudioPrevPressed);

    eleButtonAudioNext = document.getElementById('button_audio_next');
    if(eleButtonAudioNext != null) eleButtonAudioNext.addEventListener('click', buttonAudioNextPressed);
}

function colorPickerChanged(picker)
{
    console.log("color picker changed");
    
    websocket.send('{"ledring": {"h": ' + picker.channel('H')/360.0 + ', "s": ' +picker.channel('S')/100.0 + ', "v": ' + picker.channel('V')/100.0 + '}}');
}

function buttonOffPressed(){
    console.log('off pressed');
    websocket.send('buttonOffPressed');
}

function buttonWebradioPressed(){
    websocket.send('buttonWebradioPressed');
}

function buttonBluetoothPressed(){
    websocket.send('buttonBluetoothPressed');
}

function buttonAudioPrevPressed(){
    websocket.send('buttonAudioPrevPressed');
}

function buttonAudioNextPressed(){
    websocket.send('buttonAudioNextPressed');
}

function buttonConsolePressed()
{    
    console.log('{"console":{"command": "' + document.getElementById('text_console').value + '"}}');
    websocket.send('{"console":{"command": "' + document.getElementById('text_console').value + '"}}');
}

function sliderVolumeChanged()
{
    websocket.send('{\"volume\":'+document.getElementById('slider_volume').value + '}');
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