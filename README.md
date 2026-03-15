# F1 Fuel Flow Gate

Projeto da disciplina de Sistemas em Tempo Real para simular a fiscalização de fluxo de combustível na Fórmula 1 utilizando FreeRTOS em um ESP32 programado pela Arduino IDE.

## Objetivo

O projeto simula um sensor de fiscalização que realiza leituras periódicas do fluxo de combustível e um atuador que tenta manipular esse fluxo para burlar a medição.

## Contexto

Na Fórmula 1, existe um limite regulamentar de fluxo de combustível. Este projeto reproduz a ideia de uma possível tentativa de fraude baseada em sincronização temporal entre o instante de medição do sensor e a atuação do sistema de injeção.

## Hardware Utilizado

- ESP32
- 1 LED verde
- 1 LED vermelho
- 1 LED indicador de cheat mode
- 2 botões
- Resistores de 220 ohms
- Protoboard e jumpers

## Arquitetura do Sistema

O sistema foi implementado com FreeRTOS usando três tarefas principais:

- **TaskSensor**: realiza a leitura do fluxo no instante de amostragem
- **TaskInjector**: controla o valor do fluxo de combustível
- **TaskControl**: lê os botões e define o modo de operação

Além disso, foi utilizado um timer de hardware para gerar a base temporal de 1 ms.

## Estratégia de Sincronização

O sistema usa notificações do FreeRTOS entre a interrupção do timer e as tarefas.

- A cada 30 ms ocorre uma leitura oficial do sensor
- Cerca de 2 ms antes da leitura, o atuador é avisado
- No modo cheat, o fluxo cai temporariamente para 100 antes da leitura
- No modo falha, um atraso proposital faz o sensor detectar 120

## Modos de Operação

### 1. Modo Legal
Fluxo constante em 100 unidades.

### 2. Modo Cheat
Fluxo normalmente em 120 unidades, mas reduzido para 100 exatamente no instante da medição.

### 3. Modo Falha
O sistema tenta burlar a medição, mas introduz atraso na sincronização e o sensor detecta violação.

## Ligações

- LED verde -> GPIO 25
- LED vermelho -> GPIO 26
- LED cheat -> GPIO 27
- Botão cheat -> GPIO 14
- Botão fail -> GPIO 12

## Como Executar

1. Instalar a Arduino IDE
2. Instalar o pacote de placas ESP32
3. Abrir o arquivo `f1_fuel_flow_gate.ino`
4. Selecionar a placa ESP32
5. Compilar e gravar
6. Abrir o monitor serial em 115200 baud

## Resultados Esperados

- No modo legal, LED verde aceso
- No modo cheat, LED verde continua aceso, mesmo com fluxo interno acima de 100 na maior parte do tempo
- No modo falha, LED vermelho acende ao detectar valor acima do limite

## Vídeo de Demonstração

Link do vídeo no YouTube: [COLE_AQUI_O_LINK]

## Autor

Seu nome aqui
