export class WebSocketManager {
    constructor() {
        this.websocket = null;
        this.webSocketPort = 81;
        this.gateway = `ws://${window.location.hostname}:${this.webSocketPort}/ws`;
        this.listeners = [];
    }

    registerListener(listener) {
        if (listener) {
            if ( typeof listener.onMessage === 'function' && 
                 typeof listener.onOpen === 'function' && 
                 typeof listener.onClose === 'function' && 
                 typeof listener.onError === 'function' && 
                 typeof listener.getListnerName === 'function' ) 
            {
                this.listeners.push(listener);
            }
            else 
            {
                console.warn('ESP32 Web Socket: Listener does not have required methods');
            }
        } 
        else
        {
            console.warn('ESP32 Web Socket: null Listener');
        }
    }

    unregisterListener(listener) {
        this.listeners = this.listeners.filter(l => l !== listener);
    }

    initWebSocket() {
        console.log('ESP32 Web Socket: Trying to open a WebSocket connection…');
        this.websocket = new WebSocket(this.gateway);
        this.websocket.onopen = this.onOpenHandler.bind(this);
        this.websocket.onclose = this.onCloseHandler.bind(this);
        this.websocket.onmessage = this.onMessageHandler.bind(this);
        this.websocket.onerror = this.onErrorHandler.bind(this);
    }

    announceHere() {
        this.websocket.send('New client is here!');
    }

    send(message) {
        if (this.websocket) {
            if (this.websocket.OPEN) {
                this.websocket.send(message);
            } else {
                console.error('ESP32 Web Socket: Web_Socket Closed!');
            }
        } else {
            console.error('ESP32 Web Socket: Null Web_Socket!');
        }
    }

    onMessageHandler(event) {
        if (event.data instanceof Blob) {
            this.blobToString(event.data)
                .then(strMessage => {
                    console.log(`ESP32 Web Socket: Received binary data converted to string: "${strMessage}"`);
                    this.processMessage(strMessage);
                })
                .catch(error => {
                    console.error('ESP32 Web Socket: Error converting blob to string:', error);
                });
        } else if (typeof event.data === 'string') {
            console.log(`ESP32 Web Socket: Received message: "${event.data}"`);
            this.processMessage(event.data);
        } else {
            console.error('ESP32 Web Socket: Received unsupported data type:', typeof event.data);
        }
    }

    blobToString(blob) {
        return new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = function(event) {
                resolve(event.target.result); // The result is the string content of the blob
            };
            reader.onerror = function(event) {
                reject(new Error("Failed to read blob: " + event.target.error));
            };
            reader.readAsText(blob); // Read the blob as text
        });
    }
    
    processMessage(message) {
        try {
            const myObj = JSON.parse(message);
            const keys = Object.keys(myObj);
            keys.forEach(key => {
                const { Id, Value } = myObj[key];
                console.log(`ESP32 Web Socket: Parsed Rx: ID:"${Id}" Value:"${Value}"`);
                let found = false;
                this.listeners.forEach(listener => {
                    if (typeof listener.onMessage === 'function') {
                        if (typeof listener.getListnerName === 'function') {
                            if (Id == listener.getListnerName()) {
                                found = true;
                                console.log(`ESP32 Web Socket: Found Listener Rx: ID:"${Id}" Value:"${Value}"`);
                                listener.onMessage(Value);
                            }
                        }
                    }
                });
                if (!found) {
                    console.warn(`ESP32 Web Socket: No Listener for Id: "${Id}" Value: "${Value}"`);
                }
            });
        } catch (error) {
            console.error('ESP32 Web Socket: Error parsing message:', error);
        }
    }

    onOpenHandler(event) {
        console.log('ESP32 Web Socket: Connection opened');
        this.announceHere();
        this.listeners.forEach(listener => {
            listener.onOpen(event);
        });
    }

    onCloseHandler(event) {
        console.log('ESP32 Web Socket: Connection closed');
        setTimeout(() => this.initWebSocket(), 5000);
        this.listeners.forEach(listener => {
            listener.onClose(event);
        });
    }

    onErrorHandler(event) {
        console.error('ESP32 Web Socket: Connection Error:', event);
        this.websocket.close();
        setTimeout(() => this.initWebSocket(), 5000);
        this.listeners.forEach(listener => {
            listener.onError(event);
        });
    }
}
