#include <switch.h>
#include "Log.hpp"

// GPIO service session
static GpioPadSession gpioSession;
static GpioValue gpioLastValue;         // Low if plugged in, High if not

// PSC service module
#define PSC_ID 690
static PscPmModule pscModule;

namespace NX {
    bool initializeGpio() {
        // Open a session for GPIO pin 0x15, which indicates if headset is connected
        gpioLastValue = GpioValue_High;
        Result rc = gpioOpenSession(&gpioSession, (GpioPadName)0x15);
        if (R_SUCCEEDED(rc)) {
            return true;
        }
        return false;
    }

    void exitGpio() {
        gpioPadClose(&gpioSession);
    }

    bool gpioHeadsetUnplugged() {
        // Get current value
        GpioValue currentValue;
        Result rc = gpioPadGetValue(&gpioSession, &currentValue);
        if (R_FAILED(rc)) {
            return false;
        }

        // If we have changed from Low to High, the headset was unplugged
        bool unplugged = false;
        if (gpioLastValue == GpioValue_Low && currentValue == GpioValue_High) {
            unplugged = true;
        }
        gpioLastValue = currentValue;
        return unplugged;
    }

    const u16 pscmDeps[] = {PscPmModuleId_Audio};
    bool initializePsc() {
        // Register audio as a dependency so we can handle the sleep event
        // before the console actually enters sleep
        Result rc = pscmGetPmModule(&pscModule, (PscPmModuleId)PSC_ID, pscmDeps, 1, true);
        if (R_FAILED(rc)) {
            return false;
        }
        return true;
    }

    void exitPsc() {
        pscPmModuleFinalize(&pscModule);
        pscPmModuleClose(&pscModule);
    }

    bool pscEnteringSleep(const size_t ms) {
        // Wait for an event
        Result rc = eventWait(&pscModule.event, ms * 1000000);
        if (R_VALUE(rc) == KERNELRESULT(TimedOut)) {
            return false;

        } else if (R_VALUE(rc) == KERNELRESULT(Cancelled)) {
            svcSleepThread(ms * 1000000);
            return false;
        }

        // If there's an event, fetch the state
        PscPmState eventState;
        u32 flags;
        rc = pscPmModuleGetRequest(&pscModule, &eventState, &flags);
        if (R_FAILED(rc)) {
            return false;
        }

        // Check if we're entering sleep
        bool enteringSleep = false;
        if (eventState == PscPmState_ReadySleep) {
            enteringSleep = true;
        }
        pscPmModuleAcknowledge(&pscModule, eventState);
        return enteringSleep;
    }
};