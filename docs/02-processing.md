# Processing

This service follows the following steps:

1. The service reads configuration values for the speed and the mode. The speed can takes rpm values in the range of 300-900 with a default value of 600. If an incorrect value is inputed, it is handled by the driver implementation and it should just use the default one. The mode can be **0 (for Standard)**, **1 (for Express)** and **2 (for Boost)**. Then it creates a serial connection (`/dev/ttyUSB0`) with a baudrate of **115200**, from where it retrieves the device information and prints it.
2. The device can take up to 1024 measurement nodes per scan. The raw data is sorted then in ascending order by angle, then it converts the angles in degrees and the distance in millimeters. In the end the angle, distance, quality and start flag are sent in an array in a protobuf message with a timestamp.

![Example Lidar Plot](https://github.com/user-attachments/assets/b073d3a1-81de-4da2-ac49-d215327075f0)

3. Finally, the lidar scans are encoded in [`Scan` messages](https://github.com/VU-ASE/rovercom/blob/c1d6569558e26d323fecc17d01117dbd089609cc/definitions/outputs/lidar.proto#L13), which 
are encapsulated in a [`LidarSensorOutput` message](https://github.com/VU-ASE/rovercom/blob/c1d6569558e26d323fecc17d01117dbd089609cc/definitions/outputs/lidar.proto#L11). This message is written to the [`lidar-data` stream](https://github.com/VU-ASE/lidar/blob/b7bd85f99cab7a2d6beddb156aa743b05e94c351/service.yaml#L13C5-L13C15) 
    - Each `Scan` includes:
        1. `Angle`, a `float32` value representing the angle in degrees.
        2. `Distance`, a `float32` value representing the distance in millimeters.
        3. `Quality`, an `uint32` value indicating the measurement quality.
        4. `IsStart`, a `boolean` value indicating whether this measurement marks the start of a new scan.

4. **SIGINT** and **SIGTERM** signals are caught to stop the LIDAR motor and clean up all the memory allocated to the scans arrays. This ensures a safe shutdown of the service and the LIDAR sensor.

## Reference
- For more detailed specifications and additional information on the **SLAMTEC RPLIDAR A2M8** sensor, please refer to the official documentation: [RPLIDAR A Series Support](https://www.slamtec.com/en/Support#rplidar-a-series).
- For the source code and further details about the SDK, check out the GitHub repository: [slamtec/rplidar_sdk](https://github.com/slamtec/rplidar_sdk).
