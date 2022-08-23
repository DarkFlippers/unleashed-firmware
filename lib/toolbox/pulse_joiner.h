#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PulseJoiner PulseJoiner;

/**
 * @brief Alloc PulseJoiner
 * 
 * @return PulseJoiner* 
 */
PulseJoiner* pulse_joiner_alloc();

/**
 * @brief Free PulseJoiner
 * 
 * @param pulse_joiner 
 */
void pulse_joiner_free(PulseJoiner* pulse_joiner);

/**
 * @brief Push timer pulse. First negative pulse is ommited.
 * 
 * @param polarity pulse polarity: true = high2low, false = low2high
 * @param period overall period time in timer clicks
 * @param pulse pulse time in timer clicks
 * 
 * @return true - next pulse can and must be popped immediatly
 */
bool pulse_joiner_push_pulse(PulseJoiner* pulse_joiner, bool polarity, size_t period, size_t pulse);

/**
 * @brief Get the next timer pulse. Call only if push_pulse returns true.
 * 
 * @param period overall period time in timer clicks
 * @param pulse pulse time in timer clicks
 */
void pulse_joiner_pop_pulse(PulseJoiner* pulse_joiner, size_t* period, size_t* pulse);

#ifdef __cplusplus
}
#endif