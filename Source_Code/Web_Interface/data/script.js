var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var speakerImages = new Array();
var sliderTouched = false;
var sliderTimeoutHandle;
var sink_Name_Value_Changed = false;
var sink_Name_Changed_TimeoutHandle;
var source_Name_Value_Changed = false;
var source_Name_Changed_TimeoutHandle;
var compatibleDevices = [
	{ name: "Device 1", address: "00:11:22:33:44:55", rssi: -50 },
	{ name: "Device 2", address: "AA:BB:CC:DD:EE:FF", rssi: -60 },
];


let selectedDeviceIndex = -1;


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
	showContent('menu-content', 'Sound Output');
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
	websocket.send('Hello I am here!');
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
const sink_BT_Enable_Toggle_Button = document.getElementById('Sink_BT_Enable_Toggle_Button');
sink_BT_Enable_Toggle_Button.addEventListener('click', function()
{
	//FUCK THIS
	var Root = {};
	Root.WidgetValue = {};
	Root.WidgetValue.Id = 'BT_Sink_Enable';
	Root.WidgetValue.Value = String(sink_BT_Enable_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	websocket.send(Message);
});
const sink_BT_Auto_ReConnect_Toggle_Button = document.getElementById('Sink_BT_Auto_ReConnect_Toggle_Button');
sink_BT_Auto_ReConnect_Toggle_Button.addEventListener('click', function()
{
	var Root = {};
	Root.WidgetValue = {};
	Root.WidgetValue.Id = 'BT_Sink_Auto_ReConnect';
	Root.WidgetValue.Value = String(sink_BT_Auto_ReConnect_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	websocket.send(Message);
});
const source_BT_Reset_Toggle_Button = document.getElementById('Source_BT_Reset_Toggle_Button');
source_BT_Reset_Toggle_Button.addEventListener('click', function()
{
	var Root = {};
	Root.WidgetValue = {};
	Root.WidgetValue.Id = 'BT_Source_Reset';
	Root.WidgetValue.Value = String(source_BT_Reset_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	websocket.send(Message);
});
const source_BT_Auto_ReConnect_Toggle_Button = document.getElementById('Source_BT_Auto_ReConnect_Toggle_Button');
Source_BT_Auto_ReConnect_Toggle_Button.addEventListener('click', function()
{
	var Root = {};
	Root.WidgetValue = {};
	Root.WidgetValue.Id = 'BT_Source_Auto_Reconnect';
	Root.WidgetValue.Value = String(source_BT_Auto_ReConnect_Toggle_Button.checked);
	var Message = JSON.stringify(Root);
	websocket.send(Message);
});

// Menu Functions
function openNav() 
{
  document.getElementById('leftSideNavigationMenu').style.width = '200px';
  document.getElementById('MainContentArea').style.marginLeft = '200px';
}

function closeNav()
{
  document.getElementById('leftSideNavigationMenu').style.width = '0';
  document.getElementById('MainContentArea').style.marginLeft = '0';
}

//Text Box
function textBoxValueChanged(element)
{
	if(element.Id == 'Sink_Name_Text_Box')
	{
		clearTimeout(sink_Name_Changed_TimeoutHandle);
		sink_Name_Value_Changed = true;
		sink_Name_Changed_TimeoutHandle = setTimeout(Sink_Name_Changed_Timeout, 60000);
	}
	else if(element.Id == 'Source_Name_Text_Box')
	{
		clearTimeout(source_Name_Changed_TimeoutHandle);
		source_Name_Value_Changed = true;
		source_Name_Changed_TimeoutHandle = setTimeout(Source_Name_Changed_Timeout, 60000);
	}
}

function Sink_Name_Changed_Timeout()
{
	sink_Name_Value_Changed = false;
}

function Source_Name_Changed_Timeout()
{
	source_Name_Value_Changed = false;
}

function submit_New_Name(element)
{
	var ButtonId = element.id;
    if(ButtonId == 'Sink_Name_Submit_Button')
	{
		var Root = {};
		var TextboxElement;
		clearTimeout(sink_Name_Changed_TimeoutHandle);
		TextboxElement = document.getElementById('Sink_Name_Text_Box');
		sink_Name_Changed_TimeoutHandle = setTimeout(Sink_Name_Changed_Timeout, 5000);
		Root.WidgetValue = {};
		Root.WidgetValue.Id = TextboxElement.id;
		Root.WidgetValue.Value = TextboxElement.value;
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	}
	else if(ButtonId == 'Source_Name_Submit_Button')
	{
		var Root = {};
		var TextboxElement;
		clearTimeout(source_Name_Changed_TimeoutHandle);
		TextboxElement = document.getElementById('Source_Name_Text_Box');
		source_Name_Changed_TimeoutHandle = setTimeout(Source_Name_Changed_Timeout, 5000);
		Root.WidgetValue = {};
		Root.WidgetValue.Id = TextboxElement.id;
		Root.WidgetValue.Value = TextboxElement.value;
		var Message = JSON.stringify(Root);
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
	var signal = widgetToSignal[sliderName.toString()];
	if(signal)
	{
		Root.WidgetValue.Id = signal.toString();
		Root.WidgetValue.Value = sliderValue.toString();
		var Message = JSON.stringify(Root);
		websocket.send(Message);
		sliderTimeoutHandle = setTimeout(sliderNotTouched, 5000);
	}
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
	const imageOneElement = document.getElementById('L Speaker Image');
	const imageTwoElement = document.getElementById('R Speaker Image');
	var imageOne = new Image;
	var imageTwo = new Image;
	switch(state)
	{
	  case 0:
		Image1Source = 'Images/L-Speaker-Off.svg';
		Image2Source = 'Images/R-Speaker-Off.svg';
	  break;
	  case 1:
		Image1Source = 'Images/L-Speaker-0.svg';
		Image2Source = 'Images/R-Speaker-0.svg';
	  break;
	  case 2:
		Image1Source = 'Images/L-Speaker-1.svg';
		Image2Source = 'Images/R-Speaker-1.svg';
	  break;
	  case 3:
		Image1Source = 'Images/L-Speaker-2.svg';
		Image2Source = 'Images/R-Speaker-2.svg';
	  break;
	  case 4:
		Image1Source = 'Images/L-Speaker-3.svg';
		Image2Source = 'Images/R-Speaker-3.svg';
	  break;
	  case 5:
		Image1Source = 'Images/L-Speaker-4.svg';
		Image2Source = 'Images/R-Speaker-4.svg';
	  break;
	  case 6:
		Image1Source = 'Images/L-Speaker-5.svg';
		Image2Source = 'Images/R-Speaker-5.svg';
	  break;
	  case 7:
		Image1Source = 'Images/L-Speaker-6.svg';
		Image2Source = 'Images/R-Speaker-6.svg';
	  break;
	  case 8:
		Image1Source = 'Images/L-Speaker-7.svg';
		Image2Source = 'Images/R-Speaker-7.svg';
	  break;
	  case 9:
		Image1Source = 'Images/L-Speaker-8.svg';
		Image2Source = 'Images/R-Speaker-8.svg';
	  break;
	  case 10:
		Image1Source = 'Images/L-Speaker-9.svg';
		Image2Source = 'Images/R-Speaker-9.svg';
	  break;
	  case 11:
		Image1Source = 'Images/L-Speaker-10.svg';
		Image2Source = 'Images/R-Speaker-10.svg';
	  break;
	  case 12:
		Image1Source = 'Images/L-Speaker-11.svg';
		Image2Source = 'Images/R-Speaker-11.svg';
	  break;
	  default:
		Image1Source = 'Images/L-Speaker-0.svg';
		Image2Source = 'Images/R-Speaker-0.svg';
	  break;
	}
	imageTwo.src = Image2Source;
	
	
	const imageOnePromise = new Promise((resolve, reject) => {
	  imageOne.src = Image1Source;
	  imageOne.onload = () => {
		resolve();
	  };
	  imageOne.onerror = () => {
		reject('Error loading image one');
	  };
	});

	const imageTwoPromise = new Promise((resolve, reject) => {
	  imageTwo.src = Image2Source;
	  imageTwo.onload = () => {
		resolve();
	  };
	  imageTwo.onerror = () => {
		reject('Error loading image two');
	  };
	});  

	Promise.all([imageOnePromise, imageTwoPromise]).then(() => {
	  // both images have finished loading
	  imageOneElement.src = imageOne.src;
	  imageTwoElement.src = imageTwo.src;
	});
}

const messageHandlers = {
	'Sound_Input_Source': handleSoundInputSource,
	'Sound_Output_Source': handleSoundOutputSource,
	
	'Speaker_Image': handleSpeakerImage,
	'Amplitude_Gain': handleAmplitudeGain,
	'FFT_Gain': handleFFTGain,
	'FFT_Gain_slider2': handleFFTGain,
	
	'BT_Sink_Name': handleBTSinkName,
	'BT_Sink_Enable': handleBTSinkEnable,
	'BT_Sink_Auto_ReConnect': handleBTSinkAutoReConnect,
	'BT_Sink_Connection_Status': handleBTSinkConnectionStatus,
	
	'BT_Source_Enable': handleBTSourceEnable,
	'BT_Source_Auto_Reconnect': handleBTSourceAutoReconnect,
	'BT_Source_Connection_Status': handleBTSourceConnectionStatus,
	'BT_Source_Reset': handleBTSourceReset,
	'BT_Source_Target_Devices': handleBTSourceTargetDevices,
	'BT_Source_Target_Device': handleBTSourceTargetDevice,
};


function onMessage(event)
{
	console.log(event.data);
	var myObj = JSON.parse(event.data);
	var keys = Object.keys(myObj);
	console.log(event.data);
	for (var i = 0; i < keys.length; ++i)
	{
		var id = myObj[keys[i]]['Id'];
		var value = myObj[keys[i]]['Value'];
		console.log(id);
		console.log(value);
		const messageHandler = messageHandlers[id];
		if (messageHandler) 
		{
			messageHandler(id, value);
		}
		else 
		{
		  console.log('No handler found for message type:', id);
		}
	}
}

const widgetToSignal = {
	'Amplitude_Gain_Slider1': 'Amplitude_Gain',
	'Amplitude_Gain_Slider2': 'Amplitude_Gain',
	'Amplitude_Gain_Slider3': 'Amplitude_Gain',
	'Amplitude_Gain_Slider4': 'Amplitude_Gain',
	'FFT_Gain_Slider1': 'FFT_Gain',
	'FFT_Gain_Slider2': 'FFT_Gain',
	'FFT_Gain_Slider3': 'FFT_Gain',
	'FFT_Gain_Slider4': 'FFT_Gain',
};

const classToSignal = {
	'selection_tab_content_input_source': 'Sound_Input_Source',
	'selection_tab_content_output_source': 'Sound_Output_Source',
};

const contentIdToValue = {
	'Sound_Input_Selection_OFF': '0',
	'Sound_Input_Selection_Microphone': '1',
	'Sound_Input_Selection_Bluetooth': '2',
	'Sound_Output_Selection_OFF': '0',
	'Sound_Output_Selection_Bluetooth': '1',
};

function handleBTSourceTargetDevice(id, value)
{
	if(id && value)
	{
		console.log('Received BT Source Target Device!');
		console.log(value);
		const widgets = 
		[
		  'Source_Name_Text_Box',
		];
		try 
		{
			var innerData = JSON.parse(value);
			var innerKeys = Object.keys(innerData);

			for (var j = 0; j < innerKeys.length; ++j) 
			{
				var address = innerData[innerKeys[j]]['ADDRESS'];
				var name = innerData[innerKeys[j]]['NAME'];
				if(address & name)
				{
					for(widget in  widgets)
					{
						if(document.getElementById(widget))
						{
							document.getElementById(widget).value = name;
						}
					}
				}
			}
		}
		catch (error)
		{
			console.error('Error parsing JSON in handleBTSourceTargetDevice:', error);
		}
	}
}

function handleBTSourceTargetDevices(id, value)
{
	if(value)
	{
		console.log('Received BT Source Target Devices!');
		try 
		{
			var sourceTargetData = JSON.parse(value);
			var sourceTargetKeys = Object.keys(sourceTargetData);
			compatibleDevices.length = 0;

			var innerData = JSON.parse(value);
			var innerKeys = Object.keys(innerData);

			for (var j = 0; j < innerKeys.length; ++j)
			{
				var address = innerData[innerKeys[j]]['ADDRESS'];
				var name = innerData[innerKeys[j]]['NAME'];
				var rssi = innerData[innerKeys[j]]['RSSI'];
				var newValue = { name: name, address: address, rssi: rssi };
				compatibleDevices.push(newValue);
			}
			updateCompatibleDeviceList();
		} 
		catch (error)
		{
			console.error('Error parsing JSON in handleBTSourceTargetDevices:', error);
		}
	}
	else
	{
		compatibleDevices.length = 0;
		updateCompatibleDeviceList();
	}
}

function updateCompatibleDeviceList() {
	const deviceListElement = document.getElementById("compatibleDeviceList");
	deviceListElement.innerHTML = ""; // Clear previous entries

	// Iterate through the Bluetooth data and create list items
	compatibleDevices.forEach((device, index) => 
	{
		const listItem = document.createElement("li");
		listItem.className = "deviceItem";
		if (index === selectedDeviceIndex)
		{
			listItem.classList.add("selected");
		}
		listItem.innerHTML = `<strong>Name:</strong> ${device.name}<br>
							 <strong>Address:</strong> ${device.address}<br>
							 <strong>RSSI:</strong> ${device.rssi}`;
		deviceListElement.appendChild(listItem);
		
		// Attach a click listener to the list item
        listItem.addEventListener("click", () => {
            handleDeviceItemClick(device);
        });
		
	});
}



// Function to handle the click event on a device list item
function handleDeviceItemClick(device) {
    // Do something with the selected device's name and address
    console.log("Selected Name:", device.name);
    console.log("Selected Address:", device.address);

    var Root = {};
	var TextboxElement;
	Root.JSONValue = {};
	Root.JSONValue.Id = 'BT_Source_Target_Device';
	Root.JSONValue.Value = {};
	Root.JSONValue.Value.Address = device.address;
	Root.JSONValue.Value.Name = device.name;
	var Message = JSON.stringify(Root);
	websocket.send(Message);
}

function handleSoundInputSource(id, value) {
	if(id && value)
	{
		console.log('Received Sound Input Source!');
		switch(parseInt(value))
		{
			case 0:
				showContent('selection_tab_content_input_source', 'Sound_Input_Selection_OFF');
			break;
			case 1:
				showContent('selection_tab_content_input_source', 'Sound_Input_Selection_Microphone');
			break;
			case 2:
				showContent('selection_tab_content_input_source', 'Sound_Input_Selection_Bluetooth');
			break;
			default:
				console.log('Undefined Input Source State!');
			break;
		}
	}
}

function handleSoundOutputSource(id, value) {
	if(id && value)
	{
		console.log('Received Sound Output Source!');
		switch(parseInt(value))
		{
			case 0:
				showContent('selection_tab_content_output_source', 'Sound_Output_Selection_OFF');
			break;
			case 1:
				showContent('selection_tab_content_output_source', 'Sound_Output_Selection_Bluetooth');
			break;
			default:
				console.log('Undefined Input Source State!');
			break;
		}
	}
}

function handleSpeakerImage(id, value) {
	if(id && value)
	{
		console.log('Received Speaker Image!');
		setSpeakerImage(value);
	}
}

function handleAmplitudeGain(id, value) {
	if(id && value)
	{
		console.log('Received Amplitude Gain!');
		const widgets = 
		[
		  'Amplitude_Gain_Slider1',
		  'Amplitude_Gain_Slider2',
		  'Amplitude_Gain_Slider3',
		];
		for (const widget of widgets) 
		{
			if(false == sliderTouched && document.getElementById(widget))
			{
				document.getElementById(widget).value = value;
			}
			if(document.getElementById(widget + '_Value'))
			{
				document.getElementById(widget + '_Value').innerHTML = value;
			}
		}
	}
}

function handleFFTGain(id, value) {
	if(id && value)
	{
		console.log('Received FFT Gain!');
		const widgets = 
		[
			'FFT_Gain_Slider1',
			'FFT_Gain_Slider2',
			'FFT_Gain_Slider3',
			'FFT_Gain_Slider4',
		];
		for (const widget of widgets) 
		{
			if(false == sliderTouched && document.getElementById(widget))
			{
			document.getElementById(widget).value = value;
			}
			if(document.getElementById(widget + '_Value'))
			{
			document.getElementById(widget + '_Value').innerHTML = value;
			}
		}
	}
}

function handleBTSinkName(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Sink Name!');
		const widgets = 
		[
			'Sink_Name_Text_Box',
		];
		if(!sink_Name_Value_Changed)
		{
			for(widget in  widgets)
			{
				if(document.getElementById(widget))
				{
					document.getElementById(widget).value = value;
				}
			}
		}
	}
}
	
function handleBTSinkEnable(id, value) {
	if(id && value)
	{
		console.log('Received the Bluetooth Sink Enable!');
		if(value == 'true')
		{
			sink_BT_Enable_Toggle_Button.checked = true;
		}
		else
		{
			sink_BT_Enable_Toggle_Button.checked = false;
		}
	}
}
	
function handleBTSinkConnectionStatus(id, value) {
	if(id && value)
	{
		console.log('Received the Bluetooth Sink Connection Status!');
		var element = document.getElementById('Sink_Connection_Status');
		switch(parseInt(value))
		{
			case ConnectionStatus.Waiting:
				element.innerHTML = 'Waiting';
			break;
			case ConnectionStatus.Searching:
				element.innerHTML = 'Searching';
			break;
			case ConnectionStatus.Pairing:
				element.innerHTML = 'Pairing';
			break;
			case ConnectionStatus.Paired:
				element.innerHTML = 'Paired';
			break;
			case ConnectionStatus.Disconnected:
				element.innerHTML = 'Disconnected';
			break;
			default:
				element.innerHTML = 'Error';
			break;
		}
	}
}
	
function handleBTSinkAutoReConnect(id, value) {
	if(id && value)
	{
		console.log('Received the Bluetooth Sink Auto ReConnect!');
		if(value == 'true')
		{
			sink_BT_Auto_ReConnect_Toggle_Button.checked = true;
		}
		else
		{
			sink_BT_Auto_ReConnect_Toggle_Button.checked = false;
		}
	}
}


function handleBTSourceEnable(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Source Enable!');
	}
}

function handleBTSourceAutoReconnect(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Source Auto Reconnect!');
		if(value == 'true')
		{
			source_BT_Auto_ReConnect_Toggle_Button.checked = true;
		}
		else
		{
			source_BT_Auto_ReConnect_Toggle_Button.checked = false;
		}
	}
}

function handleBTSourceConnectionStatus(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Source Connection Status!');
		var element = document.getElementById('Source_Connection_Status_Textbox');
		switch(parseInt(value))
		{
			case ConnectionStatus.Waiting:
				element.innerHTML = 'Waiting';
			break;
			case ConnectionStatus.Searching:
				element.innerHTML = 'Searching';
			break;
			case ConnectionStatus.Pairing:
				element.innerHTML = 'Pairing';
			break;
			case ConnectionStatus.Paired:
				element.innerHTML = 'Paired';
			break;
			case ConnectionStatus.Disconnected:
				element.innerHTML = 'Disconnected';
			break;
			default:
				element.innerHTML = 'Error';
			break;
		}
	}
}

function handleBTSourceReset(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Source Reset!');
		if(value == 'true')
		{
			source_BT_Reset_Toggle_Button.checked = true;
		}
		else
		{
			source_BT_Reset_Toggle_Button.checked = false;
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
	
	var signal = classToSignal[classId.toString()];
	var value = contentIdToValue[contentId.toString()];
	if(signal && value)
	{
		var Root = {};
		Root.WidgetValue = {};
		Root.WidgetValue.Id = signal.toString();
		Root.WidgetValue.Value = value.toString();
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	}
}