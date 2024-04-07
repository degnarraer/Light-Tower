#ifndef ASYNCWEBSYNCHRONIZATION_H_
#define ASYNCWEBSYNCHRONIZATION_H_

// Synchronisation is only available on ESP32, as the ESP8266 isn't using FreeRTOS by default

#include <ESPAsyncWebServer.h>

namespace {
constexpr const bool asyncWebLockDebug = false;
}

#ifdef ESP32

// This is the ESP32 version of the Sync Lock, using the FreeRTOS Semaphore
class AsyncWebLock
{
private:
    SemaphoreHandle_t _lock;
    mutable TaskHandle_t _lockedBy{};
    mutable const char *_lastLockerName;

public:
    const char * const lockName;
    AsyncWebLock(const char *_lockName) :
      lockName{_lockName}
    {
        _lock = xSemaphoreCreateBinary();
        _lockedBy = NULL;
        xSemaphoreGive(_lock);
    }

    ~AsyncWebLock() {
        vSemaphoreDelete(_lock);
    }

    bool lock(const char *lockerName) const {
        const auto currentTask = xTaskGetCurrentTaskHandle();
        if (_lockedBy != currentTask) {
            while (true)
            {
                if (asyncWebLockDebug) Serial.printf("AsyncWebLock::lock this=0x%llx name=%s locker=%s task=0x%llx %s\r\n", uint64_t(this), lockName, lockerName, uint64_t(currentTask), pcTaskGetTaskName(currentTask));

                if (xSemaphoreTake(_lock, 200 / portTICK_PERIOD_MS))
                    break;
                else
                    Serial.printf("AsyncWebLock::lock FAILED(200ms) this=0x%llx name=%s locker=%s task=0x%llx %s lastLockedBy=%s\r\n", uint64_t(this), lockName, lockerName, uint64_t(currentTask), pcTaskGetTaskName(xTaskGetCurrentTaskHandle()), _lastLockerName);
            }
            _lockedBy = currentTask;
            _lastLockerName = lockerName;
            return true;
        }
        return false;
    }

    void unlock(const char *lockerName) const {
        if (asyncWebLockDebug) Serial.printf("AsyncWebLock::unlock this=0x%llx name=%s locker=%s task=0x%llx %s\r\n", uint64_t(this), lockName, lockerName, uint64_t(xTaskGetCurrentTaskHandle()), pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
        _lockedBy = NULL;
        _lastLockerName = NULL;
        xSemaphoreGive(_lock);
    }
};

#else

// This is the 8266 version of the Sync Lock which is currently unimplemented
class AsyncWebLock
{

public:
  AsyncWebLock() {
  }

  ~AsyncWebLock() {
  }

  bool lock() const {
    return false;
  }

  void unlock() const {
  }
};
#endif

class AsyncWebLockGuard
{
private:
  const AsyncWebLock *_lock;

public:
  const char * const lockerName;

  AsyncWebLockGuard(const AsyncWebLock &l, const char *_lockerName) :
    lockerName{_lockerName}
  {
    if (l.lock(lockerName)) {
      _lock = &l;
    } else {
      _lock = NULL;
    }
  }

  ~AsyncWebLockGuard() {
    if (_lock) {
      _lock->unlock(lockerName);
    }
  }

  void unlock() {
    if (_lock) {
      _lock->unlock(lockerName);
      _lock = NULL;
    }
  }
};

#endif // ASYNCWEBSYNCHRONIZATION_H_
