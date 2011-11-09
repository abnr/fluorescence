
#include "ingameobject.hpp"

#include <stdio.h>

#include <misc/log.hpp>

#include <ui/texture.hpp>
#include <ui/manager.hpp>
#include <ui/render/renderqueue.hpp>

#include "overheadmessage.hpp"

namespace fluo {
namespace world {

IngameObject::IngameObject(unsigned int objectType) : draggable_(false), objectType_(objectType), visible_(true) {

}

IngameObject::~IngameObject() {
}

bool IngameObject::isVisible() const {
    return visible_;
}

void IngameObject::setVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
        forceRepaint();
    }
}

void IngameObject::setLocation(float locX, float locY, float locZ) {
    setLocation(CL_Vec3f(locX, locY, locZ));
}

void IngameObject::setLocation(CL_Vec3f loc) {
    CL_Vec3f oldLocation = location_;

    location_ = loc;

    //LOGARG_DEBUG(LOGTYPE_WORLD, "Object location: %f %f %f", locX, locY, locZ);

    invalidateRenderPriority();
    invalidateVertexCoordinates();
}

float IngameObject::getLocX() const {
    return location_[0u];
}

float IngameObject::getLocY() const {
    return location_[1u];
}

float IngameObject::getLocZ() const {
    return location_[2u];
}

CL_Vec3f IngameObject::getLocation() const {
    return location_;
}

void IngameObject::invalidateTextureProvider() {
    worldRenderData_.invalidateTextureProvider();
    worldRenderData_.invalidateVertexCoordinates();
}

void IngameObject::invalidateVertexCoordinates() {
    worldRenderData_.invalidateVertexCoordinates();

    if (!childObjects_.empty()) {
        std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
        std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

        for (; iter != end; ++iter) {
            (*iter)->invalidateVertexCoordinates();
        }
    }
}

void IngameObject::invalidateRenderPriority() {
    worldRenderData_.invalidateRenderPriority();

    if (!childObjects_.empty()) {
        std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
        std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

        for (; iter != end; ++iter) {
            (*iter)->invalidateRenderPriority();
        }
    }
}

const CL_Vec2f* IngameObject::getVertexCoordinates() const {
    return worldRenderData_.vertexCoordinates_;
}

const CL_Vec3f& IngameObject::getHueInfo() const {
    return worldRenderData_.hueInfo_;
}

int IngameObject::getRenderPriority(unsigned int lvl) const {
    if (lvl > 5) {
        lvl = 0;
    }

    return worldRenderData_.renderPriority_[lvl];
}

const int* IngameObject::getRenderPriorities() const {
    return worldRenderData_.renderPriority_;
}

void IngameObject::updateRenderData(unsigned int elapsedMillis) {
    if (worldRenderData_.renderDataValid()) {
        bool frameChanged = updateAnimation(elapsedMillis);

        if (frameChanged) {
            updateVertexCoordinates();
            worldRenderData_.onVertexCoordinatesUpdate();
            notifyRenderQueuesWorldTexture();
        }
    } else {
        if (worldRenderData_.textureProviderUpdateRequired()) {
            updateTextureProvider();
            worldRenderData_.onTextureProviderUpdate();
            notifyRenderQueuesWorldTexture();
        }

        boost::shared_ptr<ui::Texture> tex = getIngameTexture();
        if (!tex || !tex->isReadComplete()) {
            return;
        }

        bool frameChanged = updateAnimation(elapsedMillis);
        if (frameChanged || worldRenderData_.vertexCoordinatesUpdateRequired()) {
            updateVertexCoordinates();
            worldRenderData_.onVertexCoordinatesUpdate();

            if (frameChanged) {
                notifyRenderQueuesWorldTexture();
            } else {
                notifyRenderQueuesWorldCoordinates();
            }
        }

        if (worldRenderData_.renderPriorityUpdateRequired()) {
            updateRenderPriority();
            worldRenderData_.onRenderPriorityUpdate();
            notifyRenderQueuesWorldPriority();
        }
    }
}

bool IngameObject::isInDrawArea(int leftPixelCoord, int rightPixelCoord, int topPixelCoord, int bottomPixelCoord) const {
    //LOGARG_DEBUG(LOGTYPE_WORLD, "isInDrawArea (%u %u %u %u) => x=%u y=%u\n", leftPixelCoord, rightPixelCoord, topPixelCoord, bottomPixelCoord, vertexCoordinates_[0u].x, vertexCoordinates_[0u].y);

    // almost every texture is a rectangle, with vertexCoordinates_[0] being top left and vertexCoordinates_[5] bottom right
    // all non-rectangular cases are maptiles
    return worldRenderData_.vertexCoordinates_[0].x <= rightPixelCoord &&
            worldRenderData_.vertexCoordinates_[0].y <= bottomPixelCoord &&
            worldRenderData_.vertexCoordinates_[5].x >= leftPixelCoord &&
            worldRenderData_.vertexCoordinates_[5].y >= topPixelCoord;
}

