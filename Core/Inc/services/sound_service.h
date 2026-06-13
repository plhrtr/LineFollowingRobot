#ifndef SOUND_SERVICE_H
#define SOUND_SERVICE_H

/**
 * Play the robots startup sound
 * @warning this method is blocking till the sound is finished
 */
void sound_service_start_up_sound();

/**
 * Play a sound that indicates that the battery is low
 * @warning this method is blocking till the sound is finished
 */
void sound_service_low_battery_sound();

#endif // !SOUND_SERVICE_H
