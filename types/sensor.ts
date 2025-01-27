export interface SensorData {
  _id: string;
  moisture: number;
  temperature: number;
  humidity: number;
  isRaining: boolean;
  timestamp: Date;
}

export interface SensorDataResponse {
  data: SensorData[];
  totalCount: number;
}
