/*
 *  Copyright 2013 People Power Company
 *  
 *  This code was developed with funding from People Power Company
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>

#include "ioterror.h"
#include "iotdebug.h"
#include "iotapi.h"
#include "iotxmlgen.h"
#include "timestamp.h"


/** Copy of the last Device ID we generated XML for */
static char lastDeviceId[IOTGEN_DEVICE_ID_SIZE];

/** Copy of the last param type we generated XML for */
static int lastParamType = -1;

/** True if there is a message currently being constructed */
static bool inProgress = false;

/**
 * param_type_e's map to these key words recognized by the server in XML
 */
static const char *paramTypeMap[] = {
    "profile",
    "measure",
    "alert",
};

/***************** Prototypes ****************/

/***************** Public IOT XML Generator ****************/
/**
 * Begin a new message. This locks out other callers to iotxml_newMsg(..)
 * until the current message has been sent, because the message is constructed
 * based on the context of the parameters that are being passed in across
 * mutiple function calls.
 *
 * @param dest the destination buffer for the message
 * @param maxSize the maximum size of the message
 * @return SUCCESS if we started the new message, FAIL if another message is
 *     already being constructed and needs to be sent.
 */
error_t iotxml_newMsg(char *destMsg, int maxSize) {
  if(inProgress) {
    return FAIL;
  }

  inProgress = true;

  bzero(lastDeviceId, sizeof(lastDeviceId));
  bzero(destMsg, maxSize);
  lastParamType = -1;
  return SUCCESS;
}

/**
 * Add a string to the message. This function operates similar to snprintf,
 * for example:
 *
 * <code>
 *   offset += iotxml_addString(destMsg + offset, sizeof(destMsg) - offset, ...)
 * </code>
 *
 * @param dest Starting point in a destination buffer to write data
 * @param maxSize maximum size remaining past the start point
 * @param deviceId Device ID string
 * @param deviceType Device type that is registered with the cloud service
 * @param paramType Type of parameter, see param_type_e enum in iotapi.h
 * @param asciiParamIndex Index of this parameter, for instance if you are
 *     measuring watts from a powerstrip with multiple sockets, where each
 *     socket has its own index number.  Use NULL or 0 if your param doesn't
 *     need an index.
 * @param paramValue Param value string
 * @return the size of the string written to the destination pointer
 */
int iotxml_addString(char *dest, int maxSize, const char *deviceId, int deviceType, param_type_e paramType, const char *paramName, const char *multiplier, char asciiParamIndex, const char *paramValue) {
  int offset = 0;

  // First check if we need a new param block
  if(strcmp(deviceId, lastDeviceId) != 0 || lastParamType != paramType) {
    // We need to start a new tag
    if(lastParamType >= 0) {
      // But first we need to close off the last tag
      offset += snprintf(dest + offset, maxSize - offset,
          "</%s>",
          paramTypeMap[lastParamType]);
    }

    lastParamType = paramType;
    strncpy(lastDeviceId, deviceId, sizeof(lastDeviceId));

    offset += snprintf(dest + offset, maxSize - offset,
        "<%s deviceId=\"%s\" timestamp=\"",
        paramTypeMap[lastParamType],
        deviceId);

    offset += getTimestamp(dest + offset, maxSize - offset);

    offset += snprintf(dest + offset, maxSize - offset, "\">");
  }

  // Next add in the new param
  offset += snprintf(dest + offset, maxSize - offset,
      "<param name=\"%s\"", paramName);

  // Give it an index # if a valid ASCII paramIndex was given
  if(asciiParamIndex >= '0') {
    offset += snprintf(dest + offset, maxSize - offset,
        " index=\"%c\"", asciiParamIndex);
  }

  if(multiplier != NULL) {
    if(strlen(multiplier) > 0) {
      offset += snprintf(dest + offset, maxSize - offset,
          " multiplier=\"%s\"", multiplier);
    }
  }

  // Add the value
  offset += snprintf(dest + offset, maxSize - offset,
      ">%s</param>", paramValue);

  return offset;
}

/**
 * Add an int to the message. This function operates similar to snprintf,
 * for example:
 *
 * <code>
 *   offset += iotxml_addString(destMsg + offset, sizeof(destMsg) - offset, ...)
 * </code>
 *
 * @param dest Starting point in a destination buffer to write data
 * @param maxSize maximum size remaining past the start point
 * @param deviceId Device ID string
 * @param deviceType Device type that is registered with the cloud service
 * @param paramType Type of parameter, see param_type_e enum in iotapi.h
 * @param asciiParamIndex Index of this parameter, for instance if you are
 *     measuring watts from a powerstrip with multiple sockets, where each
 *     socket has its own index number.  Use NULL or 0 if your param doesn't
 *     need an index.
 * @param paramValue Param value integer
 * @return the size of the string written to the destination pointer
 */
