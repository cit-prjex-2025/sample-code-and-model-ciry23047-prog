#include "app.h"
#include "util.h"
#include "horn.h"
#include "timer.h"

#define ONE_SECOND 10000 * 100

const int bumper_sensor = EV3_PORT_1;
const int linemon_sensor = EV3_PORT_3;
const int left_motor = EV3_PORT_A;
const int right_motor = EV3_PORT_C;

const int seat_sensor = EV3_PORT_2;

int seat_is_boarded(void) {
  return ev3_touch_sensor_is_pressed(seat_sensor);
}

const int walldetector_sensor = EV3_PORT_4;
#define WD_DISTANCE 10
int wd_distance = WD_DISTANCE;

int wall_detector_is_detected(void) {
  return ev3_ultrasonic_sensor_get_distance(walldetector_sensor) < wd_distance;
}

int bumper_is_pushed(void) {
  return ev3_touch_sensor_is_pressed(bumper_sensor);
}

#define LM_THRESHOLD 20
int lm_threshold = LM_THRESHOLD;

int linemon_is_online(void) {
  return ev3_color_sensor_get_reflect(linemon_sensor) < lm_threshold;
}

#define DR_POWER 20
int dr_power = DR_POWER;

void driver_turn_left(void) {
  ev3_motor_set_power(EV3_PORT_A, 0);
  ev3_motor_set_power(EV3_PORT_C, dr_power);
}

void driver_turn_right(void) {
  ev3_motor_set_power(EV3_PORT_A, dr_power);
  ev3_motor_set_power(EV3_PORT_C, 0);
}

void driver_stop(void) {
  ev3_motor_stop(left_motor, false);
  ev3_motor_stop(right_motor, false);
}

void tracer_run(void) {
  if( linemon_is_online() ) {
    driver_turn_left();
  } else {
    driver_turn_right();
  }
}

void tracer_stop(void) {
    driver_stop();
}

typedef enum { // <1>
  P_WAIT_FOR_LOADING, P_WAIT_FOR_INSTRUCTION, P_CONFIRM_INSTRUCTION,
  P_INSTRUCTION_BUMPER_IS_PUSHED, P_TRANSPORTING, P_TRANSPORT_CONFLICT, 
  P_TRANSPORT_CONFLICT_RESOLVE, P_PASSENGER_IS_FALLEN, P_PASSENGER_IS_EMPTY, 
  P_STOP, P_REPOSITIONING_BUMPER_IS_PUSHED, P_REPOSITIONING,
  P_REPOSITION_CONFLICT, P_REPOSITION_CONFLICT_RESOLVE
} porter_state; // <2>

porter_state p_state = P_WAIT_FOR_LOADING; // <3>

int p_entry = true;

