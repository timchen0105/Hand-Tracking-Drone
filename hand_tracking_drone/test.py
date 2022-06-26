import serial

ser = serial

try:
  ser = serial.Serial("COM9", 9600, timeout=0.1  )
  while True:
    print(ser.read())

except serial.serialutil.SerialException:
  print ('exception')