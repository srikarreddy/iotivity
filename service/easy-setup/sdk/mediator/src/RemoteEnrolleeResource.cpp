//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <functional>
#include <time.h>

#include "RemoteEnrolleeResource.h"

#include "OCPlatform.h"
#include "ESException.h"
#include "OCResource.h"
#include "logger.h"

namespace OIC
{
    namespace Service
    {
        #define ES_REMOTE_ENROLLEE_RES_TAG "ES_REMOTE_ENROLLEE_RES"
        #define DISCOVERY_TIMEOUT 5

        static const char ES_BASE_RES_URI[] = "/oic/res";
        static const char ES_PROV_RES_URI[] = "/oic/prov";
        static const char ES_PROV_RES_TYPE[] = "oic.r.prov";

        RemoteEnrolleeResource::RemoteEnrolleeResource(EnrolleeNWProvInfo enrolleeNWProvInfo)
        {
            m_enrolleeNWProvInfo = enrolleeNWProvInfo;
            m_discoveryResponse = false;
        }

        void RemoteEnrolleeResource::checkProvInformationCb(const HeaderOptions& /*headerOptions*/,
                const OCRepresentation& rep, const int eCode)
        {
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "checkProvInformationCb : %s, eCode = %d",
                    rep.getUri().c_str(),
                    eCode);

            if (eCode != 0)
            {
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                        "checkProvInformationCb : Provisioning is failed ");
                std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                        ProvisioningStatus >(ESResult::ES_ERROR, ESState::ES_PROVISIONING_ERROR);
                m_provStatusCb(provStatus);
                return;
            }

            int ps = -1;
            std::string tnn = "";
            std::string cd = "";

