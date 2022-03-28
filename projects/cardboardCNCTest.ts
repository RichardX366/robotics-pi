import { SerialPort, ReadlineParser } from 'serialport';
import { socket, args } from '..';
import { execSync } from 'child_process';

export default async function cardboardCNCTest() {
  const port = JSON.parse(
    execSync('arduino-cli board list --format json').toString(),
  ).find((v: any) => v.matching_boards).port.address;
  if (args.buildArduino) {
    execSync(
      `arduino-cli compile -b arduino:avr:uno -u -t -p ${port} ./arduino/cardboardCNC`,
    );
  }
  const arduino = new SerialPort({
    path: port,
    baudRate: 115200,
  });
  const parser = arduino.pipe(new ReadlineParser({ delimiter: '\r\n' }));
  parser.on('data', (unparsedData: Buffer) => {
    const raw = unparsedData.toString().split(':');
    const key = raw[0];
    const data = raw[1];
    switch (key) {
      case 'done':
        socket.emit('done');
        break;
      case 'error':
        console.trace(data);
        socket.emit('done');
        break;
    }
  });
  socket.on('step', (steps) => {
    arduino.write(`step:${steps}`);
  });
  socket.on('move', (position) => {
    arduino.write(`move:${position}`);
  });
  socket.on('setClosest', () => {
    arduino.write('tune:setClosest');
  });
  socket.on('setFarthest', () => {
    arduino.write('tune:setFarthest');
  });
}
