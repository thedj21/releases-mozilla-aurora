/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=8 et tw=80 : */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// HttpLog.h should generally be included first
#include "HttpLog.h"

/*
  Currently supported are spdy/3.1 and spdy/3 and spdy/2

*/

#include "nsHttp.h"
#include "nsHttpHandler.h"

#include "ASpdySession.h"
#include "PSpdyPush.h"
#include "SpdyPush3.h"
#include "SpdyPush31.h"
#include "SpdySession2.h"
#include "SpdySession3.h"
#include "SpdySession31.h"

#include "mozilla/Telemetry.h"

namespace mozilla {
namespace net {

ASpdySession *
ASpdySession::NewSpdySession(uint32_t version,
                             nsAHttpTransaction *aTransaction,
                             nsISocketTransport *aTransport,
                             int32_t aPriority)
{
  // This is a necko only interface, so we can enforce version
  // requests as a precondition
  MOZ_ASSERT(version == SPDY_VERSION_2 ||
             version == SPDY_VERSION_3 ||
             version == SPDY_VERSION_31 ,
             "Unsupported spdy version");

  // Don't do a runtime check of IsSpdyV?Enabled() here because pref value
  // may have changed since starting negotiation. The selected protocol comes
  // from a list provided in the SERVER HELLO filtered by our acceptable
  // versions, so there is no risk of the server ignoring our prefs.

  Telemetry::Accumulate(Telemetry::SPDY_VERSION2, version);

  if (version == SPDY_VERSION_3)
    return new SpdySession3(aTransaction, aTransport, aPriority);

  if (version == SPDY_VERSION_2)
    return new SpdySession2(aTransaction, aTransport, aPriority);

  return new SpdySession31(aTransaction, aTransport, aPriority);
}

SpdyInformation::SpdyInformation()
{
  Version[0] = SPDY_VERSION_2;
  VersionString[0] = NS_LITERAL_CSTRING("spdy/2");

  Version[1] = SPDY_VERSION_3;
  VersionString[1] = NS_LITERAL_CSTRING("spdy/3");

  Version[2] = SPDY_VERSION_31;
  VersionString[2] = NS_LITERAL_CSTRING("spdy/3.1");
}

bool
SpdyInformation::ProtocolEnabled(uint32_t index)
{
  MOZ_ASSERT(index < kCount, "index out of range");

  switch (index) {
  case 0:
    return gHttpHandler->IsSpdyV2Enabled();
  case 1:
    return gHttpHandler->IsSpdyV3Enabled();
  case 2:
    return gHttpHandler->IsSpdyV31Enabled();
  }
  return false;
}

nsresult
SpdyInformation::GetNPNVersionIndex(const nsACString &npnString,
                                    uint8_t *result)
{
  if (npnString.IsEmpty())
    return NS_ERROR_FAILURE;

  for (uint32_t index = 0; index < kCount; ++index) {
    if (npnString.Equals(VersionString[index])) {
      *result = Version[index];
      return NS_OK;
    }
  }

  return NS_ERROR_FAILURE;
}

//////////////////////////////////////////
// SpdyPushCache
//////////////////////////////////////////

SpdyPushCache::SpdyPushCache()
{
}

SpdyPushCache::~SpdyPushCache()
{
  mHashSpdy3.Clear();
  mHashSpdy31.Clear();
}

bool
SpdyPushCache::RegisterPushedStreamSpdy3(nsCString key,
                                         SpdyPushedStream3 *stream)
{
  LOG3(("SpdyPushCache::RegisterPushedStreamSpdy3 %s 0x%X\n",
        key.get(), stream->StreamID()));
  if(mHashSpdy3.Get(key))
    return false;
  mHashSpdy3.Put(key, stream);
  return true;
}

SpdyPushedStream3 *
SpdyPushCache::RemovePushedStreamSpdy3(nsCString key)
{
  SpdyPushedStream3 *rv = mHashSpdy3.Get(key);
  LOG3(("SpdyPushCache::RemovePushedStream %s 0x%X\n",
        key.get(), rv ? rv->StreamID() : 0));
  if (rv)
    mHashSpdy3.Remove(key);
  return rv;
}

bool
SpdyPushCache::RegisterPushedStreamSpdy31(nsCString key,
                                          SpdyPushedStream31 *stream)
{
  LOG3(("SpdyPushCache::RegisterPushedStreamSpdy31 %s 0x%X\n",
        key.get(), stream->StreamID()));
  if(mHashSpdy31.Get(key))
    return false;
  mHashSpdy31.Put(key, stream);
  return true;
}

SpdyPushedStream31 *
SpdyPushCache::RemovePushedStreamSpdy31(nsCString key)
{
  SpdyPushedStream31 *rv = mHashSpdy31.Get(key);
  LOG3(("SpdyPushCache::RemovePushedStream %s 0x%X\n",
        key.get(), rv ? rv->StreamID() : 0));
  if (rv)
    mHashSpdy31.Remove(key);
  return rv;
}

} // namespace mozilla::net
} // namespace mozilla

