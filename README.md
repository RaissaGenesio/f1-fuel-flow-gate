# F1 Fuel Flow Gate com FreeRTOS no ESP32
##Alunos Raissa Genesio da Silva, Nicholas ... Gustavo
## 1. Descrição do projeto
Este projeto simula o problema do limite de fluxo de combustível da Fórmula 1, no qual um sistema de injeção tenta burlar a fiscalização sincronizando a redução do fluxo exatamente no instante em que o sensor realiza a amostragem.

O sistema foi implementado em ESP32 utilizando FreeRTOS no framework Arduino.

## 2. Objetivo
Implementar um sistema em tempo real com tarefas concorrentes que simule:
- o sensor de fiscalização;
- o atuador de injeção de combustível;
- a lógica de controle por botões;
- a detecção de violação por LEDs.

## 3. Componentes utilizados
- ESP32
- 3 LEDs
- 2 botões
- 1 buzzer
- resistores de 220 ohms para os LEDs
- jumpers e protoboard

## 4. Mapeamento de pinos
- LED verde: GPIO 25
- LED vermelho: GPIO 26
- LED cheat: GPIO 27
- Botão cheat: GPIO 14
- Botão fail: GPIO 12

## 5. Arquitetura das tarefas
### TaskSensor
Tarefa de maior prioridade. Realiza a leitura do fluxo no instante exato de amostragem e decide se o sistema está regular ou em violação.

### TaskInjector
Tarefa responsável por simular a injeção de combustível. Em modo cheat, antecipa o instante da amostragem e reduz o fluxo para 100 antes da leitura. Em modo fail, insere atraso proposital e permite a detecção da fraude.

### TaskControl
Lê os botões e define o modo de operação:
- Legal
- Cheat
- Fail

## 6. Estratégia de sincronização
Foi utilizado um timer de hardware com base de 1 ms.
- No instante 28 ms do ciclo, a tarefa do injetor é notificada.
- No instante 30 ms, a tarefa do sensor é notificada.

Dessa forma, o injetor pode se antecipar à leitura do sensor.

## 7. Modos de operação
### Modo legal
Fluxo constante em 100.

### Modo cheat
Fluxo normalmente em 120, mas cai para 100 no momento certo da leitura.

### Modo fail
Fluxo normalmente em 120, porém o ajuste ocorre atrasado, permitindo que o sensor detecte 120.
Esse modo só é ativado quando o botão cheat + fail é acionado

## 8. Resultados esperados
- Modo legal: LED verde aceso.
- Modo cheat: LED verde aceso mesmo com trapaça ativa.
- Modo fail: LED vermelho aceso por violação detectada.


## 10. Montagem

## 11. Vídeo

