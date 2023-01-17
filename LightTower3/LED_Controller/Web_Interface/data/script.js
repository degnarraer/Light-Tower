var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function getValues(){
    websocket.send("Get All Values");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getValues();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateSliderValue(element) {
    var SliderName = element.id;
    var SliderValue = document.getElementById(SliderName).value;
    var JSONObject = {};
	JSONObject.Name = SliderName.toString();
	JSONObject.Value = SliderValue.toString();
	var Message = JSON.stringify(JSONObject);
	console.log(Message);
    console.log(SliderValue);
    websocket.send(Message);
}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);
	for (var i = 0; i < keys.length; ++i)
	{
		var Name = myObj[keys[i]]["Name"];
		var Value = myObj[keys[i]]["Value"];
		if(Name && Value)
		{
			document.getElementById(Name).value = Value;
			document.getElementById(Name + "_Value").innerHTML = Value;
		}
	}
}

function OpenTab(evt, cityName) {
  var i, TabContent, Tablinks;
  TabContent = document.getElementsByClassName("TabContent");
  for (i = 0; i < TabContent.length; i++) {
    TabContent[i].style.display = "none";
  }
  Tablinks = document.getElementsByClassName("Tablinks");
  for (i = 0; i < Tablinks.length; i++) {
    Tablinks[i].className = Tablinks[i].className.replace(" active", "");
  }
  document.getElementById(cityName).style.display = "block";
  evt.currentTarget.className += " active";
}