void porter_transport(void) {
  num_f(p_state, 2);

  switch(p_state) {
  case P_WAIT_FOR_LOADING:
    if(p_entry) {
      p_entry = false;
    }
    if(seat_is_boarded()) {  // 客が乗った時
      p_state = P_WAIT_FOR_INSTRUCTION;
      p_entry = true;
    } else if(bumper_is_pushed()) {  // 回送指示を受けた時
      p_state = P_REPOSITIONING;
      p_entry = true;
    }
    if(p_entry) {

    }
    break;
  case P_WAIT_FOR_INSTRUCTION:
    if(p_entry) {
      p_entry = false;
      timer_start(ONE_SECOND * 5);
    }
    if(timer_is_timedout()) {
      p_state = P_CONFIRM_INSTRUCTION;
      p_entry = true;
    }else if(bumper_is_pushed()) {  // 走行開始指示を受けた時
      p_state = P_INSTRUCTION_BUMPER_IS_PUSHED;
      p_entry = true;
    }
    if(p_entry) {
        
    }
    break;
  case P_INSTRUCTION_BUMPER_IS_PUSHED:
    if(p_entry) {
      p_entry = false;
    }
    if(!bumper_is_pushed()) {
      p_state = P_TRANSPORTING;
      p_entry = true;
    }
    if(p_entry) {
      horn_confirmation();
    }
    break;
  case P_CONFIRM_INSTRUCTION:
    if(p_entry) {
      p_entry = false;
      horn_confirmation();
    }
    if(true) {
      p_state = P_WAIT_FOR_INSTRUCTION;
      p_entry = true;
    }
    if(p_entry) {

    }
    break;
  case P_TRANSPORTING:
    if(p_entry) {
      p_entry = false;
      timer_start(ONE_SECOND * 3);
    }
    tracer_run();
    if(wall_detector_is_detected() && timer_is_timedout()) {  // 側壁を発見かつタイムアウト
      p_state = P_STOP;
      p_entry = true;
    } else if(bumper_is_pushed()) {
      p_state = P_TRANSPORT_CONFLICT;
      p_entry = true;
    } else if(!seat_is_boarded()) {
      p_state = P_PASSENGER_IS_FALLEN;
      p_entry = true;
    }
    if(p_entry) {
      tracer_stop();
    }
    break;
  case P_TRANSPORT_CONFLICT:
    if(p_entry) {
      p_entry = false;
      horn_warning();
    }
    if(!bumper_is_pushed()) {
      p_state = P_TRANSPORT_CONFLICT_RESOLVE;
      p_entry = true;
    }
    if(p_entry) {

    }
    break;
  case P_TRANSPORT_CONFLICT_RESOLVE:
    if(p_entry) {
      p_entry = false;
      timer_start(ONE_SECOND * 5);
    }
    if(timer_is_timedout()) {
      p_state = P_TRANSPORTING;
      p_entry = true;
      horn_confirmation();
    }
    if(p_entry) {
      
    }
    break;
  case P_PASSENGER_IS_FALLEN:
    if(p_entry) {
      p_entry = false;
      timer_start(ONE_SECOND * 5);
    }
    if(timer_is_timedout()) {
      p_state = P_PASSENGER_IS_EMPTY;
      p_entry = true;
    }else if(seat_is_boarded()) {
      p_state = P_WAIT_FOR_INSTRUCTION;
      p_entry = true;
    }
    if(p_entry) {

    }
    break;
  case P_PASSENGER_IS_EMPTY:
    if(p_entry) {
      p_entry = false;
      horn_warning();
    }
    if(true) {
      p_state = P_PASSENGER_IS_FALLEN;
      p_entry = true;
    }
    if(p_entry) {

    }
    break;
  case P_STOP:
    if(p_entry) {
      p_entry = false;
    }
    if(!seat_is_boarded()) {
      p_state = P_WAIT_FOR_LOADING;
      p_entry = true;
    }else if(bumper_is_pushed()) {
      p_state = P_INSTRUCTION_BUMPER_IS_PUSHED;
      p_entry = true;
    }
    if(p_entry) {

    }
    break;
  case P_REPOSITIONING_BUMPER_IS_PUSHED:
    if(p_entry) {
      p_entry = false;
    }
    if(!bumper_is_pushed()) {
      p_state = P_REPOSITIONING;
      p_entry = true;
    }
    if(p_entry) {
      horn_confirmation();
    }
    break;
  case P_REPOSITIONING:
    if(p_entry) {
      p_entry = false;
      timer_start(ONE_SECOND * 3);
    }
    tracer_run();
    if(wall_detector_is_detected() && timer_is_timedout()) {  // 側壁を発見かつタイムアウト
      p_state = P_WAIT_FOR_LOADING;
      p_entry = true;
    }else if(bumper_is_pushed()) {
      p_state = P_REPOSITION_CONFLICT;
      p_entry = true;
    }
    if(p_entry) {
      tracer_stop();
    }
    break;
  case P_REPOSITION_CONFLICT:
    if(p_entry) {
      p_entry = false;
      horn_warning();
    }
    if(!bumper_is_pushed()) {
      p_state = P_REPOSITION_CONFLICT_RESOLVE;
      p_entry = true;
    }
    if(p_entry) {

    }
    break;
  case P_REPOSITION_CONFLICT_RESOLVE:
    if(p_entry) {
      p_entry = false;
      timer_start(ONE_SECOND * 5);
    }
    if(timer_is_timedout()) {
      p_state = P_REPOSITIONING;
      p_entry = true;
      horn_confirmation();
    }
    if(p_entry) {
      
    }
    break;
  default:
    break;
  }
}

void main_task(intptr_t unused) {
  static int is_initialized = false;
  if(! is_initialized ) {
    is_initialized = true;
    init_f("park_ride_service");
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);
    ev3_sensor_config(walldetector_sensor, ULTRASONIC_SENSOR); // <1>
    ev3_sensor_config(linemon_sensor, COLOR_SENSOR);
    ev3_sensor_config(bumper_sensor, TOUCH_SENSOR);
    ev3_sensor_config(seat_sensor, TOUCH_SENSOR); // <2>
  }

  porter_transport();
  ext_tsk();
}