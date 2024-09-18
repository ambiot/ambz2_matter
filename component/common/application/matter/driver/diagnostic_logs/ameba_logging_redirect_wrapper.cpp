#include <platform_opts.h>

#if defined(CONFIG_ENABLE_AMEBA_DLOG) && (CONFIG_ENABLE_AMEBA_DLOG == 1)
#include <diagnostic_logs/ameba_logging_redirect_wrapper.h>
#include <diagnostic_logs/ameba_logging_redirect_handler.h>

#ifdef __cplusplus
extern "C" {
#endif

void ameba_logging_redirect_wrapper_init(void)
{
    auto & instance = AmebaLogRedirectHandler::GetInstance();
    instance.InitAmebaLogSubsystem();
    printf("Inited Ameba Log subsystem!\n");
}

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ENABLE_AMEBA_DLOG */
