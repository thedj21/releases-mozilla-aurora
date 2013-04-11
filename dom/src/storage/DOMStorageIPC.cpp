/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DOMStorageIPC.h"

#include "DOMStorageManager.h"

#include "mozilla/dom/ContentChild.h"
#include "mozilla/unused.h"

namespace mozilla {
namespace dom {

// ----------------------------------------------------------------------------
// Child
// ----------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ADDREF(DOMStorageDBChild)

NS_IMETHODIMP_(nsrefcnt) DOMStorageDBChild::Release(void)
{
  NS_PRECONDITION(0 != mRefCnt, "dup release");
  nsrefcnt count = --mRefCnt;
  NS_LOG_RELEASE(this, count, "DOMStorageDBChild");
  if (count == 1 && mIPCOpen) {
    Send__delete__(this);
    return 0;
  }
  if (count == 0) {
    mRefCnt = 1;
    delete this;
    return 0;
  }
  return count;
}

void
DOMStorageDBChild::AddIPDLReference()
{
  NS_ABORT_IF_FALSE(!mIPCOpen, "Attempting to retain multiple IPDL references");
  mIPCOpen = true;
  AddRef();
}

void
DOMStorageDBChild::ReleaseIPDLReference()
{
  NS_ABORT_IF_FALSE(mIPCOpen, "Attempting to release non-existent IPDL reference");
  mIPCOpen = false;
  Release();
}

DOMStorageDBChild::DOMStorageDBChild(DOMLocalStorageManager* aManager)
  : mManager(aManager)
  , mStatus(NS_OK)
  , mIPCOpen(false)
{
}

DOMStorageDBChild::~DOMStorageDBChild()
{
}

nsTHashtable<nsCStringHashKey>&
DOMStorageDBChild::ScopesHavingData()
{
  if (!mScopesHavingData.IsInitialized()) {
    mScopesHavingData.Init();
  }

  return mScopesHavingData;
}

nsresult
DOMStorageDBChild::Init()
{
  ContentChild* child = ContentChild::GetSingleton();
  AddIPDLReference();
  child->SendPStorageConstructor(this);
  return NS_OK;
}

nsresult
DOMStorageDBChild::Shutdown()
{
  // There is nothing to do here, IPC will release automatically and
  // the actual thread running on the parent process will also stop
  // automatically in profile-before-change topic observer.
  return NS_OK;
}

void
DOMStorageDBChild::SyncPreload(DOMStorageCacheBridge* aCache, bool aForceSync)
{
  if (NS_FAILED(mStatus)) {
    aCache->LoadDone(mStatus);
    return;
  }

  // There is no way to put the child process to a wait state to receive all
  // incoming async responses from the parent, hence we have to do a sync preload
  // instead.  We are smart though, we only demand keys that are left to load in
  // case the async preload has already loaded some keys.
  InfallibleTArray<nsString> keys, values;
  nsresult rv;
  SendPreload(aCache->Scope(), aCache->LoadedCount(), &keys, &values, &rv);

  for (uint32_t i = 0; i < keys.Length(); ++i) {
    aCache->LoadItem(keys[i], values[i]);
  }

  aCache->LoadDone(rv);
}

nsresult
DOMStorageDBChild::AsyncAddItem(DOMStorageCacheBridge* aCache,
                                const nsAString& aKey,
                                const nsAString& aValue)
{
  if (NS_FAILED(mStatus)) {
    return mStatus;
  }

  SendAsyncAddItem(aCache->Scope(), nsString(aKey), nsString(aValue));
  ScopesHavingData().PutEntry(aCache->Scope());
  return NS_OK;
}

nsresult
DOMStorageDBChild::AsyncUpdateItem(DOMStorageCacheBridge* aCache,
                                   const nsAString& aKey,
                                   const nsAString& aValue)
{
  if (NS_FAILED(mStatus)) {
    return mStatus;
  }

  SendAsyncUpdateItem(aCache->Scope(), nsString(aKey), nsString(aValue));
  ScopesHavingData().PutEntry(aCache->Scope());
  return NS_OK;
}

nsresult
DOMStorageDBChild::AsyncRemoveItem(DOMStorageCacheBridge* aCache,
                                   const nsAString& aKey)
{
  if (NS_FAILED(mStatus)) {
    return mStatus;
  }

  SendAsyncRemoveItem(aCache->Scope(), nsString(aKey));
  return NS_OK;
}

nsresult
DOMStorageDBChild::AsyncClear(DOMStorageCacheBridge* aCache)
{
  if (NS_FAILED(mStatus)) {
    return mStatus;
  }

  SendAsyncClear(aCache->Scope());
  ScopesHavingData().RemoveEntry(aCache->Scope());
  return NS_OK;
}

bool
DOMStorageDBChild::ShouldPreloadScope(const nsACString& aScope)
{
  // Return true if we didn't receive the aScope list yet.
  // I tend to rather preserve a bit of early-after-start performance
  // then a bit of memory here.
  return !mScopesHavingData.IsInitialized() ||
         mScopesHavingData.Contains(aScope);
}

bool
DOMStorageDBChild::RecvObserve(const nsCString& aTopic,
                               const nsCString& aScopePrefix)
{
  DOMStorageObserver::Self()->Notify(aTopic.get(), aScopePrefix);
  return true;
}

bool
DOMStorageDBChild::RecvScopesHavingData(const InfallibleTArray<nsCString>& aScopes)
{
  for (uint32_t i = 0; i < aScopes.Length(); ++i) {
    ScopesHavingData().PutEntry(aScopes[i]);
  }

  return true;
}

bool
DOMStorageDBChild::RecvLoadItem(const nsCString& aScope,
                                const nsString& aKey,
                                const nsString& aValue)
{
  DOMStorageCache* aCache = mManager->GetCache(aScope);
  if (aCache) {
    aCache->LoadItem(aKey, aValue);
  }

  return true;
}

bool
DOMStorageDBChild::RecvLoadDone(const nsCString& aScope, const nsresult& aRv)
{
  DOMStorageCache* aCache = mManager->GetCache(aScope);
  if (aCache) {
    aCache->LoadDone(aRv);
  }

  return true;
}

bool
DOMStorageDBChild::RecvLoadUsage(const nsCString& aScope, const int64_t& aUsage)
{
  DOMStorageDBBridge* db = DOMStorageCache::GetDatabase();
  if (!db) {
    return false;
  }

  DOMStorageUsageBridge* scopeUsage = db->GetScopeUsage(aScope);
  scopeUsage->LoadUsage(aUsage);
  return true;
}

bool
DOMStorageDBChild::RecvError(const nsresult& aRv)
{
  mStatus = aRv;
  return true;
}

// ----------------------------------------------------------------------------
// Parent
// ----------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ADDREF(DOMStorageDBParent)
NS_IMPL_THREADSAFE_RELEASE(DOMStorageDBParent)

void
DOMStorageDBParent::AddIPDLReference()
{
  NS_ABORT_IF_FALSE(!mIPCOpen, "Attempting to retain multiple IPDL references");
  mIPCOpen = true;
  AddRef();
}

void
DOMStorageDBParent::ReleaseIPDLReference()
{
  NS_ABORT_IF_FALSE(mIPCOpen, "Attempting to release non-existent IPDL reference");
  mIPCOpen = false;
  Release();
}

namespace { // anon

class SendScopesHavingDataRunnable : public nsRunnable
{
public:
  SendScopesHavingDataRunnable(DOMStorageDBParent* aParent)
    : mParent(aParent)
  {}

private:
  NS_IMETHOD Run()
  {
    DOMStorageDBBridge* db = DOMStorageCache::GetDatabase();
    if (db) {
      InfallibleTArray<nsCString> scopes;
      db->GetScopesHavingData(&scopes);
      mozilla::unused << mParent->SendScopesHavingData(scopes);
    }

    return NS_OK;
  }

