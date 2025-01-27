import { NextResponse } from "next/server";
import connectDB from "@/lib/db";
import mongoose from "mongoose";

const SensorDataSchema = new mongoose.Schema({
  moisture: Number,
  temperature: Number,
  humidity: Number,
  isRaining: Boolean,
  timestamp: { type: Date, default: Date.now },
});

// Get or create model
const SensorData =
  mongoose.models.SensorData || mongoose.model("SensorData", SensorDataSchema);

export async function GET(request: Request) {
  try {
    await connectDB();

    const { searchParams } = new URL(request.url);
    const limit = parseInt(searchParams.get("limit") || "10");
    const page = parseInt(searchParams.get("page") || "1");
    const startDate = searchParams.get("startDate");
    const endDate = searchParams.get("endDate");

    let query = {};
    if (startDate && endDate) {
      query = {
        timestamp: {
          $gte: new Date(startDate),
          $lte: new Date(endDate),
        },
      };
    }

    const skip = (page - 1) * limit;

    const [data, totalCount] = await Promise.all([
      SensorData.find(query)
        .sort({ timestamp: -1 })
        .skip(skip)
        .limit(limit)
        .lean(),
      SensorData.countDocuments(query),
    ]);

    return NextResponse.json({
      data,
      totalCount,
      currentPage: page,
      totalPages: Math.ceil(totalCount / limit),
    });
  } catch (error) {
    console.error("Database error:", error);
    return NextResponse.json(
      { error: "Failed to fetch sensor data" },
      { status: 500 }
    );
  }
}
