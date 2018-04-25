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

void inline no_op(const struct inputs *);
void solid_color_change(const struct inputs *);

void shared_sparkle_start(const struct inputs *);
void sparkle_tick(const struct inputs *);
void rainbow_sparkle_tick(const struct inputs *);

void rainbow_tick(const struct inputs *);

size_t successor(const size_t, const size_t);

// - Statics
// - - - - - - - - - - - - - - - - - - - -
static Adafruit_NeoPixel led_strip;
static struct inputs last_inputs;

static struct mode modes[] = {
  {
    .start  = solid_color_change,
    .change = solid_color_change,
    .tick   = no_op
  },
  {
    .start  = shared_sparkle_start,
    .change = no_op,
    .tick   = sparkle_tick
  },
  {
    .start  = shared_sparkle_start,
    .change = no_op,
    .tick   = rainbow_sparkle_tick
  },
  {
    .start  = no_op,
    .change = no_op,
    .tick   = rainbow_tick
  }
};
static size_t mode_index = 0;
static size_t mode_count = sizeof(modes) / sizeof(struct mode);

// pointer to the current mode
#define current_mode (&(modes[mode_index]))

#define map_to_uint8(x) (uint8_t)(map((x), 0, 1023, 0, UINT8_MAX))

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
    .red = map_to_uint8(analogRead(RED_DIAL_PIN)),
    .green = map_to_uint8(analogRead(GREEN_DIAL_PIN)),
    .blue = map_to_uint8(analogRead(BLUE_DIAL_PIN))
  };

  if (!inputs_equals(&last_inputs, &new_inputs)) {
    if (last_inputs.button != new_inputs.button && new_inputs.button == HIGH) {
      mode_index = successor(mode_index, mode_count);
      current_mode->start(&new_inputs);
    } else {
      current_mode->change(&new_inputs);
    }
  }

  current_mode->tick(&new_inputs);
  last_inputs = new_inputs;
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

// - No-Op Function
// - - - - - - - - - - - - - - - - - - - -
void inline no_op(__attribute__((unused)) const struct inputs *_unused) {};

// - Solid Color Mode
// - - - - - - - - - - - - - - - - - - - -
void solid_color_change(const struct inputs *inputs) {
  for (size_t idx = 0; idx < LED_COUNT; ++idx) {
    led_strip.setPixelColor(idx, inputs->red, inputs->green, inputs->blue);
  }
  led_strip.show();
}

// - Sparkle Mode Shared
// - - - - - - - - - - - - - - - - - - - -
void shared_sparkle_start(__attribute__((unused)) const struct inputs *inputs) {
  for (size_t idx = 0; idx < LED_COUNT; ++idx) {
    led_strip.setPixelColor(idx, 0, 0, 0);
  }
  led_strip.show();
}

// - Sparkle Mode
// - - - - - - - - - - - - - - - - - - - -
void sparkle_tick(__attribute__((unused))  const struct inputs *inputs) {
  static unsigned long next_schedule = 0;
  static const size_t index_count = 7;
  const unsigned long now = millis();
  
  if (now >= next_schedule) {
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

// - Rainbow Sparkle Mode
// - - - - - - - - - - - - - - - - - - - -
void rainbow_sparkle_tick(__attribute__((unused))  const struct inputs *inputs) {
  static unsigned long next_schedule = 0;
  static const size_t index_count = 7;
  const unsigned long now = millis();
  
  if (now >= next_schedule) {
    for (size_t idx = 0; idx < index_count; ++idx) {
      led_strip.setPixelColor(random(LED_COUNT), random(2) ? 255 : 0, random(2) ? 255 : 0, random(2) ? 255 : 0);
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
static uint32_t rainbow_wheel(uint8_t wheel_pos) {
  if (wheel_pos < 85) {
    return Adafruit_NeoPixel::Color(wheel_pos * 3, 255 - wheel_pos * 3, 0);
  } else if(wheel_pos < 170) {
    wheel_pos -= 85;
    return Adafruit_NeoPixel::Color(255 - wheel_pos * 3, 0, wheel_pos * 3);
  } else {
    wheel_pos -= 170;
    return Adafruit_NeoPixel::Color(0, wheel_pos * 3, 255 - wheel_pos * 3);
  }
}

// - - - - - - - - - - - - - - - - - - - -
void rainbow_tick(const struct inputs *inputs) {
  static unsigned long next_schedule = 0;
  const unsigned long now = millis();
  static uint8_t color_step = 0;

  if (now >= next_schedule) {
    for (size_t idx = 0; idx < LED_COUNT; ++idx) {
      const uint32_t color = rainbow_wheel(((idx * UINT8_MAX) / LED_COUNT) + color_step);
      led_strip.setPixelColor(idx, color);
    }

    // schedule next update based on red dial
    next_schedule = now + map(inputs->red, 0, UINT8_MAX, 0, 45);
    led_strip.show();
    color_step++; // rolls over to zero after 255
  }
}


