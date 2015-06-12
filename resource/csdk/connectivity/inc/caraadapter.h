/******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
******************************************************************/

/**
 * @file caraadapter.h
 * @brief This file contains the APIs for IP Adapter.
 */
#ifndef CA_RA_ADAPTER_H_
#define CA_RA_ADAPTER_H_

#include "cacommon.h"
#include "caadapterinterface.h"
#include "cathreadpool.h"

#ifdef __cplusplus
extern "C"
{
#endif

// TODO: Change to real values when available
#define XMPP_SERVER_HOST "some host"
#define XMPP_SERVER_PORT 5222
#define XMPP_SERVER_DOMAIN "some domain"

/**
 * @brief API to initialize RA Interface.
 * @param registerCallback      [IN] Callback to register RA interfaces to Connectivity
 *                                   Abstraction Layer
 * @param networkPacketCallback [IN] Callback to notify request and response messages from server(s)
 *                                   started at Connectivity Abstraction Layer.
 * @param netCallback           [IN] Callback to notify the network additions to Connectivity
 *                                   Abstraction Layer.
 * @param handle                [IN] Threadpool Handle
 * @return  #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAInitializeRA(CARegisterConnectivityCallback registerCallback,
                          CANetworkPacketReceivedCallback networkPacketCallback,
                          CANetworkChangeCallback netCallback,
                          ca_thread_pool_t handle);


/**
 * @brief Start RA Interface adapter.
 * @return  #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAStartRA();

/**
 * @brief Start listening server for receiving multicast search requests
 * Transport Specific Behavior:
 * RA Starts Multicast Server on a particular interface and prefixed port number and
 * as per OIC Specification.
 * @return  #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAStartRAListeningServer();

/**
 * @brief Start discovery servers for receiving multicast advertisements
 * Transport Specific Behavior:
 * RA Starts Start multicast server on a particular interface and prefixed port
 * number as per OIC Specification
 * @return  #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAStartRADiscoveryServer();

/**
 * @brief Sends data to the endpoint using the adapter connectivity.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port,
 * reference uri and transport type) to which the unicast data has to be sent.
 * @param   data        [IN]    Data which is required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return The number of bytes sent on the network. Return value equal to -1 indicates error.
 * @remarks dataLen must be > 0.
 */
int32_t CASendRAUnicastData(const CARemoteEndpoint_t *endpoint, const void *data,
                            uint32_t dataLen);

/**
 * @brief Sends Multicast data to the endpoint using the RA connectivity.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return The number of bytes sent on the network. Return value equal to -1 indicates error.
 * @remarks dataLen must be > 0.
 */
int32_t CASendRAMulticastData(const void *data, uint32_t dataLen);

/**
 * @brief Get RA Connectivity network information
 * @param   info        [OUT]   Local connectivity information structures
 * @param   size        [OUT]   Number of local connectivity structures.
 * @return  #CA_STATUS_OK or Appropriate error code
 * @remarks info is allocated in this API and should be freed by the caller.
 */
CAResult_t CAGetRAInterfaceInformation(CALocalConnectivity_t **info, uint32_t *size);

/**
 * @brief Read Synchronous API callback.
 * @return  #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAReadRAData();

/**
 * @brief Stops Unicast, Multicast servers and close the sockets.
 * @return  #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAStopRA();

/**
 * @brief Terminate the RA connectivity adapter.
 * Configuration information will be deleted from further use
 * @return  NONE
 */
void CATerminateRA();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // #ifndef CA_RA_ADAPTER_H_