            rep.getValue(OC_RSRVD_ES_PS, ps);
            rep.getValue(OC_RSRVD_ES_TNN, tnn);
            rep.getValue(OC_RSRVD_ES_CD, cd);

            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "checkProvInformationCb : ps - %d", ps);
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                    "checkProvInformationCb : tnn - %s", tnn.c_str());
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                    "checkProvInformationCb : cd - %s", cd.c_str());

            //Provisioning status check
            if (ps == ES_PS_PROVISIONING_COMPLETED)
            {
                if (tnn != std::string(m_enrolleeNWProvInfo.netAddressInfo.WIFI.ssid))
                {
                    OC_LOG_V (ERROR, ES_REMOTE_ENROLLEE_RES_TAG,
                            "checkProvInformationCb : Network SSID is not the same as the "
                            "SSID provisioned");
                    std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                            ProvisioningStatus >(ESResult::ES_ERROR,
                            ESState::ES_PROVISIONING_ERROR);
                    m_provStatusCb(provStatus);
                    return;
                }

                if (cd != std::string(m_enrolleeNWProvInfo.netAddressInfo.WIFI.pwd))
                {
                    OC_LOG_V (ERROR, ES_REMOTE_ENROLLEE_RES_TAG,
                            "checkProvInformationCb : Network PWD is not the same as the "
                            "PWD provisioned");
                    std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                            ProvisioningStatus >(ESResult::ES_ERROR,
                            ESState::ES_PROVISIONING_ERROR);
                    m_provStatusCb(provStatus);
                    return;
                }

                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                        "checkProvInformationCb : Provisioning is success ");
                std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                        ProvisioningStatus >(ESResult::ES_OK, ESState::ES_PROVISIONING_SUCCESS);
                m_provStatusCb(provStatus);
                return;
            }
            else
            {
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                        "checkProvInformationCb : Provisioning is failed ");
                std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                        ProvisioningStatus >(ESResult::ES_ERROR, ESState::ES_PROVISIONING_ERROR);
                m_provStatusCb(provStatus);
                return;
            }
        }

        void RemoteEnrolleeResource::getProvStatusResponse(const HeaderOptions& /*headerOptions*/,
                const OCRepresentation& rep, const int eCode)
        {
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "getProvStatusResponse : %s, eCode = %d",
                    rep.getUri().c_str(),
                    eCode);

            if (eCode != 0)
            {
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                        "getProvStatusResponse : Provisioning is failed ");
                std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                        ProvisioningStatus >(ESResult::ES_ERROR, ESState::ES_PROVISIONING_ERROR);
                m_provStatusCb(provStatus);
                return;
            }

            int ps = -1;
            std::string tnn = "";
            std::string cd = "";

            rep.getValue(OC_RSRVD_ES_PS, ps);
            rep.getValue(OC_RSRVD_ES_TNN, tnn);
            rep.getValue(OC_RSRVD_ES_CD, cd);

            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "getProvStatusResponse : ps - %d",
                    ps);
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "getProvStatusResponse : tnn - %s",
                    tnn.c_str());
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "getProvStatusResponse : cd - %s",
                    cd.c_str());

            if (ps == ES_PS_NEED_PROVISIONING) //Indicates the need for provisioning
            {
                OCRepresentation provisioningRepresentation;

                provisioningRepresentation.setValue(OC_RSRVD_ES_TNN,
                std::string(m_enrolleeNWProvInfo.netAddressInfo.WIFI.ssid));
                provisioningRepresentation.setValue(OC_RSRVD_ES_CD,
                std::string(m_enrolleeNWProvInfo.netAddressInfo.WIFI.pwd));

                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "getProvStatusResponse : ssid - %s",
                        m_enrolleeNWProvInfo.netAddressInfo.WIFI.ssid);
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "getProvStatusResponse : pwd - %s",
                        m_enrolleeNWProvInfo.netAddressInfo.WIFI.pwd);

                m_ocResource->put(provisioningRepresentation, QueryParamsMap(),
                        std::function<
                                void(const HeaderOptions& headerOptions,
                                        const OCRepresentation& rep, const int eCode) >(
                        std::bind(&RemoteEnrolleeResource::checkProvInformationCb, this,
                        std::placeholders::_1, std::placeholders::_2,
                        std::placeholders::_3)));
            }
            else if (ps == ES_PS_PROVISIONING_COMPLETED) //Indicates that provisioning is completed
            {
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                        "getProvStatusResponse : Provisioning is successful");
                std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                        ProvisioningStatus >(ESResult::ES_OK, ESState::ES_PROVISIONED_ALREADY);
                m_provStatusCb(provStatus);
            }
        }

        void RemoteEnrolleeResource::registerProvStatusCallback(ProvStatusCb provStatusCb)
        {
            m_provStatusCb = provStatusCb;
        }

        ESResult RemoteEnrolleeResource::ESDiscoveryTimeout(unsigned short waittime)
        {
            struct timespec startTime;
            startTime.tv_sec=0;
            startTime.tv_sec=0;
            struct timespec currTime;
            currTime.tv_sec=0;
            currTime.tv_nsec=0;

            ESResult res = ES_OK;
            #ifdef _POSIX_MONOTONIC_CLOCK
                int clock_res = clock_gettime(CLOCK_MONOTONIC, &startTime);
            #else
                int clock_res = clock_gettime(CLOCK_REALTIME, &startTime);
            #endif

            if (0 != clock_res)
            {
                return ES_ERROR;
            }

            while (ES_OK == res || m_discoveryResponse == false)
            {
                #ifdef _POSIX_MONOTONIC_CLOCK
                        clock_res = clock_gettime(CLOCK_MONOTONIC, &currTime);
                #else
                        clock_res = clock_gettime(CLOCK_REALTIME, &currTime);
                #endif

                if (0 != clock_res)
                {
                    return ES_ERROR;
                }
                long elapsed = (currTime.tv_sec - startTime.tv_sec);
                if (elapsed > waittime)
                {
                    return ES_OK;
                }
                if (m_discoveryResponse)
                {
                    res = ES_OK;
                }
             }
             return res;
        }

        void RemoteEnrolleeResource::onDeviceDiscovered(std::shared_ptr<OC::OCResource> resource)
        {
            OC_LOG (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "onDeviceDiscovered");

            std::string resourceURI;
            std::string hostAddress;
            try
            {
                if(resource)
                {
                    // Get the resource URI
                    resourceURI = resource->uri();
                    OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                            "URI of the resource: %s", resourceURI.c_str());

                    // Get the resource host address
                    hostAddress = resource->host();
                    OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                            "Host address of the resource: %s", hostAddress.c_str());

                    std::size_t foundIP =
                        hostAddress.find(
                                std::string(m_enrolleeNWProvInfo.netAddressInfo.WIFI.ipAddress));

                    if(resourceURI == ES_PROV_RES_URI && foundIP!=std::string::npos)
                    {
                        m_ocResource = resource;
                        m_discoveryResponse = true;

                        OC_LOG (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                                "Found the device with the resource");

                        return;
                    }
                    else
                    {
                        OC_LOG (ERROR, ES_REMOTE_ENROLLEE_RES_TAG, "NOT the intended resource.");
                    }
                }
                else
                {
                    OC_LOG (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "Resource is invalid");
                }

            }
            catch(std::exception& e)
            {
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                        "Exception in foundResource: %s", e.what());
            }
        }


        ESResult RemoteEnrolleeResource::constructResourceObject()
        {
            if (m_ocResource != nullptr)
            {
                throw ESBadRequestException("Remote resource is already created");
            }

#ifdef REMOTE_ARDUINO_ENROLEE
            //This process will create OCResource with port 55555 which is specific
            // to Arduino WiFi enrollee
            try
            {

                std::vector< std::string > interface =
                {   DEFAULT_INTERFACE};
                std::vector< std::string > resTypes =
                {   ES_PROV_RES_TYPE};

                OC_LOG(DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "Before OCPlatform::constructResourceObject");

                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "m_host = %s",
                        m_enrolleeNWProvInfo.netAddressInfo.WIFI.ipAddress);
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "ES_PROV_RES_URI = %s", ES_PROV_RES_URI);
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "m_connectivityType = %d",
                        m_enrolleeNWProvInfo.connType);
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "resTypes = %s",
                        resTypes.at(0).c_str());
                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "interface = %s", interface.at(0).c_str());

                std::string host;
                if(m_enrolleeNWProvInfo.needSecuredEasysetup)
                {
                    host.append("coaps://");
                }
                else
                {
                    host.append("coap://");
                }

                if(m_enrolleeNWProvInfo.connType == CT_ADAPTER_IP)
                {
                    // TODO : RemoteEnrollee is current handling easysetup on IP transport.
                    // WiFiRemoteEnrollee need to extend RemoteEnrollee for providing IP specific
                    // Enrollee easysetup.

                    host.append(m_enrolleeNWProvInfo.netAddressInfo.WIFI.ipAddress);
                    //TODO : If the target Enrollee is not a Arduino Wi-Fi device,
                    // then the port number will be found during resource discovery instead of
                    // using 55555
                    host.append(":55555");
                }

                OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "HOST = %s", host.c_str());

                m_ocResource = OC::OCPlatform::constructResourceObject(host,
                        ES_PROV_RES_URI,
                        m_enrolleeNWProvInfo.connType,
                        true,
                        resTypes,
                        interface);
                OC_LOG_V(DEBUG, ES_REMOTE_ENROLLEE_RES_TAG,
                        "created OCResource : %s", m_ocResource->uri().c_str());

                return ES_OK;
            }
            catch (OCException & e)
            {
                OC_LOG_V(ERROR, ES_REMOTE_ENROLLEE_RES_TAG,
                        "Exception for constructResourceObject : %s", e.reason().c_str());
            }

