const { Pelicano } = require('../dist');

const pelicano = new Pelicano({
  maxCritical: 4,
  warnToCritical: 10,
  maximumPorts: 10,
  logLevel: 1,
  logPath: 'logs/pelicano.log',
});

const connect = pelicano.connect();
console.log(`Connect retorna: ${connect.statusCode} y ${connect.message}`);

let checkDevice = pelicano.checkDevice();
console.log(`CheckDevice retorna: ${checkDevice.statusCode} y ${checkDevice.message}`);

if (checkDevice.statusCode !== 200 && checkDevice.statusCode !== 301) {
  console.log('Error detectado, no se puede continuar');
  process.exit(1);
}

const startReader = pelicano.startReader();
console.log(`StartReader retorna: ${startReader.statusCode} y ${startReader.message}`);

let total = 0;
for (let i = 0; i < 400; i++) {
  const coin = pelicano.getCoin();
  if (coin.statusCode === 303) continue;
  console.log(`GetCoin retorna. StatusCode: ${coin.statusCode} Event: ${coin.event} Coin: ${coin.coin} Message: ${coin.message} Remaining: ${coin.remaining}`);
  if (coin.remaining > 1) {
    const coinLost = pelicano.getLostCoins();
    console.log({ coinLost });
    Object.entries(coinLost).forEach(([coin, quantity]) => {
      const coinTotal = Number(coin) * quantity;
      total += coinTotal;
    });
  }
  if (coin.statusCode === 402 || coin.statusCode === 403) {
    console.log('Error detectado se detiene el loop');
    break;
  }
  total += coin.coin;
}

checkDevice = pelicano.checkDevice();
console.log(`CheckDevice retorna: ${checkDevice.statusCode} y ${checkDevice.message}`);

const stopReader = pelicano.stopReader();
console.log(`StopReader retorna: ${stopReader.statusCode} y ${stopReader.message}`);

console.log(`Total depositado: $${total}`);

const status = pelicano.testStatus();
console.log({ status });