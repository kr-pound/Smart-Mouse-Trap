#pragma once //it will check if the directory have same file name it will not compile again, avoid name clashes

#include "tensorflow/lite/c/c_api_internal.h" //It can work with language C work with API since the board run with C
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h" //send outputs to serial monitors ardunio

// Called every time the results of a person detection run are available. The
// `person_score` has the numerical confidence that the captured image contains
// a person, and `no_person_score` has the numerical confidence that the image
// does not contain a person. Typically if person_score > no person score, the
// image is considered to contain a person.  This threshold may be adjusted for
// particular applications.
void RespondToDetection(tflite::ErrorReporter *error_reporter, uint8_t Non_Animal, uint8_t Animal);
// Function is used for detecting objects if the camera capture something it will call this function or response to detection
