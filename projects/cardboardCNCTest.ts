import { SerialPort, ReadlineParser } from 'serialport';
import { args, socket } from '..';

export default async function cardboardCNCTest() {
  const arduino = new SerialPort({
    path: args.bottomPort,
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
        console.error(data);
        socket.emit('done');
        break;
    }
    console.log(raw);
  });
  socket.on('step', (steps) => {
    arduino.write(`move:${steps}`);
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
