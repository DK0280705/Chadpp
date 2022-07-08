#pragma once
#include "../application.h"
#include "../bot.h"
#include "../input.h"
#include "../language.h"

extern Bot* bot;

#define DECLARE_APPLICATION_CLASS(__app_name__)                                                    \
    class Application_##__app_name__ : public Application                                          \
    {                                                                                              \
    public:                                                                                        \
        Application_##__app_name__();                                                              \
        void call(const class Input& input) const override;                                        \
    };

DECLARE_APPLICATION_CLASS(help)