#include <Arduino.h>

// =========================
// Pinos
// =========================
static const int LED_GREEN = 25;
static const int LED_RED   = 26;
static const int LED_CHEAT = 27;

static const int BTN_CHEAT = 14;
static const int BTN_FAIL  = 12;

// =========================
// Modos
// =========================
enum Mode {
  MODE_LEGAL = 0,
  MODE_CHEAT,
  MODE_FAIL
};

// =========================
// Variáveis compartilhadas
// =========================
volatile int currentFlow = 100;         // "kg/h" simulados
volatile Mode currentMode = MODE_LEGAL;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t sensorTaskHandle   = NULL;
TaskHandle_t injectorTaskHandle = NULL;
TaskHandle_t controlTaskHandle  = NULL;

hw_timer_t *baseTimer = NULL;

// contador de fase: 0..29 ms
volatile uint8_t phaseMs = 0;

// =========================
// Funções auxiliares seguras
// =========================
void setFlow(int value) {
  portENTER_CRITICAL(&mux);
  currentFlow = value;
  portEXIT_CRITICAL(&mux);
}

int getFlow() {
  int value;
  portENTER_CRITICAL(&mux);
  value = currentFlow;
  portEXIT_CRITICAL(&mux);
  return value;
}

void setMode(Mode m) {
  portENTER_CRITICAL(&mux);
  currentMode = m;
  portEXIT_CRITICAL(&mux);
}

Mode getMode() {
  Mode m;
  portENTER_CRITICAL(&mux);
  m = currentMode;
  portEXIT_CRITICAL(&mux);
  return m;
}

// Timer ISR
// Frequência base: 1 kHz = 1 ms
// A cada 28 ms avisa o injetor
// A cada 30 ms avisa o sensor
void ARDUINO_ISR_ATTR onBaseTimer() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  phaseMs++;
  if (phaseMs >= 30) {
    phaseMs = 0;
  }

  // Pré-evento: 2 ms antes da leitura
  if (phaseMs == 28) {
    vTaskNotifyGiveFromISR(injectorTaskHandle, &xHigherPriorityTaskWoken);
  }

  // Evento de amostragem
  if (phaseMs == 0) {
    vTaskNotifyGiveFromISR(sensorTaskHandle, &xHigherPriorityTaskWoken);
  }

  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

//---------------------------------
// Task de controle
// Lê botões e define modo
//---------------------------------
void TaskControl(void *pvParameters) {
  bool lastCheat = false;
  bool lastFail  = false;

  for (;;) {
    bool cheatPressed = (digitalRead(BTN_CHEAT) == LOW);
    bool failPressed  = (digitalRead(BTN_FAIL)  == LOW);

    if (!cheatPressed) {
      setMode(MODE_LEGAL);
    } else {
      if (failPressed) {
        setMode(MODE_FAIL);
      } else {
        setMode(MODE_CHEAT);
      }
    }

    // LED de cheat aceso quando botão cheat estiver ativo
    digitalWrite(LED_CHEAT, cheatPressed ? HIGH : LOW);

    if (cheatPressed != lastCheat || failPressed != lastFail) {
      Mode m = getMode();
      Serial.print("[CONTROL] Cheat=");
      Serial.print(cheatPressed);
      Serial.print(" Fail=");
      Serial.print(failPressed);
      Serial.print(" => Mode=");
      if (m == MODE_LEGAL) Serial.println("LEGAL");
      else if (m == MODE_CHEAT) Serial.println("CHEAT");
      else Serial.println("FAIL");
    }

    lastCheat = cheatPressed;
    lastFail  = failPressed;

    vTaskDelay(pdMS_TO_TICKS(20)); // debounce simples
  }
}

// --------------------------
// Task do atuador
// Reage ao pré-evento (28 ms)
//----------------------------
void TaskInjector(void *pvParameters) {
  for (;;) {
    // comportamento de fundo
    Mode m = getMode();
    if (m == MODE_LEGAL) {
      setFlow(100);
    } else {
      // cheat e fail ficam normalmente acima do limite
      setFlow(120);
    }

    // espera o pré-evento
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1)) > 0) {
      m = getMode();

      if (m == MODE_CHEAT) {
        // cai para 100 antes da amostragem
        setFlow(100);
        vTaskDelay(pdMS_TO_TICKS(3));   // mantém uma janela legal
        setFlow(120);                   // volta a "trapacear"
      }
      else if (m == MODE_FAIL) {
        // atraso proposital: o sensor deve pegar 120
        vTaskDelay(pdMS_TO_TICKS(4));   // erra o timing de propósito
        setFlow(100);                   // tarde demais
        vTaskDelay(pdMS_TO_TICKS(2));
        setFlow(120);
      }
    }
  }
}

// ------------------------
// Task do sensor
// Prioridade mais alta
// ------------------------
void TaskSensor(void *pvParameters) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    int sampledFlow = getFlow();

    if (sampledFlow <= 100) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
    } else {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
    }

    Serial.print("[SENSOR] t=");
    Serial.print(millis());
    Serial.print(" ms | mode=");

    Mode m = getMode();
    if (m == MODE_LEGAL) Serial.print("LEGAL");
    else if (m == MODE_CHEAT) Serial.print("CHEAT");
    else Serial.print("FAIL");

    Serial.print(" | sampledFlow=");
    Serial.print(sampledFlow);
    Serial.print(" | result=");
    if (sampledFlow <= 100) Serial.println("OK");
    else Serial.println("VIOLATION");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_CHEAT, OUTPUT);

  pinMode(BTN_CHEAT, INPUT_PULLUP);
  pinMode(BTN_FAIL, INPUT_PULLUP);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_CHEAT, LOW);

  // Criação das tasks
  xTaskCreate(
    TaskSensor,
    "TaskSensor",
    4096,
    NULL,
    3,   // maior prioridade
    &sensorTaskHandle
  );

  xTaskCreate(
    TaskInjector,
    "TaskInjector",
    4096,
    NULL,
    2,
    &injectorTaskHandle
  );

  xTaskCreate(
    TaskControl,
    "TaskControl",
    2048,
    NULL,
    1,
    &controlTaskHandle
  );

  // Timer base de 1 MHz
  baseTimer = timerBegin(1000000);
  timerAttachInterrupt(baseTimer, &onBaseTimer);

  // Interrupção a cada 1000 us = 1 ms
  timerAlarm(baseTimer, 1000, true, 0);

  Serial.println("Sistema iniciado.");
  Serial.println("BTN_CHEAT pressionado => modo cheat");
  Serial.println("BTN_CHEAT + BTN_FAIL pressionados => modo fail");
}

void loop() {

  vTaskDelete(NULL);
}