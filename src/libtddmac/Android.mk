LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/../../include \
        external/libuiomux/include \

LOCAL_SRC_FILES := \
        tddmac.c

LOCAL_SHARED_LIBRARIES := libcutils libuiomux

LOCAL_MODULE := libtddmac
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
