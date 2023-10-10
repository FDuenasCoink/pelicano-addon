import { CommandResponse, DeviceStatus, UnsubscribeFunc } from "./interface";
import { CoinResult, LostCoins } from "./interface";

export interface IPelicano {
  connect(): CommandResponse;
  checkDevice(): CommandResponse;
  startReader(): CommandResponse;
  getCoin(): CoinResult;
  getLostCoins(): LostCoins;
  modifyChannels(inhibitMask1: number, inhibitMask2: number): CommandResponse;
  stopReader(): CommandResponse;
  resetDevice(): CommandResponse;
  testStatus(): DeviceStatus;
  cleanDevice(): CommandResponse;
  onCoin(callback: (coin: CoinResult) => void): UnsubscribeFunc;
  getInsertedCoins(): PelicanoUsage;
}

interface PelicanoUsage extends CommandResponse {
  insertedCoins: number;
}

export interface PelicanoOptions {
  warnToCritical: number;
  maxCritical: number;
  maximumPorts: number;
  logLevel: number;
  logPath: string;
}