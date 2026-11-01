#include "effect_library.h"
#include "effect_engine.h"
#include "gpio_controller.h"
#include <string.h>

int EffectLibrary::getEffectCount() {
    return EFFECT_COUNT;
}

const EffectDefinition* EffectLibrary::getEffect(int index) {
    if (index < 0 || index >= EFFECT_COUNT) {
        return nullptr;
    }
    static EffectDefinition def;
    def.id = PATTERNS[index].id;
    def.name = PATTERNS[index].name;
    def.description = PATTERNS[index].description;
    def.defaultSpeedMs = PATTERNS[index].defaultSpeedMs;
    def.stepCount = PATTERNS[index].stepCount;
    return &def;
}

int EffectLibrary::findEffectByName(const char* name) {
    if (name == nullptr || name[0] == '\0') {
        return -1;
    }
    for (int i = 0; i < EFFECT_COUNT; i++) {
        if (strcasecmp(PATTERNS[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

uint16_t EffectLibrary::getDefaultSpeed(int index) {
    if (index < 0 || index >= EFFECT_COUNT) return 160;
    return PATTERNS[index].defaultSpeedMs;
}

bool EffectLibrary::runEffectStep(int index, EffectEngine* engine, GPIOController* gpio) {
    if (index < 0 || index >= EFFECT_COUNT || engine == nullptr || gpio == nullptr) {
        return false;
    }

    const PatternDef& p = PATTERNS[index];
    uint32_t step = engine->getStep();

    // Read 16-bit channel bitmask directly from flash PROGMEM
    uint16_t mask = pgm_read_word(&(p.masks[step % p.stepCount]));
    gpio->setRelayPattern(mask);

    // Exact cycle completion check
    if ((step + 1) >= p.stepCount) {
        engine->setStep(0); // Reset step counter for next cycle
        return true;        // 1 complete cycle finished!
    } else {
        engine->setStep(step + 1);
        return false;
    }
}
