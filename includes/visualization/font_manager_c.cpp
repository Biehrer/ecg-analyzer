#include "font_manager_c.h"

//std::vector<Font*> FontManager_C::_fonts;
std::vector<std::shared_ptr<Font>> FontManager_C::_fonts;

FontManager_C::FontManager_C()
{
}

int
FontManager_C::AddFont(Font2D_TP font)
{

    for ( auto& f : _fonts ) {
        if ( f->GetFont() == font ) {
            // Font already initialized
            return -1;
        }
    }
    // Add font and return success
    //_fonts.emplace_back(new Font(font)); 
    _fonts.emplace_back(std::make_shared<Font>(font));
    // Todo: emplace_back can throw a exception (e.g. no memory left).
    // The memory created with 'new'-statement is not managed in this case(nobody owns it) and can not be freed.
    // => use shared ptr 
    return 1;
}
 
Character
FontManager_C::GetFontCharacter(Font2D_TP font, const GLchar character)
{
    for ( auto& f : _fonts ) {
        if ( f->GetFont()== font ) {
            return f->GetCharacter(character);
        }
    }
    
    return {};
}

//const 
//Font*
//FontManager_C::GetFont(Font2D_TP font)
//{
//    for ( auto& f : _fonts ) {
//        if ( f->GetFont()== font ) {
//            return f;
//        }
//    }
//
//    return nullptr;
//}

const
std::shared_ptr<Font>
FontManager_C::GetFont(Font2D_TP font)
{
    for ( auto& f : _fonts ) {
        if ( f->GetFont() == font ) {
            return f;
        }
    }

    return nullptr;
}
void
FontManager_C::Clean()
{
    // Make sure the context is current before calling the Fonts_C destructor 
    // ( call makeCurrent() in OGLPlotRendererWidget) before calling .Clean() on this class
    //for ( unsigned int i = 0; i < _fonts.size(); ++i ) {
    //    delete _fonts[i];
    //}

    _fonts.clear();
}

