// Need to use extern "C" to prevent C++ name mangling
extern "C" {
  #include "../lib/include/roverlib.h" 
}
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "sl_lidar.h" 
#include "sl_lidar_driver.h"

using namespace sl;

bool ctrl_c_pressed;
void ctrlc(int)
{
    ctrl_c_pressed = true;
}

long long current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}


// The main user space program
// this program has all you need from roverlib: service identity, reading, writing and configuration
int user_program(Service service, Service_configuration *configuration) {
  if (configuration == NULL) {
    printf("Configuration cannot be accessed\n");
    return 1;
  }

  double *speed = get_float_value_safe(configuration, "speed");
  if (speed == NULL) {
    printf("Failed to get configuration\n");
    return 1;
  }
  printf("Fetched runtime configuration example tunable number: %f\n", *speed);

  double *mode = get_float_value_safe(configuration, "mode");
  if (mode == NULL) {
    printf("Failed to get configuration\n");
    return 1;
  }
  printf("Fetched runtime configuration example tunable number: %f\n", *mode);


  write_stream *write_stream = get_write_stream(&service, "lidar-data");
  if (write_stream == NULL) {
    printf("Failed to create write stream 'decision'\n");
  }
  
  IChannel* _channel;
  
  
  _channel = (*createSerialPortChannel("/dev/ttyUSB0", 115200));
  ILidarDriver * drv = *createLidarDriver();

  auto res = drv->connect(_channel);
  sl_lidar_response_device_info_t deviceInfo;
  if(SL_IS_OK(res)) {
    res = drv->getDeviceInfo(deviceInfo);
    if(SL_IS_OK(res)){
      printf("Model: %d, Firmware Version: %d.%d, Hardware Version: %d\n",
      deviceInfo.model,
      deviceInfo.firmware_version >> 8, deviceInfo.firmware_version & 0xffu,
      deviceInfo.hardware_version);
    }
    else {
      fprintf(stderr, "Failed to get device information from LIDAR %08x\r\n", res);
    }
  }
  else {
    fprintf(stderr, "Failed to connect to LIDAR %08x\r\n", res);
  }

  signal(SIGINT, ctrlc);
  signal(SIGTERM, ctrlc);

  std::vector<LidarScanMode> scanModes;

  drv->getAllSupportedScanModes(scanModes);
  printf("FIRST: %s\n", scanModes[0].scan_mode);
  printf("Second: %s\n", scanModes[1].scan_mode);
  printf("Third: %s\n", scanModes[2].scan_mode);

  // sets the speed of the motor from the configuration value
  drv->setMotorSpeed(speed);
  
  // Check if the mode from the configuration is valid and set the scan mode
  if (mode == 0 || mode == 1 || mode == 2) {
    drv->startScanExpress(false, scanModes[mode].id);
  } 
  else {
    printf("Invalid mode value, please use either 0 (Standard), 1 (Express) or 2 (Boost)\n");
    drv->stop();
    drv->setMotorSpeed(0);
    delete drv;
    delete _channel;
    return 1;
  }

  //drv->startScanExpress(false, scanModes[2].id);

  while (true) {
    sl_lidar_response_measurement_node_hq_t nodes[1024];

    size_t nodeCount = sizeof(nodes)/sizeof(sl_lidar_response_measurement_node_hq_t);

    res = drv->grabScanDataHq(nodes, nodeCount);

    if(SL_IS_OK(res)) {
      drv->ascendScanData(nodes, nodeCount);
      // for (int pos = 0; pos < (int)nodeCount ; ++pos) {
      //   printf("%s theta: %03.2f Dist: %08.2f Q: %d \n", 
      //       (nodes[pos].flag & SL_LIDAR_RESP_HQ_FLAG_SYNCBIT) ?"S ":"  ", 
      //       (nodes[pos].angle_z_q14 * 90.f) / 16384.f,
      //       nodes[pos].dist_mm_q2/4.0f,
      //       nodes[pos].quality >> SL_LIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);
      // }

      ProtobufMsgs__SensorOutput lidar_msg = PROTOBUF_MSGS__SENSOR_OUTPUT__INIT;
      lidar_msg.timestamp = current_time_millis();
      lidar_msg.status = 0;
      lidar_msg.sensorid = 1;
      // Set the oneof field contents
      ProtobufMsgs__LidarSensorOutput lidar_sensor_output = PROTOBUF_MSGS__LIDAR_SENSOR_OUTPUT__INIT;
      lidar_sensor_output.n_scans = nodeCount;
      lidar_sensor_output.scans = (ProtobufMsgs__LidarSensorOutput__Scan**)malloc(sizeof(ProtobufMsgs__LidarSensorOutput__Scan) * nodeCount);
      for (int i = 0; i < nodeCount; i++) {
        lidar_sensor_output.scans[i] = (ProtobufMsgs__LidarSensorOutput__Scan*)malloc(sizeof(ProtobufMsgs__LidarSensorOutput__Scan));
        if (lidar_sensor_output.scans[i] == NULL) {
          printf("SHIT\n");  
        }
        *lidar_sensor_output.scans[i] = PROTOBUF_MSGS__LIDAR_SENSOR_OUTPUT__SCAN__INIT;
        lidar_sensor_output.scans[i]->angle = (nodes[i].angle_z_q14 * 90.f) / 16384.f;
        lidar_sensor_output.scans[i]->distance = nodes[i].dist_mm_q2/4.0f;
        lidar_sensor_output.scans[i]->quality = nodes[i].quality >> SL_LIDAR_RESP_MEASUREMENT_QUALITY_SHIFT;
        lidar_sensor_output.scans[i]->isstart = (nodes[i].flag & SL_LIDAR_RESP_HQ_FLAG_SYNCBIT) ? 1 : 0;
      }
      
      // Set the oneof field (union)
      lidar_msg.lidaroutput = &lidar_sensor_output;
      lidar_msg.sensor_output_case = PROTOBUF_MSGS__SENSOR_OUTPUT__SENSOR_OUTPUT_LIDAR_OUTPUT;

      // Send the message to the actuator
      int res = write_pb(write_stream, &lidar_msg);
      if (res <= 0) {
        printf("Could not write to actuator\n");
        return 1;
      }
    }

    if (ctrl_c_pressed){ 
      break;
    }
    
  }

  drv->stop();
  drv->setMotorSpeed(0);

  //cout << "Lidar driver created" << endl;
  delete drv;
  delete _channel;
  return 0;
}

// This is just a wrapper to run the user program
// it is not recommended to put any other logic here
int main() {
  return run(user_program);
}
