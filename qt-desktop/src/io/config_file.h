#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "model/atsc_config.h"
#include <QString>

namespace ConfigFile {
    bool load(const QString& path, AtscConfig& config);
    bool save(const QString& path, const AtscConfig& config);
}

#endif
