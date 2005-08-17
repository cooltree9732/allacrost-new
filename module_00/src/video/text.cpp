#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "coord_sys.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::local_video;

namespace hoa_video 
{

extern uint RoundUpPow2(unsigned x);

//-----------------------------------------------------------------------------
// LoadFont: loads a font of a given size. The name parameter is a string which
//           you use to refer to the font when calling SetFont().
//
//   Example:  gamevideo->LoadFont( "fonts/arial.ttf", "arial36", 36 );
//-----------------------------------------------------------------------------

bool GameVideo::LoadFont(const string &filename, const string &name, int size)
{
	if( _fontMap.find(filename) != _fontMap.end() )
	{
		// font is already loaded, nothing to do
		return true;
	}

	TTF_Font *font = TTF_OpenFont(filename.c_str(), size);
	
	if(!font)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TTF_OpenFont() failed for filename:\n" << filename.c_str() << endl;
		return false;
	}
	
	_fontMap[name] = font;
	return true;
}


//-----------------------------------------------------------------------------
// SetFont: sets the current font. The name parameter is the name that was
//          passed to LoadFont() when it was loaded
//-----------------------------------------------------------------------------

bool GameVideo::SetFont(const std::string &name)
{
	// check if font is loaded before setting it
	if( _fontMap.find(name) == _fontMap.end())
		return false;
		
	_currentFont = name;
	return true;
}


//-----------------------------------------------------------------------------
// SetTextColor: sets the color to use when rendering text
//-----------------------------------------------------------------------------

bool GameVideo::SetTextColor (const Color &color)
{
	_currentTextColor = color;
	return true;
}


//-----------------------------------------------------------------------------
// GetFont: returns the name of the current font (e.g. "verdana18")
//-----------------------------------------------------------------------------

std::string GameVideo::GetFont() const
{
	return _currentFont;
}


//-----------------------------------------------------------------------------
// GetTextColor: returns the current text color
//-----------------------------------------------------------------------------

Color GameVideo::GetTextColor () const
{
	return _currentTextColor;
}


//-----------------------------------------------------------------------------
// DrawTextHelper: since there are two DrawText functions (one for unicode and
//                 one for non-unicode), this private function is used to
//                 do all the work so that code doesn't have to be duplicated.
//                 Either text or uText is valid string and the other is NULL.
//-----------------------------------------------------------------------------

bool GameVideo::DrawTextHelper
( 
	const char   *text, 
	const Uint16 *uText, 
	float x, 
	float y 
)
{
	if(_fontMap.empty())
		return false;
		
	CoordSys tempCoordSys = _coordSys;
	
	SetCoordSys(0,1024,0,768);
	SDL_Rect location;
	SDL_Color color;
	location.x = (int)x;
	location.y = (int)y;
	
	if(_fontMap.find(_currentFont) == _fontMap.end())
		return false;
	
	TTF_Font *font = _fontMap[_currentFont];
	
	color.r = 255;
	color.g = 255;
	color.b = 255;
	
	
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	SDL_Surface *initial;
	SDL_Surface *intermediary;
	int w,h;
	GLuint texture;
	
	
	if( uText )
	{
		initial = TTF_RenderUNICODE_Blended(font, uText, color);
		
		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderUNICODE_Blended() returned NULL in DrawTextHelper()!" << endl;
			return false;
		}
	}
	else
	{
		initial = TTF_RenderText_Blended(font, text, color);

		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderText_Blended() returned NULL in DrawTextHelper()!" << endl;
			return false;
		}
	}
		
	w = RoundUpPow2(initial->w);
	h = RoundUpPow2(initial->h);
	
	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	if(!intermediary)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_CreateRGBSurface() returned NULL in DrawTextHelper()!" << endl;
		return false;
	}


	if(SDL_BlitSurface(initial, 0, intermediary, 0) < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_BlitSurface() failed in DrawTextHelper()!" << endl;
		return false;
	}


	glGenTextures(1, &texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glGenTextures() in DrawTextHelper!" << endl;
		return false;
	}
	
	BindTexture(texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glBindTexture() in DrawTextHelper!" << endl;
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, 
	             GL_UNSIGNED_BYTE, intermediary->pixels );

	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glTexImage2D() in DrawTextHelper!" << endl;
		return false;
	}

	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	glEnable(GL_TEXTURE_2D);
	BindTexture(texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after 2nd call to glBindTexture() in DrawTextHelper!" << endl;
		return false;
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	glDisable(GL_FOG);

	
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f); 
	glVertex2f((float)location.x, (float)location.y);
	glTexCoord2f(1.0f, 1.0f); 
	glVertex2f((float)location.x + w, (float)location.y);
	glTexCoord2f(1.0f, 0.0f); 
	glVertex2f((float)location.x + w, (float)location.y + h);
	glTexCoord2f(0.0f, 0.0f); 
	glVertex2f((float)location.x, (float)location.y + h);

	glEnd();
	
	if(_fogIntensity > 0.0f)
		glEnable(GL_FOG);

	glFinish();
	
	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
		
	if(!DeleteTexture(texture))
	{
		if(VIDEO_DEBUG)
			cerr << "glGetError() true after glDeleteTextures() in DrawTextHelper!" << endl;
		return false;
	}

	SetCoordSys( tempCoordSys._left, tempCoordSys._right, tempCoordSys._bottom, tempCoordSys._top);

	return true;
}


//-----------------------------------------------------------------------------
// DrawText: non unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const char *text, float x, float y)
{
	return DrawTextHelper(text, NULL, x, y);
}


//-----------------------------------------------------------------------------
// DrawText: unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const Uint16 *text, float x, float y)
{
	return DrawTextHelper(NULL, text, x, y);
}


}  // namespace hoa_video