int iotxml_addInt(char *dest, int maxSize, const char *deviceId, int deviceType, param_type_e paramType, const char *paramName, const char *multiplier, char asciiParamIndex, int paramValue) {
  char value[IOTGEN_NUMERIC_STRING_SIZE];
  snprintf(value, IOTGEN_NUMERIC_STRING_SIZE, "%d", paramValue);
  return iotxml_addString(dest, maxSize, deviceId, deviceType, paramType, paramName, multiplier, asciiParamIndex, value);
}

/**
 * Close off the last tag and send the message. This will allow other message
 * creating functions to create a new message using iotxml_newMsg(...)
 *
 * @param destMsg Pointer to the start of the destination message
 * @param maxSize Maximum size of the message buffer
 */
error_t iotxml_send(char *destMsg, int maxSize) {
  int totalSize = strlen(destMsg);

  if(lastParamType >= 0) {
    // Close off the last tag
    snprintf(destMsg + strlen(destMsg), maxSize - totalSize,
        "</%s>",
        paramTypeMap[lastParamType]);
  }

  SYSLOG_INFO("Send: %s\n", destMsg);
  lastParamType = -1;
  inProgress = false;
  return application_send(destMsg, strlen(destMsg));
}

/**
 * Abort the current message.  This is useful when we start a message to send
 * data, but then find out after iterating through our devices that we
 * have no data to send.
 */
void iotxml_abortMsg() {
  inProgress = false;
}

/**
 * Create XML to represent and convey the result of a command back to the server
 * @param commandId The command ID as the server presented to us
 * @param result The result code for the command
 * @return SUCCESS if we will send the response back to the server
 */
error_t iotxml_sendResult(int commandId, result_code_e result) {
  char xmlResult[IOTGEN_RESULT_XML_SIZE];
  bzero(xmlResult, IOTGEN_RESULT_XML_SIZE);
  snprintf(xmlResult, IOTGEN_RESULT_XML_SIZE, "<response cmdId=\"%d\" result=\"%d\"/>", commandId, result);
  SYSLOG_INFO("Sending result: %s", xmlResult);
  return application_send(xmlResult, strlen(xmlResult));
}

/**
 * Declare to the ESP that a new device is available to control
 * @param deviceId The unique ID of the device
 * @param deviceType The device type
 * @return SUCCESS if we will declare the device to the server
 */
error_t iotxml_addDevice(const char *deviceId, int deviceType) {
  char xml[IOTGEN_ADD_REMOVE_XML_SIZE];
  bzero(xml, IOTGEN_ADD_REMOVE_XML_SIZE);
  snprintf(xml, IOTGEN_ADD_REMOVE_XML_SIZE, "<add deviceId=\"%s\" deviceType=\"%d\" />", deviceId, deviceType);
  SYSLOG_INFO("Adding device %s of type %d", deviceId, deviceType);
  return application_send(xml, strlen(xml));
}

/**
 * Alert to the ESP that a device can no longer be contacted and may not be in
 * control of the network
 * @param deviceId The unique ID of the device
 * @return SUCCESS if we will declare the device is not present to the server
 */
error_t iotxml_alertDeviceIsGone(const char *deviceId) {
  char xml[IOTGEN_ADD_REMOVE_XML_SIZE];
  bzero(xml, IOTGEN_ADD_REMOVE_XML_SIZE);
  snprintf(xml, IOTGEN_ADD_REMOVE_XML_SIZE, "<alert deviceId=\"%s\" type=\"noRead\" />", deviceId);
  SYSLOG_INFO("Alerting that device %s is gone", deviceId);
  return application_send(xml, strlen(xml));
}

/**
 * Push measurements now.  This is a cloud- and iotsdk-friendly way to get
 * a p="1" string through a few sockets and buffers into the outbound message,
 * which will trigger the proxy to dump the entire contents of the buffer now.
 * This works because the server ignores the p="1" attribute
 *
 * @param deviceId the unique ID of the measurement to push now
 * @return SUCCESS if we will push the measurement now
 */
error_t iotxml_pushMeasurementNow(const char *deviceId) {
  char xml[IOTGEN_ADD_REMOVE_XML_SIZE];
  bzero(xml, IOTGEN_ADD_REMOVE_XML_SIZE);
  snprintf(xml, IOTGEN_ADD_REMOVE_XML_SIZE, "<measure deviceId=\"%s\" p=\"1\" />", deviceId);
  SYSLOG_INFO("Pushing measurement for device %s now", deviceId);
  return application_send(xml, strlen(xml));
}
