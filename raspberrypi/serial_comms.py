import serial
from datetime import datetime
import time

ser=serial.Serial('/dev/ttyAMA0',
                  baudrate = 9600,
                  parity = serial.PARITY_NONE,
                  stopbits = serial.STOPBITS_ONE,
                  bytesize = serial.EIGHTBITS,
                  timeout = 1.0)

ct=datetime.now()
timeStamp=ct.strftime("%m:%d:%Y")

file=open('FT205EV_Data_'+timeStamp+'.txt',mode='a')

while True:
    try:
        ser.write(b'$01,WV?*//\r\n')
        lineBytes = ser.readline()       
        lineString = lineBytes.decode('UTF-8')
        
        print(f'Line Bytes: {lineBytes}')
        print(f' Line String: {lineString}')
        
        speed =lineString[4:13]
        angle =lineString[14:17]
        error =lineString[18:19]
        
        ct=datetime.now()
        timeStamp=ct.strftime("%H:%M:%S")
        line = timeStamp+' >> '+speed+' '+angle+' '+error+'\n'

        file.writelines(line)
        time.sleep(1.0)
    except KeyboardInterrupt:
        break

file.write('\n')
file.close()
