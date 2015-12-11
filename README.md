### <b>RealSense Plugin for Unreal Engine 4</b>

#### Overview
The purpose of this plugin is to provide support for the RealSense SDK inside of the Unreal Engine 4 Editor. SDK features are exposed via Blueprint components and Blueprint functions, and can be referenced in C++ for developers who write code. 

Developers gain access to the SDK and its supported middleware through actor components, each of which encapsulate a specific set of features. For example, there is a Camera Streams Component that controls the resolution of the color and depth cameras and provides access to the raw image buffers. And there is a Scan 3D Component that provides support for the 3D Scanning middleware, including setting the scan configuration, accessing the preview image buffer, and saving the scan out to disk. For a complete list of featured and in-progress components, see the list below.

Multiple RealSense components of different types may be present in the same UE4 scene, as can multiple instances of the same component. This is possible because each component communicates with a global Session Manager Actor (GSM) that maintains a singular state of all RealSense settings and data. A GSM is instantiated automatically when the first RealSense component is created, and is virtually invisible to the developer. 

Behind the scenes, each component reads from and writes to the GSMr to ensure consistency across components. The GSM creates a dedicated thread that performs all RealSense processing, and the game thread polls for updates from this thread on every tick. This separation helps keep the game thread running quickly even when the RealSense thread is doing some heavy lifting.

- - -

Check out this tutorial to get started using the RealSense plugin: COMING SOON!

- - -

#### Hardware Requirements
* [F200 Front-Facing RealSense Camera](http://click.intel.com/intel-realsense-developer-kit.html) __OR__
* [R200 World-Facing RealSense Camera](http://click.intel.com/intel-realsense-developer-kit-r200.html)
* 4th Generation Intel(R) CPU or higher
* Intel Graphics supporting Open CL 1.2 (If using Scene Perception Component)

#### Software Requirements
* [Intel(R) RealSense(TM) SDK R4](https://software.intel.com/en-us/intel-realsense-sdk/download)
* Unreal Engine 4.8 (or higher)
* Windows 8.1 or 10
* Visual Studio 2013

#### Featured Components
* Camera Streams - Provides access to raw color and depth image buffers
* Scan 3D - Supports the scanning of faces and objects into 3D models

#### In-Progress Components
* Head Tracking - Provides data for the user's head position and rotation
* Scene Perception - Supports the scanning of small scenes into 3D models