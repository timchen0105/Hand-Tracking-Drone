def write_throttle(arduino ,motor):
    arduino.write(bytes(motor, 'utf-8'))