bool IngameObject::hasPixel(int pixelX, int pixelY) const {
    // almost every texture is a rectangle, with vertexCoordinates_[0] being top left and vertexCoordinates_[5] bottom right
    // all non-rectangular cases are maptiles
    bool coordinateCheck = worldRenderData_.vertexCoordinates_[0].x <= pixelX &&
            worldRenderData_.vertexCoordinates_[0].y <= pixelY &&
            worldRenderData_.vertexCoordinates_[5].x >= pixelX &&
            worldRenderData_.vertexCoordinates_[5].y >= pixelY;

    boost::shared_ptr<ui::Texture> tex = getIngameTexture();
    if (coordinateCheck && tex && tex->isReadComplete()) {
        unsigned int texPixelX = pixelX - worldRenderData_.vertexCoordinates_[0].x;
        unsigned int texPixelY = pixelY - worldRenderData_.vertexCoordinates_[0].y;

        return tex->hasPixel(texPixelX, texPixelY);
    } else {
        return false;
    }
}

bool IngameObject::hasGumpPixel(int pixelX, int pixelY) const {
    boost::shared_ptr<ui::Texture> tex = getGumpTexture();
    if (tex && tex->isReadComplete()) {
        return tex->hasPixel(pixelX, pixelY);
    } else {
        return false;
    }
}

const CL_Vec3f* IngameObject::getVertexNormals() const {
    return worldRenderData_.vertexNormals_;
}

void IngameObject::onClick() {
}

void IngameObject::onDoubleClick() {
    setVisible(false);
}

bool IngameObject::isMirrored() const {
    return false;
}

bool IngameObject::isDraggable() const {
    return draggable_;
}

void IngameObject::onDraggedOnto(boost::shared_ptr<IngameObject> obj) {
    LOG_ERROR << "Undraggable object dragged on other object" << std::endl;
}

void IngameObject::onDraggedToVoid() {
    LOG_ERROR << "Undraggable object dragged on void" << std::endl;
}

void IngameObject::onStartDrag(const CL_Point& mousePos) {
    LOG_ERROR << "Starting to drag object other than DynamicItem or Mobile" << std::endl;
}

boost::shared_ptr<IngameObject> IngameObject::getTopParent() {
    boost::shared_ptr<IngameObject> parent = parentObject_.lock();
    if (parent) {
        return parent->getTopParent();
    } else {
        return shared_from_this();
    }
}

void IngameObject::printRenderPriority() const {
    LOG_DEBUG << "Render priority: " << worldRenderData_.renderPriority_[0] << " : "
            << worldRenderData_.renderPriority_[1] << " : "
            << worldRenderData_.renderPriority_[2] << " : "
            << worldRenderData_.renderPriority_[3] << " : "
            << worldRenderData_.renderPriority_[4] << " : "
            << worldRenderData_.renderPriority_[5] << std::endl;
}

void IngameObject::setOverheadMessageOffsets() {
    std::list<boost::shared_ptr<IngameObject> >::reverse_iterator iter = childObjects_.rbegin();
    std::list<boost::shared_ptr<IngameObject> >::reverse_iterator end = childObjects_.rend();

    int offset = -5;

    for (; iter != end; ++iter) {
        if ((*iter)->isSpeech()) {
            boost::shared_ptr<OverheadMessage> msgObj = boost::dynamic_pointer_cast<OverheadMessage>(*iter);
            int curHeight = (*iter)->getIngameTexture()->getHeight();
            int curOffset = offset - curHeight;
            msgObj->setParentPixelOffset(curOffset);
            offset = curOffset - 2;
        }
    }
}

bool IngameObject::isInRenderQueue(boost::shared_ptr<ui::RenderQueue> rq) {
    std::list<boost::shared_ptr<ui::RenderQueue> >::const_iterator iter = renderQueues_.begin();
    std::list<boost::shared_ptr<ui::RenderQueue> >::const_iterator end = renderQueues_.end();
    for (; iter != end; ++iter) {
        if (*iter == rq) {
            return true;
        }
    }

    return false;
}

void IngameObject::addToRenderQueue(boost::shared_ptr<ui::RenderQueue> rq) {
    boost::shared_ptr<ui::RenderQueue> weakRq(rq);

    if (!isInRenderQueue(rq)) {
        renderQueues_.push_back(weakRq);
        rq->add(shared_from_this());

        if (!childObjects_.empty()) {
            std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
            std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

            for (; iter != end; ++iter) {
                if ((*iter)->isSpeech() || isMobile()) {
                    (*iter)->addToRenderQueue(rq);
                }
            }
        }
    }
}

