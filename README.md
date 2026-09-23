# Arm-Tank

Ecossistema experimental de controle para uma unidade de tracao com braco e garra. O repositorio reune firmware Arduino/ESP32, experimentos de bancada, material didatico e uma bancada Wokwi para validar o protocolo do atuador.

## Arquitetura de referencia

```text
ESP32-C3 transmissor -- ESP-NOW --> ESP32 core -- UART 115200 --> Arduino Nano atuador
																			|-- servos D9/D10/D11
																			`-- PWM D5/D6
```

A variante de referencia esta em `ecosystem/cinematic_Altitudinal_Blockage/`. O callback ESP-NOW apenas captura o ultimo payload; a cinemática e a UART rodam no loop principal. O Nano rejeita frames incompletos ou fora das faixas antes de atualizar atuadores e watchdog.

## Simulacao Wokwi

Abra `simulation/wokwi-actuator/diagram.json` com a extensao Wokwi do VS Code. A bancada representa o Nano, tres servos e dois LEDs para as saidas PWM. Como o diagrama nao modela dois radios ESP-NOW, a entrada do core e injetada pelo Serial Monitor:

```text
<90,120,80,40,-10>
```

O monitor deve responder `OK frame`. Um frame invalido gera `ERR invalid-frame`; depois de 1,5 s sem frame valido, o atuador retorna a `SAFE link-timeout`.

## Validacao local

```bash
g++ -std=c++17 -Wall -Wextra -Werror tests/protocol_test.cpp -o /tmp/arm-tank-protocol-test
/tmp/arm-tank-protocol-test
python3 -m json.tool simulation/wokwi-actuator/diagram.json >/dev/null
```

Para firmware fisico, instale o core ESP32 e as bibliotecas `Servo`/`esp_now` pelo Arduino IDE ou Arduino CLI. Este container nao possui `arduino-cli` nem `wokwi-cli`; a compilacao final e a execucao da simulacao devem ser feitas pela extensao Wokwi ou em uma maquina com essas ferramentas.

## Areas do repositorio

- `ecosystem/`: variantes de firmware do sistema integrado.
- `labs/`: experimentos de sensores, radio, TFT e tracao.
- `lib/`: sketches isolados de componentes.
- `cursos/`: material didatico web.
- `site/`: apresentacao web do projeto.
- `simulation/`: projetos de simulacao reproduziveis.
- `tests/`: verificacoes host que nao dependem de hardware.

## Riscos conhecidos

- A ponte H documentada usa apenas dois pinos PWM; reversao real exige pinos de direcao ou um driver com interface equivalente.
- O enlace ESP-NOW depende de MAC/canal e pinagem reais, que ainda precisam ser parametrizados para cada bancada.
- O timeout e a posicao segura devem ser calibrados com a massa do braco e a fonte desligada durante os primeiros testes.
