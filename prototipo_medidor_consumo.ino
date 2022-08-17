

#include <SoftwareSerial.h>


const int pinoTensao = A2; //PINO ANALÓGICO EM QUE O SENSOR ESTÁ CONECTADO
const int pinoCorrente = A0; //PINO ANALÓGICO EM QUE O SENSOR ESTÁ CONECTADO
const int pinoAtivarCorrente = 8;
//objetos

SoftwareSerial mySerial(11, 12); // RX, TX


//variaveis
float tensaoEntrada = 0.0; //VARIÁVEL PARA ARMAZENAR O VALOR DE TENSÃO DE ENTRADA DO SENSOR
float tensaoMedida = 0.0; //VARIÁVEL PARA ARMAZENAR O VALOR DA TENSÃO MEDIDA PELO SENSOR

float valorR1 = 30000.0; //VALOR DO RESISTOR 1 DO DIVISOR DE TENSÃO
float valorR2 = 7500.0; // VALOR DO RESISTOR 2 DO DIVISOR DE TENSÃO
int leituraSensor = 0; //VARIÁVEL PARA ARMAZENAR A LEITURA DO PINO ANALÓGICO

long int sensorValue = 0;  // variable to store the sensor value read

float voltage = 0;
float current = 0;

double consumo_total = 0;

boolean ativo = false;
unsigned long millisTarefaMedicao = millis();
int total_contagem_zero = 0;

void setup() {

  pinMode(pinoTensao, INPUT); //DEFINE O PINO COMO ENTRADA
  pinMode(pinoCorrente, INPUT); //DEFINE O PINO COMO ENTRADA
  pinMode(pinoAtivarCorrente, OUTPUT); //DEFINE O PINO COMO ENTRADA

  digitalWrite(pinoAtivarCorrente, 0);


  Serial.begin(115200); //INICIALIZA A SERIAL
  mySerial.begin(38400);

}

void loop() {
  if (ativo) {

    if ((millis() - millisTarefaMedicao) > 4000) {
      String medicao = medir();

      if (ativo) {
        mySerial.println(medicao);
      }
      millisTarefaMedicao = millis();
    }

  }else{
      millisTarefaMedicao = millis();
  }

  if (Serial.available()) {
    String teclado = "";
    while (Serial.available()) {
      char tecla = Serial.read();
      teclado += tecla;
      delay(5);
    }

    Serial.print("digitado no arduino: ");
    Serial.println(teclado);
    mySerial.println(teclado);

  }

  if (mySerial.available()) {

    String recebido = mySerial.readString();

    Serial.print("Dado recebido do esp: ");
    Serial.println(recebido);

    if (recebido.compareTo("ativar$%") && recebido.indexOf("$") > 0 && recebido.indexOf("%") > 0 ) {
      Serial.println("sinal para ativar");
      ativo = true;
      digitalWrite(pinoAtivarCorrente, 1);
    } else if (recebido.compareTo("stop#*")&& recebido.indexOf("#") > 0 && recebido.indexOf("*") > 0 ) {
      ativo = false;
      Serial.println("sinal para desativar");
      digitalWrite(pinoAtivarCorrente, 0);

    }
  }

}



String medir() {
  float current = 0;
  for (int i = 0; i < 1000; i++) {
    current = current + (.044 * analogRead(A0) - 3.78) / 1000;

    //5A mode, if 20A or 30A mode, need to modify this formula to
    //(.19 * analogRead(A0) -25) for 20A mode and
    //(.044 * analogRead(A0) -3.78) for 30A mode

    delay(1);
  }
  current = current / 2;
  Serial.println(current);

  leituraSensor = analogRead(pinoTensao); //FAZ A LEITURA DO PINO ANALÓGICO E ARMAZENA NA VARIÁVEL O VALOR LIDO
  tensaoEntrada = (leituraSensor * 5.0) / 1023.0; //VARIÁVEL RECEBE O RESULTADO DO CÁLCULO
  tensaoMedida = tensaoEntrada / (valorR2 / (valorR1 + valorR2)); //VARIÁVEL RECEBE O VALOR DE TENSÃO DC MEDIDA PELO SENSOR


  double potenciaw = tensaoMedida * current;
  double potencia = potenciaw / 1000;
  double consumo_por_segundo = potencia * 0.000277778 * 5;
  consumo_total += consumo_por_segundo;

  String sTensao = String(tensaoMedida, 2);
  String sCorrente = String(current);
  String sPotencia = String(potencia);
  String sPotenciaW = String(potenciaw);
  String sConsumoAtual = String(consumo_por_segundo, 5);
  String sConsumoTotal = String(consumo_total, 5);



  String texto =   "Tensão: " + sTensao + "V "
                   + " Corrente: " + sCorrente + "A "
                   + " Potencia(w): " + sPotenciaW + "w "
                   + " Potencia(kw): " + sPotencia + "kw "
                   + " Consumo Atual: " +  sConsumoAtual + "kWh "
                   + " Consumo Total: " +  sConsumoTotal + "kWh";



  Serial.println(texto);


  String texto_envio = sTensao + "*" +
                       sCorrente + "#" +
                       sPotenciaW + "@" +
                       sConsumoAtual + "&";

  if (current < 0.30) {
    total_contagem_zero++;
  }else{
    total_contagem_zero = 0;
  }

  if(total_contagem_zero > 2){
    Serial.println("Sem passagem de corrente, interrompendo medicao");
    ativo = false;
    digitalWrite(pinoAtivarCorrente, 0);
    total_contagem_zero=0;
  }

  return texto_envio;
}
