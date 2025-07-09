#ifndef HEADUNIT_CONFIGS_H
#define HEADUNIT_CONFIGS_H

/** Set the configs for individual headunits here
 * 
 * @param SWC_OUTPUT_ENABLE_SHORT_HOLD_MS   Time (ms) required for radio to read short input signal 
 * @param SWC_OUTPUT_ENABLE_LONG_HOLD_MS    Time (ms) required for headunit to read long input signal e.g. calibration 
 * @param SWC_OUTPUT_DISABLE_MS             Time (ms) required for controller to disable SWC until we can send another signal
 * 
 */

#if defined(ANDROID_HEADUNIT_GENERIC)
#define CONFIG_NAME "ANDROID_HEADUNIT_GENERIC"
#define SWC_OUTPUT_ENABLE_SHORT_HOLD_MS 800     
#define SWC_OUTPUT_ENABLE_LONG_HOLD_MS  4000    
#define SWC_OUTPUT_DISABLE_MS   20              

#elif defined(SONY_HEADUNIT_GENERIC)
#define CONFIG_NAME "SONY_HEADUNIT_GENERIC"
#define SWC_OUTPUT_ENABLE_SHORT_HOLD_MS 800     
#define SWC_OUTPUT_ENABLE_LONG_HOLD_MS  4000    
#define SWC_OUTPUT_DISABLE_MS   20              

#else 
    #error "Define a Stereo in the platformio.ini file"
#endif

/* Print out which config we are using */
#pragma message("Headunit defined as " CONFIG_NAME)

#endif
