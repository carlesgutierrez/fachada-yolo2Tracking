# fachada-yolo2Tracking

App yolo2 to detect persons and track them in real time.
Results are send by OSC protocol. 
Yolo2 uses GPU with Cuda8/9.

For installation:
 * Required addons: 
     - ofxDarknet: Follow setup stepts for your platform from then Readme. README.md 
     - ofxCv: 
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

     - ofxOpenCv 
     - ofxGui
     - ofxOsc
     
 * Require specific weights COCO  ( https://pjreddie.com/media/files/yolo.weights ) From Yolo: https://pjreddie.com/darknet/yolo/
    - cfg and names are already included in this repo- 
 
 
  
