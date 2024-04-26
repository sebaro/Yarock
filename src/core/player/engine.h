
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "engine_base.h"

#include <QtCore/QObject>
#include <QtCore/QString>


class Engine {
    static EngineBase         *CORE_INSTANCE;

    public:
        Engine();
        static EngineBase* instance() { return CORE_INSTANCE; }

        QString error() { return m_error;}

        static ENGINE::E_ENGINE_TYPE activeEngine();

    private:
        QString m_error;
};

#endif // _ENGINE_H_
