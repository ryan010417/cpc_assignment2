export const mqttConfig = {
  url: process.env.MQTT_BROKER_URL,
  topics: {
    moisture: "sensors/moisture",
    temperature: "sensors/temperature",
    humidity: "sensors/humidity",
    rain: "sensors/rain",
    status: "sensors/status",
  },
  options: {
    clean: true,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
  },
};
