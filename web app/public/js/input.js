const weightButton = document.getElementById("weight");
const cooldownButton = document.getElementById("cooldown");

function setWeight() {
  const message = new Paho.MQTT.Message(
    document.getElementById("userInput").value
  );
  message.destinationName = topicWeight;
  client.send(message);
}

function setCooldownTime() {
  console.log("test");
  const hours = document.getElementById("hoursInput").value || 0;
  const minutes = document.getElementById("minutesInput").value || 0;
  const cooldownTime = `${hours}:${minutes}`;
  const message = new Paho.MQTT.Message(cooldownTime);
  message.destinationName = topicCooldown;
  client.send(message);
}

weightButton.addEventListener("click", function () {
  setWeight();
});

cooldownButton.addEventListener("click", function () {
  setCooldownTime();
});
