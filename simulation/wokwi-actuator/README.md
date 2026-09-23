# Bancada Wokwi: atuador

Esta simulacao isola o contrato entre o ESP32 core e o Arduino Nano. O firmware `actuator_sim.ino` usa o mesmo frame delimitado da unidade real:

```text
<base,alcance,elevacao,velocidade,direcao>
```

Exemplo valido:

```text
<90,120,80,40,-10>
```

O Serial Monitor deve estar em `115200 baud`. Os tres servos exibem os angulos e os LEDs mostram as saidas PWM. Frames incompletos ou com valores fora das faixas sao ignorados. Sem um frame valido por 1,5 s, o firmware centraliza os servos e desliga o PWM.

## Rodar no VS Code

1. Instale a extensao Wokwi para VS Code.
2. Abra esta pasta como projeto ou abra `diagram.json`.
3. Compile o sketch pela extensao e inicie a simulacao.
4. Envie os frames no Serial Monitor.

O arquivo `wokwi.toml` aponta para os artefatos gerados pela compilacao. O firmware de producao continua em `../../ecosystem/cinematic_Altitudinal_Blockage/arm_tank.ino`; esta copia de bancada existe para permitir uma simulacao Arduino Nano independente do core ESP32.