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

#ifndef HEADER_DMGDVT_SPOTLIGHT
#define HEADER_DMGDVT_SPOTLIGHT

#include <SFML/Graphics.hpp>

#include "Light.hpp"

namespace DMGDVT {
namespace LS {

    /**
    * Would have preferred to do that with something like
    * template <typename float SpreadAngle>
    * to have simply PointLight define as typedef SpotLight<M_PIf*2.0f> PointLight
    * but ah well :<
    **/
    class SpotLight : public Light {
        public:
            SpotLight(bool iso = false);
            SpotLight(sf::Vector2f ctr, float r, sf::Color c, bool iso = false);
            SpotLight(sf::Vector2f ctr, float r, sf::Color c, float da, float sa, float i, float b, float lf, bool iso = false);
            virtual ~SpotLight();

            virtual void preRender(sf::Shader* shader);
            virtual void render(const sf::IntRect& screen, sf::RenderTarget& target, sf::Shader* shader, const sf::RenderStates &states=sf::RenderStates::Default);
            virtual void debugRender(sf::RenderTarget& target, const sf::RenderStates &states);
            virtual void drawAABB(const sf::IntRect& screen, sf::RenderTarget& target);

            virtual void computeAABB();

            /*** GETTER - SETTER ***/
            virtual sf::IntRect getAABB();

            void setPosition(sf::Vector2f c);
            sf::Vector2f getPosition() const;
            void move(sf::Vector2f delta);

            void setRadius(float r);
            float getRadius() const;

            void setColor(sf::Color c);
            sf::Color getColor() const;

            void setDirectionAngle(float da);
            float getDirectionAngle() const;
            void rotate(float delta);

            void setSpreadAngle(float sa);
            float getSpreadAngle() const;

            void setIntensity(float i);
            float getIntensity() const;

            void setBleed(float b);
            float getBleed() const;

            void setLinearity(float lf);
            float getLinearity() const;

            void setPrecision(int p);
            int getPrecision() const;

            virtual bool isNegative() const;

            bool getResizeWhenIncrease() const;
            void setResizeWhenIncrease(bool r);

        protected:
            sf::Vector2f _position;
            float _radius;
            sf::Color _color;
            float _directionAngle; //angle light is pointing, rad
            float _spreadAngle; //how wide the light covers, rad
            float _intensity; //how bright light is
            float _bleed; //radius of the light halo
            float _linearity;

            int _precision;//number of arcs in the circle

            bool _negative;
            bool _resizeWhenIncrease;//if set to true, the texture is reallocated only if the needed size increases (if new _radius is inferior to previous, it doesn't change)

    };
}
}

#endif // HEADER_DMGDVT_SPOTLIGHT

