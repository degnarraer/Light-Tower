var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var speakerImages = new Array();
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
	setupSpeakerImage();
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

function setupSpeakerImage()
{
	speakerImages[0] = new Image();
	speakerImages[0].src = "Images/L-Speaker-Off.svg"
	speakerImages[1] = new Image();
	speakerImages[1].src = "Images/R-Speaker-Off.svg"
	
	speakerImages[2] = new Image();
	speakerImages[2].src = "Images/L-Speaker-0.svg"
	speakerImages[3] = new Image();
	speakerImages[3].src = "Images/R-Speaker-0.svg"
	
	speakerImages[4] = new Image();
	speakerImages[4].src = "Images/L-Speaker-1.svg"
	speakerImages[5] = new Image();
	speakerImages[5].src = "Images/R-Speaker-1.svg"
	
	speakerImages[6] = new Image();
	speakerImages[6].src = "Images/L-Speaker-2.svg"
	speakerImages[7] = new Image();
	speakerImages[7].src = "Images/R-Speaker-2.svg"
	
	speakerImages[8] = new Image();
	speakerImages[8].src = "Images/L-Speaker-3.svg"
	speakerImages[9] = new Image();
	speakerImages[9].src = "Images/R-Speaker-3.svg"
	
	speakerImages[10] = new Image();
	speakerImages[10].src = "Images/L-Speaker-4.svg"
	speakerImages[11] = new Image();
	speakerImages[11].src = "Images/R-Speaker-4.svg"
	
	speakerImages[12] = new Image();
	speakerImages[12].src = "Images/L-Speaker-5.svg"
	speakerImages[13] = new Image();
	speakerImages[13].src = "Images/R-Speaker-5.svg"
	
	speakerImages[14] = new Image();
	speakerImages[14].src = "Images/L-Speaker-6.svg"
	speakerImages[15] = new Image();
	speakerImages[15].src = "Images/R-Speaker-6.svg"
	
	speakerImages[16] = new Image();
	speakerImages[16].src = "Images/L-Speaker-7.svg"
	speakerImages[17] = new Image();
	speakerImages[17].src = "Images/R-Speaker-7.svg"
	
	speakerImages[18] = new Image();
	speakerImages[18].src = "Images/L-Speaker-8.svg"
	speakerImages[19] = new Image();
	speakerImages[19].src = "Images/R-Speaker-8.svg"
	
	speakerImages[20] = new Image();
	speakerImages[20].src = "Images/L-Speaker-9.svg"
	speakerImages[21] = new Image();
	speakerImages[21].src = "Images/R-Speaker-9.svg"
	
	speakerImages[22] = new Image();
	speakerImages[22].src = "Images/L-Speaker-10.svg"
	speakerImages[23] = new Image();
	speakerImages[23].src = "Images/R-Speaker-10.svg"
	
	speakerImages[24] = new Image();
	speakerImages[24].src = "Images/L-Speaker-11.svg"
	speakerImages[25] = new Image();
	speakerImages[25].src = "Images/R-Speaker-11.svg"
	
	
	
	
}   
    
function setSpeakerImage(state)
{   
	switch(state)
	{
		case 0:
			document.getElementById("L Speaker Image").src = speakerImages[0].src;
			document.getElementById("R Speaker Image").src = speakerImages[1].src;
		break;
		case 1:
			document.getElementById("L Speaker Image").src = speakerImages[2].src;
			document.getElementById("R Speaker Image").src = speakerImages[3].src;
		break;
		case 2:
			document.getElementById("L Speaker Image").src = speakerImages[4].src;
			document.getElementById("R Speaker Image").src = speakerImages[5].src;
		break;
		case 3:
			document.getElementById("L Speaker Image").src = speakerImages[6].src;
			document.getElementById("R Speaker Image").src = speakerImages[7].src;
		break;
		case 4:
			document.getElementById("L Speaker Image").src = speakerImages[8].src;
			document.getElementById("R Speaker Image").src = speakerImages[9].src;
		break;
		case 5:
			document.getElementById("L Speaker Image").src = speakerImages[10].src;
			document.getElementById("R Speaker Image").src = speakerImages[11].src;
		break;
		case 6:
			document.getElementById("L Speaker Image").src = speakerImages[12].src;
			document.getElementById("R Speaker Image").src = speakerImages[13].src;
		break;
		case 7:
			document.getElementById("L Speaker Image").src = speakerImages[14].src;
			document.getElementById("R Speaker Image").src = speakerImages[15].src;
		break;
		case 8:
			document.getElementById("L Speaker Image").src = speakerImages[16].src;
			document.getElementById("R Speaker Image").src = speakerImages[17].src;
		break;
		case 9:
			document.getElementById("L Speaker Image").src = speakerImages[18].src;
			document.getElementById("R Speaker Image").src = speakerImages[19].src;
		break;
		case 10:
			document.getElementById("L Speaker Image").src = speakerImages[20].src;
			document.getElementById("R Speaker Image").src = speakerImages[21].src;
		break;
		case 11:
			document.getElementById("L Speaker Image").src = speakerImages[22].src;
			document.getElementById("R Speaker Image").src = speakerImages[23].src;
		case 12:
			document.getElementById("L Speaker Image").src = speakerImages[24].src;
			document.getElementById("R Speaker Image").src = speakerImages[25].src;
		break;
		default:
			document.getElementById("L Speaker Image").src = speakerImages[0].src;
			document.getElementById("R Speaker Image").src = speakerImages[0].src;
		break;
	}
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
			if(Name == "Speaker_Image")
			{
				setSpeakerImage(parseInt(Value))
			}
			else
			{
				document.getElementById(Name).value = Value;
				document.getElementById(Name + "_Value").innerHTML = Value;
			}
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