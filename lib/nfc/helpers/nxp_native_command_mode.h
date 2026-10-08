#pragma once

/**
 * @brief Enumeration of possible command modes.
 */
typedef enum {
    NxpNativeCommandModePlain, /**< Plain native commands. */
    NxpNativeCommandModeIsoWrapped, /**< ISO 7816-wrapped commands. */

    NxpNativeCommandModeMAX,
} NxpNativeCommandMode;
