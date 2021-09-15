/*
 * my_accessory.c
 * Define the accessory in C language using the Macro in characteristics.h
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 */

#include <homekit/characteristics.h>
#include <homekit/homekit.h>

#define HOMEKIT_CHARACTERISTIC_DOOR_STATE_OPEN 0
#define HOMEKIT_CHARACTERISTIC_DOOR_STATE_CLOSED 1
#define HOMEKIT_CHARACTERISTIC_DOOR_STATE_OPENING 2
#define HOMEKIT_CHARACTERISTIC_DOOR_STATE_CLOSING 3
#define HOMEKIT_CHARACTERISTIC_DOOR_STATE_STOPPED 4
#define HOMEKIT_CHARACTERISTIC_DOOR_STATE_UNKNOWN 255

void builtInLed_set_status(bool on) { digitalWrite(2, on ? 0 : 1); }

void builtInLed_blink(int intervalInMs, int count) {
  for (int i = 0; i < count; i++) {
    builtInLed_set_status(true);
    delay(intervalInMs);
    builtInLed_set_status(false);
    delay(intervalInMs);
  }
}

void identify(homekit_value_t _value) {
  printf("Identify\n");
  builtInLed_blink(500, 3);
}

/**
 Defines that the accessory has control over the opening of a garage door.

 Required Characteristics:
 - CURRENT_DOOR_STATE
 - TARGET_DOOR_STATE
 - OBSTRUCTION_DETECTED

 Optional Characteristics:
 - NAME
 - LOCK_STATE
 - LOCK_TARGET_STATE
 */

// Possible values for characteristic CURRENT_DOOR_STATE:
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN 0
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED 1
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING 2
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING 3
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_STOPPED 4
#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN 255

#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN 0
#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED 1
#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_UNKNOWN 255

#define OPEN_CLOSE_DURATION 22

const char *state_description(uint8_t state) {
  const char *description = "unknown";
  switch (state) {
  case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN:
    description = "open";
    break;
  case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING:
    description = "opening";
    break;
  case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED:
    description = "closed";
    break;
  case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING:
    description = "closing";
    break;
  case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_STOPPED:
    description = "stopped";
    break;
  default:;
  }
  return description;
}

homekit_value_t gdo_target_state_get();
void gdo_target_state_set(homekit_value_t new_value);
homekit_value_t gdo_current_state_get();
homekit_value_t gdo_obstruction_get();
void identify(homekit_value_t _value);

// Declare global variables:

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(
            .id = 1, .category = homekit_accessory_category_garage,
            .services =
                (homekit_service_t *[]){
                    HOMEKIT_SERVICE(
                        ACCESSORY_INFORMATION,
                        .characteristics =
                            (homekit_characteristic_t *[]){
                                HOMEKIT_CHARACTERISTIC(NAME, "Garage"),
                                HOMEKIT_CHARACTERISTIC(MANUFACTURER,
                                                       "Arsalan®"),
                                HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "123456"),
                                HOMEKIT_CHARACTERISTIC(MODEL, "Brno"),
                                HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION,
                                                       "1.0"),
                                HOMEKIT_CHARACTERISTIC(IDENTIFY, identify),
                                NULL}),
                    HOMEKIT_SERVICE(
                        GARAGE_DOOR_OPENER, .primary = true,
                        .characteristics =
                            (homekit_characteristic_t *[]){
                                HOMEKIT_CHARACTERISTIC(NAME, "Garage"),
                                HOMEKIT_CHARACTERISTIC(
                                    CURRENT_DOOR_STATE,
                                    HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED,
                                    .getter = gdo_current_state_get,
                                    .setter = NULL),
                                HOMEKIT_CHARACTERISTIC(
                                    TARGET_DOOR_STATE,
                                    HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED,
                                    .getter = gdo_target_state_get,
                                    .setter = gdo_target_state_set),
                                HOMEKIT_CHARACTERISTIC(
                                    OBSTRUCTION_DETECTED,
                                    HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED,
                                    .getter = gdo_obstruction_get,
                                    .setter = NULL),
                                NULL}),
                    NULL}),
    NULL};

bool relay_on = false;
uint8_t current_door_state = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN;

homekit_value_t gdo_obstruction_get() { return HOMEKIT_BOOL(false); }

