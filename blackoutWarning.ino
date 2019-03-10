/*
  Rilevatore di blackout
  versione 1.0 del 10/03/2019
  Sviluppato da Francesco Tucci (https://www.iltucci.com/blog/?p=4292)

  Distinta base:
  - Arduino MKR GSM 1400
  - Resistenza da 1k
  - Resistenza da 2k
  - LiPo da almeno 1200mAh
  - una SIM che possa inviare SMS
*/

// Libreria per la gestione del GSM
#include <MKRGSM.h>

// PIN della SIM
const char PINNUMBER[] = "0000";

// numero di cellulare al quale inviare gli allarmi
const char TEL_AVVISO[] = "+391231234567";

// variabili per il controllo dello stato di alimentazione
int rete;
int inviato;
// se si imposta questa variabile a 0 all'avvio il sistema manderà 
// un SMS alla riaccensione, nel caso in cui la batteria si sia
// scaricata prima
int stato = 1; // 0: manca corrente - 1: c'è corrente

// varabili per il controllo dello stato della batteria
int vBatt;
float tensione;
int vBattPerc; 
int ledBlinkBattery;
int warningBatt = 0;
int vA6;

// istanze per le librerie GSM
GSM gsmAccess;
GSM_SMS sms;

void setup() {

  // attivo il pin per accendere e spegnere il LED integrato nella scheda
  pinMode(6, OUTPUT);
  // attivo il pin analogico per vedere lo stato della corrente
  pinMode(A6, INPUT);
  
  // attivo la seriale
  Serial.begin(9600);

  // stato della connessione GSM
  bool connected = false;

  // Avvio il modem GSM
  // Serve passare il PIN per l'attivazione della SIM
  while (!connected) {
    // se in connessione il LED un lampeggio ad ogni ciclo
    digitalWrite(6, HIGH);
    delay(150);
    digitalWrite(6, LOW);
    delay(150);       

    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      connected = true;
    } else {
      Serial.println("Non riesco a connettermi");
      // se non è connesso
      // lampeggia 5 volte ravvicinate
      for (int b = 0; b <6; b++)
      {
        digitalWrite(6, HIGH);
        delay(100);
        digitalWrite(6, LOW);
        delay(100);       
      }      
      delay(1000);
    }
  }

  Serial.println("GSM attivato");
  // al GSM attivo faccio 3 lampeggi lunghi
  for (int b = 0; b <4; b++)
  {
    digitalWrite(6, HIGH);
    delay(350);
    digitalWrite(6, LOW);
    delay(150);       
  }  
}

void loop() {

  // controllo se il PIN ha tensione
  rete = 1;
  vA6 = analogRead(A6);
  Serial.println(vA6);

  if (vA6 < 800)
  {
    rete = 0;
  }

  if (rete == 1)
  {
    Serial.println("220V Presente");
    digitalWrite(6, HIGH);
    if (stato == 0) // se lo stato è a zero vuol dire che al controllo prima non c'era corrente. Quindi avviso
    {
      Serial.println("220V Tornata");
      sms.beginSMS(TEL_AVVISO);
      sms.print("220V Tornata!");
      sms.endSMS();
      Serial.println("SMS INVIATO");
      stato = 1;
      warningBatt = 0;
    }
  }
  else
  {
    Serial.println("220V Assente");
    digitalWrite(6, LOW);

    // leggo la carica della batteria in V e in percentuale
    vBatt = analogRead(ADC_BATTERY);
    tensione = vBatt*(4.3/1023);
    vBattPerc=((tensione-3.6)/0.6)*100;
    Serial.print("Batteria: ");
    Serial.print(tensione);
    Serial.print("V - ");
    Serial.print(vBattPerc);
    Serial.println("%");
    
 
    if (stato == 1) // se lo stato è a uno vuol dire che al controllo prima c'era corrente. Quidni avviso
    {
      Serial.println("Ehi! c'è un blackout!");
      sms.beginSMS(TEL_AVVISO);
      sms.print("Blackout!");
      sms.endSMS();
      Serial.println("SMS INVIATO");
      stato = 0;
    }  

    // faccio lampeggiare il LED una volta ogni 10% di batteria
    ledBlinkBattery = vBattPerc/10;
   
    if (ledBlinkBattery<1)
    {
       for (int b = 0; b <5; b++)
      {
        digitalWrite(6, HIGH);
        delay(100);
        digitalWrite(6, LOW);
        delay(50);       
      } 
    }
    else
    {
    for (int blink = 0; blink < ledBlinkBattery; blink++)
      {
        digitalWrite(6, HIGH);
        delay(300);
        digitalWrite(6, LOW);
        delay(200);     
      }
    }

    // mando SMS di batteria scarica alla prima volta
    // che la percentuale scende sotto il 20% 
    if ((ledBlinkBattery < 1) && (warningBatt == 0))
    {
      sms.beginSMS(TEL_AVVISO);
      sms.print("ATTENZIONE, Batteria al 10%");
      sms.endSMS();
      Serial.println("SMS INVIATO");
      warningBatt = 1;
    }

    delay(5000);

  }

  delay(10000);
  
}
