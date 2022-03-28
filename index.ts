import { io } from 'socket.io-client';
import stepper from './projects/stepper';
import dotenv from 'dotenv';
import cardboardCNCTest from './projects/cardboardCNCTest';
dotenv.config();

interface Args {
  [key: string]: string;
}
export const args: Args = Object.fromEntries(
  process.argv
    .slice(2)
    .map((v: string) => (v.split('=').length === 1 ? [v, true] : v.split('='))),
);

export const socket = io(process.env.SOCKET_URL as string);
socket.on('connect', () => {
  socket.emit('init', {
    secret: process.env.SECRET,
    configuration: args.config,
  });
  switch (args.config) {
    case 'stepper':
      stepper();
      break;
    case 'cardboardCNCTest':
      cardboardCNCTest();
      break;
  }
});
