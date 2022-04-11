import { socket } from '..';
import Arduino from '../arduino';

export default async function cardboardCNCTest() {
  const arduino = new Arduino((key, data) => {
    switch (key) {
      case 'done':
        socket.emit('done');
        break;
      case 'error':
        socket.emit('error', data);
        break;
    }
  }, './arduino/cardboardCNC');
  socket.onAny((...data) => arduino.emit(data[0], data[1]));
}
