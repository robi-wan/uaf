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

#ifndef UAF_SESSIONSETTINGS_H_
#define UAF_SESSIONSETTINGS_H_


// STD
#include <string>
#include <stdint.h>
#include <sstream>
// SDK
// UAF
#include "uaf/util/stringifiable.h"
#include "uaf/util/handles.h"
#include "uaf/client/settings/sessionsecuritysettings.h"
#include "uaf/client/settings/readsettings.h"


namespace uaf
{


    /*******************************************************************************************//**
    * An uaf::SessionSettings instance stores the settings of a uaf::Session.
    *
    * @ingroup ClientSettings
    ***********************************************************************************************/
    class UAF_EXPORT SessionSettings
    {
    public:


        /**
         * Construct default settings.
         *
         * The following default values are given:
         *   - sessionTimeoutSec  = 1200.0
         *   - connectTimeoutSec  = 2.0
         *   - watchdogTimeoutSec = 2.0
         *   - watchdogTimeSec    = 5.0
         *   - unique             = false
         */
        SessionSettings();

        /** The time a session should remain valid on the server after a connection error. **/
        double      sessionTimeoutSec;

        /** The timeout for the connect call. **/
        double      connectTimeoutSec;

        /** The time between watchdog calls. **/
        double      watchdogTimeSec;

        /** The timeout of watchdog calls. **/
        double      watchdogTimeoutSec;

        /** Should this session that uses these settings be unique, or not? **/
        bool        unique;

        /** The settings to be used to read the namespace array and server array, when the session
         *  is first connected (UAF clients will do this automatically in the background). */
        uaf::ReadSettings readServerInfoSettings;

        /** The security settings to be used to connect the session */
        uaf::SessionSecuritySettings securitySettings;


        /**
         * Get a string representation of the settings.
         *
         * @return  String representation.
         */
        std::string toString(const std::string& indent="", std::size_t colon=26) const;


        // comparison operators
        friend bool UAF_EXPORT operator< (
                const SessionSettings& object1,
                const SessionSettings& object2);
        friend bool UAF_EXPORT operator==(
                const SessionSettings& object1,
                const SessionSettings& object2);
        friend bool UAF_EXPORT operator!=(
                const SessionSettings& object1,
                const SessionSettings& object2);



    };
}

#endif /* UAF_SESSIONSETTINGS_H_ */
