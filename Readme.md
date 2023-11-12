## PI UART
Simples módulo de kernel para Linux que recebe dados por UART e deixa o usuário ler esses dados atravéz de um arquivo proc.

Só foi testado em um Raspberry Pi 4 Model B.

## Configurar Serial Port no Raspberry Pi
Abra o terminal no Raspberry Pi e digite o comando:

`sudo raspi-config`

Vá em 
"Interface options">
"Serial Port">
"Login shell over serial?" selecionar "No">
"Enable serial port hardware?" selecionar "Yes"

Após isso fazer reboot.

Após o reboot o comando abaixo deve mostrar "serial0" no resultado:

`ls /dev/serial*`
  
## Instalação no Raspberry Pi
Para instalar esse módulo siga os comandos abaixo dentro do terminal do seu Raspberry Pi.

`sudo apt-get update && sudo apt-get upgrade`  

`sudo apt-get install raspberrypi-kernel-headers git build-essential`

`git clone https://github.com/IgorXVI/pi-uart.git`

`cd pi-uart`

`make`

`sudo cp echodev.dtbo /boot/overlays/`

em "/boot/config.txt" adicionar uma linha no fim do arquivo: `dtoverlay=echodev` 

`sudo reboot`

Depois do reboot entre de novo no terminal.

`cd pi-uart`

`sudo insmod pi_uart.ko`

`dmesg | tail`

  

Confira se apareceu o aviso de que a função "probe" foi chamada, se sim deu tudo certo, se não, algo deu errado no processo de instalação.

## Mandar dados por UART

Usar pi-uart-client.
