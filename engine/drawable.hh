// ----------------------------------------------------------------------------
//  Description      : Drawable gamejects
// ----------------------------------------------------------------------------
//  Remarks          :
//    When draw(wImage &,wRectangle &) is called, the clipping rectangle will
//    be set correctly to the rectangle passed, and is expected back to that
//    state when draw(...) returns.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_DRAWABLE
#define WAM_DRAWABLE




#include <ixlib_numconv.hh>
#include "utility/manager.hh"
#include "output/image.hh"
#include "engine/gameject.hh"




void registerDrawableManagerCommands( wGame &g );




// Constants ------------------------------------------------------------------
#define WAM_DRAWPRIO_ABOVEALL  500
#define WAM_DRAWPRIO_CONSOLE  470
#define WAM_DRAWPRIO_STATUS  450
#define WAM_DRAWPRIO_DIALOG  430
#define WAM_DRAWPRIO_MENU  420
#define WAM_DRAWPRIO_GADGET  400
#define WAM_DRAWPRIO_FX   250
#define WAM_DRAWPRIO_SPRITE  240
#define WAM_DRAWPRIO_DEFAULT  230
#define WAM_DRAWPRIO_FOREGROUND  220
#define WAM_DRAWPRIO_LANDSCAPE  100
#define WAM_DRAWPRIO_BACKGROUND_1 20
#define WAM_DRAWPRIO_BACKGROUND_2 10
#define WAM_DRAWPRIO_BACKGROUND_3 0




// common types ---------------------------------------------------------------
typedef sdl::coordinate_vector wScreenVector;
typedef sdl::coordinate_rectangle wDisplayExtent;




// wDrawable -----------------------------------------------------------------
class wDrawable : virtual public wGameject
{
  private:
    typedef wGameject Super;
    int Visible; // == 0 if visible

  public:
    wDrawable( wGame &g );

    void registerMe();
    void prepareToDie();
    void unregisterMe();

    virtual TPriority getDrawPriority() const = 0;
    virtual wDisplayExtent getDisplayExtent() = 0;
    virtual void draw( drawable &screen ) = 0;

    bool isVisible();
    void show();
    void hide();

  protected:
    void requestRedraw();
};




// wDrawableManager -----------------------------------------------------------
class wDrawableManager : public wPriorityManager<wDrawable>
{
    typedef region<TCoordinate> wUpdateRegion;
    wUpdateRegion UpdateRegion;
    bool Everything;
    TSize UpdateArea;
    display &Display;

  public:
    wDrawableManager( display &dpy );

    TPriority getPriority( wDrawable const *o ) const;
    void requestRedraw( wDisplayExtent const &ext );
    void requestRedrawEverything();
    void run();

    display &getDisplay() const
    {
      return Display;
    }
};




#endif
