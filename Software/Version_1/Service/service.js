const express = require('express');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const moment = require('moment-timezone');
const http = require('http');
const socketIo = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);
const port = 4000;

let puerto;
let attemptCount = 0;
const maxAttempts =150; //Cantidad de intentos
const attemptInterval = 5000; // Tiempo de espera
let isPortOpen = false; // Bandera para verificar el puerto

function openSerialPort() {
  puerto = new SerialPort({ path: 'COM20', baudRate: 9600 }, (err) => {
    if (err) {
      console.log(`Error al abrir el puerto: ${err.message}`);
      io.emit('statusCOM',`Error al abrir el puerto: ${err.message}`)
      attemptReconnect();
    }
  });
// actualiza la bandera para indicar Puerto abierto
  puerto.on('open', () => {
    console.log('Puerto COM20 abierto.. ');
    io.emit('statusCOM','El puerto COM20 abierto..')
    isPortOpen = true; 
    handleSerialData();
    attemptCount = 0; // reinicia el contador cuando se conecta con éxito
  });

// actualiza la bandera para indicar Puerto cerrado
  puerto.on('error', (err) => {
    console.log(`Error del puerto: ${err.message}`);
    isPortOpen = false; 
    attemptReconnect();
  });

  puerto.on('close', () => {
    console.log('El puerto COM20 se ha cerrado.');
    io.emit('statusCOM','El puerto COM20 se ha cerrado.')
    isPortOpen = false; // actualiza la bandera para indicar que el Puerto cerrado
    attemptReconnect();
  });
}

function attemptReconnect() {
  if (isPortOpen) {
    return;
  }

  attemptCount++;
  if (attemptCount >= maxAttempts) {
    console.log('No se pudo reconectar después los intentos. Cerrando el programa...');
    io.emit('ProgramStatus', 'No se pudo reconectar.... Cerrando el programa...')
    //process.exit(1); // Cierra el programa
  } else {
    console.log(`Intentando reconectar ..(${attemptCount}) en 5 seg...`);
    setTimeout(openSerialPort, attemptInterval); // Intenta reconectar 
  }
}

function handleSerialData() {
  const parser = puerto.pipe(new ReadlineParser({ delimiter: '\n' }));

  parser.on('data', (data) => {
    const receivedData = data.trim();

    console.log('Datos recibidos:', receivedData);

    try {
      // Parsear el JSON
      const parsedData = JSON.parse(receivedData);

      // Verificar que el JSON contenga los campos esperados
      if (parsedData.id && parsedData.tarjeta) {
        const timestamp = moment().tz('America/Argentina/Buenos_Aires').format('YYYY-MM-DD HH:mm:ss');

        // Extraer y convertir los valores hexadecimales a decimal
        const tarjetaHex = parsedData.tarjeta;
        const serieHex = tarjetaHex.slice(0, 2);
        const cardHex = tarjetaHex.slice(2);

        const serieValue = parseInt(serieHex, 16);
        const cardValue = parseInt(cardHex, 16);

        const serieValueStr = serieValue.toString().padStart(3, '0');
        const cardValueStr = cardValue.toString().padStart(5, '0');

        const RFID = serieValueStr + cardValueStr;

        // emitir los datos a través de Socket.IO
        io.emit('rfidCode', {
          id: parsedData.id,
          rfid: RFID,
          timestamp,
        });

        console.log('Datos emitidos:', {
          id: parsedData.id,
          rfid: RFID,
          timestamp,
        });
      } else {
        console.warn('JSON recibido sin los campos esperados:', parsedData);
      }
    } catch (error) {
      console.error('Error al parsear JSON:', error);
    }
  });
}

// Iniciar el puerto serial
openSerialPort();

// Configurar Socket.IO para clientes
io.on('connection', (socket) => {
  console.log('Cliente conectado');

  // Enviar un mensaje de conexión al cliente
  socket.emit('connection', { message: 'Conectado al servidor' });
});

server.listen(port, () => {
  console.log(`Servidor Socket.IO escuchando en http://localhost:${port}`);
});