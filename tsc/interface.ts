export interface CommandResponse {
  statusCode: number;
  message: string;
}

export interface DeviceStatus {
  version: string;
  device: number;
  errorType: number;
  errorCode: number;
  message: string;
  aditionalInfo: string;
  priority: number;
}

export interface CoinResult extends CommandResponse {
  event: number;
  coin: number;
  remaining: number;
}

export interface LostCoins {
  "50": number;
  "100": number;
  "200": number;
  "500": number;
  "1000": number;
}

export type UnsubscribeFunc = () => void;