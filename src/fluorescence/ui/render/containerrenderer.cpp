/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "containerrenderer.hpp"

#include <ClanLib/Display/Render/program_object.h>

#include <ui/manager.hpp>
#include <ui/render/renderqueue.hpp>
#include <ui/render/shadermanager.hpp>
#include <ui/texture.hpp>
#include <ui/components/containerview.hpp>

#include <misc/log.hpp>

#include <world/ingameobject.hpp>

#include <data/manager.hpp>
#include <data/huesloader.hpp>

namespace fluo {
namespace ui {

ContainerRenderer::ContainerRenderer(boost::shared_ptr<RenderQueue> renderQueue, components::ContainerView* containerView) : IngameObjectRenderer(IngameObjectRenderer::TYPE_WORLD),
            containerView_(containerView), renderQueue_(renderQueue) {
}

ContainerRenderer::~ContainerRenderer() {
    renderQueue_->clear();
}


void ContainerRenderer::checkTextureSize() {
    if (!texture_ || texture_->getWidth() != containerView_->getWidth() || texture_->getHeight() != containerView_->getHeight()) {
        texture_.reset(new ui::Texture(false));
        texture_->initPixelBuffer(containerView_->getWidth(), containerView_->getHeight());
        texture_->setReadComplete();
    }
}

boost::shared_ptr<Texture> ContainerRenderer::getTexture(CL_GraphicContext& gc) {
    if (renderQueue_->requireWorldRepaint() || !texture_ || requireInitialRepaint()) {
        checkTextureSize();
        CL_FrameBuffer origBuffer = gc.get_write_frame_buffer();

        CL_FrameBuffer fb(gc);
        fb.attach_color_buffer(0, *texture_->getTexture());

        gc.set_frame_buffer(fb);

        render(gc);

        gc.set_frame_buffer(origBuffer);
    }

    return texture_;
}


void ContainerRenderer::render(CL_GraphicContext& gc) {
    renderQueue_->preRender();

    gc.clear(CL_Colorf(0.f, 0.f, 0.f, 0.f));

    boost::shared_ptr<CL_ProgramObject> shader = ui::Manager::getShaderManager()->getGumpShader();
    gc.set_program_object(*shader, cl_program_matrix_modelview_projection);

    CL_Rectf texture_unit1_coords(0.0f, 0.0f, 1.0f, 1.0f);

    CL_Vec2f tex1_coords[6] = {
        CL_Vec2f(texture_unit1_coords.left, texture_unit1_coords.top),
        CL_Vec2f(texture_unit1_coords.right, texture_unit1_coords.top),
        CL_Vec2f(texture_unit1_coords.left, texture_unit1_coords.bottom),
        CL_Vec2f(texture_unit1_coords.right, texture_unit1_coords.top),
        CL_Vec2f(texture_unit1_coords.left, texture_unit1_coords.bottom),
        CL_Vec2f(texture_unit1_coords.right, texture_unit1_coords.bottom)
    };

    gc.set_texture(0, *(data::Manager::getSingleton()->getHuesLoader()->getHuesTexture()->getTexture()));
    shader->set_uniform1i("HueTexture", 0);
    shader->set_uniform1i("ObjectTexture", 1);

    RenderQueue::const_iterator igIter = renderQueue_->begin();
    RenderQueue::const_iterator igEnd = renderQueue_->end();

    CL_Vec2f vertexCoords[6];

    bool renderingComplete = true;

    // draw background container texture
    boost::shared_ptr<ui::Texture> bgTex = containerView_->getBackgroundTexture();
    if (bgTex && bgTex->isReadComplete()) {
        CL_Vec3f hueInfo(0, 0, 1);

        CL_Rectf rect(0, 0, CL_Sizef(bgTex->getWidth(), bgTex->getHeight()));

        vertexCoords[0] = CL_Vec2f(rect.left, rect.top);
        vertexCoords[1] = CL_Vec2f(rect.right, rect.top);
        vertexCoords[2] = CL_Vec2f(rect.left, rect.bottom);
        vertexCoords[3] = CL_Vec2f(rect.right, rect.top);
        vertexCoords[4] = CL_Vec2f(rect.left, rect.bottom);
        vertexCoords[5] = CL_Vec2f(rect.right, rect.bottom);

        CL_PrimitivesArray primarray(gc);
        primarray.set_attributes(0, vertexCoords);
        primarray.set_attributes(1, tex1_coords);

        primarray.set_attribute(2, hueInfo);

        gc.set_texture(1, *bgTex->getTexture());
        gc.draw_primitives(cl_triangles, 6, primarray);
    } else {
        renderingComplete = false;
    }

    for (; igIter != igEnd; ++igIter) {
        boost::shared_ptr<world::IngameObject> curObj = *igIter;

        // just items in a container
        if (!curObj->isDynamicItem()) {
            continue;
        }

        // check if texture is ready to be drawn
        boost::shared_ptr<ui::Texture> tex = curObj->getIngameTexture();

        if (!tex) {
            continue;
        }

        if (!tex->isReadComplete()) {
            renderingComplete = false;
            continue;
        }

        CL_Rectf rect(curObj->getLocX(), curObj->getLocY(), CL_Sizef(tex->getWidth(), tex->getHeight()));

        vertexCoords[0] = CL_Vec2f(rect.left, rect.top);
        vertexCoords[1] = CL_Vec2f(rect.right, rect.top);
        vertexCoords[2] = CL_Vec2f(rect.left, rect.bottom);
        vertexCoords[3] = CL_Vec2f(rect.right, rect.top);
        vertexCoords[4] = CL_Vec2f(rect.left, rect.bottom);
        vertexCoords[5] = CL_Vec2f(rect.right, rect.bottom);

        CL_PrimitivesArray primarray(gc);
        primarray.set_attributes(0, vertexCoords);
        primarray.set_attributes(1, tex1_coords);

        primarray.set_attribute(2, curObj->getHueInfo());

        gc.set_texture(1, *tex->getTexture());
        gc.draw_primitives(cl_triangles, 6, primarray);
    }

    gc.reset_textures();
    gc.reset_program_object();

    renderQueue_->postRender(renderingComplete);
}

boost::shared_ptr<RenderQueue> ContainerRenderer::getRenderQueue() const {
    return renderQueue_;
}

}
}