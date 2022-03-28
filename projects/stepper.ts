import { socket } from '..';
import Arduino from '../arduino';

export default async function stepper() {
  const arduino = new Arduino((data) => {
    if (data === 'done') socket.emit('done');
    console.log(data);
  }, './arduino/stepper');
  socket.onAny(arduino.emit);
}
