import sys
import serial
import platform 

def find_ports():
    sys = platform.system()
    portbase = 'COM'
    if sys == 'Windows':
        portbase = 'COM'
    elif sys == 'Linux':
        portbase = '/dev/ttyACM'
        #portbase = '/dev/ttyUSB'
    
    ports = []
    for i in range(100): 
        try:
            port = portbase + str(i)
            s = serial.Serial(port)
            ports.append(port)
        except serial.SerialException:
            continue

    return ports


def main():
    ports = find_ports()
    if len(sys.argv) > 1:
        f = open(sys.argv[1], 'w')
        for port in ports:
            f.write(port + '\n')
    elif len(sys.argv) == 1:
        for port in ports:
            print(port)


if __name__ == '__main__': main()