void IngameObject::removeFromRenderQueue(boost::shared_ptr<ui::RenderQueue> rq) {
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator iter = renderQueues_.begin();
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator end = renderQueues_.end();
    for (; iter != end; ++iter) {
        if (*iter == rq) {
            break;
        }
    }

    if (iter != end) {
        renderQueues_.erase(iter);
        rq->remove(shared_from_this());

        if (!childObjects_.empty()) {
            std::list<boost::shared_ptr<IngameObject> >::iterator iter = childObjects_.begin();
            std::list<boost::shared_ptr<IngameObject> >::iterator end = childObjects_.end();

            for (; iter != end; ++iter) {
                if ((*iter)->isSpeech() || isMobile()) {
                    (*iter)->addToRenderQueue(rq);
                }
            }
        }
    }
}

void IngameObject::addChildObject(boost::shared_ptr<IngameObject> obj) {
    std::list<boost::shared_ptr<IngameObject> >::iterator iter = std::find(childObjects_.begin(), childObjects_.end(), obj);

    if (iter == childObjects_.end()) {
        childObjects_.push_back(obj);
        obj->setParentObject(shared_from_this());
        onChildObjectAdded(obj);

        if (obj->isSpeech()) {
            setOverheadMessageOffsets();
        }
    }
}

void IngameObject::removeChildObject(boost::shared_ptr<IngameObject> obj) {
    std::list<boost::shared_ptr<IngameObject> >::iterator iter = std::find(childObjects_.begin(), childObjects_.end(), obj);

    if (iter != childObjects_.end()) {
        onChildObjectRemoved(*iter);
        obj->setParentObject();
        childObjects_.erase(iter);
    }
}

void IngameObject::onAddedToParent() {
}

void IngameObject::onRemovedFromParent() {
}

void IngameObject::onChildObjectAdded(boost::shared_ptr<IngameObject> obj) {
}

void IngameObject::onChildObjectRemoved(boost::shared_ptr<IngameObject> obj) {
}

void IngameObject::setParentObject() {
    boost::shared_ptr<IngameObject> parent = parentObject_.lock();

    if (parent) {
        onRemovedFromParent();
        parentObject_.reset();
    }
}

void IngameObject::setParentObject(boost::shared_ptr<IngameObject> parent) {
    boost::shared_ptr<IngameObject> curParent = parentObject_.lock();
    if (curParent) {
        setParentObject();
    }

    parentObject_ = parent;
    onAddedToParent();
}

bool IngameObject::isMap() const {
    return objectType_ == TYPE_MAP;
}

bool IngameObject::isStaticItem() const {
    return objectType_ == TYPE_STATIC_ITEM;
}

bool IngameObject::isDynamicItem() const {
    return objectType_ == TYPE_DYNAMIC_ITEM;
}

bool IngameObject::isMobile() const {
    return objectType_ == TYPE_MOBILE;
}

bool IngameObject::isSpeech() const {
    return objectType_ == TYPE_SPEECH;
}

void IngameObject::onDelete() {
    boost::shared_ptr<IngameObject> parent = parentObject_.lock();
    if (parent) {
        parent->removeChildObject(shared_from_this());
    }

    std::list<boost::shared_ptr<ui::RenderQueue> > rqsToRemove(renderQueues_.begin(), renderQueues_.end());
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = rqsToRemove.begin();
    std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = rqsToRemove.end();
    for (; rqIter != rqEnd; ++rqIter) {
        removeFromRenderQueue(*rqIter);
    }
}

boost::shared_ptr<ui::Texture> IngameObject::getGumpTexture() const {
    LOG_ERROR << "getGumpTexture called on IngameObject" << std::endl;
    return getIngameTexture();
}


void IngameObject::notifyRenderQueuesWorldTexture() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onObjectWorldTextureChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onObjectWorldTextureChanged();
        }
    }
}

void IngameObject::notifyRenderQueuesWorldCoordinates() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onObjectWorldCoordinatesChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onObjectWorldCoordinatesChanged();
        }
    }
}

void IngameObject::notifyRenderQueuesWorldPriority() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onObjectWorldPriorityChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onObjectWorldPriorityChanged();
        }
    }
}

void IngameObject::notifyRenderQueuesGump() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->onGumpChanged();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->onGumpChanged();
        }
    }
}

void IngameObject::forceRepaint() {
    switch (renderQueues_.size()) {
    case 0:
        // do nothing
        break;
    case 1:
        renderQueues_.front()->forceRepaint();
        break;
    default:
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqIter = renderQueues_.begin();
        std::list<boost::shared_ptr<ui::RenderQueue> >::iterator rqEnd = renderQueues_.end();
        for (; rqIter != rqEnd; ++rqIter) {
            (*rqIter)->forceRepaint();
        }
    }
}

std::list<boost::shared_ptr<ui::RenderQueue> >::iterator IngameObject::rqBegin() {
    return renderQueues_.begin();
}

std::list<boost::shared_ptr<ui::RenderQueue> >::iterator IngameObject::rqEnd() {
    return renderQueues_.end();
}

const ui::WorldRenderData& IngameObject::getWorldRenderData() const {
    return worldRenderData_;
}

void IngameObject::updateGumpTextureProvider() {
}

}
}
