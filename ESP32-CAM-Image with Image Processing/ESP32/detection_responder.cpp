
#include "detection_responder.h"
#include <Arduino.h>

// This dummy implementation writes person and no person scores to the error
// console. Real applications will want to take some custom action instead, and
// should implement their own versions of this function.
// void RespondToDetection(tflite::ErrorReporter *error_reporter, uint8_t Animal, uint8_t Non_Animal)
void RespondToDetection(tflite::ErrorReporter *error_reporter, uint8_t Animal, uint8_t Non_Animal)
{
    if (Animal > 100)
    {
        error_reporter->Report("Animal Detected!");
    }
    //else if (Non_Animal > 200)
    //{
        //error_reporter->Report("Non_Animal Detected!");
    //}
    
    //error_reporter->Report("Non_Animal :%d Animal is:%d", Non_Animal, Animal);
    //if (Animal > Non_Animal) 
    //{
        // respond function when person is detected. 
        //digitalWrite(LED_BUILTIN, Animal > Non_Animal);
        //delay(1);
        //digitalWrite(LED_BUILTIN, false);
    //}
}
