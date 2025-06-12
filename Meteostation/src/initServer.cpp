#include "../include/initServer.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include "../include/config.h"
#include <LittleFS.h>

extern float lastTemp;
extern float lastHumidity;

WebServer server(80);

const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Meteostation</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.15.4/css/all.css" crossorigin="anonymous" />
  <style>
    body {
      margin: 0;
      font-family: 'Segoe UI', sans-serif;
      background: #eef3f6;
      color: #333;
    }

    header {
      background: linear-gradient(to right, #3a7bd5, #00d2ff);
      color: white;
      padding: 20px 0;
      text-align: center;
      box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
    }

    header h1 {
      margin: 0;
      font-size: 2.5rem;
    }

    .container {
      max-width: 800px;
      margin: 30px auto;
      padding: 20px;
      background: white;
      border-radius: 15px;
      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
    }

    .data-display {
      display: flex;
      justify-content: space-around;
      align-items: center;
      flex-wrap: wrap;
      margin-bottom: 30px;
    }

    .data-box {
      background: #f7f9fa;
      border: 1px solid #ddd;
      border-radius: 10px;
      width: 40%;
      padding: 20px;
      margin: 10px;
      text-align: center;
    }

    .data-box h2 {
      margin: 10px 0;
      font-size: 1.6rem;
    }

    .data-box span {
      font-size: 2.5rem;
      font-weight: bold;
    }

    hr {
      border: none;
      height: 3px;
      background: #3a7bd5;
      width: 80%;
      margin: 40px auto 30px;
      border-radius: 2px;
    }

    canvas {
      max-width: 100%;
    }
  </style>
</head>
<body>

  <header>
    <h1><i class="fas fa-cloud-sun"></i> Meteostation</h1>
  </header>

  <div class="container">
    <div class="data-display">
      <div class="data-box">
        <h2><i class="fas fa-thermometer-half" style="color:#e55353;"></i> Temperature</h2>
        <span id="temp">--</span> &deg;C
      </div>
      <div class="data-box">
        <h2><i class="fas fa-tint" style="color:#3498db;"></i> Humidity</h2>
        <span id="humidity">--</span> &percnt;
      </div>
    </div>

    <hr />

    <canvas id="chart" height="200"></canvas>
  </div>

  <script>
    const tempEl = document.getElementById("temp");
    const humidityEl = document.getElementById("humidity");

    const tempData = [];
    const humidityData = [];
    const labels = [];

    async function getData() {
      try {
        const response = await fetch("/data");
        const data = await response.json();

        tempEl.textContent = data.temperature;
        humidityEl.textContent = data.humidity;

        labels.push(new Date().toLocaleTimeString());
        tempData.push(data.temperature);
        humidityData.push(data.humidity);

        if (labels.length > 20) {
          labels.shift();
          tempData.shift();
          humidityData.shift();
        }

        chart.update();
      } catch (error) {
        console.log("Error fetching data:", error);
      }
    }

    const ctx = document.getElementById("chart").getContext("2d");
    const chart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: labels,
        datasets: [
          {
            label: 'Temperature (°C)',
            data: tempData,
            borderColor: '#e55353',
            backgroundColor: 'rgba(229,83,83,0.1)',
            fill: true,
            tension: 0.3
          },
          {
            label: 'Humidity (%)',
            data: humidityData,
            borderColor: '#3498db',
            backgroundColor: 'rgba(52,152,219,0.1)',
            fill: true,
            tension: 0.3
          }
        ]
      },
      options: {
        responsive: true,
        scales: {
          y: { beginAtZero: true }
        },
        plugins: {
          legend: { position: 'top' }
        }
      }
    });

    setInterval(getData, 1000);
  </script>
</body>
</html>
)rawliteral";

void handleRoot()
{
  server.send(200, "text/html", html);
}

void sendData()
{
  String response = String("{\"temperature\":") + lastTemp + ",\"humidity\":" + lastHumidity + "}";
  server.send(200, "application/json", response);
}

void handleDb()
{
  File file = LittleFS.open("/database.csv", "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    server.send(404, "text/plain", "File not found");
    return;
  }
  String content = file.readString();
  file.close();
  server.send(200, "text/plain", content);
}

void initServer()
{
  if (LittleFS.begin())
  {
    Serial.println("LittleFS mounted successfully");
  }
  else
  {
    Serial.println("Failed to mount LittleFS");
  }

  server.on("/", handleRoot);
  server.on("/data", sendData);
  server.on("/database", handleDb);
  server.begin();
}