#else
            std::string host("");
            std::string query("");

            if (m_enrolleeNWProvInfo.needSecuredEasysetup)
            {
                host.append("coaps://");
            }
            else
            {
                host.append("coap://");
            }

            if (m_enrolleeNWProvInfo.connType == CT_ADAPTER_IP)
            {
                // TODO : RemoteEnrollee is current handling easysetup on IP transport.
                // WiFiRemoteEnrollee need to extend RemoteEnrollee for providing IP specific
                // Enrollee easysetup.

                host.append(m_enrolleeNWProvInfo.netAddressInfo.WIFI.ipAddress);
            }

            query.append(ES_BASE_RES_URI);
            query.append("?rt=");
            query.append(ES_PROV_RES_TYPE);

            OC_LOG(DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "Before OCPlatform::constructResourceObject");

            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "host = %s",
                    host.c_str());
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "query = %s", query.c_str());
            OC_LOG_V (DEBUG, ES_REMOTE_ENROLLEE_RES_TAG, "m_connectivityType = %d",
                    m_enrolleeNWProvInfo.connType);

            m_discoveryResponse = false;
            std::function< void (std::shared_ptr<OC::OCResource>) > onDeviceDiscoveredCb =
                    std::bind(&RemoteEnrolleeResource::onDeviceDiscovered, this,
                                                    std::placeholders::_1);
            OCStackResult result = OC::OCPlatform::findResource("", query, CT_DEFAULT,
                    onDeviceDiscoveredCb);

            if (result != OCStackResult::OC_STACK_OK)
            {
                OC_LOG(ERROR,ES_REMOTE_ENROLLEE_RES_TAG,
                        "Failed to create device using constructResourceObject");
                return ES_ERROR;
            }


            ESResult foundResponse = ESDiscoveryTimeout (DISCOVERY_TIMEOUT);

            if (!m_discoveryResponse)
            {
                OC_LOG(ERROR,ES_REMOTE_ENROLLEE_RES_TAG,
                        "Failed to create device using constructResourceObject");
                return ES_ERROR;
            }

            return ES_OK;
