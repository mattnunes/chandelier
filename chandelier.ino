#include <Adafruit_NeoPixel.h>

// - Constants
// - - - - - - - - - - - - - - - - - - - -
#define DEBUG           1
#define LED_COUNT       178
#define LED_STRIP_PIN   6
#define PUSH_BUTTON_PIN 12
#define RED_DIAL_PIN    A2
#define GREEN_DIAL_PIN  A1
#define BLUE_DIAL_PIN   A0

// - Types
// - - - - - - - - - - - - - - - - - - - -
struct inputs {
  uint8_t button;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct mode {
  void (*start)(const struct inputs *);
  void (*change)(const struct inputs *);
  void (*tick)(const struct inputs *);
};

// - Prototypes
// - - - - - - - - - - - - - - - - - - - -
bool inputs_equals(const struct inputs *, const struct inputs *);

void solid_color_change(const struct inputs *);

void sparkle_start(const struct inputs *);
void sparkle_tick(const struct inputs *);

void rainbow_tick(const struct inputs *);

size_t successor(const size_t, const size_t);

// - Statics
// - - - - - - - - - - - - - - - - - - - -
static Adafruit_NeoPixel led_strip;
static struct inputs last_inputs;

static struct mode modes[] = {
  {
    .start  = NULL,
    .change = NULL,
    .tick   = rainbow_tick
  },
  {
    .start  = NULL,
    .change = solid_color_change,
    .tick   = NULL
  },
  {
    .start  = sparkle_start,
    .change = NULL,
    .tick   = sparkle_tick
  }
};
static size_t mode_index = 0;
static size_t mode_count = sizeof(modes) / sizeof(struct mode);

#define current_mode (&(modes[mode_index]))

// - Setup
// - - - - - - - - - - - - - - - - - - - -
void setup() {
#if DEBUG
  { // Serial initialization
    Serial.begin(9600);
  }
#endif

  { // strip initialization;
    led_strip = Adafruit_NeoPixel(LED_COUNT, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
    led_strip.begin();
  }
}

// - Loop
// - - - - - - - - - - - - - - - - - - - -
void loop() {
  const struct inputs new_inputs = (struct inputs) {
    .button = (uint8_t)digitalRead(PUSH_BUTTON_PIN),
    .red = (uint8_t)(analogRead(RED_DIAL_PIN) / 4),
    .green = (uint8_t)(analogRead(GREEN_DIAL_PIN) / 4),
    .blue = (uint8_t)(analogRead(BLUE_DIAL_PIN) / 4)
  };

  if (!inputs_equals(&last_inputs, &new_inputs)) {
    if (last_inputs.button != new_inputs.button && new_inputs.button == HIGH) {
      mode_index = successor(mode_index, mode_count);
      if (current_mode->start) {
        current_mode->start(&new_inputs);
      }
    } else {
      if (current_mode->change) {
        current_mode->change(&new_inputs);
      }
    }
  }

  if (current_mode->tick) {
    current_mode->tick(&new_inputs);
  }

  last_inputs = new_inputs;
  led_strip.show();
}

// - - - - - - - - - - - - - - - - - - - -
bool inputs_equals(const struct inputs *lhs, const struct inputs *rhs) {
  return (lhs->button == rhs->button)
      && (lhs->red == rhs->red)
      && (lhs->green == rhs->green)
      && (lhs->blue == rhs->blue);
}

// - - - - - - - - - - - - - - - - - - - -
size_t successor(const size_t start, const size_t maximum) {
  if ((maximum - 1) == start) {
    return 0;
  } else {
    return start + 1;
  }
}

// - Solid Color Mode
// - - - - - - - - - - - - - - - - - - - -
void solid_color_change(const struct inputs *inputs) {
  for (size_t idx = 0; idx < LED_COUNT; ++idx) {
    led_strip.setPixelColor(idx, inputs->red, inputs->green, inputs->blue);
  }
  led_strip.show();
}

// - Sparkle Mode
// - - - - - - - - - - - - - - - - - - - -
void sparkle_start(__attribute__((unused)) const struct inputs *inputs) {
  for (size_t idx = 0; idx < LED_COUNT; ++idx) {
    led_strip.setPixelColor(idx, 0, 0, 0);
  }
  led_strip.show();
}

// - - - - - - - - - - - - - - - - - - - -
void sparkle_tick(__attribute__((unused))  const struct inputs *inputs) {
  static unsigned long next_schedule = 0;
  static const size_t index_count = 7;
  const unsigned long now = millis();
  
  if (now > next_schedule) {
    for (size_t idx = 0; idx < index_count; ++idx) {
      led_strip.setPixelColor(random(LED_COUNT), 255, 255, 255);
    }
    led_strip.show();
    next_schedule = now + 25;
  } else {
    for (size_t idx = 0; idx < LED_COUNT; ++idx) {
      led_strip.setPixelColor(idx, 0, 0, 0);
    }
    led_strip.show();
    next_schedule = 0;
  }
}

// - Rainbow Mode
// - - - - - - - - - - - - - - - - - - - -
void rainbow_tick(const struct inputs *inputs) {

}


