# ArchieRFID

Este proyecto se centra en la lectura de tarjetas RFID de 125 kHz, una tecnología que permite identificar el acceso de las personas , ofrecer la posibilidad de monitorear puestos de trabajo, optimizando la gestión de recursos y mejorando la seguridad en diversos entornos. 

<h1>Descripcion del proyecto </h1>

El sistema se enfoca en la adquisición de datos de las tarjetas RFID a través de  puerto serial. Una vez capturada la información, esta puede ser adaptada para diversas aplicaciones. En nuestro caso, hemos implementado este sistema para identificar y validar la presencia de personal autorizados en puestos de trabajo específicos, garantizando así un control y una trazabilidad.

Un sistema de RFID  funciona mediante un proceso directo. En este contexto, el módulo RFID genera una frecuencia portadora de 125 kHz. Cuando una tarjeta RFID se acerca al módulo, se establece una interacción entre la bobina del dispositivo y la tarjeta. Esta interacción crea un campo magnético que energiza la tarjeta, permitiendo que su chip interno responda. Una vez activada, la tarjeta emite una señal que contiene información codificada.La señal emitida es una señal analógica modulada, la cual es recibida por el integrado interno (U2270B) del lector RFID a través de su antena. Este integrado convierte la señal en datos digitales utilizando un método de codificación conocido como codificación Manchester. Posteriormente, el microcontrolador realiza el proceso de decodificación de la señal, teniendo en cuenta la base de tiempo de espera en relación con la señal. Una vez completada la decodificación, se obtienen los bits necesarios para interpretar el número de la tarjeta.

Las tarjetas en este caso son de 39 bits, estan compuesta con 15 bits de paridad y 24 bits de datos donde proviene el numero de la tarjeta, el microcontrolador traduce la señal por paquetes y hace el cambio de base binario a base hexadecimal.

![Group 4](https://github.com/user-attachments/assets/04e1c3cc-3a03-4207-9d2f-2c8c5ad525b5)
