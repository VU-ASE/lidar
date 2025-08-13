import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

# Overview

## Purpose 

The `lidar` service uses the **SLAMTEC RPLIDAR A2M8** sensor to capture and process 2D laser scan data.

:::info

This service requires an extra sensor that does not come preinstalled with every Rover. See the [requirements](#requirements) section.

:::

## Installation

To install this service, the latest release of [`roverctl`](https://ase.vu.nl/docs/framework/Software/rover/roverctl/installation) should be installed for your system and your Rover should be powered on.

<Tabs groupId="installation-method">
<TabItem value="roverctl" label="Using roverctl" default>

1. Clone this repository to your machine
```bash
git clone https://github.com/VU-ASE/lidar.git
```
2. Enter the newly created *lidar* directory
```bash
cd lidar
```
3. Upload the *lidar* service to your Rover. Do not forget the trailing `.`!
```bash
# Replace ROVER_NUMBER with your the number label on your Rover (e.g. 7)
roverctl upload -r <ROVER_NUMBER> .
```

</TabItem>
<TabItem value="roverctl-web" label="Using roverctl-web">

This service is not released automatically so it cannot be installed through `roverctl-web`.

</TabItem>
</Tabs>

Follow [this tutorial](https://ase.vu.nl/docs/tutorials/write-a-service/upload) to understand how to use an ASE service. You can find more useful `roverctl` commands [here](/docs/framework/Software/rover/roverctl/usage)

## Requirements

- A SLAMTEC RPLIDAR A2M8 needs to be connected over USB for this service to work

## Inputs

As defined in the [*service.yaml*](https://github.com/VU-ASE/lidar/blob/main/service.yaml), this service does not depend on any other service.

## Outputs

As defined in the [*service.yaml*](https://github.com/VU-ASE/lidar/blob/main/service.yaml), this service exposes the following write streams:

- `lidar-data`:
    - To this stream, [`LidarSensorOutput`](https://github.com/VU-ASE/rovercom/blob/c1d6569558e26d323fecc17d01117dbd089609cc/definitions/outputs/lidar.proto#L11) messages will be written. Each message will be wrapped in a [`SensorOutput` wrapper message](https://github.com/VU-ASE/rovercom/blob/main/definitions/outputs/wrapper.proto)
