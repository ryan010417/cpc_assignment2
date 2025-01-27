import mqtt from "mqtt";
import { mqttConfig } from "../config/mqtt.js";
import { saveSensorData } from "./databaseService.js";

class MQTTService {
  constructor() {
    this.client = null;
    this.sensorData = {
      moisture: null,
      temperature: null,
      humidity: null,
      isRaining: null,
    };
  }

  connect() {
    this.client = mqtt.connect(mqttConfig.url, mqttConfig.options);

    this.client.on("connect", () => {
      console.log("Connected to MQTT broker");
      this.subscribeToTopics();
    });

    this.client.on("error", (error) => {
      console.error("MQTT connection error:", error);
    });

    this.client.on("message", this.handleMessage.bind(this));
  }

  subscribeToTopics() {
    Object.values(mqttConfig.topics).forEach((topic) => {
      this.client.subscribe(topic, (err) => {
        if (!err) {
          console.log(`Subscribed to ${topic}`);
        }
      });
    });
  }

  handleMessage(topic, message) {
    const payload = message.toString();

    switch (topic) {
      case mqttConfig.topics.moisture:
        this.sensorData.moisture = parseFloat(payload);
        break;
      case mqttConfig.topics.temperature:
        this.sensorData.temperature = parseFloat(payload);
        break;
      case mqttConfig.topics.humidity:
        this.sensorData.humidity = parseFloat(payload);
        break;
      case mqttConfig.topics.rain:
        this.sensorData.isRaining = payload === "1";
        break;
      case mqttConfig.topics.status:
        try {
          const statusData = JSON.parse(payload);
          this.sensorData = {
            moisture: statusData.moisture,
            temperature: statusData.temperature,
            humidity: statusData.humidity,
            isRaining: statusData.isRaining,
          };
          this.saveSensorDataIfComplete();
        } catch (error) {
          console.error("Error parsing status message:", error);
        }
        break;
    }

    this.saveSensorDataIfComplete();
  }

  saveSensorDataIfComplete() {
    if (this.isDataComplete()) {
      saveSensorData(this.sensorData);
      this.resetSensorData();
    }
  }

  isDataComplete() {
    return Object.values(this.sensorData).every((value) => value !== null);
  }

  resetSensorData() {
    this.sensorData = {
      moisture: null,
      temperature: null,
      humidity: null,
      isRaining: null,
    };
  }
}

export const mqttService = new MQTTService();
