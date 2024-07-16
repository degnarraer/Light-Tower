import { WebSocketManager } from './WebSocket.js';
import { SoundInputSource_Signal } from './SoundInputSource.js';
import { SoundOutputSource_Signal } from './SoundOutputSource.js';
import { Boolean_Signal } from './Boolean.js';

const wsManager = new WebSocketManager();

var speakerImages = new Array();
var sliderTouched = false;
var sliderTimeoutHandle;
var sink_Name_Value_Changed = false;
var sink_Name_Changed_TimeoutHandle;
var source_Name_Value_Changed = false;
var source_Name_Changed_TimeoutHandle;

var CurrentSoundInputSource = new SoundInputSource_Signal("Input_Source", SoundInputSource_Signal.values.OFF, wsManager);
window.CurrentSoundInputSource = CurrentSoundInputSource;

var CurrentSoundOutputSource = new SoundOutputSource_Signal("Output_Source", SoundOutputSource_Signal.values.OFF, wsManager);
window.CurrentSoundOutputSource = CurrentSoundOutputSource;

var BT_SinkEnable = new Boolean_Signal("BT_Sink_Enable", Boolean_Signal.values.False, wsManager);
var BT_SourceEnable = new Boolean_Signal("BT_Source_Enable", Boolean_Signal.values.False, wsManager);
var Sink_Connect = new Boolean_Signal("Sink_Connect", Boolean_Signal.values.False, wsManager);
var Sink_Disconnect = new Boolean_Signal("Sink_Disconnect", Boolean_Signal.values.False, wsManager);
var Sink_Auto_Reconnect = new Boolean_Signal("BT_Sink_Auto_Reconnect", Boolean_Signal.values.False, wsManager);
var Source_Connect = new Boolean_Signal("Source_Connect", Boolean_Signal.values.False, wsManager);
var Source_Disconnect = new Boolean_Signal("Source_Disconnect", Boolean_Signal.values.False, wsManager);
var Source_Auto_Reconnect = new Boolean_Signal("BT_Source_Auto_Reconnect", Boolean_Signal.values.False, wsManager);
var Source_Reset = new Boolean_Signal("BT_Source_Reset", Boolean_Signal.values.False, wsManager);

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

window.showContent = showContent;
function showContent(classId, contentId) {
	// Hide all tab contents
	var tabContents = document.querySelectorAll('.' + classId);
	tabContents.forEach(function (tabContent) {
		tabContent.classList.remove('active');
	});
	var heading = document.getElementById("mainMenu_Heading");
	heading.innerText = contentId;
	
	// Show the selected tab content
	document.getElementById(contentId).classList.add('active');
}

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
	wsManager.initWebSocket();
	var sink_BT_Auto_ReConnect_Toggle_Button;
	sink_BT_Auto_ReConnect_Toggle_Button = document.getElementById('Sink_BT_Auto_ReConnect_Toggle_Button');
	sink_BT_Auto_ReConnect_Toggle_Button.addEventListener('change', function()
	{
		Sink_Auto_Reconnect.setValue(sink_BT_Auto_ReConnect_Toggle_Button.checked? "1" : "0");
	});

	var source_BT_Reset_Toggle_Button;
	source_BT_Reset_Toggle_Button = document.getElementById('Source_BT_Reset_Toggle_Button');
	source_BT_Reset_Toggle_Button.addEventListener('change', function()
	{
		Source_Reset.setValue(source_BT_Reset_Toggle_Button.checked? "1" : "0");
	});

	var source_BT_Auto_ReConnect_Toggle_Button;
	source_BT_Auto_ReConnect_Toggle_Button = document.getElementById('Source_BT_Auto_ReConnect_Toggle_Button');
	Source_BT_Auto_ReConnect_Toggle_Button.addEventListener('change', function()
	{
		Source_Auto_Reconnect.setValue(source_BT_Auto_ReConnect_Toggle_Button.checked? "1" : "0");
	});
}

