#pragma once

#include <stdint.h>

struct QuestionDef {
    const char* text;          // max 37 chars
    const char* const* options;
    uint8_t numOptions;
};

// ── Tutorial option lists ────────────────────────────────────────────────────

// Step 1: user starts on "X" (index 0) and rotates right to reach "OK".
// Rotating left at X is allowed (encoder registers the intent) but stays clamped at 0.
static const char* const TUTORIAL_STEP1_OPTIONS[] = { "X", "OK" };
static const uint8_t TUTORIAL_STEP1_START   = 0;   // initial position: X
static const uint8_t TUTORIAL_STEP1_OK_IDX  = 1;   // OK is at index 1

// Step 2: five X options, user can scroll freely
static const char* const TUTORIAL_STEP2_OPTIONS[] = { "X", "X", "X", "X", "X" };
static const uint8_t TUTORIAL_STEP2_NUM     = 5;

// Step 3 (T1 confirmed): single "OK" option
static const char* const TUTORIAL_STEP3_OPTIONS[] = { "OK" };

// ── Survey questions ─────────────────────────────────────────────────────────

static const char* const Q1_OPTIONS[] = { "1", "2", "3", "4", "5" };
static const char* const Q2_OPTIONS[] = { "Very Low", "Low", "Neutral", "High", "Very High" };
static const char* const Q3_OPTIONS[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
static const char* const Q4_OPTIONS[] = { "Tetra", "Betta", "Gourami", "Shark", "Crab" };

static const QuestionDef QUESTIONS[4] = {
    { "How would you rate J&J on innovation?", Q1_OPTIONS, 5  },
    { "Happy with J&J interactions?",          Q2_OPTIONS, 5  },
    { "Recommend J&J to a colleague?",         Q3_OPTIONS, 11 },
    { "Pick your fish type!",                  Q4_OPTIONS, 5  },
};

static const uint8_t NUM_QUESTIONS = 4;

// ── WaitOrClick messages ─────────────────────────────────────────────────────

static const char* const MSG_SUCCESS     = "Thanks!\n\nFind yourself at the bigger screen.";
static const char* const MSG_SEND_FAILED = "Error!\n\nSend failed! Please try again.";
