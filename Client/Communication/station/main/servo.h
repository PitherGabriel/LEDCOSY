#ifndef SERVO_H
#define SERVO_H

// Please consult the datasheet of your servo before changing the following parameters
#define SERVO_MIN_PULSEWIDTH_US 200  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2450  // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE        0 // Minimum angle
#define SERVO_MAX_DEGREE         225// Maximum angle

#define SERVO_PULSE_GPIO             2        // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD        20000    // 20000 ticks, 30ms

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void move_servo(float *);

#endif //SERVO_H