void gdo_current_state_notify_homekit() {

  homekit_value_t new_value = HOMEKIT_UINT8(current_door_state);
  printf("Notifying homekit that current door state is now '%s'\n",
         state_description(current_door_state));

  // Find the current door state characteristic c:
  homekit_accessory_t *accessory = accessories[0];
  homekit_service_t *service = accessory->services[1];
  homekit_characteristic_t *c = service->characteristics[1];

  assert(c);

  printf("Notifying changed '%s'\n", c->description);
  homekit_characteristic_notify(c, new_value);
}

void gdo_target_state_notify_homekit() {

  homekit_value_t new_value = gdo_target_state_get();
  printf("Notifying homekit that target door state is now '%s'\n",
         state_description(new_value.int_value));

  // Find the target door state characteristic c:
  homekit_accessory_t *accessory = accessories[0];
  homekit_service_t *service = accessory->services[1];
  homekit_characteristic_t *c = service->characteristics[2];

  assert(c);

  printf("Notifying changed '%s'\n", c->description);
  homekit_characteristic_notify(c, new_value);
}

void current_state_set(uint8_t new_state) {
  // if (current_door_state != new_state) {
  current_door_state = new_state;
  gdo_target_state_notify_homekit();
  delay(2000);
  gdo_current_state_notify_homekit();
  // }
}

int sensor_state = 0;

void current_door_state_update_from_sensor() {

  switch (sensor_state) {
  case 1: // closed
    current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN);
    break;
  case 0:
    current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED);
    break;
  default:
    printf("Unknown contact sensor event: %d\n", sensor_state);
  }
}

homekit_value_t gdo_current_state_get() {
  if (current_door_state == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN) {
    current_door_state_update_from_sensor();
  }
  printf("returning current door state '%s'.\n",
         state_description(current_door_state));

  return HOMEKIT_UINT8(current_door_state);
}

homekit_value_t gdo_target_state_get() {
  uint8_t result = gdo_current_state_get().int_value;
  if (result == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING) {
    result = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN;
  } else if (result == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING) {
    result = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED;
  }

  printf("Returning target door state '%s'.\n", state_description(result));

  return HOMEKIT_UINT8(result);
}

/**
 * Called (indirectly) from the interrupt handler to notify the client of a
 *state change.
 **/
void contact_sensor_state_changed(uint8_t gpio, int state) {

  printf("contact sensor state '%s'.\n", state == 0 ? "open" : "closed");

  if (current_door_state == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING ||
      current_door_state == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING) {
    // Ignore the event - the state will be updated after the time expired!
    printf(
        "contact_sensor_state_changed() ignored during opening or closing.\n");
    return;
  }
  current_door_state_update_from_sensor();
}

void gdo_target_state_set(homekit_value_t new_value) {

  if (new_value.format != homekit_format_uint8) {
    printf("Invalid value format: %d\n", new_value.format);
    return;
  }

  if (current_door_state != HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN &&
      current_door_state != HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED) {
    printf("gdo_target_state_set() ignored: current state not open or closed "
           "(%s).\n",
           state_description(current_door_state));
    return;
  }

  if (current_door_state == new_value.int_value) {
    printf("gdo_target_state_set() ignored: new target state == current state "
           "(%s)\n",
           state_description(current_door_state));
    return;
  }

  // Toggle the garage door by toggling the relay connected to the GPIO (on -
  // off): Turn ON GPIO:
  // relay_write(true);
  // Wait for some time:
  // vTaskDelay(400 / portTICK_PERIOD_MS);
  // Turn OFF GPIO:
  // relay_write(false);
  if (current_door_state == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED) {
    current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN);
  } else {
    current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED);
  }
  // Wait for the garage door to open / close,
  // then update current_door_state from sensor:
  // sdk_os_timer_arm(&update_timer, OPEN_CLOSE_DURATION * 1000, false);
  // delay(3000);

  // current_door_state_update_from_sensor();
  // sensor_state = rand() % 2;
}

// static void timer_callback(void *arg) {

//   printf("Timer fired. Updating state from sensor.\n");
//   sdk_os_timer_disarm(&update_timer);
//   current_door_state_update_from_sensor();
// }

homekit_server_config_t config = {.accessories = accessories,
                                  .password = "111-11-111"};
