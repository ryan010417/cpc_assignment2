import { SensorData } from "../models/sensorData.js";

export const saveSensorData = async (data) => {
  try {
    const sensorData = new SensorData(data);
    await sensorData.save();
    console.log("Sensor data saved successfully:", data);
  } catch (error) {
    console.error("Error saving sensor data:", error);
  }
};
