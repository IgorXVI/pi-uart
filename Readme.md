## PI UART
Simples módulo de kernel para Linux que recebe dados por UART e deixa o usuário ler esses dados atravéz de um arquivo proc.

Só foi testado em um Raspberry Pi 4 Model B.
  
## Instalação no Raspberry Pi
Para instalar esse módulo siga os comandos abaixo dentro do terminal do seu Raspberry Pi.

  

`sudo apt install raspberrypi-kernel-headers git build-essential`

`git clone https://github.com/IgorXVI/pi-uart.git`

`cd pi-uart`

`make`

`sudo cp pi_uart_overlay.dtbo /boot/firmware/overlays/`

`sudo nano /boot/config.txt`

adicionane a seguinte linha no fim do arquivo: `dtoverlay=pi_uart_overlay.dtbo`

`sudo reboot`

Depois do reboot entre de novo no terminal.

`cd pi-uart`

`sudo insmod pi_uart.ko`

`dmesg | tail`

  

Confira se tem alguma mensagem de erro.

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

