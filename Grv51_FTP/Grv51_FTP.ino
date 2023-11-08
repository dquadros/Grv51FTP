/*
   Gravador AT89S51 / AT89S52 via FTP
*/

#include "Grv51_FTP.h"
#include <Wire.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <SimpleFTPServer.h>

FtpServer ftpSrv;

char *versao = "v1.00";
char localIP[20];

bool recebeuArq = false;
bool tratarArq = false;
char *nomeArq = NULL;

// Firmware a gravar
const int tamFlash = 8*1024;
byte firmware[tamFlash];
int tamFw = 0;


// Iniciação
void setup() {
  // Inicia serial
  Serial.begin(115200);
  Serial.println("Gravador AT89S5x via FTP");

  // Inicia interface de gravação
  grvReset();

  // Inicia o display
  Wire.begin();
  Display_init();
  Splash();

  // Inicia WiFi
  SPIFFS.begin(true);       // Na primeira execução formata a Flash
  WiFiSettings.onFailure = AvisoConfig;
  WiFiSettings.connect();   // Conecta à rede ou, se não encontrar cria AP p/ configurar
  strcpy (localIP, WiFi.localIP().toString().c_str());

  // Inicia FTP Server
  ftpSrv.setCallback(CallbackFTP);
  ftpSrv.setTransferCallback(CallbackTfa);
  ftpSrv.begin("garoa","pi");

  AvisoPronto();
}

// Laco Principal
void loop() {
  ftpSrv.handleFTP();
  if (tratarArq) {
    if (ProcessaArquivo()) {
      GravaFirmware();
    }
    AvisoPronto();
  }
}

// Tela de Apresentação
void Splash() {
  Display_clear ();
  Display_print (0, 2, "Grv51 FTP", VID_NORMAL);
  Display_print (1, 5, versao, VID_NORMAL);
  Display_print (3, 0, "Daniel Quadros", VID_NORMAL);
  delay(1000);
}

// Avisa para configurar
void AvisoConfig() {
  Display_clear ();
  Display_print (0, 0, "NAO CONECTOU", VID_REVERSO);
  Display_print (2, 0, "Configure WiFi", VID_NORMAL);
}

// Avisa que está pronto para operar
void AvisoPronto() {
  Display_clear ();
  Display_print (0, 0, localIP, VID_REVERSO);
  Display_print (1, 0, "Pronto", VID_NORMAL);
}

// Informa andamento da transferência e gravação
void AvisoTfa(const char *msg1, const char *msg2, const char *msg3) {
  Display_clear ();
  Display_print (0, 0, localIP, VID_REVERSO);
  if (msg1) {
    Display_print (1, 0, (char *)msg1, VID_NORMAL);
  }
  if (msg2) {
    Display_print (2, 0, (char *)msg2, VID_NORMAL);
  }
  if (msg3) {
    Display_print (3, 0, (char *)msg3, VID_NORMAL);
  }
}

// Chamada quando é feita uma operação no FTP
void CallbackFTP (FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace) {
  switch (ftpOperation) {
    case FTP_DISCONNECT:
      tratarArq = recebeuArq;
      recebeuArq = false;
      break;
    case FTP_CONNECT:
      AvisoTfa("Conectado", NULL, NULL);
      tratarArq = recebeuArq = false;
      break;
  }
}

// Chamada quando FTP realiza uma transferencia
void CallbackTfa (FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize) {
  switch (ftpOperation) {
    case FTP_UPLOAD_START:
      AvisoTfa("Recebendo", name, NULL);
      break;
    case FTP_TRANSFER_STOP:
      AvisoTfa("Recebido", name, NULL);
      recebeuArq = true;
      // Salva o nome do arquivo, com / na frente
      // codificação C old-style!
      if (nomeArq) {
        free(nomeArq);
      }
      nomeArq = (char *) malloc (strlen(name) + 2);
      *nomeArq = '/';
      strcpy (nomeArq+1, name);
      break;
  }
}

// Processa arquivo recebido
bool ProcessaArquivo () {
  tratarArq = false;

  memset (firmware, 0xFF, tamFlash);
  tamFw = 0;
  int pos = 0;
  File fp = SPIFFS.open(nomeArq, "r");
  if (fp) {
    uint8_t buf[512];
    byte linha[100];
    int size;
    while ((size = fp.read(buf, sizeof(buf))) > 0) {
      for (int i = 0; i < size; i++) {
        byte c = buf[i];
        if (c == '\n') {
          // fim de uma linha, processar se
          // linha começa com ':', tem mais de 10 caracteres e tem número ímpar de caracteres
          if ((pos > 10) && (linha[0] == ':') && ((pos & 1) == 1)) {
            // Extrai tamanho, endereço e tipo
            // Vamos ignorar o checksum e assumir que o tamanho está correto
            byte tam = (byte) decodHex (linha+1, 2);
            uint16_t addr = decodHex (linha+3, 4);
            byte tipo = (byte) decodHex (linha+7, 2);
            if (tipo == 0) {
              for (int j = 0; j < tam; j++) {
                byte dado = (byte) decodHex (linha+9+2*j, 2);
                if (addr < tamFlash) {
                  firmware[addr] =  dado;
                  if (addr > tamFw) {
                    tamFw = addr;       // lembra a última posição escrita
                  }
                }
                addr++;
              }
            }
          }
          pos = 0;  // iniciar nova linha
        } else if ((c > 0x20 ) && (c < 0x7F) && (pos < sizeof(linha))) {
          linha[pos++] = c;
        }
      }
    }
    tamFw++;  // tamFw estava com o ultimo endereco usado
    fp.close();
    Serial.print("Carregados ");
    Serial.print(tamFw);
    Serial.println(" bytes.");
  }
  return tamFw != 0;
}

// Rotina auxiliar para converte tam dígitos hexadecimais em um valor inteiro
uint16_t decodHex (byte *p, int tam) {
  uint16_t ret = 0;
  for (int i = 0; i < tam; i++) {
    byte c = *p++;
    ret = ret << 4;
    if ((c >= '0') && (c <= '9')) {
      ret += c - '0';
    } else if ((c >= 'A') && (c <= 'F')) {
      ret += c - 'A' + 10;
    } else if ((c >= 'a') && (c <= 'f')) {
      ret += c - 'a' + 10;
    }
  }
  return ret;
}

// Efetua a gravação do firmware recebido
void GravaFirmware() {
  switch (grvGrava()) {
    case Sucesso:
      AvisoTfa("SUCESSO", NULL, NULL);
      break;
    case ErroProg:
      AvisoTfa("ERRO", "Programacao", NULL);
      break;
    case ErroId:
      AvisoTfa("ERRO", "Chip incorreto", NULL);
      break;
    case ErroErase:
      AvisoTfa("ERRO", "Apagamento", NULL);
      break;
    case ErroGravacao:
      AvisoTfa("ERRO", "Gravacao", NULL);
      break;
    case ErroVerificacao:
      AvisoTfa("ERRO", "Verificacao", NULL);
      break;
  }
  delay(2000);
}
