#ifndef TINYNETFLOW_APPLICATION_H
#define TINYNETFLOW_APPLICATION_H

#include <chrono>
#include <memory>
#include <set>
#include <map>
#include <string>
#include <unistd.h>

namespace netflow::net::someip
{
class SomeIpPayload;
class SomeIpEvent;
class SomeIpPolicy;
    
class Application {
public:
    Application();
    ~Application();

    const std::string& getName() const;
    uint16_t getClientId() const;
    bool init();
    void start();
    void stop();

    void offerService(uint16_t serviceId, uint16_t instanceId, 
                        int majorVersion, int minorVersion);
    void stopOfferService(uint16_t serviceId, uint16_t instanceId, 
                        int majorVersion, int minorVersion);

    void offerEvent();  // xxxx
    void stopOfferEvent();

    void requestService(uint16_t serviceId, uint16_t instanceId, 
                        int majorVersion, int minorVersion);
    void releaseService(uint16_t serviceId, uint16_t instanceId);

    void subscribe(uint16_t serviceId, uint16_t instanceId, int eventGroupId,
                    int majorVersion, int eventId);

    void unsubscribe(uint16_t serviceId, uint16_t instanceId, int eventGroupId);

    void notify();
    void notifyOne();

    bool isAvailable();

    void registerStateHandle();
    void unregisterStateHandle();
    void setStateCallback();

    void setMessageCallback();
    void setSomeIpConnectCallback();
    void setSubscribeCallback();


    bool isRouting() const;
    void setRoutingState();
    void send();
private:


    
};

} // namespace netflow::net::someip


#endif // TINYNETFLOW_APPLICATION_H