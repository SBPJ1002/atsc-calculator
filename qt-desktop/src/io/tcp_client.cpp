#include "io/tcp_client.h"

TcpClient::TcpClient(QObject* parent) : QObject(parent) {
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
}

bool TcpClient::connectToServer(const QString& host, int port) {
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(3000);
}

void TcpClient::disconnectFromServer() {
    m_socket->disconnectFromHost();
}

bool TcpClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

bool TcpClient::sendCommand(const QString& cmd) {
    if (!isConnected()) return false;
    m_socket->write((cmd + "\n").toUtf8());
    return m_socket->waitForBytesWritten(2000);
}

bool TcpClient::sendConfig(const AtscConfig& config) {
    if (!isConnected()) return false;

    const auto& b = config.bootstrap;
    const auto& p = config.preamble;

    sendCommand(QString("SET_MAJOR_VERSION=%1").arg(b.major_version));
    sendCommand(QString("SET_MINOR_VERSION=%1").arg(b.minor_version));
    sendCommand(QString("SET_BOOTSTRAP_SYMBOL=%1").arg(b.bootstrap_symbol));
    sendCommand(QString("SET_EA_WAKEUP=%1").arg(b.ea_wakeup));
    sendCommand(QString("SET_SYSTEM_BANDWIDTH=%1").arg(b.system_bandwidth));
    sendCommand(QString("SET_BSR_COEFFICIENT=%1").arg(b.bsr_coefficient));
    sendCommand(QString("SET_MIN_TIME_TO_NEXT=%1").arg(b.min_time_to_next));
    sendCommand(QString("SET_PREAMBLE_STRUCTURE=%1").arg(b.preamble_structure));
    sendCommand(QString("SET_FRAME_COUNT=%1").arg(b.number_of_frames));

    sendCommand(QString("SET_L1B_VERSION=%1").arg(p.l1b_version));
    sendCommand(QString("SET_L1B_DETAIL_FEC_TYPE=%1").arg(p.l1b_detail_fec_type));
    sendCommand(QString("SET_L1B_TIME_INFO_FLAG=%1").arg(p.l1b_time_info_flag));
    sendCommand(QString("SET_L1B_FRAME_LENGTH_MODE=%1").arg(p.l1b_frame_length_mode));
    sendCommand(QString("SET_L1B_FRAME_LENGTH=%1").arg(p.l1b_frame_length));
    sendCommand(QString("SET_NUM_SUBFRAMES=%1").arg(p.l1b_num_subframes));
    sendCommand(QString("SET_L1D_VERSION=%1").arg(p.l1d_version));
    sendCommand(QString("SET_L1D_BSID=%1").arg(p.l1d_bsid));

    for (int i = 0; i < (int)config.subframes.size(); i++) {
        const auto& sf = config.subframes[i];
        QString pfx = QString("SET_SUBFRAME_%1_").arg(i);
        sendCommand(pfx + QString("FFT_SIZE=%1").arg(sf.fft_size));
        sendCommand(pfx + QString("GUARD_INTERVAL=%1").arg(sf.guard_interval));
        sendCommand(pfx + QString("SCATTERED_PILOT_PATTERN=%1").arg(sf.scattered_pilot_pattern));
        sendCommand(pfx + QString("SCATTERED_PILOT_BOOST=%1").arg(sf.scattered_pilot_boost));
        sendCommand(pfx + QString("NUM_OFDM_SYMBOLS=%1").arg(sf.num_ofdm_symbols));
        sendCommand(pfx + QString("MIMO=%1").arg(sf.mimo));
        sendCommand(pfx + QString("MISO=%1").arg(sf.miso));
        sendCommand(pfx + QString("REDUCED_CARRIERS=%1").arg(sf.reduced_carriers));
        sendCommand(pfx + QString("SBS_FIRST=%1").arg(sf.sbs_first));
        sendCommand(pfx + QString("SBS_LAST=%1").arg(sf.sbs_last));
        sendCommand(pfx + QString("FREQUENCY_INTERLEAVER=%1").arg(sf.frequency_interleaver));

        for (int j = 0; j < (int)sf.plps.size(); j++) {
            const auto& plp = sf.plps[j];
            QString pp = pfx + QString("PLP_%1_").arg(j);
            sendCommand(pp + QString("ID=%1").arg(plp.id));
            sendCommand(pp + QString("FEC_TYPE=%1").arg(plp.fec_type));
            sendCommand(pp + QString("MOD=%1").arg(plp.mod));
            sendCommand(pp + QString("COD=%1").arg(plp.cod));
            sendCommand(pp + QString("LAYER=%1").arg(plp.layer));
            sendCommand(pp + QString("TI_MODE=%1").arg(plp.ti_mode));
            sendCommand(pp + QString("START=%1").arg(plp.start));
            sendCommand(pp + QString("SIZE=%1").arg(plp.size));
        }
    }

    return true;
}
