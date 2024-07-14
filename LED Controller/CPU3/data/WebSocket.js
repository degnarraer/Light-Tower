export class WebSocketManager {
    constructor() {
        this.websocket = null;
        this.gateway = `ws://${window.location.hostname}/ws`;
        this.messageHandler = null;
    }

    initWebSocket() {
        console.log('Trying to open a WebSocket connection…');
        try {
            this.websocket = new WebSocket(this.gateway);
            this.websocket.onopen = this.onOpen.bind(this);
            this.websocket.onclose = this.onClose.bind(this);
            this.websocket.onmessage = this.handleMessage.bind(this);
            this.websocket.onerror = this.onError.bind(this);
        } catch (error) {
            console.error('WebSocket initialization error:', error.message);
            // Retry connection after a delay
            setTimeout(() => this.initWebSocket(), 5000);
        }
    }

    handleMessage(event) {
        console.log('Message received:', event.data);
        if (typeof this.messageHandler === 'function') {
            this.messageHandler(event);
        } else {
            console.warn('No message handler set.');
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

    // Method to set an external message handler
    onMessage(handler) {
        console.log('Handler Set:', handler);
        this.messageHandler = handler;
    }
}