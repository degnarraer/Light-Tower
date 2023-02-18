var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var speakerImages = new Array();
var SliderTouched = false;
var SliderTimeoutHandle;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
	openTab(event, 'Bluetooth In')
}

function getValues(){
    var Root = {};
	Root.Message = "Get All Values";
	var Message = JSON.stringify(Root);
	console.log(Message);
    websocket.send(Message);
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
	clearTimeout(SliderTimeoutHandle);
    SliderTouched = true;
	var SliderName = element.id;
    var SliderValue = document.getElementById(SliderName).value;
    var Root = {};
	Root.WidgetValue = {};
	Root["WidgetValue"].Widget = SliderName.toString();
	Root["WidgetValue"].Value = SliderValue.toString();
	var Message = JSON.stringify(Root);
	console.log(Message);
    console.log(SliderValue);
    websocket.send(Message);
	SliderTimeoutHandle = setTimeout(SliderNotTouched, 5000);
}

function SliderNotTouched()
{
    SliderTouched = false;
}
 
function setSpeakerImage(state)
{   
	var Image1Source;
	var Image2Source;
	const imageOneElement = document.getElementById("L Speaker Image");
	const imageTwoElement = document.getElementById("R Speaker Image");
	var imageOne = new Image;
	var imageTwo = new Image;
	switch(state)
	{
	  case 0:
		Image1Source = "Images/L-Speaker-Off.svg";
		Image2Source = "Images/R-Speaker-Off.svg";
	  break;
	  case 1:
		Image1Source = "Images/L-Speaker-0.svg";
		Image2Source = "Images/R-Speaker-0.svg";
	  break;
	  case 2:
		Image1Source = "Images/L-Speaker-1.svg";
		Image2Source = "Images/R-Speaker-1.svg";
	  break;
	  case 3:
		Image1Source = "Images/L-Speaker-2.svg";
		Image2Source = "Images/R-Speaker-2.svg";
	  break;
	  case 4:
		Image1Source = "Images/L-Speaker-3.svg";
		Image2Source = "Images/R-Speaker-3.svg";
	  break;
	  case 5:
		Image1Source = "Images/L-Speaker-4.svg";
		Image2Source = "Images/R-Speaker-4.svg";
	  break;
	  case 6:
		Image1Source = "Images/L-Speaker-5.svg";
		Image2Source = "Images/R-Speaker-5.svg";
	  break;
	  case 7:
		Image1Source = "Images/L-Speaker-6.svg";
		Image2Source = "Images/R-Speaker-6.svg";
	  break;
	  case 8:
		Image1Source = "Images/L-Speaker-7.svg";
		Image2Source = "Images/R-Speaker-7.svg";
	  break;
	  case 9:
		Image1Source = "Images/L-Speaker-8.svg";
		Image2Source = "Images/R-Speaker-8.svg";
	  break;
	  case 10:
		Image1Source = "Images/L-Speaker-9.svg";
		Image2Source = "Images/R-Speaker-9.svg";
	  break;
	  case 11:
		Image1Source = "Images/L-Speaker-10.svg";
		Image2Source = "Images/R-Speaker-10.svg";
	  break;
	  case 12:
		Image1Source = "Images/L-Speaker-11.svg";
		Image2Source = "Images/R-Speaker-11.svg";
	  break;
	  default:
		Image1Source = "Images/L-Speaker-0.svg";
		Image2Source = "Images/R-Speaker-0.svg";
	  break;
	}
	imageTwo.src = Image2Source;
	
	
	const imageOnePromise = new Promise((resolve, reject) => {
	  imageOne.src = Image1Source;
	  imageOne.onload = () => {
		resolve();
	  };
	  imageOne.onerror = () => {
		reject("Error loading image one");
	  };
	});

	const imageTwoPromise = new Promise((resolve, reject) => {
	  imageTwo.src = Image2Source;
	  imageTwo.onload = () => {
		resolve();
	  };
	  imageTwo.onerror = () => {
		reject("Error loading image two");
	  };
	});  

	Promise.all([imageOnePromise, imageTwoPromise]).then(() => {
	  // both images have finished loading
	  imageOneElement.src = imageOne.src;
	  imageTwoElement.src = imageTwo.src;
	});
}

function openNav() {
  document.getElementById("LeftSideNavigationMenu").style.width = "200px";
  document.getElementById("ContentArea").style.marginLeft = "200px";
}

function closeNav() {
  document.getElementById("LeftSideNavigationMenu").style.width = "0";
  document.getElementById("ContentArea").style.marginLeft = "0";
}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);
	for (var i = 0; i < keys.length; ++i)
	{
		var Name = myObj[keys[i]]["Name"];
		var Value = myObj[keys[i]]["Value"];
		if(null != Name && null != Value)
		{
			if(Name == "Speaker_Image")
			{
				setSpeakerImage(parseInt(Value))
			}
			else if( Name == "Amplitude_Gain_Slider1" || 
					 Name == "Amplitude_Gain_Slider2" || 
					 Name == "FFT_Gain_Slider1" ||
					 Name == "FFT_Gain_Slider2" )
			{
				if(false == SliderTouched)
				{
					document.getElementById(Name).value = Value;
				}
				document.getElementById(Name + "_Value").innerHTML = Value;
			}
			else if( Name == "Sink_SSID" )
			{
				document.getElementByName("Sink SSID").value = Value;
			}
		}
	}
}

function openTab(evt, TabTitle) {
  var i, TabContent, Tablinks;
  TabContent = document.getElementsByClassName("TabContent");
  for (i = 0; i < TabContent.length; i++) {
    TabContent[i].style.display = "none";
  }
  Tablinks = document.getElementsByClassName("Tablinks");
  for (i = 0; i < Tablinks.length; i++) {
    Tablinks[i].className = Tablinks[i].className.replace(" active", "");
  }
  document.getElementById(TabTitle).style.display = "block";
  evt.currentTarget.className += " active";
  document.getElementById("TabHeader_Heading").innerHTML = TabTitle;
}

const toggleBtn = document.getElementById("togBtn");
toggleBtn.addEventListener("click", function() {
  console.log(`Toggle switch is now ${toggleBtn.checked}`);
});