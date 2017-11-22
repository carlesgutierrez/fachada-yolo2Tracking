# fachada-yolo2Tracking

App yolo2 to detect persons and track them in real time.
Results are send by OSC protocol. 
Yolo2 uses GPU with Cuda8/9.

For installation:
 * Required addons: 
     - ofxDarknet: (https://github.com/mrzl/ofxDarknet)
              There are some more necessary steps that don't work with the OF project generator
              Compile as Debug or Release in x64 mode
              Within VS2015 Solution Explorer, rightclick on the generated project -> Build Dependencies -> Build Customizations -> Tick CUDA 8.0
              Copy pthreadVC2.dll from ofxDarknet\libs\3rdparty\dll\x64 to your applications bin folder

     
     - ofxCv: (https://github.com/kylemcdonald/ofxCv/tree/stable)
               - stable version and reset this propertie: c++ -> output files --> Object File name --> $(IntDir)/%(RelativeDir)/
               # require small hack: add this func as public:
              
               ```
               //Header Tracker.h
               vector<TrackedObject<T> > getCurrentRaw();

               //Source Tracker.cpp
               template<class T>
               inline vector<TrackedObject<T>> Tracker<T>::getCurrentRaw()
               {
                    return current;
               }
               ```

     - ofxOpenCv (internal OF)
     - ofxGui (internal OF)
     - ofxOsc (internal OF)
     
 * Require specific weights COCO  ( https://pjreddie.com/media/files/yolo.weights ) From Yolo: https://pjreddie.com/darknet/yolo/
    - cfg and names are already included in this repo- 
 
 
  
