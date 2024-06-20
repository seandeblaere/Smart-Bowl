// app.js
const broker = "ws://192.168.50.74:9001/mqtt";
const topicWeight = "arduino/smartbowl/weight";
const topicFeed = "arduino/smartbowl/feed";
const topicCooldown = "arduino/smartbowl/cooldown";
const topicNotifications = "arduino/smartbowl/notifications";

const client = new Paho.MQTT.Client(
  "192.168.50.74",
  Number(9001),
  "clientId-webapp"
);

client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

client.connect({ onSuccess: onConnect });

function onConnect() {
  console.log("Connected to broker");
  client.subscribe(topicWeight);
  client.subscribe(topicFeed);
  client.subscribe(topicCooldown);
  client.subscribe(topicNotifications);
}

function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:" + responseObject.errorMessage);
  }
}

function onMessageArrived(message) {
  console.log("onMessageArrived:" + message.payloadString);

  if (message.destinationName === topicNotifications) {
    alert("Notification: " + message.payloadString);
  }
}
