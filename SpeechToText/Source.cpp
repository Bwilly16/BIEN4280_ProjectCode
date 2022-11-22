#include <iostream> 
#include <stdlib.h>
#include <stdio.h>
#include <speechapi_cxx.h>
#include <windows.h>
#include <tchar.h>


using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

std::string GetEnvironmentVariable(const char* name);
HANDLE hComm;
//typedef const wchar_t* gszPort = ("COM9");
TCHAR* pcCommPort = TEXT("COM9");

void PrintCommState(DCB dcb)
{
    //  Print some of the DCB structure values
    _tprintf(TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"),
        dcb.BaudRate,
        dcb.ByteSize,
        dcb.Parity,
        dcb.StopBits);
}


void SerialComs() {
    hComm = CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (hComm == INVALID_HANDLE_VALUE)
        // error opening port; abort
}

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
        std::cout << "\n\rRECOGNIZED: Text=" << result->Text << std::endl;
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