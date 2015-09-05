/* This file is part of the UAF (Unified Architecture Framework) project.
 *
 * Copyright (C) 2012 Wim Pessemier (Institute of Astronomy, KULeuven)
 *
 * Project website: http://www.ster.kuleuven.be/uaf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UAF_SUBSCRIPTIONFACTORY_H_
#define UAF_SUBSCRIPTIONFACTORY_H_


// STD
#include <string>
#include <sstream>
#include <map>
// SDK
#include "uaclient/uaclientsdk.h"
#include "uaclient/uasession.h"
#include "uabase/uastring.h"
// UAF
#include "uaf/util/status.h"
#include "uaf/util/logger.h"
#include "uaf/util/handles.h"
#include "uaf/client/clientexport.h"
#include "uaf/client/subscriptions/subscription.h"
#include "uaf/client/clientinterface.h"





namespace uaf
{



    /*******************************************************************************************//**
    * An uaf::SubscriptionFactory creates and owns uaf::Subscription instances.
    *
    * @ingroup ClientSubscriptions
    ***********************************************************************************************/
    class UAF_EXPORT SubscriptionFactory : private UaClientSdk::UaSubscriptionCallback
    {
    public:


        /**
         * Construct a subscription factory.
         *
         * @param loggerFactory      Logger factory to log all messages to.
         * @param clientConnectionId Id of the session that owns this subscription factory.
         * @param uaSession          SDK session instance of the uaf::Session instance that owns
         *                           this subscription factory.
         * @param clientInterface    Client interface to call when asynchronous
         *                           communication is received.
         * @param database           Pointer to the client database.
         */
        SubscriptionFactory(
                uaf::LoggerFactory*             loggerFactory,
                uaf::ClientConnectionId         clientConnectionId,
                UaClientSdk::UaSession*         uaSession,
                uaf::ClientInterface*          clientInterface,
                uaf::Database*                 database);


        /**
         * Destruct the subscription factory.
         */
        virtual ~SubscriptionFactory();


        /**
         * Manually create a subscription.
         *
         * For more info about "manual" methods, see the documentation on the
         * uaf::Client::manuallyConnect method.
         *
         * @param settings              The settings of the subscription you'd like to create.
         * @param clientSubscriptionHandle  Output parameter, giving you the handle of the newly
         *                                  created subscription.
         * @return                      Good if the subscription was successfully created, Bad if
         *                              something went wrong.
         */
        uaf::Status manuallySubscribe(
                const uaf::SubscriptionSettings&   settings,
                uaf::ClientSubscriptionHandle&      clientSubscriptionHandle);

        /**
         * Manually delete a subscription.
         *
         * @param clientSubscriptionHandle  The handle of the subscription.
         * @return                          Good if the subscription could be deleted, bad if there
         *                                  was some problem.
         */
        uaf::Status manuallyUnsubscribe(uaf::ClientSubscriptionHandle clientSubscriptionHandle);


        /**
         * Get information about a particular subscription.
         *
         * @param clientSubscriptionHandle  The handle identifying the subscription.
         * @param subscriptionInformation   Output parameter, giving you the information about the
         *                                  subscription.
         * @return                          Good if the subscription could be found, Bad if not.
         */
        uaf::Status subscriptionInformation(
                 uaf::ClientSubscriptionHandle      clientSubscriptionHandle,
                 uaf::SubscriptionInformation&     subscriptionInformation);


        /**
         * Get information about all subscriptions.
         *
         * @return  A vector of all available SubscriptionInformation.
         */
        std::vector<uaf::SubscriptionInformation> allSubscriptionInformations();


        /**
         * Get some information about the specified monitored item.
         *
         * @param clientHandle                  The handle identifying the monitored item.
         * @param monitoredItemInformation      Output parameter: the requested information.
         * @return                              True if the monitored item was found, False if not.
         */
         bool monitoredItemInformation(
                uaf::ClientHandle               clientHandle,
                uaf::MonitoredItemInformation& monitoredItemInformation);


        /**
         * Set the publishing mode of a particular subscription.
         *
         * @param clientSubscriptionHandle  The handle identifying the subscription.
         * @param publishingEnabled         True to enable the subscription, false if not.
         * @param serviceSettings           The service settings to be used.
         * @param subscriptionFound         Output parameter, True if the subscription specified
         *                                  by the clientSubscriptionHandle was found, False if not.
         * @return                          Result of the service call.
         */
        uaf::Status setPublishingMode(
                uaf::ClientSubscriptionHandle  clientSubscriptionHandle,
                bool                           publishingEnabled,
                const uaf::ServiceSettings*    serviceSettings,
                bool&                          subscriptionFound);


       /**
        * Set the monitoring mode for the given ClientHandles, if they are owned by one of the
        * subscriptions.
        *
        * @param clientHandles     The ClientHandles of the monitored items to be affected.
        * @param monitoringMode    The new monitoring mode.
        * @param serviceSettings   The service settings to be used.
        * @param results           A vector of statuses (one result for each ClientHandle).
        * @return                  The immediate result of the service call.
        */
        uaf::Status setMonitoringModeIfNeeded(
               std::vector<uaf::ClientHandle>          clientHandles,
               uaf::monitoringmodes::MonitoringMode    monitoringMode,
               const uaf::ServiceSettings*            serviceSettings,
               std::vector<uaf::Status>&               results);


        /**
         * Execute a service invocation in a generic way.
         *
         * @tparam _Service      The service to be invoked (such as uaf::ReadService,
         *                       uaf::AsyncMethodCallService, etc.).
         * @param invocation     The invocation to be executed.
         * @param nameSpaceArray The name space array as fetched by the client.
         * @param serverArray    The server array as fetched by the client.
         */
        template<typename _Service>
        uaf::Status invokeService(
                typename _Service::Invocation&  invocation,
                const uaf::BaseSubscriptionRequest<typename _Service::Settings,
                                                   typename _Service::RequestTarget,
                                                   _Service::asynchronous>& request,
                const uaf::NamespaceArray&      nameSpaceArray,
                const uaf::ServerArray&         serverArray)
        {
            logger_->debug("Processing %sInvocation %d",
                           _Service::name().c_str(), invocation.requestHandle());

            uaf::Status ret;

            // if we need to invoke the request asynchronously, then generate a transaction id
            // first and map it to the request handle
            if (invocation.asynchronous())
                storeRequestHandle(invocation.requestHandle());

            // try to acquire a subscription for the given subscription settings
            uaf::Subscription* subscription = 0;

            if (request.clientSubscriptionHandleGiven)
            {
                ret = acquireExistingSubscription(request.clientSubscriptionHandle, subscription);
            }
            else if (request.subscriptionSettingsGiven)
            {
                ret = acquireSubscription(request.subscriptionSettings, subscription);
            }
            else
            {
                ret = acquireSubscription(
                        database_->clientSettings.defaultSubscriptionSettings,
                        subscription);
            }

             // check if the subscription was acquired
            if (ret.isGood())
            {
                // copy the subscription information to the invocation
                logger_->debug("Copying the subscription information to the invocation");
                invocation.setSubscriptionInformation(subscription->subscriptionInformation());

                if (subscription->isCreated())
                {
                    logger_->debug("Forwarding the invocation to subscription %d",
                                   subscription->clientSubscriptionHandle());
                    ret = subscription->invokeService(invocation, nameSpaceArray, serverArray);
                }
                else
                {
                    ret = uaf::SubscriptionNotCreatedError();
                }

                releaseSubscription(subscription);
            }

            return ret;
        }


    private:


        DISALLOW_COPY_AND_ASSIGN(SubscriptionFactory);


        /** private typedef: the number of activities (i.e. active service invocations). */
        typedef uint32_t Activity;

        // private typedef: a map to store all subscriptions
        typedef std::map<uaf::ClientSubscriptionHandle, uaf::Subscription*> SubscriptionMap;

        // private typedef: a map to store the activities
        typedef std::map<uaf::ClientSubscriptionHandle, Activity>            ActivityMap;

        // private typedef: a map to relate transaction ids with request handles
        typedef std::map<uaf::TransactionId, uaf::RequestHandle> TransactionMap;


        /**
         * Acquire a subscription with the given properties (by getting an existing one, or creating
         * a new one if a suitable one doesn't exist already).
         *
         * The 'subscription' pointer can be used safely as long as releaseSession() is not called
         * by the same thread.
         *
         * @param subscriptionSettings  Settings of the subscription to be acquired.
         * @param session               Pointer to the requested subscription.
         * @return                      Status object, will be erroneous in case no suitable
         *                              subscription could be provided via the 'session' argument.
         */
        uaf::Status acquireSubscription(
                const uaf::SubscriptionSettings&    subscriptionSettings,
                uaf::Subscription*&                 subscription);


        /**
         * Acquire an existing subscription with the given client connection ID.
         *
         * The 'subscription' pointer can be used safely as long as releaseSubscription() is not
         * called by the same thread.
         *
         * @param clientSubscriptionHandle  Client handle of the Subscription to be found.
         * @param subscription              Pointer to the requested subscription.
         * @return                          Status object, will be erroneous in case no suitable
         *                                  subscription  could be provided via the 'subscription'
         *                                  argument.
         */
        uaf::Status acquireExistingSubscription(
                uaf::ClientSubscriptionHandle  clientSubscriptionHandle,
                uaf::Subscription*&           subscription);


        /**
         * Release the subscription, so it can be garbage collected if necessary.
         *
         * Only call this function if the subscription was acquired, i.e. only if the status as
         * returned by acquireSubscription() was good!
         * After this function has been called, it is no longer safe to use the subscription
         * pointer!
         *
         * @param subscription  Pointer to the subscription which we like to release.
         * @return              Good unless the subscription could not be released because it's
         *                      already fully released (i.e. its semaphore-like activity count is
         *                      already zero). This means that a programming bug is present, most
         *                      likely in the subscription factory!
         */
        uaf::Status releaseSubscription(
                uaf::Subscription*&    subscription,
                bool                    allowGarbageCollection=true);


        // pointer to the SDK session instance of the uaf::Session instance that owns
        // this subscription factory.
        UaClientSdk::UaSession* uaSession_;
        // the connection ID of the uaf::Session instance that owns this subscription factory.
        uaf::ClientConnectionId clientConnectionId_;
        // logger of the subscription factory
        uaf::Logger* logger_;
        // pointer to the client database
        uaf::Database* database_;
        // the map storing all subscriptions, and its mutex
        SubscriptionMap subscriptionMap_;
        UaMutex         subscriptionMapMutex_;
        // map storing all activity counts, and its mutex
        ActivityMap activityMap_;
        UaMutex     activityMapMutex_;
        // the mutex to safeguard the critical sections
        UaMutex subscriptionMutex_;
        // the RequesterInterface to call when asynchronous messages are received
        uaf::ClientInterface* clientInterface_;
        // the current transaction id (gets incremented every time), and its mutex
        uaf::TransactionId transactionId_;
        UaMutex            transactionIdMutex_;
        // the container that stores the transactions, and its mutex
        TransactionMap transactionMap_;
        UaMutex        transactionMapMutex_;


        /**
         * Called every time the subscription status has changed,
         * overridden from UaSubscriptionCallback.
         */
        virtual void subscriptionStatusChanged(
                OpcUa_UInt32    clientSubscriptionHandle,
                const UaStatus& status);


        /**
         * Called every time a keep alive message (sort of heartbeat) is received,
         * overridden from UaSubscriptionCallback.
         */
        virtual void keepAlive(OpcUa_UInt32 clientSubscriptionHandle);


        /**
         * Called every time notifications are missing,
         * overridden from UaSubscriptionCallback.
         */
        virtual void notificationsMissing(
                OpcUa_UInt32 clientSubscriptionHandle,
                OpcUa_UInt32 previousSequenceNumber,
                OpcUa_UInt32 newSequenceNumber);


        /**
         * Called every time a monitored item has changed,
         * overridden from UaSubscriptionCallback.
         */
        virtual void dataChange(
                OpcUa_UInt32 clientSubscriptionHandle,
                const UaDataNotifications &dataNotifications,
                const UaDiagnosticInfos &diagnosticInfos);


        /**
         * Called every new events have been received,
         * overridden from UaSubscriptionCallback.
         */
        virtual void newEvents(
                OpcUa_UInt32        clientSubscriptionHandle,
                UaEventFieldLists&  eventFieldList);


        /**
         * Called every time monitored items have been created,
         * overridden from UaSubscriptionCallback.
         */
        virtual void createMonitoredItemsComplete(
                OpcUa_UInt32 transactionId,
                const UaStatus &result,
                const UaMonitoredItemCreateResults &createResults,
                const UaDiagnosticInfos &diagnosticInfos);


        /**
         * Called every time monitored items have been modified,
         * overridden from UaSubscriptionCallback.
         */
        virtual void modifyMonitoredItemsComplete(
                OpcUa_UInt32 transactionId,
                const UaStatus &result,
                const UaMonitoredItemModifyResults &modifyResults,
                const UaDiagnosticInfos &diagnosticInfos);


        /**
         * Called every time the monitoring mode of monitored items has changed,
         * overridden from UaSubscriptionCallback.
         */
        virtual void setMonitoringModeComplete(
                OpcUa_UInt32 transactionId,
                const UaStatus &result,
                const UaStatusCodeArray &setModeResults,
                const UaDiagnosticInfos &diagnosticInfos);


        /**
         * Called every time monitored items have been deleted,
         * overridden from UaSubscriptionCallback.
         */
        virtual void deleteMonitoredItemsComplete(
                OpcUa_UInt32 transactionId,
                const UaStatus &result,
                const UaStatusCodeArray &deleteResults,
                const UaDiagnosticInfos &diagnosticInfos);


        /**
         * Get a new transaction id
         *
         * @return      A unique transaction ID.
         */
        uaf::TransactionId getNewTransactionId_();


        /**
         * Store a request handle.
         *
         * @param requestHandle The request handle to store.
         */
        void storeRequestHandle(uaf::RequestHandle requestHandle)
        {
            transactionMapMutex_.lock();
            transactionMap_[getNewTransactionId_()]  = requestHandle;
            transactionMapMutex_.unlock();
        }



    };
}





#endif /* UAF_SUBSCRIPTIONFACTORY_H_ */