  nsRefPtr<DOMStorageDBParent> mParent;
};

} // anon

DOMStorageDBParent::DOMStorageDBParent()
: mIPCOpen(false)
{
  DOMStorageObserver* observer = DOMStorageObserver::Self();
  if (observer) {
    observer->AddSink(this);
  }

  // We are always open by IPC only
  AddIPDLReference();

  // Cannot send directly from here since the channel
  // is not completely built at this moment.
  nsRefPtr<SendScopesHavingDataRunnable> r =
    new SendScopesHavingDataRunnable(this);
  NS_DispatchToCurrentThread(r);
}

DOMStorageDBParent::~DOMStorageDBParent()
{
  DOMStorageObserver* observer = DOMStorageObserver::Self();
  if (observer) {
    observer->RemoveSink(this);
  }
}

DOMStorageDBParent::CacheParentBridge*
DOMStorageDBParent::NewCache(const nsACString& aScope)
{
  return new CacheParentBridge(this, aScope);
}

bool
DOMStorageDBParent::RecvAsyncPreload(const nsCString& aScope, const bool& aPriority)
{
  DOMStorageDBBridge* db = DOMStorageCache::StartDatabase();
  if (!db) {
    return false;
  }

  db->AsyncPreload(NewCache(aScope), aPriority);
  return true;
}

bool
DOMStorageDBParent::RecvAsyncGetUsage(const nsCString& aScope)
{
  DOMStorageDBBridge* db = DOMStorageCache::StartDatabase();
  if (!db) {
    return false;
  }

  // The object releases it self in LoadUsage method
  UsageParentBridge* usage = new UsageParentBridge(this, aScope);
  db->AsyncGetUsage(usage);
  return true;
}

namespace { // anon

// We need another implementation of DOMStorageCacheBridge to do
// synchronous IPC preload.  This class just receives Load* notifications
// and fills the returning arguments of RecvPreload with the database
// values for us.
class SyncLoadCacheHelper : public DOMStorageCacheBridge
{
public:
  SyncLoadCacheHelper(const nsCString& aScope,
                      uint32_t aAlreadyLoadedCount,
                      InfallibleTArray<nsString>* aKeys,
                      InfallibleTArray<nsString>* aValues,
                      nsresult* rv)
  : mMonitor("DOM Storage SyncLoad IPC")
  , mScope(aScope)
  , mKeys(aKeys)
  , mValues(aValues)
  , mRv(rv)
  , mLoaded(false)
  , mLoadedCount(aAlreadyLoadedCount)
  {
    // Precaution
    *mRv = NS_ERROR_UNEXPECTED;
  }

