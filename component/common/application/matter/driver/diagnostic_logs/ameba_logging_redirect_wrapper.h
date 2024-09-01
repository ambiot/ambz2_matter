#ifndef AMEBA_LOGGING_REDIRECT_WRAPPER_H
#define AMEBA_LOGGING_REDIRECT_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Logging Redirect wrapper function to access C++ class from C code
 */
void ameba_logging_redirect_wrapper_init(void);

#ifdef __cplusplus
}
#endif

#endif