#endif
        }

        void RemoteEnrolleeResource::provisionEnrollee()

        {
            if (m_ocResource == nullptr)
            {
                throw ESBadRequestException("Resource is not initialized");
            }

            OC::QueryParamsMap query;
            OC::OCRepresentation rep;

            std::function< OCStackResult(void) > getProvisioingStatus = [&]
            {   return m_ocResource->get(m_ocResource->getResourceTypes().at(0),
                        m_ocResource->getResourceInterfaces().at(0), query,
                        std::function<
                        void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                const int eCode) >(
                                std::bind(&RemoteEnrolleeResource::getProvStatusResponse, this,
                                        std::placeholders::_1, std::placeholders::_2,
                                        std::placeholders::_3)));
            };

            OCStackResult result = getProvisioingStatus();

            if (result != OCStackResult::OC_STACK_OK)
            {
                std::shared_ptr< ProvisioningStatus > provStatus = std::make_shared<
                        ProvisioningStatus >(ESResult::ES_ERROR, ESState::ES_PROVISIONING_ERROR);
                m_provStatusCb(provStatus);
                return;
            }
        }

        void RemoteEnrolleeResource::unprovisionEnrollee()
        {
            if (m_ocResource == nullptr)
            {
                throw ESBadRequestException("Resource is not initialized");
            }

            OCRepresentation provisioningRepresentation;

            provisioningRepresentation.setValue(OC_RSRVD_ES_TNN, "");
            provisioningRepresentation.setValue(OC_RSRVD_ES_CD, "");

            m_ocResource->post(provisioningRepresentation, QueryParamsMap(),
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                    std::bind(&RemoteEnrolleeResource::checkProvInformationCb, this,
                    std::placeholders::_1, std::placeholders::_2,
                    std::placeholders::_3)));
        }
    }
}