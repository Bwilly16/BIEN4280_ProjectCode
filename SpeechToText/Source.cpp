#include <iostream> 
#include <stdlib.h>
#include <stdio.h>
#include <speechapi_cxx.h>
#include <windows.h>
#include <tchar.h>
#include <cstring>
#include <WinBase.h>
#include <strsafe.h>

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

std::string GetEnvironmentVariable(const char* name);
HANDLE hComm;
char DataBuffer[50];
char DataRead[50];
DWORD dwBytesToWrite = 1;
DWORD dwBytesWritten;
DWORD dwBytesToRead = 16; //change to whatever length we want to read from MBED
                         // will wait forever if length we expect is longer than what MBED sends
DWORD dwBytesRead;
BOOL bErrorFlag = FALSE;
BOOL bErrorFlag1 = FALSE;
BOOL bErrorFlagOrigin = FALSE;

void DisplayError(LPTSTR lpszFunction);

#define BUFFERSIZE 5
DWORD g_BytesTransferred = 0;
char   ReadBuffer[BUFFERSIZE] = { 0 };
OVERLAPPED ol = { 0 };


VOID CALLBACK FileIOCompletionRoutine(
    __in  DWORD dwErrorCode,
    __in  DWORD dwNumberOfBytesTransfered,
    __in  LPOVERLAPPED lpOverlapped
);

