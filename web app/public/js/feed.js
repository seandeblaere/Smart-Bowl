const feedButton = document.getElementById("feed");

function feedDog() {
  console.log("feeding dog");
  const message = new Paho.MQTT.Message("feed");
  message.destinationName = topicFeed;
  client.send(message);
}

feedButton.addEventListener("click", function () {
  feedDog();
});
