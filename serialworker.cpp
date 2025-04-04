#include "serialworker.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QThread>

SerialWorker::SerialWorker(QObject *parent)
    : QObject{parent}
    , m_port(this)
{}

template <size_t N>
bool SerialWorker::waitForLine(char (&buf)[N])
{
    static_assert(N > 1, "readLine() requires the array size to be greater than 1.");
    // If there is at least a line already for us to read, read it.
    while (!m_port.canReadLine())
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            m_port.write("M410\n"); // stop all movement
            m_port.close();
            emit stopped();
            return false;
        }
        if (!m_port.waitForReadyRead(m_timeout_ms))
        {
            m_port.close();
            emit error(QStringLiteral("Read timing out after %1 seconds.").arg(m_timeout_ms/1000));
            return false;
        }
    }
    m_port.readLine(buf, N);
    return true;
}

template <size_t N>
bool SerialWorker::waitForOK(char (&buf)[N])
{
    do
    {
        if (!waitForLine(buf)) return false;
    } while (std::strcmp(buf, "ok\n") != 0);
    return true;
}

void SerialWorker::start(const QString& filename, const SerialConfig& config)
{
    m_port.setPortName(config.name);
    if (!m_port.setBaudRate(config.baudrate))
    {
        emit error(m_port.errorString());
        return;
    }
    if (!m_port.setStopBits(config.stopbits))
    {
        emit error(m_port.errorString());
        return;
    }
    if (!m_port.setDataBits(config.databits))
    {
        emit error(m_port.errorString());
        return;
    }
    if (!m_port.setParity(config.parity))
    {
        emit error(m_port.errorString());
        return;
    }
    if (!m_port.setFlowControl(config.flowControl))
    {
        emit error(m_port.errorString());
        return;
    }

    if (!parseFile(filename)) return;
    char buffer[256];
    if (!m_port.open(QIODeviceBase::ReadWrite | QIODeviceBase::ExistingOnly))
    {
        emit error(m_port.errorString());
        return;
    }

    // confirm we are actually talking to a Marlin 3D printer
    m_port.write("M115\n"); // obtaining firmware info
    if (!waitForLine(buffer)) return;
    if (!std::strstr(buffer, "Marlin"))
    {
        m_port.close();
        emit error(QStringLiteral("Cannot detect a Marlin 3D printer - make sure your serial settings are correct!"));
        return;
    }
    if (!waitForOK(buffer)) return;

    if (QThread::currentThread()->isInterruptionRequested())
    {
        m_port.close();
        emit stopped();
        return;
    }

    // home tool head
    m_port.write("G28\n");
    if (!waitForOK(buffer)) return;

    // get tool head current position, which is the center of the printer bed
    static const QRegularExpression re("X:([0-9]+\\.[0-9]+) Y:([0-9]+\\.[0-9]+) Z");
    m_port.write("M114\n");
    if (!waitForLine(buffer)) return;
    auto match = re.match(buffer);
    if (!match.hasMatch())
    {
        m_port.close();
        emit error(QStringLiteral("Unable to parse current tool head position!"));
        return;
    }
    if (!waitForOK(buffer)) return;

    // now we parse x and y
    bool ok1, ok2;
    double centerx = match.captured(1).toDouble(&ok1);
    double centery = match.captured(2).toDouble(&ok2);
    if (!ok1 || !ok2)
    {
        m_port.close();
        emit error(QStringLiteral("Unable to parse current tool head position!"));
        return;
    }
    qDebug("Tool center position x=%f, y=%f", centerx, centery);

    // now we punch holes
    if (!punchHoles(centerx, centery, buffer)) return;

    // we tell the main thread that we have finished
    m_port.close();
    emit finished();
}

