/**
 * Configuration header for standalone behavior.
 *
 * This header must be the FIRST include of every compilation unit (.cpp file)
 * thats part of the standalone behavior. It should not be included in any
 * header files.
 */
#pragma once

// forbid access to global blackboards, as they are not available
//#pragma GCC poison tafel settings