int main()
{

    

    auto speechKey = GetEnvironmentVariable("SPEECH_KEY");
    auto speechRegion = GetEnvironmentVariable("SPEECH_REGION");

    if ((size(speechKey) == 0) || (size(speechRegion) == 0)) {
        std::cout << "Please set both SPEECH_KEY and SPEECH_REGION environment variables." << std::endl;
        return -1;
    }

    auto speechConfig = SpeechConfig::FromSubscription(speechKey, speechRegion);

    speechConfig->SetSpeechRecognitionLanguage("en-US");

    auto audioConfig = AudioConfig::FromDefaultMicrophoneInput();
    auto recognizer = SpeechRecognizer::FromConfig(speechConfig, audioConfig);

    printf("Speak into your microphone!\n\n\r");
    printf("-------------------------------\n\n\r");
    printf("SAY ONE OF THESE COMMANDS EXACTLY:\n");
    printf("What color am I looking at?\n"); // 1
    printf("Set proximity to 10.\n");// 2
    printf("What's the temperature in Celsius?\n"); // 3
    printf("What's the temperature in Fahrenheit?\n"); // 4

    auto result = recognizer->RecognizeOnceAsync().get();

    
    if (result->Reason == ResultReason::RecognizedSpeech)
    {

        std::cout << "\n\rRECOGNIZED: Text = " << result->Text << std::endl; //Text.c_str() changed to Text
        //result->Text.copy(DataBuffer, sizeof(DataBuffer));
        
       // DataBuffer[0] = 0;

        if (result->Text.compare("What color am I looking at?") == 0) { // Command 1 
            DataBuffer[0] = '1'; // set command 1 to send 1 // no matter what this line happens
        }
        
        else if(result->Text.compare("Set proximity to 10.") == 0) { // Command 5
            DataBuffer[0] = '2';
        }
        else if(result->Text.compare("What's the temperature in Celsius?") == 0) { // Command 6 // possibly delete
            DataBuffer[0] = '3';
        }
        else if (result->Text.compare("What's the temperature in Fahrenheit?") == 0) { // Command 7
            DataBuffer[0] = '4';
        }
        else {
            DataBuffer[0] = '0';
            std::cout << "Given command was not recognized :( Try Again\n" << std::endl;
        }

        std::cout << "Whats in data buffer strings are compared:\n " << DataBuffer << std::endl;



    }
    else if (result->Reason == ResultReason::NoMatch)
    {
        std::cout << "NOMATCH: Speech could not be recognized. Here is the list of valid commands:" << std::endl;

    }
    else if (result->Reason == ResultReason::Canceled)
    {
        auto cancellation = CancellationDetails::FromResult(result);
        std::cout << "CANCELED: Reason=" << (int)cancellation->Reason << std::endl;

        if (cancellation->Reason == CancellationReason::Error)
        {
            std::cout << "CANCELED: ErrorCode=" << (int)cancellation->ErrorCode << std::endl;
            std::cout << "CANCELED: ErrorDetails=" << cancellation->ErrorDetails << std::endl;
            std::cout << "CANCELED: Did you set the speech resource key and region values?" << std::endl;
        }
    }

    
    //set up communications 
    hComm = CreateFile(L"\\\\.\\COM10", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //if stops working, change EXISTING to CREATE_NEW, FILE_SHARE_READ to 0

    if (hComm == INVALID_HANDLE_VALUE) {
        std::cout << "Serial port not open\n" << std::endl;
    }
    else {
        std::cout << "Serial port open!\n" << std::endl;
    }
    EscapeCommFunction(hComm, CLRDTR);//set up the "acceptance of a message"
    EscapeCommFunction(hComm, SETDTR);//set up the "acceptance of a message"

    //Write what we say to the microcontroller

    bErrorFlag = WriteFile(
        hComm,           // open file handle
        DataBuffer,       // start of data to write
        dwBytesToWrite,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);

    if (bErrorFlag == FALSE) {
        std::cout << "\n Fail to Write\n" << std::endl;
    }
    else {
        std::cout << "Successful write\n" << std::endl;
    }
    std::cout << "Whats was just written to the chip?:\n " << DataBuffer << std::endl;

    //read once Mbed sends a message back

    bErrorFlagOrigin = ReadFile(
        hComm,           // open file handle
        DataRead,       // start of data to read
        dwBytesToRead,  // number of bytes to read
        &dwBytesRead, // number of bytes that were read
        NULL);
        

    if (bErrorFlagOrigin == FALSE) {
        std::cout << "\n Fail to read\n" << std::endl;
    }
    else {
        std::cout << "Successful read\n" << std::endl;
    }

    std::cout <<"This is the temperature: \n" << DataRead << std::endl;
    //std::cout << "This is the what is in the &dwBytesRead:" << dwBytesRead << std::endl;

   // std::cout << "this is the value of hComm" << std::endl;
   // std::cout << hComm << std::endl;
   // EscapeCommFunction(hComm, CLRDTR);//set up the "acceptance of a message"
   // EscapeCommFunction(hComm, SETDTR);//set up the "acceptance of a message"

   
   // std::cout << "this is the number of bytes written: " << dwBytesWritten << std::endl;
    
   // EscapeCommFunction(hComm, CLRDTR);//set up the "acceptance of a message"
   // EscapeCommFunction(hComm, SETDTR);//set up the "acceptance of a message"
    //reading the file


    // std::cout << "bErrorFlag1 BOOL: " << std::endl;
    // std::cout << bErrorFlag1 << std::endl;
    //printf("Number of bytes read: %d\n", dwBytesRead);
    
     // files get read into this buffer
    //SleepEx(5000, TRUE);

    //std::cout << "Finished setting up communications" << std::endl;
    
   // std::cout << "This is the data buffer after writing" << std::endl;
    //std::cout << DataBuffer << std::endl; // files get read into this buffer

    //SleepEx(5000, TRUE);
    
 
}


void DisplayError(LPTSTR lpszFunction)
// Routine Description:
// Retrieve and output the system error message for the last-error code
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);

    lpDisplayBuf =
        (LPVOID)LocalAlloc(LMEM_ZEROINIT,
            (lstrlen((LPCTSTR)lpMsgBuf)
                + lstrlen((LPCTSTR)lpszFunction)
                + 40) // account for format string
            * sizeof(TCHAR));

    if (FAILED(StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error code %d as follows:\n%s"),
        lpszFunction,
        dw,
        lpMsgBuf)))
    {
        printf("FATAL ERROR: Unable to output error code.\n");
    }

    _tprintf(TEXT("ERROR: %s\n"), (LPCTSTR)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}


std::string GetEnvironmentVariable(const char* name)
{
#if defined(_MSC_VER)
    size_t requiredSize = 0;
    (void)getenv_s(&requiredSize, nullptr, 0, name);
    if (requiredSize == 0)
    {
        return "";
    }
    auto buffer = std::make_unique<char[]>(requiredSize);
    (void)getenv_s(&requiredSize, buffer.get(), requiredSize, name);
    return buffer.get();
#else
    auto value = getenv(name);
    return value ? value : "";
#endif
}
