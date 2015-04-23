LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#OPENCV_CAMERA_MODULES:=off
#OPENCV_INSTALL_MODULES:=off
#OPENCV_LIB_TYPE:=SHARED
include /home/cip/nf/ar79yxiw/ciptmp/BA/opencv-android-sdk-2.4.10/native/jni/OpenCV.mk

LOCAL_SRC_FILES  := GoBoardReaderNative.cpp  intersectionDetection.cpp  lineDetection.cpp  util.cpp backported/lsd.cpp pieceDetection.cpp gapsFilling.cpp evaluation.cpp colorDetection.cpp boardSegmenter.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_LDLIBS     += -llog -ldl

LOCAL_MODULE     := goboardreader

include $(BUILD_SHARED_LIBRARY)
