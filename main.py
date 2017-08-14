import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BCM)

GPIO.setup(16, GPIO.IN)

while 1:
	print("The measured voltage is: {0}".format(GPIO.input(16)))
	sleep(0.5)
