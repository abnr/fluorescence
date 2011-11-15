#ifndef FLUO_WORLD_MANAGER_HPP
#define FLUO_WORLD_MANAGER_HPP

#include <boost/shared_ptr.hpp>

#include <list>
#include <map>

#include <typedefs.hpp>
#include <misc/config.hpp>

namespace fluo {

namespace world {

class SectorManager;
class LightManager;
class SmoothMovementManager;
class Mobile;
class DynamicItem;
class OverheadMessage;

class Manager {
public:
    ~Manager();
    static bool create(const Config& config);
    static void destroy();
    static Manager* getSingleton();

    static boost::shared_ptr<SectorManager> getSectorManager();
    static boost::shared_ptr<LightManager> getLightManager();
    static boost::shared_ptr<SmoothMovementManager> getSmoothMovementManager();

    unsigned int getCurrentMapId();
    void setCurrentMapId(unsigned int id);

    boost::shared_ptr<Mobile> initPlayer(Serial serial);
    boost::shared_ptr<Mobile> getPlayer();

    boost::shared_ptr<Mobile> getMobile(Serial serial, bool createIfNotExists = true);
    boost::shared_ptr<DynamicItem> getDynamicItem(Serial serial, bool createIfNotExists = true);

    void deleteObject(Serial serial);

    void step(unsigned int elapsedMillis);

    void registerOverheadMessage(boost::shared_ptr<OverheadMessage> msg);
    void unregisterOverheadMessage(boost::shared_ptr<OverheadMessage> msg);

private:
    static Manager* singleton_;
    Manager(const Config& config);
    Manager(const Manager& copy) {}
    void operator=(const Manager& copy) {}

    boost::shared_ptr<SectorManager> sectorManager_;
    unsigned int currentMapId_;

    boost::shared_ptr<LightManager> lightManager_;

    boost::shared_ptr<Mobile> player_;
    std::map<Serial, boost::shared_ptr<Mobile> > mobiles_;

    std::map<Serial, boost::shared_ptr<DynamicItem> > dynamicItems_;

    void update(unsigned int millis);

    boost::shared_ptr<SmoothMovementManager> smoothMovementManager_;

    std::list<boost::shared_ptr<OverheadMessage> > overheadMessages_;
};

}
}

#endif
