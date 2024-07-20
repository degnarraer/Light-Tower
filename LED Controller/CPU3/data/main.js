import { WebSocketManager } from './WebSocket.js';
import { SoundInputSource_Signal } from './SoundInputSource.js';
import { SoundOutputSource_Signal } from './SoundOutputSource.js';
import { Boolean_Signal } from './Boolean.js';
import { NumericalValue_Signal } from './NumericalValue.js';
import { StringValue_Signal } from './StringValue.js';

const wsManager = new WebSocketManager();

var speakerImages = new Array();
var sliderTouched = false;
var sliderTimeoutHandle;
var sink_Name_Value_Changed = false;
var sink_Name_Changed_TimeoutHandle;
var source_Name_Value_Changed = false;
var source_Name_Changed_TimeoutHandle;

export const CurrentSoundInputSource = new SoundInputSource_Signal('Input_Source', SoundInputSource_Signal.values.OFF, wsManager);
export const CurrentSoundOutputSource = new SoundOutputSource_Signal('Output_Source', SoundOutputSource_Signal.values.OFF, wsManager);
export const Source_Connect = new Boolean_Signal('Src_Connect', Boolean_Signal.values.False, wsManager);
export const Source_Disconnect = new Boolean_Signal('Src_Disconnect', Boolean_Signal.values.False, wsManager);
export const Sink_Connect = new Boolean_Signal('Sink_Connect', Boolean_Signal.values.False, wsManager);
export const Sink_Disconnect = new Boolean_Signal('Sink_Disconnect', Boolean_Signal.values.False, wsManager);
export const Source_Reset = new Boolean_Signal('BT_Src_Reset', Boolean_Signal.values.False, wsManager);
export const Amplitude_Gain = new NumericalValue_Signal('Amp_Gain', 2.0, wsManager);
export const FFT_Gain = new NumericalValue_Signal('FFT_Gain', 2.0, wsManager);
export const BT_SinkEnable = new Boolean_Signal('BT_Sink_En', Boolean_Signal.values.False, wsManager);
export const BT_SourceEnable = new Boolean_Signal('BT_Source_En', Boolean_Signal.values.False, wsManager);
export const Sink_Auto_Reconnect = new Boolean_Signal('BT_Sink_AR', Boolean_Signal.values.False, wsManager);
export const Source_Auto_Reconnect = new Boolean_Signal('BT_Source_AR', Boolean_Signal.values.False, wsManager);
export const SSID = new StringValue_Signal('SSID', 'LED Tower of Power', wsManager);
export const Password = new StringValue_Signal('Password', 'LEDs Rock', wsManager);
export const SinkName = new StringValue_Signal('Sink_Name', 'LED Tower of Power Rock', wsManager);
export const SourceName = new StringValue_Signal('Source_Name', '', wsManager);

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

export function showContent(classId, contentId) {
    // Select all elements with the given classId
    var tabContents = document.querySelectorAll('.' + classId);
    
    // Hide all tab contents
    tabContents.forEach(function (tabContent) {
        tabContent.classList.remove('active');
    });
    
    // Update the main menu heading
    var heading = document.getElementById("mainMenu_Heading");
    if (heading) {
        heading.innerText = contentId;
    } else {
        console.warn("Element with ID 'mainMenu_Heading' not found.");
    }
    
    // Show the selected tab content
    var selectedContent = document.getElementById(contentId);
    if (selectedContent) {
        selectedContent.classList.add('active');
    } else {
        console.warn("Element with ID '" + contentId + "' not found.");
    }
}

export function openNav() 
{
  document.getElementById('leftSideNavigationMenu').style.width = '200px';
}

export function closeNav()
{
  document.getElementById('leftSideNavigationMenu').style.width = '0';
}

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
