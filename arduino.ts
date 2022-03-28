import { execSync } from 'child_process';
import { ReadlineParser, SerialPort } from 'serialport';
import { writeFileSync, readFileSync } from 'fs';
import { args } from '.';

export default class Arduino {
  arduino: SerialPort;
  parser: ReadlineParser;
  constructor(onData: (key: string, data: string) => any, buildFile: string) {
    try {
      this.arduino = new SerialPort({
        path: process.env.PORT || ' ',
        baudRate: 115200,
      });
    } catch {
      const port = JSON.parse(
        execSync('arduino-cli board list --format json').toString(),
      ).find((v: any) => v.matching_boards).port.address;
      process.env.PORT = port;
      const env = readFileSync('.env').toString().split('\n');
      env[0] = `PORT='${port}'`;
      writeFileSync('.env', env.join('\n'));
      this.arduino = new SerialPort({ path: port, baudRate: 115200 });
    }
    if (args.buildArduino) {
      execSync(
        `arduino-cli compile -b arduino:avr:uno -u -t -p ${process.env.PORT} ${buildFile}`,
      );
    }
    this.parser = this.arduino.pipe(new ReadlineParser({ delimiter: '\r\n' }));
    this.parser.on('data', (buffer: Buffer) => {
      const raw = buffer.toString().split(':');
      onData(raw[0], raw[1]);
    });
  }
  emit(key: string, data?: string) {
    this.arduino.write(key + (data ? `:${data}` : ''));
  }
}