template <size_t N>
bool SerialWorker::punchHoles(double cx, double cy, char (&buf)[N])
{
    double svgWidth = m_svgCanvas.size.width, svgHeight = m_svgCanvas.size.height;
    double printerWidth = cx * 2, printerHeight = cy * 2;
    double scale = std::min(printerWidth / svgWidth, printerHeight / svgHeight);
    double svgcx = svgWidth / 2, svgcy = svgHeight / 2;

    if (QThread::currentThread()->isInterruptionRequested())
    {
        m_port.close();
        emit stopped();
        return false;
    }

    m_port.write("G90\n"); // use absolute positioning
    if (!waitForOK(buf)) return false;
    m_port.write("G0 Z50\n"); // set z=50
    if (!waitForOK(buf)) return false;
    if (QThread::currentThread()->isInterruptionRequested())
    {
        m_port.write("M410\n"); // stop all movement
        m_port.close();
        emit stopped();
        return false;
    }
    m_port.write("M400\n");
    if (!waitForOK(buf)) return false;
    for (const auto& h : std::as_const(m_svgCanvas.holes))
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            m_port.close();
            emit stopped();
            return false;
        }
        // We try to fit the SVG canvas on top of the printer bed. Instead of their centers being aligned, now their bottom left corners are aligned.
        double printerx = h.x * scale, printery = (svgHeight - h.y) * scale; // flip y-axis
        qDebug("Punching hole at X=%.2f, Y=%.2f", printerx, printery);
        m_port.write(QStringLiteral("G0 X%1 Y%2\n").arg(printerx, 0, 'f', 2).arg(printery, 0, 'f', 2).toUtf8());
        if (!waitForOK(buf)) return false;
        if (QThread::currentThread()->isInterruptionRequested())
        {
            m_port.write("M410\n"); // stop all movement
            m_port.close();
            emit stopped();
            return false;
        }
        m_port.write("M400\n"); // wait for printer head to move to the correct position
        if (!waitForOK(buf)) return false;
        // TODO: lowerHolePunch();
        QThread::sleep(2); // wait for 2 seconds
        // TODO: raiseHolePunch();
    }
    return true;
}

bool SerialWorker::parseFile(const QString& filename)
{
    if (filename.isEmpty())
    {
        emit error("SVG file name is not set.");
        return false;
    }
    qDebug() << "Parsing file " << filename;
    m_svgCanvas.holes.clear();
    QFile svg(filename);
    if (!svg.open(QIODeviceBase::ReadOnly))
    {
        emit error(svg.errorString());
        return false;
    }
    QXmlStreamReader svgReader(&svg);
    QXmlStreamReader::TokenType tokentype;
    bool hassvg = false;
    while ((tokentype = svgReader.readNext()) != QXmlStreamReader::EndDocument)
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            emit stopped();
            return false;
        }
        if (tokentype == QXmlStreamReader::Invalid)
        {
            static constexpr auto getXmlErrorString = [](QXmlStreamReader::Error err)
            {
                static const QString errString[] {
                    "No error has occurred.",
                    "Parser encountered unexpected element.",
                    "Custom error has occurred: ",
                    "SVG is not well-formed.",
                    "The SVG document has ended prematurely.",
                };
                return errString[err];
            };
            emit error(getXmlErrorString(svgReader.error()));
            return false;
        }
        if (tokentype == QXmlStreamReader::StartElement)
        {
            if (svgReader.name() == "svg")
            {
                auto attr = svgReader.attributes();
                bool ok1, ok2;
                m_svgCanvas.size.width = attr.value("width").toDouble(&ok1);
                m_svgCanvas.size.height = attr.value("height").toDouble(&ok2);
                hassvg = ok1 && ok2;
            }
            else if (svgReader.name() == "ellipse" || svgReader.name() == "circle")
            {
                auto attr = svgReader.attributes();
                bool ok1, ok2;
                double x = attr.value("cx").toDouble(&ok1);
                double y = attr.value("cy").toDouble(&ok2);
                if (ok1 && ok2) m_svgCanvas.holes.emplace_back(x, y);
                qDebug("Circle at x=%f, y=%f", x, y);
            }
        }
    }
    if (!hassvg)
    {
        emit error(QStringLiteral("Cannot detect SVG elements."));
        return false;
    }
    if (m_svgCanvas.size.width < 0 || m_svgCanvas.size.height < 0)
    {
        emit error(QStringLiteral("SVG width or height is negative."));
        return false;
    }
    qDebug("SVG width=%f, height=%f", m_svgCanvas.size.width, m_svgCanvas.size.height);
    return true;
}
