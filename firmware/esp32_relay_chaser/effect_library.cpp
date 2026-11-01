#include "effect_library.h"
#include "effect_engine.h"
#include "gpio_controller.h"
#include <string.h>

// Definition array for all 30 effects
const EffectDefinition EffectLibrary::effects[EFFECT_COUNT] = {
    {0,  "Sequential Forward Chase",     "One relay ON at a time moving forward (0 to 15)",             EffectLibrary::effectSequentialForward},
    {1,  "Sequential Reverse Chase",     "One relay ON at a time moving backward (15 to 0)",            EffectLibrary::effectSequentialReverse},
    {2,  "Left-to-Right Running Light",  "Clean running light from left to right",                      EffectLibrary::effectLeftToRight},
    {3,  "Right-to-Left Running Light",  "Clean running light from right to left",                      EffectLibrary::effectRightToLeft},
    {4,  "Center-Out Chase",             "Expands outward from center channels (7,8) to ends (0,15)",   EffectLibrary::effectCenterOut},
    {5,  "Outside-In Chase",             "Collapses inward from ends (0,15) to center (7,8)",           EffectLibrary::effectOutsideIn},
    {6,  "Alternate Channel Flash",      "Odd channels flash followed by even channels",                EffectLibrary::effectAlternateChannelFlash},
    {7,  "Odd-Even Alternating",         "Paired alternating pattern across all 16 channels",           EffectLibrary::effectOddEvenAlternating},
    {8,  "Double Running Chase",         "Two adjacent relays ON running continuously",                 EffectLibrary::effectDoubleRunningChase},
    {9,  "Triple Running Chase",         "Three adjacent relays ON running continuously",               EffectLibrary::effectTripleRunningChase},
    {10, "Four-Channel Running Pattern", "Four adjacent relays ON running continuously",                EffectLibrary::effectFourChannelPattern},
    {11, "Ping-Pong Chase",              "Single relay bounces smoothly back and forth",                EffectLibrary::effectPingPongChase},
    {12, "Knight Rider Effect",          "Classic Larson scanner with 3-relay wide comet body",         EffectLibrary::effectKnightRider},
    {13, "Comet Chase",                  "Leading relay with 3-channel trailing tail",                  EffectLibrary::effectCometChase},
    {14, "Bouncing Light",               "Non-linear light trajectory with bounce dynamics",            EffectLibrary::effectBouncingLight},
    {15, "Waterfall Effect",             "Sequential cascade filling all channels then emptying",       EffectLibrary::effectWaterfall},
    {16, "Wave Effect",                  "Sine-like undulating multi-crest wave shifting channels",     EffectLibrary::effectWave},
    {17, "Fill and Empty",               "Symmetrical inward fill followed by outward clearing",        EffectLibrary::effectFillAndEmpty},
    {18, "Random Channel Chase",         "Pseudo-random single channel jump pattern",                   EffectLibrary::effectRandomChannelChase},
    {19, "Random Group Flash",           "Pseudo-random clusters of 4 to 8 relays flashing",            EffectLibrary::effectRandomGroupFlash},
    {20, "Binary Counter Pattern",       "Counts upward in binary across 16 channels",                  EffectLibrary::effectBinaryCounter},
    {21, "Progressive Fill",             "Progressively fills left-to-right then blanks out",           EffectLibrary::effectProgressiveFill},
    {22, "Progressive Empty",            "Starts fully illuminated and progressively turns off",        EffectLibrary::effectProgressiveEmpty},
    {23, "Split Half Chase",             "Left 8 and right 8 channels chase in parallel",               EffectLibrary::effectSplitHalfChase},
    {24, "Symmetrical Mirror Chase",     "Symmetrical mirror bouncing inward and outward",              EffectLibrary::effectSymmetricalMirrorChase},
    {25, "Fast Strobe",                  "All 16 relays flash simultaneously at rapid cadence",         EffectLibrary::effectFastStrobe},
    {26, "Slow Strobe",                  "All 16 relays pulse simultaneously with steady duty cycle",   EffectLibrary::effectSlowStrobe},
    {27, "Cross Pattern",                "Alternating geometric cross pattern channels",                EffectLibrary::effectCrossPattern},
    {28, "Spiral-Style Sequence",        "Inward winding sequence from extremities to center",          EffectLibrary::effectSpiralSequence},
    {29, "Combination Chase",            "Sequences 4 distinct motion styles in one continuous cycle",  EffectLibrary::effectCombinationChase}
};

