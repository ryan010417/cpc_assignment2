import { Suspense } from "react";
import SensorDashboard from "@/components/SensorDashboard";
import { Card } from "@/components/ui/card";

export default function DashboardPage() {
  return (
    <div className="container mx-auto p-4">
      <h1 className="text-4xl font-bold mb-8">IoT Sensor Dashboard</h1>
      <Suspense
        fallback={
          <Card className="p-8">
            <div className="h-96 flex items-center justify-center">
              Loading sensor data...
            </div>
          </Card>
        }
      >
        <SensorDashboard />
      </Suspense>
    </div>
  );
}
