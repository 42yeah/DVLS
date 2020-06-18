/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus' Graphical User Interface
// Copyright (C) 2012-2020 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef TGUI_WIDGET_RENDERER_HPP
#define TGUI_WIDGET_RENDERER_HPP


#include <TGUI/Config.hpp>
#include <TGUI/ObjectConverter.hpp>
#include <TGUI/Loading/DataIO.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Shared data used in renderer classes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct RendererData
    {
        RendererData() = default;

        static std::shared_ptr<RendererData> create(const std::map<String, ObjectConverter>& init = {})
        {
            auto data = std::make_shared<RendererData>();
            data->propertyValuePairs = init;
            return data;
        }

        /// @internal
        static std::shared_ptr<RendererData> createFromDataIONode(const DataIO::Node* rendererNode)
        {
            auto rendererData = std::make_shared<RendererData>();
            rendererData->shared = false;

            for (const auto& pair : rendererNode->propertyValuePairs)
                rendererData->propertyValuePairs[pair.first] = ObjectConverter(pair.second->value); // Did not compile with VS2015 Update 2 when using braces

            for (const auto& nestedProperty : rendererNode->children)
            {
                std::stringstream ss;
                DataIO::emit(nestedProperty, ss);
                rendererData->propertyValuePairs[nestedProperty->name] = {String("{\n" + ss.str() + "}")};
            }

            return rendererData;
        };

        std::map<String, ObjectConverter> propertyValuePairs;
        std::map<const void*, std::function<void(const String& property)>> observers;
        bool shared = true;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Base class for all renderer classes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API WidgetRenderer
    {
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Default constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        WidgetRenderer() = default;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Construct the renderer from renderer data
        ///
        /// @param data  Renderer data to initialize the renderer with
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        WidgetRenderer(const std::shared_ptr<RendererData>& data)
        {
            setData(data);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Virtual destructor
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual ~WidgetRenderer() = default;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the opacity of the widget
        ///
        /// @param opacity  The opacity of the widget. 0 means completely transparent, while 1 (default) means fully opaque
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setOpacity(float opacity);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the opacity of the widget
        ///
        /// @return The opacity of the widget. 0 means completely transparent, while 1 (default) means fully opaque
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        float getOpacity() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the opacity of the widget when it is disabled
        ///
        /// @param opacity  The opacity of the widget in disabled state.
        ///                 0 means completely transparent, while 1 (default) means fully opaque.
        ///                 Set to -1 (default) to use the normal opacity value even when the widget is disabled.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setOpacityDisabled(float opacity);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the opacity of the widget when it is disabled
        ///
        /// @return The opacity of the widget in disabled state, or -1 when using the not using a different opacity when
        ///         the widget is enabled or disabled.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        float getOpacityDisabled() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the font used for the text in the widget
        ///
        /// @param font  The new font
        ///
        /// When you don't call this function then the font from the parent widget will be used.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setFont(Font font);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the font associated with the widget (if any)
        ///
        /// @return Font used by widget
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Font getFont() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Sets whether mouse events should be ignored on transparent parts of the texture of the widget in normal state
        ///
        /// @brief ignoreTransparentParts  Should mouse events on transparent texture parts be ignored?
        ///
        /// When mouse events are ignored, they are passed to a widget behind the widget.
        /// By default, mouse events are NOT ignored and the widget will receive mouse events even on transparent texture parts.
        ///
        /// This property does nothing if the widget doesn't use textures.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTransparentTexture(bool ignoreTransparentParts);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns whether mouse events should be ignored on transparent parts of the texture of the widget
        /// @return Whether mouse events on transparent texture parts are ignored
        /// @see setTransparentTexture
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool getTransparentTexture() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes a property of the renderer
        ///
        /// @param property  The property that you would like to change
        /// @param value     The new value that you like to assign to the property.
        ///                  The ObjectConverter is implicitly constructed from the possible value types
        ///
        /// @throw Exception for unknown properties or when value was of a wrong type
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setProperty(const String& property, ObjectConverter&& value);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Retrieves the value of a certain property
        ///
        /// @param property  The property that you would like to retrieve
        ///
        /// @return The value inside a ObjectConverter object which you can extract with the correct get function or
        ///         an ObjectConverter object with type ObjectConverter::Type::None when the property did not exist.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ObjectConverter getProperty(const String& property) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Gets a map with all properties and their values
        ///
        /// @return Property-value pairs of the renderer
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const std::map<String, ObjectConverter>& getPropertyValuePairs() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Subscribes a callback function to changes in the renderer
        ///
        /// @param id       Unique identifier for this callback function so that you can unsubscribe it later
        /// @param function Callback function to call when the renderer changes
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void subscribe(const void* id, const std::function<void(const String& property)>& function);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Subscribes a callback function to changes in the renderer
        ///
        /// @param id  Unique identifier used when subscribing the callback function
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void unsubscribe(const void* id);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /// @brief Directly change all the data of this renderer
        ///
        /// You are not supposed to call this function yourself, use the setRenderer function of the widget instead.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setData(const std::shared_ptr<RendererData>& data);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the renderer data
        ///
        /// @return Data that is shared between the renderers
        ///
        /// @warning You should not make changed to this data directly. Instead, use the function from the renderer classes
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::shared_ptr<RendererData> getData() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Gets a clone of the renderer data
        ///
        /// You can pass this to a widget with the setRenderer function to have a separate non-shared copy of this renderer.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::shared_ptr<RendererData> clone() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        std::shared_ptr<RendererData> m_data = RendererData::create();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_WIDGETS_HPP
