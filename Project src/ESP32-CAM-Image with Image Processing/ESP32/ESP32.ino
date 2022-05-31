//Tensorflow 
#include <esp32-hal-log.h> //include board driver

#include <TensorFlowLite_ESP32.h> //include tensorflow lite library for converting microcode and let esp read the model
#include "main_functions.h" //declare 2 function setup and loop

#include "detection_responder.h" //
#include "image_provider.h"
#include "model_settings.h"
#include "classification_model_data.h"
#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
//#include "tensorflow/lite/version.h"

#include "soc/rtc_cntl_reg.h"

//ESP32 Cam


#include <dummy.h>
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <base64.h>
#include <PubSubClient.h>
#include <WiFi.h>


const char* ssid = "auk_2g";
const char* password =  "0863238992";
const char* mqttServer = "projecttech.thddns.net";
const int mqttPort = 5050;
const char* mqttUser = "";
const char* mqttPassword = "";
String test; //global test --> Use for getting the save data buffer and put it in the test

#define PUBLISH_TOPIC "esp32cam/image_data"
#define RAT_DETECTED "esp32cam/image_processing"

WiFiClient espClient;
PubSubClient client(espClient);


// Globals, used for compatibility with Arduino-style sketches.
namespace
{
  tflite::ErrorReporter     *error_reporter = nullptr;
  const tflite::Model       *model = nullptr;
  tflite::MicroInterpreter  *interpreter = nullptr; //declare interpreter Tflite
  TfLiteTensor              *input = nullptr;

  // An area of memory to use for input, output, and intermediate arrays.
  constexpr int             kTensorArenaSize = 65 * 1024;
  static uint8_t            tensor_arena[kTensorArenaSize];

} // namespace

void setupWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

// Connect with MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Tensorflow", mqttUser, mqttPassword )) {
      Serial.println("connected ");
    } 
    
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
            WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
            Serial.begin(115200);
            
  // Connect to Wifi
            WiFi.mode(WIFI_STA);
            setupWifi();
            client.setServer(mqttServer, mqttPort);

            /* Cam IP */
            Serial.println();
            Serial.print("ESP32-CAM IP Address: ");
            Serial.println(WiFi.localIP());
            Serial.println("Successfully Connect to WIFI\n");

  //Tensorflow Part
            // Disable brownout
            WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

            //Serial.setDebugOutput(true);
            esp_log_level_set("*", ESP_LOG_VERBOSE);

            log_i("CPU Freq = %d Mhz", getCpuFrequencyMhz());
            log_i("Starting ESP32Cam...");



  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)'=
  
            static tflite::MicroErrorReporter micro_error_reporter;
            error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  
  
            model = tflite::GetModel(g_classification_model_data);
            
  //Check if our model can read or not
            //if (model->version() != TFLITE_SCHEMA_VERSION)
            //{
              //error_reporter->Report("Model provided is schema version %d not equal to supported version %d.", model->version(), TFLITE_SCHEMA_VERSION);
              //return;
            //}
  //Get model will be in function set up()

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::ops::micro::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)

  
          static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
          micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D, tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
          micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D, tflite::ops::micro::Register_CONV_2D());
          micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_AVERAGE_POOL_2D, tflite::ops::micro::Register_AVERAGE_POOL_2D());

  // Build an interpreter to run the model with.
  
          static tflite::MicroInterpreter static_interpreter(model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
          interpreter = &static_interpreter; //pointer queue to run interpreter

  
  //Build interpreter, declare interpreter function in variable interpreter //reserve point address in C

  // Allocate memory from the tensor_arena for the model's tensors.
          TfLiteStatus allocate_status = interpreter->AllocateTensors();
          if (allocate_status != kTfLiteOk)
          {
            error_reporter->Report("AllocateTensors() failed");
            return;
          }
  // If one condition in setup fail, the whole process will not work, but if it all work it will go to function loop

  // Get information about the memory area to use for the model's input.
  
          input = interpreter->input(0); //get input from image
}


void loop() {
   /* MQTT Part */
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Predict sectiion 

  //Tensorflow
  
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels, input->data.uint8)) // Call GetImage
  {
    error_reporter->Report("Image capture failed.");
  }
  if (kTfLiteOk != interpreter->Invoke())
  {
    error_reporter->Report("Invoke failed.");
  }

  TfLiteTensor *output = interpreter->output(0);

  // Process the inference results.
  uint8_t Non_Animal = output->data.uint8[kPersonIndex]; // Non_Animal
  uint8_t Animal = output->data.uint8[kNotAPersonIndex]; //Animal
  RespondToDetection(error_reporter, Non_Animal, Animal);

 // send predicted values : Non_Animal, Animal via MQTT protocal

 if(Animal > 100){
  client.publish(RAT_DETECTED, "1");
 }
}
