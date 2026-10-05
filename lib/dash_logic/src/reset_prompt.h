#pragma once

// How long "TAP TO CLEAR" stays up after a long press before cancelling.
const unsigned long RESET_PROMPT_MS = 3000;

// Two-step reset for trip and chart pages: a long press opens the prompt,
// and only a tap while it's open clears the data.
class ResetPrompt {
public:
    void open(unsigned long nowMs) {
        openedMs = nowMs;
        opened = true;
    }

    bool isOpen(unsigned long nowMs) const {
        return opened && nowMs - openedMs < RESET_PROMPT_MS;
    }

    // Call on a tap. True means reset now; the prompt closes either way.
    bool confirm(unsigned long nowMs) {
        bool yes = isOpen(nowMs);
        opened = false;
        return yes;
    }

private:
    bool opened = false;
    unsigned long openedMs = 0;
};
