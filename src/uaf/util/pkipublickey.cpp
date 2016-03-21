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

#include "uaf/util/pkipublickey.h"

namespace uaf
{
    using namespace uaf;
    using std::string;
    using std::stringstream;
    using std::vector;
    using std::size_t;


    // Constructor
    // =============================================================================================
    PkiPublicKey::PkiPublicKey()
    {}


    // Constructor
    // =============================================================================================
    PkiPublicKey::PkiPublicKey(const UaByteArray& uaByteArray)
    : uaPkiPublicKey_(UaPkiPublicKey::fromDER(uaByteArray))
    {}


    // Constructor
    // =============================================================================================
    PkiPublicKey::PkiPublicKey(const UaPkiPublicKey& uaKey)
    : uaPkiPublicKey_(uaKey)
    {}


    // Get a DER encoded bytestring
    // =============================================================================================
    ByteString PkiPublicKey::toDER() const
    {
        UaByteArray uaByteArray = uaPkiPublicKey_.toDER();
        ByteString ret;
        ret.fromSdk(uaByteArray);
        return ret;
    }


    // Read a DER encoded bytestring
    // =============================================================================================
    PkiPublicKey PkiPublicKey::fromDER(const ByteString& data)
    {
        UaByteArray uaByteArray;
        data.toSdk(uaByteArray);
        return PkiPublicKey(uaByteArray);
    }


    // Get the key type
    // =============================================================================================
    PkiPublicKey::KeyType PkiPublicKey::keyType() const
    {
#if UASDK_VERSION > 14
        if (uaPkiPublicKey_.keyType() == UaPkiKeyType_DSA)
            return DSA;
        else if (uaPkiPublicKey_.keyType() == UaPkiKeyType_RSA)
            return RSA;
        else
            return Unknown;
#elif UASDK_VERSION > 13
        if (uaPkiPublicKey_.keyType() == UaPkiPublicKey::DSA)
            return DSA;
        else if (uaPkiPublicKey_.keyType() == UaPkiPublicKey::RSA)
            return RSA;
        else
            return Unknown;
#else
#error UASDK_VERSION is not defined!
#endif
    }


    // Get the key type
    // =============================================================================================
    int PkiPublicKey::keySize() const
    {
        return uaPkiPublicKey_.keySize();
    }


    // Get any errors
    // =============================================================================================
    std::vector<std::string> PkiPublicKey::getErrors()
    {
        std::vector<std::string> ret;
        std::list<UaString> errors = uaPkiPublicKey_.getErrors();
        std::list<UaString>::const_iterator iter;
        for (iter = errors.begin(); iter != errors.end(); ++iter)
            ret.push_back(iter->toUtf8());
        return ret;
    }


    // Get a string representation
    // =============================================================================================
    string PkiPublicKey::toString() const
    {
        stringstream ss;

        ss << "keyType=";

        if (keyType() == PkiPublicKey::DSA)
            ss << "DSA";
        else if (keyType() == PkiPublicKey::RSA)
            ss << "DSA";
        else
            ss << "Unknown";

        ss << ", keySize=" << keySize();

        return ss.str();
    }


    // operator==
    // =============================================================================================
    bool operator==(const PkiPublicKey& object1, const PkiPublicKey& object2)
    {
        return    object1.keySize() == object2.keySize()
               && object1.keyType() == object2.keyType()
               && object1.toDER() == object2.toDER();
    }


    // operator!=
    // =============================================================================================
    bool operator!=(const PkiPublicKey& object1, const PkiPublicKey& object2)
    {
        return !(object1 == object2);
    }


    // operator<
    // =============================================================================================
    bool operator<(const PkiPublicKey& object1, const PkiPublicKey& object2)
    {
        if (object1.keySize() != object2.keySize())
            return object1.keySize() < object2.keySize();
        else if (object1.keyType() != object2.keyType())
            return object1.keyType() < object2.keyType();
        else
            return object1.toDER() < object2.toDER();
    }

}
