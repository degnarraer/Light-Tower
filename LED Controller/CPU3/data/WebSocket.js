export class WebSocketManager {
    constructor() {
        this.websocket = null;
        this.gateway = `ws://${window.location.hostname}/ws`;
        this.listeners = []; // Array to hold registered listeners
    }

    registerListener(listener) {
        if (listener){
            if(typeof listener.onMessage === 'function'){
                if(typeof listener.getSignalName === 'function') {
                    this.listeners.push(listener);
                } else {
                    console.warn('Listener does not have getName method');
                }
            } else {
                console.warn('Listener does not have onMessage method');
            }
        } else {
            console.warn('null Listener');
        }
    }

    unregisterListener(listener) {
        this.listeners = this.listeners.filter(l => l !== listener);
    }

    initWebSocket() {
        console.log('Trying to open a WebSocket connection…');
        try {
            this.websocket = new WebSocket(this.gateway);
            this.websocket.onopen = this.onOpen.bind(this);
            this.websocket.onclose = this.onClose.bind(this);
            this.websocket.onmessage = this.onMessage.bind(this);
            this.websocket.onerror = this.onError.bind(this);
        } catch (error) {
            console.error('WebSocket initialization error:', error.message);
            setTimeout(() => this.initWebSocket(), 5000);
        }
    }

    Send_Signal_Value_To_Web_Socket(signal, value)
    {
        if(signal && value) {
            console.log('Updating Signal: \'' + signal + '\' to value: \'' + value + '\'');
            var Root = {};
            Root.SignalValue = {};
            Root.SignalValue.Id = signal.toString();
            Root.SignalValue.Value = value.toString();
            var Message = JSON.stringify(Root);
            wsManager.send(Message);
        } else {
            console.error('Invalid Call to Update_Signal_Value_To_Web_Socket!');
        }
    }

    onMessage(event) {
        console.log('Message received:', event.data);
        try {
            const myObj = JSON.parse(event.data);
            const keys = Object.keys(myObj);
            keys.forEach(key => {
                const { Id, Value } = myObj[key];
                var found = false;
                this.listeners.forEach(listener => {
                    if(typeof listener.onMessage === 'function'){
                        if(typeof listener.getSignalName === 'function') {
                            if(Id == listener.getSignalName())
                            {
                                found = true;
                                listener.onMessage(Value);
                            }
                        }
                    }
                });
                if(!found){
                    console.warn("No Listener for Id: \"" + Id + "\" Value: \"" + Value + "\"");
                }
            });
        } catch (error) {
            console.error('Error parsing message:', error);
        }
    }

    send(message){
        this.websocket.send(message);
    }

    onOpen(event) {
        console.log('Connection opened');
        this.websocket.send('Hello I am here!');
    }

    onClose(event) {
        console.log('Connection closed');
        setTimeout(() => this.initWebSocket(), 5000);
    }

    onError(event) {
        console.error('Connection Error:', event);
        setTimeout(() => this.initWebSocket(), 5000);
    }

}