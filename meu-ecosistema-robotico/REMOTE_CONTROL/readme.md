Esquema de Conexões no ESP-C3
​O ESP-C3 possui pinos SPI nativos que serão compartilhados entre a Tela TFT e o módulo de rádio NRF24L01. Eles usam as mesmas linhas de dados (MOSI e SCK), mas pinos de seleção (CS / CSN) diferentes.
​Módulo NRF24L01:
​VCC -> 3.3V (O ESP-C3 opera estritamente em 3.3V)
​GND -> GND
​CE -> GPIO 2
​CSN -> GPIO 3
​SCK -> GPIO 4 (SPI SCK)
​MOSI -> GPIO 6 (SPI MOSI)
​MISO -> GPIO 5 (SPI MISO)
​Tela TFT 1.69" IPS (ST7789 ou similar):
​VCC -> 3.3V ou 5V (depende da placa do display)
​GND -> GND
​SCL/SCK -> GPIO 4 (Compartilha o SPI SCK)
​SDA/MOSI -> GPIO 6 (Compartilha o SPI MOSI)
​RES/RESET -> GPIO 7
​DC/RS -> GPIO 8
​CS -> GPIO 9 (Pino de seleção da Tela)
​Botões de Comando (Frente, Trás, Esquerda, Direita):
​Ligados nos pinos GPIO 10, GPIO 18, GPIO 19 e GPIO 20 com resistores de pull-up internos ativados via software.