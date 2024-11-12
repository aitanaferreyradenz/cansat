#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;
float temperature, pressure, altitude;

const char* ssid = "MI_RED_WIFI";
const char* password = "MI_PASSWORD";
WebServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2, 
                  Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16, 
                  Adafruit_BMP280::STANDBY_MS_500);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/temperature", handle_Temperature);
  server.on("/pressure", handle_Pressure);
  server.on("/altitude", handle_Altitude);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handle_OnConnect() {
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude(1013.25);
  server.send(200, "text/html", SendHTML(temperature, pressure, altitude));
}

void handle_Temperature() {
  temperature = bmp.readTemperature();
  server.send(200, "text/plain", String(temperature));
}

void handle_Pressure() {
  pressure = bmp.readPressure() / 100.0F;
  server.send(200, "text/plain", String(pressure));
}

void handle_Altitude() {
  altitude = bmp.readAltitude(1013.25);
  server.send(200, "text/plain", String(altitude));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature, float pressure, float altitude) {
  String ptr = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  ptr += "<script src=\"https://code.highcharts.com/highcharts.js\"></script>";
  ptr += "<style>body { min-width: 310px; max-width: 800px; height: 400px; margin: 0 auto; } h2 { font-family: Arial; font-size: 2.5rem; text-align: center; }</style></head><body>";
  ptr += "<h2>CANSAT Andes: Medicion de datos</h2>";00;
  ptr += "<div id=\"chart-temperature\" class=\"container\"></div>";
  ptr += "<div id=\"chart-pressure\" class=\"container\"></div>";
  ptr += "<div id=\"chart-altitude\" class=\"container\"></div>";
  
  ptr += "<script>"; 
  ptr += "var chartT = new Highcharts.Chart({ chart:{ renderTo : 'chart-temperature' }, title: { text: 'Temperatura' }, series: [{ showInLegend: false, data: [] }],";
  ptr += "plotOptions: { line: { animation: false, dataLabels: { enabled: true } }, series: { color: '#059e8a' } }, xAxis: { type: 'datetime', dateTimeLabelFormats: { second: '%H:%M:%S' } },";
  ptr += "yAxis: { title: { text: 'Temperatura (Celsius)' } }, credits: { enabled: false } });";
  
  ptr += "setInterval(function () { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {";
  ptr += "var x = (new Date()).getTime(), y = parseFloat(this.responseText); if(chartT.series[0].data.length > 40) { chartT.series[0].addPoint([x, y], true, true, true); } else { chartT.series[0].addPoint([x, y], true, false, true); } } };";
  ptr += "xhttp.open('GET', '/temperature', true); xhttp.send(); }, 1000);";

  ptr += "var chartP = new Highcharts.Chart({ chart:{ renderTo:'chart-pressure' }, title: { text: 'Presion' }, series: [{ showInLegend: false, data: [] }],";
  ptr += "plotOptions: { line: { animation: false, dataLabels: { enabled: true } }, series: { color: '#18009c' } }, xAxis: { type: 'datetime', dateTimeLabelFormats: { second: '%H:%M:%S' } },";
  ptr += "yAxis: { title: { text: 'Presion (hPa)' } }, credits: { enabled: false } });";
  
  ptr += "setInterval(function () { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {";
  ptr += "var x = (new Date()).getTime(), y = parseFloat(this.responseText); if(chartP.series[0].data.length > 40) { chartP.series[0].addPoint([x, y], true, true, true); } else { chartP.series[0].addPoint([x, y], true, false, true); } } };";
  ptr += "xhttp.open('GET', '/pressure', true); xhttp.send(); }, 1000);";

  ptr += "var chartA = new Highcharts.Chart({ chart:{ renderTo:'chart-altitude' }, title: { text: 'Altitud' }, series: [{ showInLegend: false, data: [] }],";
  ptr += "plotOptions: { line: { animation: false, dataLabels: { enabled: true } }, series: { color: '#ff0000' } }, xAxis: { type: 'datetime', dateTimeLabelFormats: { second: '%H:%M:%S' } },";
  ptr += "yAxis: { title: { text: 'Altitud (m)' } }, credits: { enabled: false } });";

  ptr += "setInterval(function () { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) {";
  ptr += "var x = (new Date()).getTime(), y = parseFloat(this.responseText); if(chartA.series[0].data.length > 40) { chartA.series[0].addPoint([x, y], true, true, true); } else { chartA.series[0].addPoint([x, y], true, false, true); } } };";
  ptr += "xhttp.open('GET', '/altitude', true); xhttp.send(); }, 1000);";
  
  ptr += "</script></body></html>";
  return ptr;
}
