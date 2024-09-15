#include "ameba_logging_redirect_wrapper.h"
#include "ameba_logging_redirect_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    wrapper function for chiptest example to init C++ class from C code
*/
void ameba_logging_redirect_wrapper_init()
{
    auto & instance = AmebaLogRedirectHandler::GetInstance();
    instance.InitAmebaLogSubsystem();
    printf("Inited Ameba Log subsystem!\n");
}

#ifdef __cplusplus
}
#endif