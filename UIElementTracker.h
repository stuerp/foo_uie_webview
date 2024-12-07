
/** $VER: UIElementTracker.h (2024.12.07) P. Stuer - Tracks the instances of the panel. **/

#pragma once

#include "framework.h"

#include "UIElement.h"

class uielement_tracker_t
{
public:
    uielement_tracker_t() : _CurrentUIElement()
    {
    }

    void Add(UIElement * element) noexcept
    {
        std::unique_lock Lock(_Mutex);

        _UIElements.push_back(element);

        SetCurrentElement(element);
    }

    void Remove(UIElement * element) noexcept
    {
        std::unique_lock Lock(_Mutex);

        auto Iter = std::find(_UIElements.begin(), _UIElements.end(), element); 
  
        if (Iter != _UIElements.end())
        {
            _UIElements.erase(Iter); 

            SetCurrentElement(nullptr);
        }
    }

    UIElement * GetCurrentElement() const noexcept
    {
        return _CurrentUIElement;
    }

    void SetCurrentElement(UIElement * element) noexcept
    {
        _CurrentUIElement = element;
    }

private:
    std::mutex _Mutex;

    UIElement * _CurrentUIElement;
    std::vector<UIElement *> _UIElements;
};

extern uielement_tracker_t _UIElementTracker;
