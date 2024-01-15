var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var speakerImages = new Array();
var sliderTouched = false;
var sliderTimeoutHandle;
var Sink_SSID_Value_Changed = false;
var Sink_SSID_Changed_TimeoutHandle;
var Source_SSID_Value_Changed = false;
var Source_SSID_Changed_TimeoutHandle;

const ConnectionStatus = 
{
	Disconnected: 0,
	Waiting: 1,
	Searching: 2,
	Pairing: 3,
	Paired: 4
}

//Window and Web Socket Functions
window.addEventListener('load', onload);
function onload(event)
{
    initWebSocket();
	showContent('menu-content', 'Speaker Status');
}
function initWebSocket()
{
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
	websocket.error = onError;
}
function onOpen(event)
{
    console.log('Connection opened');
}
function onClose(event)
{
    console.log('Connection closed');
    setTimeout(initWebSocket, 5000);
}
function onError(event)
{
    console.log('Connection Error');
    setTimeout(initWebSocket, 5000);
}


// Toggle Switch Handlers
const sink_BT_Enable_Toggle_Button = document.getElementById("Sink_BT_Enable_Toggle_Button");
sink_BT_Enable_Toggle_Button.addEventListener("click", function()
{
	var Root = {};
	Root.WidgetValue = {};
	Root["WidgetValue"].Id = "Sink_BT_Enable_Toggle_Button";
	Root["WidgetValue"].Value = String(sink_BT_Enable_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	console.log(Message);
	websocket.send(Message);
});
const sink_BT_Auto_ReConnect_Toggle_Button = document.getElementById("Sink_BT_Auto_ReConnect_Toggle_Button");
sink_BT_Auto_ReConnect_Toggle_Button.addEventListener("click", function()
{
	var Root = {};
	Root.WidgetValue = {};
	Root["WidgetValue"].Id = "Sink_BT_Auto_ReConnect_Toggle_Button";
	Root["WidgetValue"].Value = String(sink_BT_Auto_ReConnect_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	console.log(Message);
	websocket.send(Message);
});
const source_BT_Reset_Toggle_Button = document.getElementById("Source_BT_Reset_Toggle_Button");
source_BT_Reset_Toggle_Button.addEventListener("click", function()
{
	var Root = {};
	Root.WidgetValue = {};
	Root["WidgetValue"].Id = "Source_BT_Reset_Toggle_Button";
	Root["WidgetValue"].Value = String(source_BT_Reset_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	console.log(Message);
	websocket.send(Message);
});
const source_BT_Auto_ReConnect_Toggle_Button = document.getElementById("Source_BT_Auto_ReConnect_Toggle_Button");
Source_BT_Auto_ReConnect_Toggle_Button.addEventListener("click", function()
{
	var Root = {};
	Root.WidgetValue = {};
	Root["WidgetValue"].Id = "Source_BT_Auto_ReConnect_Toggle_Button";
	Root["WidgetValue"].Value = String(source_BT_Auto_ReConnect_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	console.log(Message);
	websocket.send(Message);
});

// Menu Functions
function openNav() 
{
  document.getElementById("leftSideNavigationMenu").style.width = "200px";
  document.getElementById("MainContentArea").style.marginLeft = "200px";
}

function closeNav()
{
  document.getElementById("leftSideNavigationMenu").style.width = "0";
  document.getElementById("MainContentArea").style.marginLeft = "0";
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
		Root["WidgetValue"].Id = TextboxElement.id;
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
		Root["WidgetValue"].Id = TextboxElement.id;
		Root["WidgetValue"].Value = TextboxElement.value;
		var Message = JSON.stringify(Root);
		console.log(Message);
		websocket.send(Message);
	}
}

// slider Functions
function updatesliderValue(element)
{
	clearTimeout(sliderTimeoutHandle);
    sliderTouched = true;
	var sliderName = element.id;
    var sliderValue = document.getElementById(sliderName).value;
    var Root = {};
	Root.WidgetValue = {};
	Root["WidgetValue"].Id = sliderName.toString();
	Root["WidgetValue"].Value = sliderValue.toString();
	var Message = JSON.stringify(Root);
	console.log(Message);
    websocket.send(Message);
	sliderTimeoutHandle = setTimeout(sliderNotTouched, 5000);
}

function sliderNotTouched()
{
    sliderTouched = false;
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
		console.log("WIDGET: " + Id + " = " + Value);
		if(null != Id && null != Value)
		{
			if(Id == "Speaker_Image")
			{
				setSpeakerImage(Value);
			}
			else if( Id == "Amplitude_Gain_slider1" || 
					 Id == "Amplitude_Gain_slider2" || 
					 Id == "Amplitude_Gain_slider3" || 
					 Id == "FFT_Gain_slider1" ||
					 Id == "FFT_Gain_slider2" )
			{
				if(false == sliderTouched)
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
			else if(Id == "Source_Connection_Status")
			{
				var element = document.getElementById("Source_Connection_Status");
				switch(parseInt(Value))
				{
					case ConnectionStatus.Waiting:
						element.innerHTML = "Waiting";
					break;
					case ConnectionStatus.Searching:
						element.innerHTML = "Searching";
					break;
					case ConnectionStatus.Pairing:
						element.innerHTML = "Pairing";
					break;
					case ConnectionStatus.Paired:
						element.innerHTML = "Paired";
					break;
					case ConnectionStatus.Disconnected:
						element.innerHTML = "Disconnected";
					break;
					default:
						element.innerHTML = "Error";
					break;
				}
			}
			else if(Id == "Source_BT_Reset_Toggle_Button")
			{
				if(Value == "true")
				{
					source_BT_Reset_Toggle_Button.checked = true;
				}
				else
				{
					source_BT_Reset_Toggle_Button.checked = false;
				}
			}
			else if(Id == "Source_BT_Auto_ReConnect_Toggle_Button")
			{
				if(Value == "true")
				{
					source_BT_Auto_ReConnect_Toggle_Button.checked = true;
				}
				else
				{
					source_BT_Auto_ReConnect_Toggle_Button.checked = false;
				}
			}
			else if(Id == "Sink_Connection_Status")
			{
				var element = document.getElementById("Sink_Connection_Status");
				switch(parseInt(Value))
				{
					case ConnectionStatus.Waiting:
						element.innerHTML = "Waiting";
					break;
					case ConnectionStatus.Searching:
						element.innerHTML = "Searching";
					break;
					case ConnectionStatus.Pairing:
						element.innerHTML = "Pairing";
					break;
					case ConnectionStatus.Paired:
						element.innerHTML = "Paired";
					break;
					case ConnectionStatus.Disconnected:
						element.innerHTML = "Disconnected";
					break;
					default:
						element.innerHTML = "Error";
					break;
				}
			}
			else if(Id == "Sink_BT_Auto_ReConnect_Toggle_Button")
			{
				if(Value == "true")
				{
					sink_BT_Auto_ReConnect_Toggle_Button.checked = true;
				}
				else
				{
					sink_BT_Auto_ReConnect_Toggle_Button.checked = false;
				}
			}
			else if(Id == "Sink_BT_Enable_Toggle_Button")
			{
				if(Value == "true")
				{
					sink_BT_Enable_Toggle_Button.checked = true;
				}
				else
				{
					sink_BT_Enable_Toggle_Button.checked = false;
				}
			}
			else if(Id == "Input_Sound_Source")
			{
				var radioOption = document.getElementById(Id);
				if (radioOption)
				{
					radioOption.checked = true;
				}
			}
		}
	}
}

function showContent(classId, contentId) {
	// Hide all tab contents
	var tabContents = document.querySelectorAll('.' + classId);
	tabContents.forEach(function (tabContent) {
		tabContent.classList.remove('active');
	});

	// Show the selected tab content
	document.getElementById(contentId).classList.add('active');
}