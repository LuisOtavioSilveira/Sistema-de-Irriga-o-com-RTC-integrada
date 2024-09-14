#include <Wire.h>
#include <RTClib.h>  // Biblioteca RTC
RTC_DS3231 rtc;     // Instância do RTC

const int buttonPin = 2;  // Pino onde o botão está conectado
const int relayPin = 7;   // Pino onde o relé (válvula solenoide) está conectado
const int moistureSensorPin = A0; // Pino do sensor de umidade

int buttonState = 0;      // Estado atual do botão
int lastButtonState = 0;  // Estado anterior do botão
bool relayState = LOW;    // Estado atual do relé (válvula solenoide)
bool timerStarted = false; // Controla se o temporizador manual começou
unsigned long startTime = 0; // Tempo de início para o temporizador manual

// Tempo de delay para o botão (30 minutos em milissegundos)
const unsigned long delayTime = 30 * 60 * 1000;

// Variáveis de controle automático
bool autoRelayOn = false;         // Estado para indicar se o relé foi ligado automaticamente
unsigned long autoStartTime = 0;  // Momento em que o relé foi ligado automaticamente
const unsigned long autoDelayTime = 30 * 60 * 1000;  // 30 minutos para desligar automaticamente

void setup() {
  // Inicializa o RTC
  if (!rtc.begin()) {
    Serial.println("Erro ao inicializar o RTC!");
    while (1);
  }

  if (rtc.lostPower()) {
    // Ajusta o RTC com a hora correta se ele perdeu a alimentação
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Configura o pino do botão como entrada
  pinMode(buttonPin, INPUT);
  
  // Configura o pino do relé como saída
  pinMode(relayPin, OUTPUT);
  
  // Inicializa o relé desligado
  digitalWrite(relayPin, LOW);
  relayState = LOW;  // Garantir que o estado do relé seja desligado

  Serial.begin(9600);  // Para depuração
}

void loop() {
  DateTime now = rtc.now();  // Obtém a hora atual do RTC
  unsigned long currentMillis = millis(); // Tempo atual desde que o Arduino foi ligado
  
  // Leitura do botão e controle manual:
  buttonState = digitalRead(buttonPin);
  
  // Verifica o sensor de umidade
  int moistureLevel = analogRead(moistureSensorPin);

  // Detecta se o botão foi pressionado (mudança de estado de LOW para HIGH)
  if (buttonState == HIGH && lastButtonState == LOW) {
    if (!relayState && moistureLevel < 500) {  // Apenas liga o relé se o solo estiver seco
      relayState = HIGH;
      digitalWrite(relayPin, HIGH);  // Liga o relé manualmente
      startTime = millis();          // Armazena o tempo de início
      timerStarted = true;
    }
  }

  // Verifica se o temporizador manual de 30 minutos foi iniciado e se o tempo ultrapassou
  if (timerStarted && currentMillis - startTime >= delayTime) {
    relayState = LOW;
    digitalWrite(relayPin, LOW);  // Desliga o relé após 30 minutos
    timerStarted = false;         // Reseta o temporizador manual
  }

  // Armazena o estado anterior do botão
  lastButtonState = buttonState;

  // Controle automático com RTC:
  // 4h30 (04:30) ou 16h30 (16:30) - horário para ligar o relé automaticamente
  if (((now.hour() == 4 && now.minute() == 30) || (now.hour() == 16 && now.minute() == 30)) && !autoRelayOn) {
    if (moistureLevel < 500) {  // Apenas liga o relé se o solo estiver seco
      relayState = HIGH;
      digitalWrite(relayPin, HIGH);  // Liga o relé automaticamente
      autoStartTime = millis();      // Armazena o tempo de início automático
      autoRelayOn = true;            // Indica que o relé foi ligado automaticamente
    }
  }

  // Verifica se 30 minutos passaram desde que o relé foi ligado automaticamente
  if (autoRelayOn && currentMillis - autoStartTime >= autoDelayTime) {
    relayState = LOW;
    digitalWrite(relayPin, LOW);  // Desliga o relé automaticamente
    autoRelayOn = false;          // Reseta o estado de controle automático
  }

  delay(200);  // Reduz a frequência do loop
}
