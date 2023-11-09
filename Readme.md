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

  

Confira se apareceu o aviso de que a função "probe" foi chamada, se sim deu tudo certo, se não, algo deu errado no processo de instação.

## Mandar dados por UART

Se você tiver um adaptador USB para UART você pode conectar a parte USB no seu computador pessoal e a parte de UART nos pinos GPIO 14 (TX) e GPIO 15 (RX) do Raspberry Pi.

Depois execute os comandos abaixo no terminal do seu computador.
  

`sudo apt install git python3`

`git clone https://github.com/IgorXVI/pi-uart.git`

`cd pi-uart`

`sudo chmod 777 /dev/ttyUSB0`

`python3 send_uart_data.py`

  

Depois volte para o terminal do Raspberry Pi e execute o comando abaixo.

`cat /proc/pi-uart-data`

Confira se o texto "Igor de Almeida" apareceu, se sim deu tudo certo.

