export class WebSocketManager {
    constructor() {
        this.websocket = null;
        this.webSocketPort = 81;
        this.gateway = `ws://${window.location.hostname}:${this.webSocketPort}/ws`;
        this.listeners = [];
    }

    registerListener(listener) {
        if (listener) {
            if ( typeof listener.onmessage === 'function' && 
                 typeof listener.onopen === 'function' && 
                 typeof listener.onclose === 'function' && 
                 typeof listener.onerror === 'function' && 
                 typeof listener.getListnerName === 'function' ) 
            {
                this.listeners.push(listener);
            }
            else 
            {
                console.warn('Listener does not have required methods');
            }
        } 
        else
        {
            console.warn('null Listener');
        }
    }

    unregisterListener(listener) {
        this.listeners = this.listeners.filter(l => l !== listener);
    }

    initWebSocket() {
        console.log('Trying to open a WebSocket connection…');
        this.websocket = new WebSocket(this.gateway);
        this.websocket.onopen = this.onOpenHandler.bind(this);
        this.websocket.onclose = this.onCloseHandler.bind(this);
        this.websocket.onmessage = this.onMessageHandler.bind(this);
        this.websocket.onerror = this.onErrorHandler.bind(this);
    }

    announceHere() {
        this.websocket.send('Hello I am here!');
    }

    send(message) {
        if (this.websocket) {
            this.websocket.send(message);
        } else {
            console.error('Null Web_Socket!');
        }
    }

    onMessageHandler(event) {
        console.log(`Web Socket Rx: "${event.data}"`);
        try {
            const myObj = JSON.parse(event.data);
            const keys = Object.keys(myObj);
            keys.forEach(key => {
                const { Id, Value } = myObj[key];
                console.log(`Parsed Rx: ID:"${Id}" Value:"${Value}"`);
                var found = false;
                this.listeners.forEach(listener => {
                    if (typeof listener.onMessage === 'function') {
                        if (typeof listener.getListnerName === 'function') {
                            if (Id == listener.getListnerName()) {
                                found = true;
                                console.log(`Found Listener Rx: ID:"${Id}" Value:"${Value}"`);
                                listener.onMessage(Value);
                            }
                        }
                    }
                });
                if (!found) {
                    console.warn("No Listener for Id: \"" + Id + "\" Value: \"" + Value + "\"");
                }
            });
        } catch (error) {
            console.error('Error parsing message:', error);
        }
    }

    onOpenHandler(event) {
        console.log('Connection opened');
        this.listeners.forEach(listener => {
            listener.onopen(event);
        });
    }

    onCloseHandler(event) {
        console.log('Connection closed');
        setTimeout(() => this.initWebSocket(), 5000);
        this.listeners.forEach(listener => {
            listener.onclose(event);
        });
    }

    onErrorHandler(event) {
        console.error('Connection Error:', event);
        this.websocket.close();
        setTimeout(() => this.initWebSocket(), 5000);
        this.listeners.forEach(listener => {
            listener.onerror(event);
        });
    }
}
