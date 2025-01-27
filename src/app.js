import "dotenv/config";
import { connectDB } from "./config/mongodb.js";
import { mqttService } from "./services/mqttService.js";

const startServer = async () => {
  try {
    // Connect to MongoDB
    await connectDB();

    // Connect to MQTT broker
    mqttService.connect();

    console.log("Server started successfully");
  } catch (error) {
    console.error("Server startup error:", error);
    process.exit(1);
  }
};

startServer();
