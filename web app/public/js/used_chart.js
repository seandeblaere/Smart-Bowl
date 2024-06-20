// used_chart.js
const ctx = document.getElementById("myChart").getContext("2d");

const chartData = {
  labels: [
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
  ],
  datasets: [
    {
      label: "Grams eaten",
      data: [0, 0, 0, 0, 0, 0, 0],
      borderWidth: 1,
    },
  ],
};

const myChart = new Chart(ctx, {
  type: "bar",
  data: chartData,
  options: {
    scales: {
      y: {
        beginAtZero: true,
      },
    },
  },
});
