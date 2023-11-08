// Endereço I2C do display
const byte DISP_Addr = 0x3C;

// Seleção de vídeo normal ou reverso
const byte VID_NORMAL = 0x00;
const byte VID_REVERSO = 0xFF;

// Conexão ao AT89S5x
const int pinEnable =  D7;
const int VSPI_MOSI = D10;
const int VSPI_MISO = D9;
const int VSPI_SCK = D8;

// IDs dos chips
const uint32_t ID_AT89S51 = 0x1E5106;
const uint32_t ID_AT89S52 = 0x1E5206;

// Resultados da Gravação
typedef enum { 
  Sucesso,          // Sucesso
  ErroProg,         // Não conseguiu colocar no modo programação
  ErroId,           // Id do chip não é do AT80S51
  ErroErase,        // Erro ao apagar
  ErroGravacao,     // Erro ao gravar
  ErroVerificacao   // Erro ao verificar
} ResultGravacao;

