# fachada-yolo2Tracking

App yolo2 to detect persons and track them in real time.
Results are send by OSC protocol. 
Yolo2 uses GPU with Cuda8/9.

For installation:
 * Required addons: 
     - ofxDarknet: read the Readme. 
     - ofxCv: stable version and reset this propertie: c++ -> output files --> Object File name --> $(IntDir)/%(RelativeDir)/
     - ofxOpenCv 
     - ofxGui
     - ofxOsc
     
 * Require speciful COCO files. Download from Yolo: https://pjreddie.com/darknet/yolo/
 
 
  
