<script>
    import MqttForm from '../MqttForm.svelte';
    import { writable } from 'svelte/store';
    import { onMount, onDestroy } from 'svelte';
    import mqtt from 'mqtt';
        //Define MQTT credentials
    let mqttUsername = "alwin.forslund@hitachigymnasiet.se";
    let mqttHost = "maqiatto.com";
    let mqttTopicControl = `${mqttUsername}/driverbot/control`;
    let mqttTopicDistance = `${mqttUsername}/driverbot/distance`;
    let mqttPassword = "Kingsvilleracecrew";
    let mqttPort = "8883";
    $: connectionString = `mqtt://${mqttHost}:${mqttPort}`;

    let servoDegrees = 85; //Default value for the slider

    let instructionStringStore = writable('');
    let distance = writable(0); //Store for distance

    let client;
        //Connect to MQTT
    function mqttConnect() {
        client = mqtt.connect(connectionString, {
            username: mqttUsername,
            password: mqttPassword,
        });
            //Subscribe to the Topics
        client.on('connect', () => {
            console.log("Connected to MQTT");
            client.subscribe(mqttTopicControl);
            client.subscribe(mqttTopicDistance);
        });

        client.on('disconnect', () => {
            console.log('Client disconnected');
        });
            //Decode MQTT_Messages
        client.on('message', (topic, message) => {
            const decodedMessage = new TextDecoder().decode(message);
            console.log("Received message:", topic, decodedMessage);
                //Set distance to the value from the MQTT_Message
            if (topic === mqttTopicDistance) {
                const parsedDistance = parseFloat(decodedMessage);
                console.log("Parsed distance:", parsedDistance);
                distance.set(parsedDistance);
            }
        });
            //Publish Messages to MQTT
        instructionStringStore.subscribe(value => {
            console.log(`Sending ${value} to ${mqttTopicControl}`);
            client.publish(mqttTopicControl, value);
        });
    }
        //Configure different movementfunctions
    function Forward() {
        instructionStringStore.set('forward');
    }

    function Hardstop() {
        instructionStringStore.set('hardstop');
    }

    function Reverse() {
        instructionStringStore.set('reverse');
    }

    function Servosend() {
        instructionStringStore.set(servoDegrees.toString());
    }

    function Roam() {
        instructionStringStore.set('roam');
    }

    function Stop_Roam() {
        instructionStringStore.set('stoproam');
    }
        //Convert keyboard_input to movementfunctions
    function handleKeydown(event) {
        if (event.key === 'w') {
            Forward();
        } else if (event.key === 's') {
            Reverse();
        } else if (event.key === 'a') {
            servoDegrees = 40;
            Servosend();
        } else if (event.key === 'd') {
            servoDegrees = 130;
            Servosend();
        }
    }

    function handleKeyup(event) {
        if (event.key === 'w') {
            Hardstop();
        } else if (event.key === 's') {
            Hardstop();
        } else if (event.key === 'a' || event.key === 'd') {
            servoDegrees = 85;
            Servosend();
        }
    }
        //Add listeners for Keyboard input
    onMount(() => {
        if (typeof window !== 'undefined') {
            window.addEventListener('keydown', handleKeydown);
            window.addEventListener('keyup', handleKeyup);
        }
    });
        //Remowe listeners for Keyboard input
    onDestroy(() => {
        if (typeof window !== 'undefined') {
            window.removeEventListener('keydown', handleKeydown);
            window.removeEventListener('keyup', handleKeyup);
        }
    });
</script>
<!-- Design of website -->
<h1>Driverbot Alwin Forslund:</h1>
<p>Visit <a href="https://github.com/abbindustrigymnasium/driverbot-kjell-urban">https://github.com/abbindustrigymnasium/driverbot-kjell-urban</a> to get more info</p>

<button on:click={mqttConnect}>Connect to MQTT</button>
<button on:click={Forward}>Forward</button>
<button on:click={Hardstop}>Stop</button>
<button on:click={Reverse}>Reverse</button>
<button on:click={Roam}>Roam</button>
<button on:click={Stop_Roam}>Stop Roam</button>

<input type="range" min="0" max="180" step="15" bind:value={servoDegrees} on:change={Servosend} />
<p>Current Servo Position: {servoDegrees}</p>
<p>Current Distance: {$distance} cm</p>