function handleSoundInputSource(id, value) {
	if(id && value){
		console.log('Received Sound Input Source! ID:' + id + ' Value: ' + value );
		CurrentSoundInputSource.FromString(value);
		switch (CurrentSoundInputSource.GetSource()) {
            case SoundInputSource.OFF:
                showContent('selection_tab_content_input_source', 'Sound_Input_Selection_OFF');
				Update_Signal_Value_To_Web_Socket(id, value);
                break;
            case SoundInputSource.Microphone:
                showContent('selection_tab_content_input_source', 'Sound_Input_Selection_Microphone');
				Update_Signal_Value_To_Web_Socket(id, value);
                break;
            case SoundInputSource.Bluetooth:
                showContent('selection_tab_content_input_source', 'Sound_Input_Selection_Bluetooth');
				Update_Signal_Value_To_Web_Socket(id, value);
                break;
            default:
                console.log('Undefined Input Source!');
                break;
        }
	}
}

function handleSoundOutputSource(id, value) {
    if (id && value) {
        console.log('Received Sound Output Source! ID: ' + id + ' Value: ' + value);
		CurrentSoundOutputSource.FromString(value);
		switch (CurrentSoundOutputSource.GetSource()) {
            case SoundOutputSource.OFF:
                showContent('selection_tab_content_output_source', 'Sound_Output_Selection_OFF');
				Update_Signal_Value_To_Web_Socket(id, value);
                break;
            case SoundOutputSource.Bluetooth:
                showContent('selection_tab_content_output_source', 'Sound_Output_Selection_Bluetooth');
				Update_Signal_Value_To_Web_Socket(id, value);
                break;
            default:
                console.log('Undefined Output Source!');
                break;
        }
    }
}

window.openNav = openNav;
function openNav() 
{
  document.getElementById('leftSideNavigationMenu').style.width = '200px';
}

window.closeNav = closeNav;
function closeNav()
{
  document.getElementById('leftSideNavigationMenu').style.width = '0';
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

function submit_New_Value_From_TextBox(element)
{
	clearTimeout(sink_Name_Changed_TimeoutHandle);
	var TextboxId = element.getAttribute("for");
	var TextboxElement = document.getElementById(TextboxId);
	var Root = {};
	Root.SignalValue = {};
	Root.SignalValue.Id = TextboxElement.getAttribute("data-Signal");
	Root.SignalValue.Value = TextboxElement.value;
	console.log('Submit New Name: \"' + TextboxElement.value + '\" Signal: \"' + TextboxElement.getAttribute("data-Signal") + '\"');
	wsManager.send(JSON.stringify(Root));
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
		Root.SignalValue.Value = isPressed ? "1" : "0";
		wsManager.send(JSON.stringify(Root));
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
		Root.SignalValue.Value = isPressed ? "1" : "0";
		var Message = JSON.stringify(Root);
		wsManager.send(Message);
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
		Root.SignalValue.Value = isPressed ? "1" : "0";
		var Message = JSON.stringify(Root);
		wsManager.send(Message);
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
		Root.SignalValue.Value = isPressed ? "1" : "0";
		var Message = JSON.stringify(Root);
		wsManager.send(Message);
	}
}

// slider Functions
window.updatesliderValue = updatesliderValue;
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
		wsManager.send(JSON.stringify(Root));
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
	wsManager.send(Message);
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

function handleSSID(id, value){
	if(id && value)
	{
		console.log('Received SSID!');
		var elementsWithDataValue = document.querySelectorAll('[data-Signal="SSID"]');
		elementsWithDataValue.forEach(function(element){
			if (element.hasAttribute("value")) {
				element.value = value;
			} else if (element.childNodes.length > 0) {
				element.innerHTML = value;
			} else {
				console.log('handleBTSinkName Unsupported Element: ' + element.id);
			}
		});
	}
}

function handlePassword(id, value){
	if(id && value)
	{
		console.log('Received Password!');
		var elementsWithDataValue = document.querySelectorAll('[data-Signal="Password"]');
		elementsWithDataValue.forEach(function(element){
			if (element.hasAttribute("value")) {
				element.value = value;
			} else if (element.childNodes.length > 0) {
				element.innerHTML = value;
			} else {
				console.log('handleBTSinkName Unsupported Element: ' + element.id);
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
			if (element.hasAttribute("value")) {
				element.value = value;
			} else if (element.childNodes.length > 0) {
				element.innerHTML = value;
			} else {
				console.log('handleBTSinkName Unsupported Element: ' + element.id);
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
