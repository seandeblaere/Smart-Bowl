const broker = "ws://192.168.50.74:9001/mqtt";
const topicWeight = "arduino/smartbowl/weight";
const topicFeed = "arduino/smartbowl/feed";
const topicCooldown = "arduino/smartbowl/cooldown";
const feedButton = document.getElementById("feed");
const weightButton = document.getElementById("weight");
const cooldownButton = document.getElementById("cooldown");

console.log("test");

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
}

function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:" + responseObject.errorMessage);
  }
}

function onMessageArrived(message) {
  console.log("onMessageArrived:" + message.payloadString);
}

function sendWeight() {
  const message = new Paho.MQTT.Message(
    document.getElementById("userInput").value
  );
  message.destinationName = topicWeight;
  client.send(message);
}

function setCooldownTime() {
  const hours = document.getElementById("hoursInput").value || 0;
  const minutes = document.getElementById("minutesInput").value || 0;
  const cooldownTime = `${hours}:${minutes}`;
  const message = new Paho.MQTT.Message(cooldownTime);
  message.destinationName = topicCooldown;
  client.send(message);
}

function feedDog() {
  console.log("feeding dog");
  const message = new Paho.MQTT.Message("feed");
  message.destinationName = topicFeed;
  client.send(message);
}

feedButton.addEventListener("click", function () {
  feedDog();
});
