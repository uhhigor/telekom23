#include "SerialPort.h"
#include "variables.h"

void SerialPort::initialize(char *chosenPort) {
    handleCom = CreateFile(chosenPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    isReadyPort = SetupComm(handleCom, 1, 128);    // USTAWIANIE WIELKOSCI BUFORA
    isReadyPort = GetCommState(handleCom, &controlDCB);
    controlDCB.BaudRate = BaudRate;
    controlDCB.ByteSize = 8;
    controlDCB.Parity = NOPARITY;                  //EVENPARITY;
    controlDCB.StopBits = ONESTOPBIT;
    controlDCB.fAbortOnError = TRUE;
    controlDCB.fOutX = FALSE;                      // XON/XOFF WYLACZANIE DO TRANSMISJI
    controlDCB.fInX = FALSE;                       // XON/XOFF WYLACZANIE DO ODBIERANIA
    controlDCB.fOutxCtsFlow = FALSE;               // WLACZANIE CTS flow control
    controlDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
    controlDCB.fOutxDsrFlow = FALSE;               //WLACZENIE DSR FLOW CONTROL
    controlDCB.fDtrControl = DTR_CONTROL_ENABLE;
    controlDCB.fDtrControl = DTR_CONTROL_DISABLE;
    controlDCB.fDtrControl = DTR_CONTROL_HANDSHAKE;
    isReadyPort = SetCommState(handleCom, &controlDCB);
}