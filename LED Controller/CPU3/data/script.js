var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var speakerImages = new Array();
var sliderTouched = false;
var sliderTimeoutHandle;
var sink_Name_Value_Changed = false;
var sink_Name_Changed_TimeoutHandle;
var source_Name_Value_Changed = false;
var source_Name_Changed_TimeoutHandle;

//Toggle Buttons
var source_BT_Reset_Toggle_Button;
var source_BT_Reset_Toggle_Button;
var source_BT_Auto_ReConnect_Toggle_Button;

//Compatible Devices
var compatibleDevices = [
	{ name: "Device 1", address: "00:11:22:33:44:55", rssi: -50 },
	{ name: "Device 2", address: "AA:BB:CC:DD:EE:FF", rssi: -60 },
];

let selectedDeviceIndex = -1;

const ConnectionState = 
{
	Disconnected: 0,
	Connecting: 1,
	Connected: 2,
	Disconnecting: 3
}
const ConnectionStateString = 
{
	0: 'Disconnected',
	1: 'Connecting',
	2: 'Connected',
	3: 'Disconnecting'
}

const messageHandlers = {
	'Amplitude_Gain': handleAmplitudeGain,
	'FFT_Gain': handleFFTGain,
	'Sound_Input_Source': handleSoundInputSource,
	'Sound_Output_Source': handleSoundOutputSource,
	'BT_Sink_Enable' : handleStubFunction,
	'Sink_Name': handleBTSinkName,
	'Source_Name' : handleStubFunction,
	'BT_Sink_Connection_State': handleBTSinkConnectionState,
	'BT_Sink_Auto_ReConnect': handleBTSinkAutoReConnect,
	'BT_Source_Enable' : handleStubFunction,
	'BT_Source_Target_Device': handleBTSourceTargetDevice,
	'Sink_Connect' : handleStubFunction,
	'Sink_Disconnect' : handleStubFunction,
	'Source_Connect' : handleStubFunction,
	'Source_Disconnect' : handleStubFunction,
	'BT_Source_Target_Devices': handleBTSourceTargetDevices,
	'BT_Source_Auto_Reconnect': handleBTSourceAutoReconnect,
	'BT_Source_Connection_State': handleBTSourceConnectionState,
	'BT_Source_Reset': handleBTSourceReset,
	'Speaker_Image': handleSpeakerImage,
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

//Window and Web Socket Functions
window.addEventListener('load', onload);
function onload(event)
{
    initWebSocket();
	// Toggle Switch Handlers
	sink_BT_Auto_ReConnect_Toggle_Button = document.getElementById('Sink_BT_Auto_ReConnect_Toggle_Button');
	sink_BT_Auto_ReConnect_Toggle_Button.addEventListener('change', function()
	{
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = 'BT_Sink_Auto_ReConnect';
		Root.SignalValue.Value = String(sink_BT_Auto_ReConnect_Toggle_Button.checked);
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	});

	source_BT_Reset_Toggle_Button = document.getElementById('Source_BT_Reset_Toggle_Button');
	source_BT_Reset_Toggle_Button.addEventListener('change', function()
	{
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = 'BT_Source_Reset';
		Root.SignalValue.Value = String(source_BT_Reset_Toggle_Button.checked);
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	});

	source_BT_Auto_ReConnect_Toggle_Button = document.getElementById('Source_BT_Auto_ReConnect_Toggle_Button');
	Source_BT_Auto_ReConnect_Toggle_Button.addEventListener('change', function()
	{
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = 'BT_Source_Auto_Reconnect';
		Root.SignalValue.Value = String(source_BT_Auto_ReConnect_Toggle_Button.checked);
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	});
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
	clearTimeout(sink_Name_Changed_TimeoutHandle);
	var TextboxId = element.getAttribute("for");
	var TextboxElement = document.getElementById(TextboxId);
	var Root = {};
	Root.SignalValue = {};
	Root.SignalValue.Id = TextboxElement.getAttribute("data-Signal");
	Root.SignalValue.Value = TextboxElement.value;
	console.log('Submit New Name: \"' + TextboxElement.value + '\" Signal: \"' + TextboxElement.getAttribute("data-Signal") + '\"');
	websocket.send(JSON.stringify(Root));
	sink_Name_Changed_TimeoutHandle = setTimeout(Sink_Name_Changed_Timeout, 5000);
}

function sink_Connect(element, isPressed)
{
	var ButtonId = element.id;
    if(ButtonId == 'Sink_Connect_Button')
	{
		if(isPressed)console.log('Sink Connect Button Pressed:');
		if(!isPressed)console.log('Sink Connect Button Released:');
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = element.getAttribute("data-Signal");
		Root.SignalValue.Value = isPressed.toString();
		websocket.send(JSON.stringify(Root));
	}
}

function sink_Disconnect(element, isPressed)
{
	var ButtonId = element.id;
    if(ButtonId == 'Sink_Disconnect_Button')
	{
		if(isPressed)console.log('Sink Disconnect Button Pressed:');
		if(!isPressed)console.log('Sink Disconnect Button Released:');
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = element.getAttribute("data-Signal");
		Root.SignalValue.Value = isPressed.toString();
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	}
}

function source_Connect(element, isPressed)
{
	var ButtonId = element.id;
    if(ButtonId == 'Source_Connect_Button')
	{
		if(isPressed)console.log('Source Connect Button Pressed:');
		if(!isPressed)console.log('Source Connect Button Released:');
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = element.getAttribute("data-Signal");
		Root.SignalValue.Value = isPressed.toString();
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	}
}

function source_Disconnect(element, isPressed)
{
	var ButtonId = element.id;
    if(ButtonId == 'Source_Disconnect_Button')
	{
		if(isPressed)console.log('Source Disconnect Button Pressed:');
		if(!isPressed)console.log('Source Disconnect Button Released:');
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = element.getAttribute("data-Signal");
		Root.SignalValue.Value = isPressed.toString();
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	}
}

// slider Functions
function updatesliderValue(element)
{
	clearTimeout(sliderTimeoutHandle);
    sliderTouched = true;
    var Root = {};
	Root.SignalValue = {};
	if(element.getAttribute("data-Signal"))
	{
		Root.SignalValue.Id = element.getAttribute("data-Signal");
		Root.SignalValue.Value = element.value;
		websocket.send(JSON.stringify(Root));
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

function onMessage(event)
{
	console.log(event.data);
	var myObj = JSON.parse(event.data);
	var keys = Object.keys(myObj);
	for (var i = 0; i < keys.length; ++i)
	{
		var id = myObj[keys[i]]['Id'];
		var value = myObj[keys[i]]['Value'];
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

function handleBTSourceTargetDevice(id, value)
{
	if(id && value)
	{
		console.log('Received BT Source Target Device!' && value);
		var elementsWithDataValue = document.querySelectorAll('[data-Signal="Source_Name"]');
		elementsWithDataValue.forEach(function(element)
		{
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
						element.value = name;
					}
				}
			}
			catch (error)
			{
				console.error('Error parsing JSON in handleBTSourceTargetDevice:', error);
			}
		});
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
	var elementsWithDataValue = document.querySelectorAll('[data-Signal="BT_Source_Target_Devices"]');
	elementsWithDataValue.forEach(function(element){
		if(element.hasOwnProperty("innerHTML")){
			deviceListElement.innerHTML = ""; // Clear previous entries
			// Iterate through the Bluetooth data and create list items
			compatibleDevices.forEach((device, index) => {
				const listItem = document.createElement("li");
				listItem.className = "deviceItem";
				if (index === selectedDeviceIndex){
					listItem.classList.add("selected");
				}
				listItem.innerHTML = `<strong>Name:</strong> ${device.name}<br>
									<strong>Address:</strong> ${device.address}<br>
									<strong>RSSI:</strong> ${device.rssi}`;
				element.appendChild(listItem);
				
				// Attach a click listener to the list item
				listItem.addEventListener("click", () => {
					handleTargetDeviceItemClick(device);
				});
			});
		}
		else{
			console.log('updateCompatibleDeviceList Unsupported Element!');
		}
	});
}

// Function to handle the click event on a device list item
function handleTargetDeviceItemClick(device) {
    // Do something with the selected device's name and address
    console.log('Selected Name:', device.name);
    console.log('Selected Address:', device.address);

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
				console.log('Undefined Input Source!');
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
				console.log('Undefined Output Source!');
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

function handleStubFunction(id, value) {

}

function handleAmplitudeGain(id, value) {
    if (id && value) {
        console.log('Received Amplitude Gain:' + value);
        var elementsWithDataValue = document.querySelectorAll('[data-Signal="Amplitude_Gain"]');
        elementsWithDataValue.forEach(function (element) {
            if (element.hasAttribute("value")) {
                element.value = value;
            } else if (element.childNodes.length > 0) {
                element.innerHTML = value;
            } else {
                console.log('handleAmplitudeGain Unsupported Element: ' + element.id);
            }
        });
    }
}

function handleFFTGain(id, value) {
    if (id && value) {
        console.log('Received FFT Gain: ' + value);
        var elementsWithDataValue = document.querySelectorAll('[data-Signal="FFT_Gain"]');
        elementsWithDataValue.forEach(function (element) {
            if (element.hasAttribute("value")) {
                element.value = value;
            } else if (element.childNodes.length > 0) {
                element.innerHTML = value;
            } else {
                console.log('handleAmplitudeGain Unsupported Element: ' + element.id);
            }
        });
    }
}

function handleBTSinkName(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Sink Name!');
		var elementsWithDataValue = document.querySelectorAll('[data-Signal="Sink_Name"]');
		elementsWithDataValue.forEach(function(element){
			if(!sink_Name_Value_Changed)
			{
				if (element.hasAttribute("value")) {
					element.value = value;
				} else if (element.childNodes.length > 0) {
					element.innerHTML = value;
				} else {
					console.log('handleBTSinkName Unsupported Element: ' + element.id);
				}
			}
		});
	}
}
	
function handleBTSinkConnectionState(id, value) {
    if (id && value) {
        console.log('Received Bluetooth Source Connection State!');
        var elementsWithDataValue = document.querySelectorAll('[data-Signal="BT_Sink_Connection_State"]');
        elementsWithDataValue.forEach(function (element) {
            if (element.textContent !== undefined) {
                element.textContent = ConnectionStateString[parseInt(value)].toString();
            } else {
				console.log('handleBTSinkConnectionState Unsupported Element: ' + element.id);
			}
        });
    }
}
	
function handleBTSinkAutoReConnect(id, value) {
	if(id && value)
	{
		console.log('Received the Bluetooth Sink Auto ReConnect!');
		var elementsWithDataValue = document.querySelectorAll('[data-Signal="BT_Sink_Auto_ReConnect"]');
		elementsWithDataValue.forEach(function(element){
			if(element.tagName.toLowerCase() === "input" && element.type.toLowerCase() === "checkbox"){
				if(value == 'true'){
					element.checked = true;
				}else{
					element.checked = false;
				}
			} else {
				console.log('handleBTSinkAutoReConnect Unsupported Element!');
			}
		});
	}
}

function handleBTSourceAutoReconnect(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Source Auto Reconnect!');
		var elementsWithDataValue = document.querySelectorAll('[data-Signal="BT_Source_Auto_ReConnect"]');
		elementsWithDataValue.forEach(function(element){
			if(element.tagName.toLowerCase() === "input" && element.type.toLowerCase() === "checkbox"){
				if(value == 'true'){
					element.checked = true;
				}else{
					element.checked = false;
				}
			} else {
				console.log('handleBTSourceAutoReconnect Unsupported Element!');
			}
		});
	}
}

function handleBTSourceConnectionState(id, value) {
  if (id && value) {
    console.log('Received Bluetooth Source Connection State!');
	var elementsWithDataValue = document.querySelectorAll('[data-Signal="BT_Source_Connection_State"]');
	elementsWithDataValue.forEach(function(element){
		if(element.hasOwnProperty("textContent")){
			element.textContent = ConnectionStateString[parseInt(value)].toString();
		} else {
			console.log('handleBTSourceConnectionState Unsupported Element!');
		}
	});
  }
}

function handleBTSourceReset(id, value) {
	if(id && value)
	{
		console.log('Received Bluetooth Source Reset!');
		var elementsWithDataValue = document.querySelectorAll('[data-Signal="BT_Source_Reset"]');
		elementsWithDataValue.forEach(function(element){
			if(element.tagName.toLowerCase() === "input" && element.type.toLowerCase() === "checkbox"){
				if(value == 'true'){
					element.checked = true;
				}else{
					element.checked = false;
				}
			} else {
				console.log('handleBTSourceReset Unsupported Element!');
			}
		});
	}
}

function showContent(classId, contentId, updateWebSocket = false) {
	// Hide all tab contents
	var tabContents = document.querySelectorAll('.' + classId);
	tabContents.forEach(function (tabContent) {
		tabContent.classList.remove('active');
	});

	// Show the selected tab content
	document.getElementById(contentId).classList.add('active');
	
	var signal = classToSignal[classId.toString()];
	var value = contentIdToValue[contentId.toString()];
	if(updateWebSocket && signal && value)
	{
		var Root = {};
		Root.SignalValue = {};
		Root.SignalValue.Id = signal.toString();
		Root.SignalValue.Value = value.toString();
		var Message = JSON.stringify(Root);
		websocket.send(Message);
	}
}