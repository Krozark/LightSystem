/*
Copyright DaiMysha (c) 2015, All rights reserved.
DaiMysha@gmail.com
https://github.com/DaiMysha

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <DMUtils/maths.hpp>
#include <DMUtils/sfml.hpp>

#include <Lightsystem/SpotLight.hpp>


namespace DMGDVT {
namespace LS {

    SpotLight::~SpotLight() {
	}

    SpotLight::SpotLight(const sf::Vector2f& p, float r, const sf::Color& c) : SpotLight(p,r,c,0.0f,2.0f*M_PIf,1.0f,0.0f,1.0f) {
    }

    SpotLight::SpotLight(const sf::Vector2f& p, float r, const sf::Color& c, float da, float sa, float i, float b, float lf) : Light(p,c),
     _radius(r), _directionAngle(DMUtils::maths::degToRad(da)), _spreadAngle(sa), _bleed(b), _linearity(lf), _resizeWhenIncrease(false) {
        setSpreadAngle(sa);
        setIntensity(i);
        computeAABB();

        //32 points for a full circle, so we keep same spacing
        if(_spreadAngle > M_PIf * 3.0f/4.0f) _precision = 32;
        else if(_spreadAngle > M_PIf) _precision = 16;
        else if (_spreadAngle < M_PIf/2.0f) _precision = 4;
        else _precision = 8;
    }

    void SpotLight::preRender(sf::Shader* shader) {

        setNegative(_intensity<0.0f);

        if(shader==nullptr) return; //oopsie, can't work without the shader

        const float diam = _radius*2.0f;

        if(_renderTexture==nullptr) _renderTexture = new sf::RenderTexture();

        bool resizeTexture = false;

        if(_resizeWhenIncrease && _renderTexture->getSize().x < diam) resizeTexture = true;
        else if(_renderTexture->getSize().x != diam) resizeTexture = true;

        if(resizeTexture && !_renderTexture->create(diam,diam)) {
            delete _renderTexture;
            _renderTexture=nullptr;
            return; //somehow texture failed, maybe too big, abort
        }

        sf::Vector2f center(_renderTexture->getSize().x/2.0f,_renderTexture->getSize().y/2.0f);

        _renderTexture->clear();
        _render(*_renderTexture,sf::RenderStates::Default,shader,center);
        _renderTexture->display();

        _sprite.setOrigin(center);
        _sprite.setPosition(_position);
        _sprite.setTexture(_renderTexture->getTexture());
        _sprite.setRotation(DMUtils::maths::radToDeg(_directionAngle));

        computeAABB();
    }

    void SpotLight::render(const sf::IntRect& screen, sf::RenderTarget& target, sf::Shader* shader, const sf::RenderStates &states) {
        if(_intensity == 0.0f) return;
        if(!isActive()) return;

        if(_renderTexture!=nullptr) {
            target.draw(_sprite,states);
        } else {
            sf::Vector2f newCenter = _position - sf::Vector2f(screen.left,screen.top);
            _render(target,states,shader,sf::Vector2f(newCenter.x,screen.height - newCenter.y),_position,sf::Vector2f(_radius,_radius),DMUtils::maths::radToDeg(_directionAngle));
        }
	}

    void SpotLight::debugRender(sf::RenderTarget& target, const sf::RenderStates &states) {
        if(_intensity == 0.0f) return;
        if(!isActive()) return;

        if(_spreadAngle == M_PIf*2.0f) {
            sf::CircleShape shape(_radius);
            shape.setPosition(_position);
            shape.setOrigin(sf::Vector2f(1,1)*_radius);
            shape.setFillColor(_color);
            if(isIsometric()) shape.setScale(1.0f,0.5f);
            target.draw(shape,states);
        } else {
            sf::ConvexShape shape;
            shape.setPointCount(4);
            shape.setFillColor(sf::Color(_color.r,_color.g,_color.b,125));
            sf::Vector2f v(0,_radius);
            //*
            shape.setPosition(_position);shape.setPointCount(_precision+1);
            shape.setPoint(0,sf::Vector2f(0,0));

            float deltaAngle = _spreadAngle / (float)(_precision-1);
            for(int i=0;i<_precision;++i) {
                float angle = _directionAngle - _spreadAngle/2.0f + (float)i*deltaAngle;
                sf::Vector2f p(DMUtils::sfml::rotate(shape.getPoint(0)+sf::Vector2f(0.0f,_radius),angle,shape.getPoint(0)));
                if(isIsometric()) {
                    p.y /= 2.0f;
                }
                shape.setPoint(i+1,p);
            }

            target.draw(shape,states);

            /*//draws the target angle
            sf::Vertex line[2];
            line[0].position = _position + sf::Vector2f(0.0f,0.0f);
            line[0].color = _color;
            line[1].position = _position +  DMUtils::sfml::rotate(v,_directionAngle);
            line[1].color = _color;

            if(isIsometric()) {
                line[1].position.y -= (line[1].position.y-_position.y)/2.0f;
            }

            target.draw(line,2,sf::Lines, states);*/
        }
	}

    void SpotLight::computeAABB() {

        if(_spreadAngle == M_PIf*2.0f) {
            _aabb.left = -_radius;
            _aabb.top = -_radius;
            _aabb.width = _aabb.height = _radius*2.0f;
        } else {
            // @TODO : move the rotation function to a real good DMGDVT::sfUtils folder
            sf::Vector2f v = DMUtils::sfml::rotate<float>(sf::Vector2f(0.0,_radius),_directionAngle);
            sf::Vector2f left = DMUtils::sfml::rotate<float>(v,-_spreadAngle/2.0f,_directionAngle);
            sf::Vector2f right = DMUtils::sfml::rotate<float>(v,_spreadAngle/2.0f,_directionAngle);

            int xmin = DMUtils::maths::min<float>(v.x, left.x, right.x, 0.0f);
            int xmax = DMUtils::maths::max<float>(v.x, left.x, right.x, 0.0f);
            int ymin = DMUtils::maths::min<float>(v.y, left.y, right.y, 0.0f);
            int ymax = DMUtils::maths::max<float>(v.y, left.y, right.y, 0.0f);

            _aabb.left = xmin;
            _aabb.top = ymin;
            _aabb.width = xmax - xmin;
            _aabb.height = ymax - ymin;
        }
    }

    void SpotLight::setRadius(float r) {
        _radius = r;
	}

    float SpotLight::getRadius() const {
        return _radius;
	}

    void SpotLight::setDirectionAngle(float da) {
        _directionAngle = da;
        _sprite.setRotation(DMUtils::maths::radToDeg(_directionAngle));

        computeAABB();
	}

    float SpotLight::getDirectionAngle() const {
        return DMUtils::maths::radToDeg(_directionAngle);
	}

    void SpotLight::rotate(float delta) {
        float a =  _directionAngle + DMUtils::maths::degToRad(delta);
        while(a > 2.0*M_PIf) a -= 2.0*M_PIf;
        while(a < 0) a += 2.0*M_PIf;
        setDirectionAngle(a);
    }

    void SpotLight::setSpreadAngle(float sa) {
        sa = DMUtils::maths::degToRad(sa);
        if(sa<0) sa = -sa;
        while(sa > 2.0f*M_PIf) sa = 2.0f*M_PIf;
        _spreadAngle = sa;
    }

    float SpotLight::getSpreadAngle() const {
        return DMUtils::maths::radToDeg(_spreadAngle);
    }

    void SpotLight::setIntensity(float i) {
        _intensity = DMUtils::maths::clamp(i,-1.0f,1.0f);;
	}

    float SpotLight::getIntensity() const {
        return DMUtils::maths::abs(_intensity);
	}

    void SpotLight::setBleed(float b) {
        _bleed = b;
	}

    float SpotLight::getBleed() const {
        return _bleed;
	}

    void SpotLight::setLinearity(float lf) {
        _linearity = lf;
	}

    float SpotLight::getLinearity() const {
        return _linearity;
	}

    void SpotLight::setPrecision(int p) {
        _precision = p;
    }

    int SpotLight::getPrecision() const {
        return _precision;
    }

    bool SpotLight::getResizeWhenIncrease() const {
        return _resizeWhenIncrease;
    }

    void SpotLight::setResizeWhenIncrease(bool r) {
        _resizeWhenIncrease = r;
    }

	/*** PROTECTED ***/

	void SpotLight::_render(sf::RenderTarget& target, const sf::RenderStates& states, sf::Shader* shader, sf::Vector2f center, sf::Vector2f shapePosition, sf::Vector2f shapeOrigin, float shapeRotation) {

        float r = _color.r * DMUtils::maths::abs(_intensity);
        float g = _color.g * DMUtils::maths::abs(_intensity);
        float b = _color.b * DMUtils::maths::abs(_intensity);
        sf::Color c(r,g,b,255);

        sf::RenderStates st(states);
        st.shader = shader;

        shader->setParameter(DMGDVT::LS::Light::LAS_PARAM_CENTER,center);
        shader->setParameter(DMGDVT::LS::Light::LAS_PARAM_RADIUS,_radius);
        shader->setParameter(DMGDVT::LS::Light::LAS_PARAM_COLOR,c);
        shader->setParameter(DMGDVT::LS::Light::LAS_PARAM_BLEED,_bleed);
        shader->setParameter(DMGDVT::LS::Light::LAS_PARAM_LINEARITY,_linearity);
        //shader->setParameter(DMGDVT::LS::Light::LAS_PARAM_ISOMETRIC,isIsometric());
        shader->setParameter(DMGDVT::LS::Light::LAS_PARAM_ISOMETRIC,false);//cannot be for now

        sf::ConvexShape shape = _makeShape();

        if(_spreadAngle != M_PIf*2.0f)
            shape.setRotation(shapeRotation);

        shape.setPosition(shapePosition);
        shape.setOrigin(shapeOrigin);
        target.draw(shape,st);
	}

    sf::ConvexShape SpotLight::_makeShape() {
        sf::ConvexShape shape;
        if(_spreadAngle==M_PIf*2.0f) {

            float diam = _radius*2.0f;

            shape.setPointCount(4);
            shape.setPoint(0,sf::Vector2f(0.0f,0.0f));
            shape.setPoint(1,sf::Vector2f(diam,0.0f));
            shape.setPoint(2,sf::Vector2f(diam,diam));
            shape.setPoint(3,sf::Vector2f(0.0f,diam));

        } else {

            float deltaAngle = _spreadAngle / (float)(_precision-1);

            shape.setPointCount(_precision+1);
            shape.setPoint(0,sf::Vector2f(_radius,_radius));

            for(int i=0;i<_precision;++i) {
                float angle = - _spreadAngle/2.0f + (float)i*deltaAngle;
                shape.setPoint(i+1,DMUtils::sfml::rotate(shape.getPoint(0)+sf::Vector2f(0.0f,_radius),angle,shape.getPoint(0)));
            }
        }

        return shape;
    }
}
}
