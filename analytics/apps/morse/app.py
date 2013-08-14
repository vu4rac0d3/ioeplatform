'''
Created on Aug 14, 2013
@author: Arun Varma
'''
import time


ON = "ON"
OFF = "OFF"
OUTLET_STATUS = "outletStatus"
DOT = "."
DASH = "-"
SHORT = 1
LONG = 3
DEFAULT_PRODUCT_ID = 2012
DEFAULT_MESSAGE = "Hello World"
morseCode = {
        'A': '.-',              'a': '.-',
        'B': '-...',            'b': '-...',
        'C': '-.-.',            'c': '-.-.',
        'D': '-..',             'd': '-..',
        'E': '.',               'e': '.',
        'F': '..-.',            'f': '..-.',
        'G': '--.',             'g': '--.',
        'H': '....',            'h': '....',
        'I': '..',              'i': '..',
        'J': '.---',            'j': '.---',
        'K': '-.-',             'k': '-.-',
        'L': '.-..',            'l': '.-..',
        'M': '--',              'm': '--',
        'N': '-.',              'n': '-.',
        'O': '---',             'o': '---',
        'P': '.--.',            'p': '.--.',
        'Q': '--.-',            'q': '--.-',
        'R': '.-.',             'r': '.-.',
        'S': '...',             's': '...',
        'T': '-',               't': '-',
        'U': '..-',             'u': '..-',
        'V': '...-',            'v': '...-',
        'W': '.--',             'w': '.--',
        'X': '-..-',            'x': '-..-',
        'Y': '-.--',            'y': '-.--',
        'Z': '--..',            'z': '--..',
        '0': '-----',           ',': '--..--',
        '1': '.----',           '.': '.-.-.-',
        '2': '..---',           '?': '..--..',
        '3': '...--',           ';': '-.-.-.',
        '4': '....-',           ':': '---...',
        '5': '.....',           "'": '.----.',
        '6': '-....',           '-': '-....-',
        '7': '--...',           '/': '-..-.',
        '8': '---..',           '(': '-.--.-',
        '9': '----.',           ')': '-.--.-',
        ' ': ' ',               '_': '..--.-',
}


'''
run
@param user: User
turns the user's default devices off automatically after the default time
'''
def run(user):
    # timerOff all devices with the default product id
    devices = user.getDevicesByProductId(DEFAULT_PRODUCT_ID)
    for device in devices:
        morseFunction(device, DEFAULT_MESSAGE)


'''
morseFunction
@param device: Device
@param message: String
turns the given device on and off according to morse code, depending on the given message
'''
def morseFunction(device, message):
    # turn off device initially
    turnOffOutletStatus(device)

    # convert message to morse code
    morse = toMorse(message)
    # for each character, 
    for character in morse:
        morseSwitch(device, character)


'''
morseSwitch
@param character: char
turns on/off the outletStatus depending on character
'''
def morseSwitch(device, character):
    if character == DOT:
        turnOnOutletStatus(device)
        time.sleep(SHORT)
        turnOffOutletStatus(device)
    elif character == DASH:
        turnOnOutletStatus(device)
        time.sleep(LONG)
        turnOffOutletStatus(device)


'''
turnOffOutletStatus
@param device: Device
turn device's outletStatus off
'''
def turnOffOutletStatus(device):
    device.sendCommand(OUTLET_STATUS, None, OFF)


'''
turnOnOutletStatus
turn device's outletStatus off
'''
def turnOnOutletStatus(device):
    device.sendCommand(OUTLET_STATUS, None, ON)


'''
toMorse
@param message: String
@return the given message in morse code
'''
def toMorse(message):
    morse = ""
    for character in message:
        morse += morseCode[character]
    return morse