int EffectLibrary::getEffectCount() {
    return EFFECT_COUNT;
}

const EffectDefinition* EffectLibrary::getEffect(int index) {
    if (index < 0 || index >= EFFECT_COUNT) {
        return nullptr;
    }
    return &effects[index];
}

int EffectLibrary::findEffectByName(const char* name) {
    if (name == nullptr || name[0] == '\0') {
        return -1;
    }
    for (int i = 0; i < EFFECT_COUNT; i++) {
        if (strcasecmp(effects[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

bool EffectLibrary::runEffectStep(int index, EffectEngine* engine, GPIOController* gpio) {
    if (index < 0 || index >= EFFECT_COUNT || engine == nullptr || gpio == nullptr) {
        return false;
    }
    return effects[index].stepFunc(engine, gpio);
}

// ----------------------------------------------------------------------------
// 1. Sequential Forward Chase
// ----------------------------------------------------------------------------
bool EffectLibrary::effectSequentialForward(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= RELAY_COUNT) pos = 0;

    gpio->setRelayPattern(static_cast<uint16_t>(1) << pos);

    pos++;
    bool cycleDone = false;
    if (pos >= RELAY_COUNT) {
        pos = 0;
        cycleDone = true;
    }
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 2. Sequential Reverse Chase
// ----------------------------------------------------------------------------
bool EffectLibrary::effectSequentialReverse(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= RELAY_COUNT) pos = RELAY_COUNT - 1;

    gpio->setRelayPattern(static_cast<uint16_t>(1) << pos);

    bool cycleDone = false;
    if (pos <= 0) {
        pos = RELAY_COUNT - 1;
        cycleDone = true;
    } else {
        pos--;
    }
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 3. Left-to-Right Running Light
// ----------------------------------------------------------------------------
bool EffectLibrary::effectLeftToRight(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= RELAY_COUNT) pos = 0;

    gpio->setRelayPattern(static_cast<uint16_t>(1) << pos);

    pos++;
    bool cycleDone = (pos >= RELAY_COUNT);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 4. Right-to-Left Running Light
// ----------------------------------------------------------------------------
bool EffectLibrary::effectRightToLeft(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= RELAY_COUNT) pos = RELAY_COUNT - 1;

    gpio->setRelayPattern(static_cast<uint16_t>(1) << pos);

    bool cycleDone = (pos <= 0);
    if (cycleDone) {
        pos = RELAY_COUNT - 1;
    } else {
        pos--;
    }
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 5. Center-Out Chase
// Channels 7 & 8 outward to 0 & 15
// ----------------------------------------------------------------------------
bool EffectLibrary::effectCenterOut(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= 8) pos = 0;

    int leftCh = 7 - pos;
    int rightCh = 8 + pos;
    uint16_t mask = (static_cast<uint16_t>(1) << leftCh) | (static_cast<uint16_t>(1) << rightCh);
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= 8);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 6. Outside-In Chase
// Channels 0 & 15 inward to 7 & 8
// ----------------------------------------------------------------------------
bool EffectLibrary::effectOutsideIn(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= 8) pos = 0;

    int leftCh = pos;
    int rightCh = 15 - pos;
    uint16_t mask = (static_cast<uint16_t>(1) << leftCh) | (static_cast<uint16_t>(1) << rightCh);
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= 8);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 7. Alternate Channel Flash
// 0xAAAA (Odd: 1,3,5...) then 0x5555 (Even: 0,2,4...)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectAlternateChannelFlash(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 6) step = 0;

    if (step % 2 == 0) {
        gpio->setRelayPattern(0xAAAA);
    } else {
        gpio->setRelayPattern(0x5555);
    }

    step++;
    bool cycleDone = (step >= 6);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 8. Odd-Even Alternating
// Alternates pairs: 0x3333 and 0xCCCC
// ----------------------------------------------------------------------------
bool EffectLibrary::effectOddEvenAlternating(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 6) step = 0;

    if (step % 2 == 0) {
        gpio->setRelayPattern(0x3333);
    } else {
        gpio->setRelayPattern(0xCCCC);
    }

    step++;
    bool cycleDone = (step >= 6);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 9. Double Running Chase
// 2 adjacent relays ON
// ----------------------------------------------------------------------------
bool EffectLibrary::effectDoubleRunningChase(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= RELAY_COUNT) pos = 0;

    uint16_t mask = (static_cast<uint16_t>(1) << pos) | 
                    (static_cast<uint16_t>(1) << ((pos + 1) % RELAY_COUNT));
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= RELAY_COUNT);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 10. Triple Running Chase
// 3 adjacent relays ON
// ----------------------------------------------------------------------------
bool EffectLibrary::effectTripleRunningChase(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= RELAY_COUNT) pos = 0;

    uint16_t mask = (static_cast<uint16_t>(1) << pos) | 
                    (static_cast<uint16_t>(1) << ((pos + 1) % RELAY_COUNT)) |
                    (static_cast<uint16_t>(1) << ((pos + 2) % RELAY_COUNT));
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= RELAY_COUNT);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 11. Four-Channel Running Pattern
// 4 adjacent relays ON
// ----------------------------------------------------------------------------
bool EffectLibrary::effectFourChannelPattern(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= RELAY_COUNT) pos = 0;

    uint16_t mask = (static_cast<uint16_t>(1) << pos) | 
                    (static_cast<uint16_t>(1) << ((pos + 1) % RELAY_COUNT)) |
                    (static_cast<uint16_t>(1) << ((pos + 2) % RELAY_COUNT)) |
                    (static_cast<uint16_t>(1) << ((pos + 3) % RELAY_COUNT));
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= RELAY_COUNT);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 12. Ping-Pong Chase
// Single relay bouncing 0 -> 15 -> 0
// ----------------------------------------------------------------------------
bool EffectLibrary::effectPingPongChase(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    int dir = engine->getDirection();
    if (dir != 1 && dir != -1) dir = 1;
    if (pos < 0 || pos >= RELAY_COUNT) pos = 0;

    gpio->setRelayPattern(static_cast<uint16_t>(1) << pos);

    pos += dir;
    bool cycleDone = false;
    if (pos >= 15) {
        pos = 15;
        dir = -1;
    } else if (pos <= 0) {
        pos = 0;
        dir = 1;
        cycleDone = true;
    }

    engine->setPosition(pos);
    engine->setDirection(dir);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 13. Knight Rider Effect
// 3 relays wide bouncing Larson scanner
// ----------------------------------------------------------------------------
bool EffectLibrary::effectKnightRider(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    int dir = engine->getDirection();
    if (dir != 1 && dir != -1) dir = 1;
    if (pos < 0 || pos >= RELAY_COUNT) pos = 0;

    uint16_t mask = (static_cast<uint16_t>(1) << pos);
    if (pos > 0) {
        mask |= (static_cast<uint16_t>(1) << (pos - 1));
    }
    if (pos < 15) {
        mask |= (static_cast<uint16_t>(1) << (pos + 1));
    }
    gpio->setRelayPattern(mask);

    pos += dir;
    bool cycleDone = false;
    if (pos >= 15) {
        pos = 15;
        dir = -1;
    } else if (pos <= 0) {
        pos = 0;
        dir = 1;
        cycleDone = true;
    }

    engine->setPosition(pos);
    engine->setDirection(dir);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 14. Comet Chase
// Leading head + 3-channel trailing tail
// ----------------------------------------------------------------------------
bool EffectLibrary::effectCometChase(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= 20) pos = 0;

    uint16_t mask = 0;
    for (int i = 0; i < 4; i++) {
        int ch = pos - i;
        if (ch >= 0 && ch < RELAY_COUNT) {
            mask |= (static_cast<uint16_t>(1) << ch);
        }
    }
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= 20);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 15. Bouncing Light
// Non-linear trajectory simulating bounce acceleration
// ----------------------------------------------------------------------------
bool EffectLibrary::effectBouncingLight(EffectEngine* engine, GPIOController* gpio) {
    static const uint8_t bounceCurve[16] = {
        0, 1, 3, 6, 9, 12, 14, 15, 14, 12, 9, 6, 4, 2, 1, 0
    };

    int step = engine->getPosition();
    if (step < 0 || step >= 16) step = 0;

    gpio->setRelayPattern(static_cast<uint16_t>(1) << bounceCurve[step]);

    step++;
    bool cycleDone = (step >= 16);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 16. Waterfall Effect
// Sequential fill (0..15) then sequential empty (0..15)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectWaterfall(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 32) step = 0;

    uint16_t mask = 0;
    if (step < 16) {
        // Filling 0 up to step
        mask = (step == 15) ? 0xFFFF : static_cast<uint16_t>((1 << (step + 1)) - 1);
    } else {
        // Emptying: relays below (step - 15) turn off
        int offCount = step - 15;
        mask = static_cast<uint16_t>(0xFFFF << offCount);
    }
    gpio->setRelayPattern(mask);

    step++;
    bool cycleDone = (step >= 32);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 17. Wave Effect
// Multi-crest undulating wave pattern shifting across 16 channels
// ----------------------------------------------------------------------------
bool EffectLibrary::effectWave(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= 16) pos = 0;

    const uint16_t baseWave = 0x1C1C; // 0001 1100 0001 1100
    uint16_t mask = (baseWave << pos) | (baseWave >> (16 - pos));
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= 16);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 18. Fill and Empty
// Outside-in fill followed by outside-in empty
// ----------------------------------------------------------------------------
bool EffectLibrary::effectFillAndEmpty(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 16) step = 0;

    uint16_t mask = 0;
    if (step < 8) {
        // Progressively fill inward
        for (int i = 0; i <= step; i++) {
            mask |= (static_cast<uint16_t>(1) << i) | (static_cast<uint16_t>(1) << (15 - i));
        }
    } else {
        // Progressively empty inward
        int cleared = step - 7;
        mask = 0xFFFF;
        for (int i = 0; i < cleared; i++) {
            mask &= ~((static_cast<uint16_t>(1) << i) | (static_cast<uint16_t>(1) << (15 - i)));
        }
    }
    gpio->setRelayPattern(mask);

    step++;
    bool cycleDone = (step >= 16);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 19. Random Channel Chase
// Random single relay each step (16 steps per cycle)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectRandomChannelChase(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 16) step = 0;

    uint8_t randomCh = esp_random() % RELAY_COUNT;
    gpio->setRelayPattern(static_cast<uint16_t>(1) << randomCh);

    step++;
    bool cycleDone = (step >= 16);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 20. Random Group Flash
// Random cluster of 4 to 8 relays per step (12 steps per cycle)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectRandomGroupFlash(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 12) step = 0;

    int activeCount = 4 + (esp_random() % 5); // 4 to 8 active relays
    uint16_t mask = 0;
    while (activeCount > 0) {
        uint8_t ch = esp_random() % RELAY_COUNT;
        uint16_t bit = (static_cast<uint16_t>(1) << ch);
        if (!(mask & bit)) {
            mask |= bit;
            activeCount--;
        }
    }
    gpio->setRelayPattern(mask);

    step++;
    bool cycleDone = (step >= 12);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 21. Binary Counter Pattern
// Binary count 0 to 63 displayed on relays
// ----------------------------------------------------------------------------
bool EffectLibrary::effectBinaryCounter(EffectEngine* engine, GPIOController* gpio) {
    uint32_t val = engine->getStep() & 0x3F; // 64 steps (0..63)
    gpio->setRelayPattern(static_cast<uint16_t>(val));
    return (val == 0x3F);
}

// ----------------------------------------------------------------------------
// 22. Progressive Fill
// Progressively turn ON left to right, then turn all OFF
// ----------------------------------------------------------------------------
bool EffectLibrary::effectProgressiveFill(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= 17) pos = 0;

    if (pos < 16) {
        uint16_t mask = (pos == 15) ? 0xFFFF : static_cast<uint16_t>((1 << (pos + 1)) - 1);
        gpio->setRelayPattern(mask);
    } else {
        gpio->setRelayPattern(0x0000);
    }

    pos++;
    bool cycleDone = (pos >= 17);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 23. Progressive Empty
// Starts all ON, turns OFF left to right
// ----------------------------------------------------------------------------
bool EffectLibrary::effectProgressiveEmpty(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= 16) pos = 0;

    uint16_t mask = 0xFFFF & ~static_cast<uint16_t>((1 << (pos + 1)) - 1);
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= 16);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 24. Split Half Chase
// Left 8 (0..7) and right 8 (8..15) chase synchronously
// ----------------------------------------------------------------------------
bool EffectLibrary::effectSplitHalfChase(EffectEngine* engine, GPIOController* gpio) {
    int pos = engine->getPosition();
    if (pos < 0 || pos >= 8) pos = 0;

    uint16_t mask = (static_cast<uint16_t>(1) << pos) | 
                    (static_cast<uint16_t>(1) << (pos + 8));
    gpio->setRelayPattern(mask);

    pos++;
    bool cycleDone = (pos >= 8);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 25. Symmetrical Mirror Chase
// Outside to center and back (14 steps per cycle)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectSymmetricalMirrorChase(EffectEngine* engine, GPIOController* gpio) {
    static const uint8_t mirrorSteps[14] = {
        0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1
    };

    int step = engine->getPosition();
    if (step < 0 || step >= 14) step = 0;

    uint8_t left = mirrorSteps[step];
    uint8_t right = 15 - left;
    gpio->setRelayPattern((static_cast<uint16_t>(1) << left) | (static_cast<uint16_t>(1) << right));

    step++;
    bool cycleDone = (step >= 14);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 26. Fast Strobe
// All ON / all OFF at rapid cadence (16 toggles = 8 flashes)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectFastStrobe(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 16) step = 0;

    gpio->setRelayPattern((step % 2 == 0) ? 0xFFFF : 0x0000);

    step++;
    bool cycleDone = (step >= 16);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 27. Slow Strobe
// Slower duty strobe (8 toggles = 4 flashes)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectSlowStrobe(EffectEngine* engine, GPIOController* gpio) {
    int step = engine->getPosition();
    if (step < 0 || step >= 8) step = 0;

    gpio->setRelayPattern((step % 2 == 0) ? 0xFFFF : 0x0000);

    step++;
    bool cycleDone = (step >= 8);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 28. Cross Pattern
// Channels 1,6,11,16 (indices 0,5,10,15) and 4,5,12,13 (indices 3,4,11,12) alternating
// ----------------------------------------------------------------------------
bool EffectLibrary::effectCrossPattern(EffectEngine* engine, GPIOController* gpio) {
    const uint16_t patternA = (1 << 0) | (1 << 5) | (1 << 10) | (1 << 15); // 0x8421
    const uint16_t patternB = (1 << 3) | (1 << 4) | (1 << 11) | (1 << 12); // 0x1818

    int step = engine->getPosition();
    if (step < 0 || step >= 6) step = 0;

    gpio->setRelayPattern((step % 2 == 0) ? patternA : patternB);

    step++;
    bool cycleDone = (step >= 6);
    if (cycleDone) step = 0;
    engine->setPosition(step);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 29. Spiral-Style Sequence
// Sequence: 1,16,2,15,3,14,4,13,5,12,6,11,7,10,8,9 (1-based)
// Zero-based: 0, 15, 1, 14, 2, 13, 3, 12, 4, 11, 5, 10, 6, 9, 7, 8
// ----------------------------------------------------------------------------
bool EffectLibrary::effectSpiralSequence(EffectEngine* engine, GPIOController* gpio) {
    static const uint8_t spiralOrder[16] = {
        0, 15, 1, 14, 2, 13, 3, 12, 4, 11, 5, 10, 6, 9, 7, 8
    };

    int pos = engine->getPosition();
    if (pos < 0 || pos >= 16) pos = 0;

    gpio->setRelayPattern(static_cast<uint16_t>(1) << spiralOrder[pos]);

    pos++;
    bool cycleDone = (pos >= 16);
    if (cycleDone) pos = 0;
    engine->setPosition(pos);
    return cycleDone;
}

// ----------------------------------------------------------------------------
// 30. Combination Chase
// Sequences 4 sub-patterns:
// Phase 0: Ping-pong (30 steps)
// Phase 1: Alternate Odd/Even (6 steps)
// Phase 2: Center-Out (8 steps)
// Phase 3: Waterfall (32 steps)
// ----------------------------------------------------------------------------
bool EffectLibrary::effectCombinationChase(EffectEngine* engine, GPIOController* gpio) {
    uint32_t phase = engine->getCustomState();
    bool subCycleDone = false;

    switch (phase) {
        case 0:
            subCycleDone = effectPingPongChase(engine, gpio);
            break;
        case 1:
            subCycleDone = effectAlternateChannelFlash(engine, gpio);
            break;
        case 2:
            subCycleDone = effectCenterOut(engine, gpio);
            break;
        case 3:
        default:
            subCycleDone = effectWaterfall(engine, gpio);
            break;
    }

    if (subCycleDone) {
        phase++;
        engine->setPosition(0);
        engine->setDirection(1);
        if (phase >= 4) {
            phase = 0;
            engine->setCustomState(0);
            return true; // Complete combination cycle finished
        }
        engine->setCustomState(phase);
    }

    return false;
}
