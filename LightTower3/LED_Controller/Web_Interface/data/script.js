var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var speakerImages = new Array();
var SliderTouched = false;
var SliderTimeoutHandle;
var Sink_SSID_Value_Changed = false;
var Sink_SSID_Changed_TimeoutHandle;
var Source_SSID_Value_Changed = false;
var Source_SSID_Changed_TimeoutHandle;

//Window and Web Socket Functions
window.addEventListener('load', onload);
function onload(event)
{
    initWebSocket();
}
function initWebSocket()
{
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event)
{
    console.log('Connection opened');
    getValues();
}
function onClose(event)
{
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}


// Toggle Switch Handlers
const sink_BT_Reset_toggle_Button = document.getElementById("Sink_BT_Reset");
sink_BT_Reset_toggle_Button.addEventListener("click", function()
{
  console.log(`Sink Bluetooth Reset Toggle switch is now ${sink_BT_Reset_toggle_Button.checked}`);
});
const sink_BT_Auto_ReConnect_toggle_Button = document.getElementById("Sink_BT_Auto_ReConnect");
sink_BT_Auto_ReConnect_toggle_Button.addEventListener("click", function()
{
  console.log(`Sink Auto ReConnectToggle switch is now ${sink_BT_Auto_ReConnect_toggle_Button.checked}`);
});
const source_BT_Reset_toggle_Button = document.getElementById("Source_BT_Reset");
source_BT_Reset_toggle_Button.addEventListener("click", function()
{
  console.log(`Source Bluetooth Reset Toggle switch is now ${source_BT_Reset_toggle_Button.checked}`);
});
const source_BT_ReConnect_toggle_Button = document.getElementById("Source_BT_Auto_ReConnect");
source_BT_ReConnect_toggle_Button.addEventListener("click", function()
{
  console.log(`Source Auto ReConnect Toggle switch is now ${source_BT_ReConnect_toggle_Button.checked}`);
});

// Get All Values
function getValues()
{
    var Root = {};
	Root.Message = "Get All Values";
	var Message = JSON.stringify(Root);
	console.log(Message);
    websocket.send(Message);
}

// Menu Functions
function openNav() 
{
  document.getElementById("LeftSideNavigationMenu").style.width = "200px";
  document.getElementById("ContentArea").style.marginLeft = "200px";
}

function closeNav()
{
  document.getElementById("LeftSideNavigationMenu").style.width = "0";
  document.getElementById("ContentArea").style.marginLeft = "0";
}

//Text Box
function textBoxValueChanged(element)
{
	if(element.id == "Sink_SSID_Text_Box")
	{
		clearTimeout(Sink_SSID_Changed_TimeoutHandle);
		Sink_SSID_Value_Changed = true;
		Sink_SSID_Changed_TimeoutHandle = setTimeout(Sink_SSID_Changed_Timeout, 60000);
	}
	else if(element.id == "Source_SSID_Text_Box")
	{
		clearTimeout(Source_SSID_Changed_TimeoutHandle);
		Source_SSID_Value_Changed = true;
		Source_SSID_Changed_TimeoutHandle = setTimeout(Source_SSID_Changed_Timeout, 60000);
	}
}

function Sink_SSID_Changed_Timeout()
{
	Sink_SSID_Value_Changed = false;
}

function Source_SSID_Changed_Timeout()
{
	Source_SSID_Value_Changed = false;
}

function submit_New_SSID(element)
{
	var ButtonId = element.id;
    if(ButtonId == "Sink_SSID_Submit_Button")
	{
		var Root = {};
		var TextboxElement;
		clearTimeout(Sink_SSID_Changed_TimeoutHandle);
		TextboxElement = document.getElementById("Sink_SSID_Text_Box");
		Sink_SSID_Changed_TimeoutHandle = setTimeout(Sink_SSID_Changed_Timeout, 5000);
		Root.WidgetValue = {};
		Root["WidgetValue"].Widget = TextboxElement.id;
		Root["WidgetValue"].Value = TextboxElement.value;
		var Message = JSON.stringify(Root);
		console.log(Message);
		websocket.send(Message);
	}
	else if(ButtonId == "Source_SSID_Submit_Button")
	{
		var Root = {};
		var TextboxElement;
		clearTimeout(Source_SSID_Changed_TimeoutHandle);
		TextboxElement = document.getElementById("Source_SSID_Text_Box");
		Source_SSID_Changed_TimeoutHandle = setTimeout(Source_SSID_Changed_Timeout, 5000);
		Root.WidgetValue = {};
		Root["WidgetValue"].Widget = TextboxElement.id;
		Root["WidgetValue"].Value = TextboxElement.value;
		var Message = JSON.stringify(Root);
		console.log(Message);
		websocket.send(Message);
	}
}

// Slider Functions
function updateSliderValue(element)
{
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
    websocket.send(Message);
	SliderTimeoutHandle = setTimeout(sliderNotTouched, 5000);
}

function sliderNotTouched()
{
    SliderTouched = false;
}
 
function setSpeakerImage(value)
{   
	var Image1Source;
	var Image2Source;
	var state = parseInt(value);
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

function onMessage(event)
{
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);
	for (var i = 0; i < keys.length; ++i)
	{
		var Id = myObj[keys[i]]["Id"];
		var Value = myObj[keys[i]]["Value"];
		if(null != Id && null != Value)
		{
			if(Id == "Speaker_Image")
			{
				setSpeakerImage(Value);
			}
			else if( Id == "Amplitude_Gain_Slider1" || 
					 Id == "Amplitude_Gain_Slider2" || 
					 Id == "FFT_Gain_Slider1" ||
					 Id == "FFT_Gain_Slider2" )
			{
				if(false == SliderTouched)
				{
					document.getElementById(Id).value = Value;
				}
				document.getElementById(Id + "_Value").innerHTML = Value;
			}
			else if( Id == "Sink_SSID_Text_Box" && false == Sink_SSID_Value_Changed)
			{
				document.getElementById(Id).value = Value;
			}
			else if( Id == "Source_SSID_Text_Box" && false == Source_SSID_Value_Changed)
			{
				document.getElementById(Id).value = Value;
			}
			else if( Id == "
		}
	}
}

function openTab(evt, TabTitle) 
{
  var i, TabContent, Tablinks;
  TabContent = document.getElementsByClassName("TabContent");
  for (i = 0; i < TabContent.length; i++) 
  {
    TabContent[i].style.display = "none";
  }
  Tablinks = document.getElementsByClassName("Tablinks");
  for (i = 0; i < Tablinks.length; i++) 
  {
    Tablinks[i].className = Tablinks[i].className.replace(" active", "");
  }
  document.getElementById(TabTitle).style.display = "block";
  evt.currentTarget.className += " active";
  document.getElementById("TabHeader_Heading").innerHTML = TabTitle;
}
