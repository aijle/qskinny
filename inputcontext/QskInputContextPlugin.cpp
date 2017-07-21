#include <qpa/qplatforminputcontextplugin_p.h>

#include "QskInputContext.h"

class QskInputContextPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "metadata.json")
public:
    QPlatformInputContext* create( const QString& system, const QStringList& params ) override
    {
        Q_UNUSED( params );
        if ( system.compare( system, QLatin1String( "skinny" ), Qt::CaseInsensitive ) == 0 )
            return new QskInputContext;
        return nullptr;
    }
};

#include "QskInputContextPlugin.moc"