  virtual const nsCString& Scope() const { return mScope; }
  virtual bool Loaded() { return mLoaded; }
  virtual uint32_t LoadedCount() { return mLoadedCount; }
  virtual bool LoadItem(const nsAString& aKey, const nsString& aValue)
  {
    // Called on the aCache background thread
    if (mLoaded) {
      return false;
    }

    ++mLoadedCount;
    mKeys->AppendElement(aKey);
    mValues->AppendElement(aValue);
    return true;
  }

  virtual void LoadDone(nsresult aRv)
  {
    // Called on the aCache background thread
    MonitorAutoLock monitor(mMonitor);
    mLoaded = true;
    *mRv = aRv;
    monitor.Notify();
  }

  virtual void LoadWait()
  {
    // Called on the main thread, exits after LoadDone() call
    MonitorAutoLock monitor(mMonitor);
    while (!mLoaded) {
      monitor.Wait();
    }
  }

private:
  Monitor mMonitor;
  nsCString mScope;
  InfallibleTArray<nsString>* mKeys;
  InfallibleTArray<nsString>* mValues;
  nsresult* mRv;
  bool mLoaded;
  uint32_t mLoadedCount;
};

} // anon

bool
DOMStorageDBParent::RecvPreload(const nsCString& aScope,
                                const uint32_t& aAlreadyLoadedCount,
                                InfallibleTArray<nsString>* aKeys,
                                InfallibleTArray<nsString>* aValues,
                                nsresult* aRv)
{
  DOMStorageDBBridge* db = DOMStorageCache::StartDatabase();
  if (!db) {
    return false;
  }

  nsRefPtr<SyncLoadCacheHelper> aCache(
    new SyncLoadCacheHelper(aScope, aAlreadyLoadedCount, aKeys, aValues, aRv));

  db->SyncPreload(aCache, true);
  return true;
}

bool
DOMStorageDBParent::RecvAsyncAddItem(const nsCString& aScope,
                                     const nsString& aKey,
                                     const nsString& aValue)
{
  DOMStorageDBBridge* db = DOMStorageCache::StartDatabase();
  if (!db) {
    return false;
  }

  nsresult rv = db->AsyncAddItem(NewCache(aScope), aKey, aValue);
  if (NS_FAILED(rv)) {
    mozilla::unused << SendError(rv);
  }

  return true;
}

bool
DOMStorageDBParent::RecvAsyncUpdateItem(const nsCString& aScope,
                                        const nsString& aKey,
                                        const nsString& aValue)
{
  DOMStorageDBBridge* db = DOMStorageCache::StartDatabase();
  if (!db) {
    return false;
  }

  nsresult rv = db->AsyncUpdateItem(NewCache(aScope), aKey, aValue);
  if (NS_FAILED(rv)) {
    mozilla::unused << SendError(rv);
  }

  return true;
}

bool
DOMStorageDBParent::RecvAsyncRemoveItem(const nsCString& aScope,
                                        const nsString& aKey)
{
  DOMStorageDBBridge* db = DOMStorageCache::StartDatabase();
  if (!db) {
    return false;
  }

  nsresult rv = db->AsyncRemoveItem(NewCache(aScope), aKey);
  if (NS_FAILED(rv)) {
    mozilla::unused << SendError(rv);
  }

  return true;
}

bool
DOMStorageDBParent::RecvAsyncClear(const nsCString& aScope)
{
  DOMStorageDBBridge* db = DOMStorageCache::StartDatabase();
  if (!db) {
    return false;
  }

  nsresult rv = db->AsyncClear(NewCache(aScope));
  if (NS_FAILED(rv)) {
    mozilla::unused << SendError(rv);
  }

  return true;
}

bool
DOMStorageDBParent::RecvAsyncFlush()
{
  DOMStorageDBBridge* db = DOMStorageCache::GetDatabase();
  if (!db) {
    return false;
  }

  db->AsyncFlush();
  return true;
}

// DOMStorageObserverSink

nsresult
DOMStorageDBParent::Observe(const char* aTopic,
                            const nsACString& aScopePrefix)
{
  mozilla::unused << SendObserve(nsDependentCString(aTopic),
                                 nsCString(aScopePrefix));
  return NS_OK;
}

namespace { // anon

// Results must be sent back on the main thread
class LoadRunnable : public nsRunnable
{
public:
  enum TaskType {
    loadItem,
    loadDone
  };

