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
  socket.on('step', (steps) => {
    arduino.emit('step', steps);
  });
  socket.on('move', (position) => {
    arduino.emit('move', position);
  });
  socket.on('setClosest', () => {
    arduino.emit('tune', 'setClosest');
  });
  socket.on('setFarthest', () => {
    arduino.emit('tune', 'setFarthest');
  });
}
