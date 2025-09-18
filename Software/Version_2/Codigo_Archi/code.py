import board
import time
import simpleio
import neopixel
from adafruit_pixel_framebuf import PixelFramebuffer
import json
import asyncio
import busio
from digitalio import DigitalInOut

import adafruit_connection_manager
from adafruit_wiznet5k.adafruit_wiznet5k import WIZNET5K
import adafruit_minimqtt.adafruit_minimqtt as MQTT


class I2CPCF8574Interface:
    def __init__(self, i2c, address):
        self.i2c = i2c
        self.address = address
        self.backlight = 0x08
        self.ENABLE = 0b00000100
        self.CHR = 1
        self.CMD = 0

    def _write_byte(self, data):
        while not self.i2c.try_lock():
            pass
        try:
            self.i2c.writeto(self.address, bytes([data | self.backlight]))
        finally:
            self.i2c.unlock()

    def _pulse_enable(self, data):
        self._write_byte(data | self.ENABLE)
        time.sleep(0.0005)
        self._write_byte(data & ~self.ENABLE)
        time.sleep(0.0001)

    def _write_four_bits(self, data):
        self._write_byte(data)
        self._pulse_enable(data)

    def _send(self, data, mode):
        high = data & 0xF0
        low = (data << 4) & 0xF0
        self._write_four_bits(high | mode)
        self._write_four_bits(low | mode)

    def init_lcd(self):
        time.sleep(0.05)
        self._write_four_bits(0x30)
        time.sleep(0.005)
        self._write_four_bits(0x30)
        time.sleep(0.0001)
        self._write_four_bits(0x30)
        self._write_four_bits(0x20)
        self._send(0x28, self.CMD)
        self._send(0x08, self.CMD)
        self._send(0x01, self.CMD)
        time.sleep(0.002)
        self._send(0x06, self.CMD)
        self._send(0x0C, self.CMD)

    def clear(self):
        self._send(0x01, self.CMD)
        time.sleep(0.002)

    def print(self, text):
        for char in text:
            if char == '\n':
                self._send(0xC0, self.CMD)
            else:
                self._send(ord(char), self.CHR)

class LCD:
    def __init__(self, interface, num_rows=2, num_cols=16):
        self.interface = interface
        self.interface.init_lcd()

    def clear(self):
        self.interface.clear()

    def print(self, text):
        self.interface.print(text)

# ========================== RFID ==========================
class RFIDReader:
    def __init__(self, tx_pin, rx_pin, baudrate=9600, timeout=0.001):
        self.uart = busio.UART(tx=tx_pin, rx=rx_pin, baudrate=baudrate, timeout=timeout)

    def read_rfid(self):
        data = self.uart.read()
        if data and len(data) >= 14:
            start = data.find(b"\x02")
            if start != -1 and start + 13 < len(data):
                if data[start + 13] == 0x03:
                    try:
                        return data[start + 5:start + 11].decode("ascii").upper()
                    except:
                        pass
        return None

TX_PIN = board.GP16
RX_PIN = board.GP17
BUZZER_PIN = board.GP22
PIXEL_PIN = board.GP24

i2c = busio.I2C(scl=board.GP27, sda=board.GP26)
lcd = LCD(I2CPCF8574Interface(i2c, 0x27))

pixeles = neopixel.NeoPixel(PIXEL_PIN, 64, brightness=0.05, auto_write=False)
matriz = PixelFramebuffer(pixeles, 8, 8, alternating=False)

lcd.clear()
for i in range(4):
    lcd.clear()
    lcd.print("Iniciando" + "." * i)
    time.sleep(0.1)

def pintar_matriz(color):
    matriz.fill(color)
    matriz.display()

def tono_inicio(pin):
    simpleio.tone(pin, 440, duration=0.08)
    time.sleep(0.04)
    simpleio.tone(pin, 494, duration=0.08)
    time.sleep(0.04)
    simpleio.tone(pin, 523, duration=0.08)

# ========================== Ethernet ==========================
cs = DigitalInOut(board.GP9)
spi_bus = busio.SPI(board.GP10, MOSI=board.GP11, MISO=board.GP12)
eth = WIZNET5K(spi_bus, cs)

print(" IP:", eth.pretty_ip(eth.ip_address))

BROKER_HOST = "---------" #colocar IP del broker
BROKER_PORT = 9999 #colocar puerto del broker
PUB_TOPIC = "PLANTA3/EXPENDEDORA/ARCHINET00/RFID"
SUB_TOPIC = "PLANTA3/EXPENDEDORA/ARCHINET00/OPERARIO"

def connected(client, userdata, flags, rc):
    print(f" Conectado a {BROKER_HOST}")
    lcd.clear()
    lcd.print("broker activo")
    client.subscribe(SUB_TOPIC)

def disconnected(client, userdata, rc):
    print(" Desconectado MQTT")
    lcd.clear()
    lcd.print("MQTT Desconectado")

def message(client, topic, message):
    lcd.clear()
    lcd.print(f"operario:\n{message}")
    time.sleep(1)
    lcd.clear()
    lcd.print("esperando\ntarjeta....")

pool = adafruit_connection_manager.get_radio_socketpool(eth)
ssl_context = adafruit_connection_manager.get_radio_ssl_context(eth)

mqtt_client = MQTT.MQTT(
    broker=BROKER_HOST,
    port=BROKER_PORT,
    is_ssl=False,
    socket_pool=pool,
    ssl_context=ssl_context,
)
mqtt_client.on_connect = connected
mqtt_client.on_disconnect = disconnected
mqtt_client.on_message = message

# ========================== inicialización ==========================
tono_inicio(BUZZER_PIN)
pintar_matriz(0xff0000)

reader = RFIDReader(tx_pin=TX_PIN, rx_pin=RX_PIN)
cola = []
ultima_tarjeta = None
tiempo_verde = 0.01

def fichada(rfid):
    print(json.dumps({"id": "GDS_Compra04", "tarjeta": rfid}))
    cola.append(rfid)
    pintar_matriz(0x33ff33)
    simpleio.tone(BUZZER_PIN, 1000, duration=0.3)

async def conectar_mqtt():
    while not mqtt_client.is_connected():
        try:
            print(" Conectando al broker...")
            mqtt_client.connect()
        except Exception as e:
            await asyncio.sleep(2)
        else:
            print(" Conectado al broker")

async def leer_rfid():
    global ultima_tarjeta
    while True:
        rfid = reader.read_rfid()
        if rfid and rfid != ultima_tarjeta:
            fichada(rfid)
            ultima_tarjeta = rfid
            await asyncio.sleep(tiempo_verde)
            pintar_matriz(0xff0000)
        elif not rfid:
            ultima_tarjeta = None
        await asyncio.sleep(0)

async def publicar_mqtt():
    while True:
        try:
            mqtt_client.loop()
            if mqtt_client.is_connected() and cola:
                dato = cola.pop(0)
                mqtt_client.publish(PUB_TOPIC, dato, qos=0)
        except Exception as e:
            print("Error MQTT:", e)
            await asyncio.sleep(1)
            await conectar_mqtt()
        await asyncio.sleep(0)

async def main():
    await conectar_mqtt()
    await asyncio.gather(
        leer_rfid(),
        publicar_mqtt()
    )

asyncio.run(main())