  LoadRunnable(DOMStorageDBParent* aParent,
               TaskType aType,
               const nsACString& aScope,
               const nsAString& aKey = EmptyString(),
               const nsAString& aValue = EmptyString())
  : mParent(aParent)
  , mType(aType)
  , mScope(aScope)
  , mKey(aKey)
  , mValue(aValue)
  { }

  LoadRunnable(DOMStorageDBParent* aParent,
               TaskType aType,
               const nsACString& aScope,
               nsresult aRv)
  : mParent(aParent)
  , mType(aType)
  , mScope(aScope)
  , mRv(aRv)
  { }

private:
  nsRefPtr<DOMStorageDBParent> mParent;
  TaskType mType;
  nsCString mScope;
  nsString mKey;
  nsString mValue;
  nsresult mRv;

  NS_IMETHOD Run()
  {
    if (!mParent->IPCOpen()) {
      return NS_OK;
    }

    switch (mType)
    {
    case loadItem:
      mozilla::unused << mParent->SendLoadItem(mScope, mKey, mValue);
      break;
    case loadDone:
      mozilla::unused << mParent->SendLoadDone(mScope, mRv);
      break;
    }

    return NS_OK;
  }
};

} // anon

// DOMStorageDBParent::CacheParentBridge

bool
DOMStorageDBParent::CacheParentBridge::LoadItem(const nsAString& aKey, const nsString& aValue)
{
  if (mLoaded) {
    return false;
  }

  ++mLoadedCount;

  nsRefPtr<LoadRunnable> r =
    new LoadRunnable(mParent, LoadRunnable::loadItem, mScope, aKey, aValue);
  NS_DispatchToMainThread(r);
  return true;
}

void
DOMStorageDBParent::CacheParentBridge::LoadDone(nsresult aRv)
{
  // Prevent send of duplicate LoadDone.
  if (mLoaded) {
    return;
  }

  mLoaded = true;

  nsRefPtr<LoadRunnable> r =
    new LoadRunnable(mParent, LoadRunnable::loadDone, mScope, aRv);
  NS_DispatchToMainThread(r);
}

void
DOMStorageDBParent::CacheParentBridge::LoadWait()
{
  // Should never be called on this implementation
  MOZ_ASSERT(false);
}

// DOMStorageDBParent::UsageParentBridge

namespace { // anon

class UsageRunnable : public nsRunnable
{
public:
  UsageRunnable(DOMStorageDBParent* aParent, const nsACString& aScope, const int64_t& aUsage)
  : mParent(aParent)
  , mScope(aScope)
  , mUsage(aUsage)
  {}

private:
  NS_IMETHOD Run()
  {
    mozilla::unused << mParent->SendLoadUsage(mScope, mUsage);
    return NS_OK;
  }

  nsRefPtr<DOMStorageDBParent> mParent;
  nsCString mScope;
  int64_t mUsage;
};

} // anon

void
DOMStorageDBParent::UsageParentBridge::LoadUsage(const int64_t aUsage)
{
  nsRefPtr<UsageRunnable> r = new UsageRunnable(mParent, mScope, aUsage);
  NS_DispatchToMainThread(r);
  delete this;
}

} // ::dom
} // ::mozilla
