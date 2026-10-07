# 🪖 Projeto Arm-Tank: Ecossistema Robótico de Baixa Latência

<p align="center">
  <img src="https://img.shields.io/badge/Status-Em%20Desenvolvimento-orange?style=for-the-badge" alt="Status">
  <img src="https://img.shields.io/badge/Plataforma-Mecatr%C3%B4nica%20V4-blue?style=for-the-badge" alt="Plataforma">
  <img src="https://img.shields.io/badge/Firmware-C%2B%2B%20%2F%20Arduino-darkgreen?style=for-the-badge" alt="Firmware">
</p>

## 📌 Visão Geral
O **Arm-Tank** é um ecossistema robótico modular de alta performance projetado pela **DSBRTI (DestinyServicesBR)** para controle em tempo real de uma unidade de tração lagarta integrada a um braço mecânico/garra manipuladora. 

Diferente de abordagens maker tradicionais que sofrem com atrasos e travamentos, o foco absoluto deste projeto é o **determinismo temporal, imunidade a ruídos eletromagnéticos e fail-safe ativo**, tornando-o uma base sólida para pesquisa aplicada em automação industrial e robótica móvel.

---

## ⚡ Diferenciais de Engenharia (Por que apoiar este projeto?)

* **Latência Sub-20ms:** Comunicação sem fio via protocolo nativo **ESP-NOW** operando sem o *overhead* de pilhas TCP/IP convencionais.
* **Imunidade a Ruído (Hardware e Software):** Barramentos lógicos e de potência isolados com condutores de cobre rígido e filtragem digital via **Média Móvel Exponencial (EMA)** direto no conversor analógico-digital (ADC).
* **Arquitetura Descentralizada (Multi-Core Lógico):** 1. Uma placa dedicada exclusivamente à interface humana (IHM) e filtragem de sinal.
  2. Um cérebro central para interpolação cinemática e decisões de controle.
  3. Um controlador periférico escravo focado puramente em temporização de hardware (PWM para servos).

---

## 🗺️ Topologia e Arquitetura do Sistema

O ecossistema é fracionado em três nós de processamento interconectados:

