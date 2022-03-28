import { execSync } from 'child_process';
import { SerialPort, ReadlineParser } from 'serialport';
import { socket, args } from '..';

export default async function stepper() {
  const port = JSON.parse(
    execSync('arduino-cli board list --format json').toString(),
  ).find((v: any) => v.matching_boards).port.address;
  if (args.buildArduino) {
    execSync(
      `arduino-cli compile -b arduino:avr:uno -u -t -p ${port} ./arduino/stepper`,
    );
  }
  const arduino = new SerialPort({
    path: port,
    baudRate: 115200,
  });
  const parser = arduino.pipe(new ReadlineParser({ delimiter: '\r\n' }));
  parser.on('data', (unparsedData: Buffer) => {
    const data = unparsedData.toString();
    if (data === 'done') socket.emit('done');
    console.log(data);
  });
  socket.on('step', (steps) => {
    arduino.write(`step:${steps}`);
  });
  socket.on('pins', (pins) => {
    arduino.write(`pins:${pins}`);
  });
}
