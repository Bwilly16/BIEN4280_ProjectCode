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
char DataBuffer[];
DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
DWORD dwBytesWritten = 0;
BOOL bErrorFlag = FALSE;

//void DisplayError(LPTSTR lpszFunction);

int main()
{
    // This example requires environment variables named "SPEECH_KEY" and "SPEECH_REGION"
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
    printf("Command 1: Activate Color Sensor\n");
    printf("Command 2: Add (Color Value)\n");
    printf("Command 3: Reset Color Value\n");
    printf("Command 4: Activate Proximity Sensor\n");
    printf("Command 5: Set Proximity to (Height)\n");
    printf("Command 6: Activate Temperature Sensor\n");
    printf("Command 7: What's the temperature in Celcius\n");
    printf("Command 8: What's the temperature in Farinheit\n");
    printf("Command 9: Set Temperature to (Number)\n");

    auto result = recognizer->RecognizeOnceAsync().get();

    if (result->Reason == ResultReason::RecognizedSpeech)
    {
        std::cout << "\n\rRECOGNIZED: Text=" << result->Text.c_str() << std::endl;
        //DataBuffer = result->Text.c_str();
        char hold[20];
        result->Text.copy(hold, sizeof hold);
        std::cout << hold << '\n'; 
        
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
    hComm = CreateFileA("COM10", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    
    std::cout << "Finished setting up communications" << std::endl;
    
    bErrorFlag = WriteFile(
        hComm,           // open file handle
        DataBuffer,       // start of data to write
        dwBytesToWrite,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);
        

    std::cout << "Done writing file" << std::endl;
}
/*void DisplayError(LPTSTR lpszFunction)
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
*/
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