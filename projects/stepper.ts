import { SerialPort, ReadlineParser } from 'serialport';
import { args, socket } from '..';

export default async function stepper() {
  const arduino = new SerialPort({
    path: args.port,
    baudRate: 115200,
  });
  const parser = arduino.pipe(new ReadlineParser({ delimiter: '\r\n' }));
  parser.on('data', (unparsedData: Buffer) => {
    const data = unparsedData.toString();
    if (data === 'stepped') socket.emit('stepped');
  });
  socket.on('step', (steps) => {
    arduino.write(steps);
  });
}
