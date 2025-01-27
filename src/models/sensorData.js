import mongoose from "mongoose";

const sensorDataSchema = new mongoose.Schema({
  moisture: {
    type: Number,
    required: true,
  },
  temperature: {
    type: Number,
    required: true,
  },
  humidity: {
    type: Number,
    required: true,
  },
  isRaining: {
    type: Boolean,
    required: true,
  },
  timestamp: {
    type: Date,
    default: Date.now,
  },
});

export const SensorData = mongoose.model("SensorData", sensorDataSchema);
