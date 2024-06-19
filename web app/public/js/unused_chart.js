const ctx = document.getElementById('myChart');

  new Chart(ctx, {
    type: 'bar',
    data: {
      labels: ['Monday', 'Tuesday', 'Wednessday', 'Thursday', 'Friday', 'Saturday', 'Sunday'],
      datasets: [{
        label: 'Grams eaten',
        data: [12, 19, 3, 5, 2, 3],
        borderWidth: 1
      }]
    },
    options: {
        indexAxis: 'y',
        scales: {
            y: {
                beginAtZero: true,
            }
        }
    }
  });