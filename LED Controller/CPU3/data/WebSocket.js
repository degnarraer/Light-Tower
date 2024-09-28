export class WebSocketManager {
    constructor() {
        this.websocket = null;
        this.webSocketPort = 81;
        this.gateway = `ws://${window.location.hostname}:${this.webSocketPort}/ws`;
        this.listeners = []; // Array to hold registered listeners
        this.loadingAnimation;
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

    show_Connecting_Modal() {
        const loadingTextElement = document.querySelector('#loadingModal .modal-content h2');
        let dotCount = 0;
        loadingTextElement.textContent = 'Connecting';
        document.getElementById('loadingPage').style.display = 'flex';
        this.loadingAnimation = setInterval(() => {
            dotCount = (dotCount + 1) % 4;
            loadingTextElement.textContent = 'Connecting' + '.'.repeat(dotCount);
        }, 1000);
    }

    initWebSocket() {
        console.log('Trying to open a WebSocket connection…');
        this.show_Connecting_Modal();
        this.websocket = new WebSocket(this.gateway);
        this.websocket.onopen = this.onOpen.bind(this);
        this.websocket.onclose = this.onClose.bind(this);
        this.websocket.onmessage = this.onMessage.bind(this);
        this.websocket.onerror = this.onError.bind(this);
    }

    Send_Signal_Value_To_Web_Socket(signal, value)
    {
        if(this.websocket) {
            if(signal && value) {
            console.log('Web Socket Tx: \'' + signal + '\' Value: \'' + value + '\'');
            var Root = {};
            Root.SignalValue = {};
            Root.SignalValue.Id = signal.toString();
            Root.SignalValue.Value = value.toString();
            var Message = JSON.stringify(Root);
            this.websocket.send(Message);
            } else {
                console.error('Invalid Call to Update_Signal_Value_To_Web_Socket!');
            }
        } else {
            console.error('Null Web_Socket!');
        }
    }

    onMessage(event) {
        console.log(`Web Socket Rx: "${event.data}"`);
        try {
            const myObj = JSON.parse(event.data);
            const keys = Object.keys(myObj);
            keys.forEach(key => {
                const { Id, Value } = myObj[key];
                console.log(`Parsed Rx: ID:"${Id}" Value:"${Value}"`);
                var found = false;
                this.listeners.forEach(listener => {
                    if(typeof listener.onMessage === 'function'){
                        if(typeof listener.getSignalName === 'function') {
                            if(Id == listener.getSignalName())
                            {
                                found = true;
                                console.log(`Found Listener Rx: ID:"${Id}" Value:"${Value}"`);
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

    announceHere(){
        this.websocket.send('Hello I am here!');
    }

    send(message){
        if(this.websocket) {
            this.websocket.send(message);
        } else {
            console.error('Null Web_Socket!');
        }
    }

    onOpen(event) {
        if(this.websocket) {
            console.log('Connection opened');
            clearInterval(this.loadingAnimation);
            document.querySelector('#loadingModal .modal-content h2').textContent = 'Connected!';
            this.announceHere();
            setTimeout(() => {
                document.getElementById('loadingPage').style.display = 'none';
            }, 1000 );
        } else {
            console.error('Null Web_Socket!');
        }
    }

    onClose(event) {
        console.log('Connection closed');
        document.querySelector('#loadingModal .modal-content h2').textContent = 'Connection Closed!';
        document.getElementById('loadingPage').style.display = 'flex';
        setTimeout(() => this.initWebSocket(), 5000);
    }

    onError(event) {
        console.error('Connection Error:', event);
        this.websocket.close();
        document.querySelector('#loadingModal .modal-content h2').textContent = 'Connection Error!';
        document.getElementById('loadingPage').style.display = 'flex';
        setTimeout(() => this.initWebSocket(), 5000);
    }

}