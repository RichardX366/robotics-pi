import { execSync } from 'child_process';
import { ReadlineParser, SerialPort } from 'serialport';
import { writeFileSync, readFileSync } from 'fs';
import { args } from '.';

export default class Arduino {
  arduino: SerialPort;
  parser: ReadlineParser;
  async initialize (onData: (key: string, data: string) => any, buildFile: string) {
    const arduino: SerialPort = await new Promise(((res) => {
      const first = new SerialPort({
        path: process.env.PORT,
        baudRate: 115200,
      }, (error) => {
        let finalArduino: SerialPort;
        if (error) {
          const port = JSON.parse(
            execSync('arduino-cli board list --format json').toString(),
          ).find((v: any) => v.matching_boards).port.address;
          process.env.PORT = port;
          const env = readFileSync('.env').toString().split('\n');
          env[0] = `PORT='${port}'`;
          writeFileSync('.env', env.join('\n'));
          finalArduino = new SerialPort({ path: port, baudRate: 115200 });
        } else {
          finalArduino = first;
        }
        if (args.buildArduino) {
          execSync(
            `arduino-cli compile -b arduino:avr:uno -u -t -p ${process.env.PORT} ./arduino/cardboardCNC`,
          );
        }
        res(finalArduino);
      }
    )}));
    const parser = arduino.pipe(new ReadlineParser({ delimiter: '\r\n' }));
    parser.on('data', (buffer: Buffer) => {
      const raw = buffer.toString().split(':');
      onData(raw[0], raw[1]);
    });
    this.arduino = arduino;
    this.parser = parser;
    return this;
  }
  emit(key: string, data?: string) {
    this.arduino.write(key + (data ? `:${data}` : ''));
  }
}
