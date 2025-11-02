SKETCH = 3W_V2.ino

BOARD = arduino:avr:uno

PORT = /dev/ttyACM0

all:compile upload monitor

compile:
	arduino-cli compile --fqbn $(BOARD) $(SKETCH)
upload:
	arduino-cli upload -p $(PORT) --fqbn $(BOARD) $(SKETCH)
clean:
	rm -rf build

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=9600
