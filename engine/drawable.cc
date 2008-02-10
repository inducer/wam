// ----------------------------------------------------------------------------
//  Description      : Drawable clijects
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <algorithm>
#include <ixlib_geometry_impl.hh>
#include "utility/debug.hh"
#include "utility/exgame.hh"
#include "utility/string_pack.hh"
#include "utility/manager_impl.hh"
#include "engine/gamebase.hh"
#include "engine/console.hh"
#include "engine/commandmanager.hh"
#include "engine/drawable.hh"




#define XML_ATTR_VISIBLE          "isvisible"




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( list_drawables, "LiDr", "list objects registered to drawable manager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_drawables", 0, 0 )

  auto_ptr<js_array> arr( new js_array() );

  TIndex idx = 0;
  FOREACH_CONST( first, Game.getDrawableManager(), wDrawableManager )
  ( *arr ) [ idx++ ] = makeConstant( ( *first ) ->id() );
  return arr.release();
}




void registerDrawableManagerCommands( wGame &g )
{
  register_list_drawables( g );
}




// wDrawable -----------------------------------------------------------------
wDrawable::wDrawable( wGame &g )
    : wGameject( g ), Visible( 1 )
{}




void wDrawable::registerMe()
{
  Game.getDrawableManager().registerObject( this );
}




void wDrawable::prepareToDie()
{
  hide();
  requestRedraw();
}




void wDrawable::unregisterMe()
{
  Game.getDrawableManager().unregisterObject( this );
}




bool wDrawable::isVisible()
{
  return Visible == 0;
}




void wDrawable::show()
{
  if ( Visible == 0 )
    EXGAME_THROWINFO( ECGAME_GENERAL, "trying to wDrawable::show too often" );

  Visible--;
  if ( Visible == 0 )
    requestRedraw();
}




void wDrawable::hide()
{
  if ( Visible == 0 )
    requestRedraw();
  Visible++;
}




void wDrawable::requestRedraw()
{
  Game.getDrawableManager().requestRedraw( getDisplayExtent() );
}




// Template instantiation -----------------------------------------------------
template class region<TCoordinate>;
template class wPriorityManager<wDrawable>;




// wDrawableManager -----------------------------------------------------------
wDrawableManager::wDrawableManager( display &dpy )
    : Everything( true ), UpdateArea( 0 ), Display( dpy )
{}




TPriority wDrawableManager::getPriority( wDrawable const *o ) const
{
  return o->getDrawPriority();
}




void wDrawableManager::requestRedraw( wDisplayExtent const &ext )
{
  if ( !Everything )
  {
    UpdateArea += ext.getArea();
    if ( UpdateRegion.size() > 20
         || UpdateArea > ( Display.width() * Display.height() * 3 ) / 4 )
    {
      Everything = true;
      return ;
    }
    wDisplayExtent copy_ext( ext );
    copy_ext.intersect( Display.extent() );
    if ( !copy_ext.isEmpty() )
      UpdateRegion.add( copy_ext );
  }
}




void wDrawableManager::requestRedrawEverything()
{
  Everything = true;
}




void wDrawableManager::run()
{
  bool hw_display = Display.flags() & SDL_HWSURFACE;

  if ( Everything )
  {
    Display.clearClipping();

    FOREACH( first, ObjectList, wObjectList )
    if ( ( *first ) ->isVisible() )
      ( *first ) ->draw( Display );
    Display.flip();
  }
  else
  {
    FOREACH( first, UpdateRegion, wUpdateRegion )
    {
      Display.clipping( *first );

      FOREACH( first_dwbl, ObjectList, wObjectList )
      if ( ( *first_dwbl ) ->isVisible() && first->doesIntersect( ( *first_dwbl ) ->getDisplayExtent() ) )
        ( *first_dwbl ) ->draw( Display );
      if ( !hw_display )
        Display.update( *first );
    }
    if ( hw_display )
      Display.flip();
  }

  UpdateRegion.clear();
  Everything = hw_display;
  UpdateArea = 0;
}
